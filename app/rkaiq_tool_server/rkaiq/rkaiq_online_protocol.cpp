#include "rkaiq_online_protocol.h"

#include "rkaiq_protocol.h"
#include "tcp_client.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

extern int g_width;
extern int g_height;

static uint16_t capture_check_sum;
static int capture_status = READY;
static int capture_frames = 0;
static int capture_frames_index = 0;

static void DoAnswer(int sockfd, CommandData_t* cmd, int cmd_id, int ret_status) {
  char send_data[MAXPACKETSIZE];
  LOG_INFO("enter\n");

  strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
  cmd->cmdType = CMD_TYPE_CAPTURE;
  cmd->cmdID = cmd_id;
  strncpy((char*)cmd->version, RKAIQ_TOOL_VERSION, sizeof(cmd->version));
  cmd->datLen = 4;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = ret_status;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++) {
    cmd->checkSum += cmd->dat[i];
  }

  memcpy(send_data, cmd, sizeof(CommandData_t));
  send(sockfd, send_data, sizeof(CommandData_t), 0);
  LOG_INFO("exit\n");
}

static void DoAnswer2(int sockfd, CommandData_t* cmd, int cmd_id, uint16_t check_sum, uint32_t result) {
  char send_data[MAXPACKETSIZE];
  LOG_INFO("enter\n");
  strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
  cmd->cmdType = CMD_TYPE_CAPTURE;
  cmd->cmdID = cmd_id;
  strncpy((char*)cmd->version, RKAIQ_TOOL_VERSION, sizeof(cmd->version));
  cmd->datLen = 4;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = result;
  cmd->dat[1] = check_sum & 0xFF;
  cmd->dat[2] = (check_sum >> 8) & 0xFF;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++) {
    cmd->checkSum += cmd->dat[i];
  }

  memcpy(send_data, cmd, sizeof(CommandData_t));
  send(sockfd, send_data, sizeof(CommandData_t), 0);
  LOG_INFO("exit\n");
}

static int DoCheckSum(int sockfd, uint16_t check_sum) {
  char recv_data[MAXPACKETSIZE];
  size_t recv_size = 0;
  int param_size = sizeof(CommandData_t);
  int remain_size = param_size;
  int try_count = 3;
  LOG_INFO("enter\n");

  struct timeval interval = {3, 0};
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&interval, sizeof(struct timeval));

  while (remain_size > 0) {
    int offset = param_size - remain_size;
    recv_size = recv(sockfd, recv_data + offset, remain_size, 0);
    if (recv_size < 0) {
      if (errno == EAGAIN) {
        LOG_INFO("recv size %zu, do try again, count %d\n", recv_size, try_count);
        try_count--;
        if (try_count < 0) {
          break;
        }
        continue;
      } else {
        LOG_ERROR("Error socket recv failed %d\n", errno);
      }
    }
    remain_size = remain_size - recv_size;
  }
  LOG_INFO("recv_size: 0x%lx expect 0x%lx\n", recv_size, sizeof(CommandData_t));

  CommandData_t* cmd = (CommandData_t*)recv_data;
  uint16_t recv_check_sum = 0;
  recv_check_sum += cmd->dat[0] & 0xff;
  recv_check_sum += (cmd->dat[1] & 0xff) << 8;
  LOG_INFO("check_sum local: 0x%x pc: 0x%x\n", check_sum, recv_check_sum);

  if (check_sum != recv_check_sum) {
    LOG_INFO("check_sum fail!\n");
    return -1;
  }

  LOG_INFO("exit\n");
  return 0;
}

static void OnLineSet(int sockfd, CommandData_t* cmd, uint16_t& check_sum, uint32_t& result) {
  int recv_size = 0;
  int param_size = *(int*)cmd->dat;
  int remain_size = param_size;

  LOG_INFO("enter\n");
  LOG_INFO("expect recv param_size 0x%x\n", param_size);
  char* param = (char*)malloc(param_size);
  while (remain_size > 0) {
    int offset = param_size - remain_size;
    recv_size = recv(sockfd, param + offset, remain_size, 0);
    remain_size = remain_size - recv_size;
  }

  LOG_INFO("recv ready\n");

  for (int i = 0; i < param_size; i++) {
    check_sum += param[i];
  }

  LOG_INFO("DO Sycn Setting, CmdId: 0x%x, expect ParamSize %d\n", cmd->cmdID, param_size);
#if 0
  if (rkaiq_manager) {
    result = rkaiq_manager->IoCtrl(cmd->cmdID, param, param_size);
  }
#endif
  if (param != NULL) {
    free(param);
  }
  LOG_INFO("exit\n");
}

static int OnLineGet(int sockfd, CommandData_t* cmd) {
  int ret = 0;
  int ioRet = 0;
  int send_size = 0;
  int param_size = *(int*)cmd->dat;
  int remain_size = param_size;
  LOG_INFO("enter\n");
  LOG_INFO("ParamSize: 0x%x\n", param_size);

  uint8_t* param = (uint8_t*)malloc(param_size);

  LOG_INFO("DO Get Setting, CmdId: 0x%x, expect ParamSize %d\n", cmd->cmdID, param_size);
#if 0
  if (rkaiq_manager) {
    ioRet = rkaiq_manager->IoCtrl(cmd->cmdID, param, param_size);
    if (ioRet != 0) {
      LOG_INFO("DO Get Setting, io get data failed. return\n");
      return 1;
    }
  }
#endif

  while (remain_size > 0) {
    int offset = param_size - remain_size;
    send_size = send(sockfd, param + offset, remain_size, 0);
    remain_size = param_size - send_size;
  }

  uint16_t check_sum = 0;
  for (int i = 0; i < param_size; i++) {
    check_sum += param[i];
  }

  ret = DoCheckSum(sockfd, check_sum);

  if (param != NULL) {
    free(param);
    param = NULL;
  }
  return ret;
}

static void SendYuvData(int socket, int index, void* buffer, int size) {
  LOG_INFO("SendYuvData\n");
  char* buf = NULL;
  int total = size;
  int packet_len = MAXPACKETSIZE;
  int send_size = 0;
  int ret_val;
  uint16_t check_sum = 0;

  buf = (char*)buffer;
  for (int i = 0; i < size; i++) {
    check_sum += buf[i];
  }
  capture_check_sum += check_sum;
  LOG_INFO("capture yuv index %d, check_sum %d capture_check_sum %d\n", index, check_sum, capture_check_sum);

  buf = (char*)buffer;
  while (total > 0) {
    if (total < packet_len) {
      send_size = total;
    } else {
      send_size = packet_len;
    }
    ret_val = send(socket, buf, send_size, 0);
    total -= send_size;
    buf += ret_val;
  }
}

static void SendYuvDataResult(int sockfd, CommandData_t* cmd, CommandData_t* recv_cmd) {
  unsigned short* checksum;
  char send_data[MAXPACKETSIZE];
  checksum = (unsigned short*)&recv_cmd->dat[1];
  strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
  cmd->cmdType = CMD_TYPE_CAPTURE;
  cmd->cmdID = CMD_ID_CAPTURE_YUV_CAPTURE;
  cmd->datLen = 2;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = 0x04;
  LOG_INFO("capture_check_sum %d, recieve %d\n", capture_check_sum, *checksum);
  if (capture_check_sum == *checksum) {
    cmd->dat[1] = RES_SUCCESS;
  } else {
    cmd->dat[1] = RES_FAILED;
  }
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++) {
    cmd->checkSum += cmd->dat[i];
  }

  memcpy(send_data, cmd, sizeof(CommandData_t));
  send(sockfd, send_data, sizeof(CommandData_t), 0);
}

#ifndef __ANDROID__
extern std::shared_ptr<easymedia::Flow> video_dump_flow;
#endif
static const uint32_t kSocket_fd = (1 << 0);
static const uint32_t kEnable_Link = (1 << 1);

void LinkCaptureCallBack(unsigned char* buffer, unsigned int buffer_size, int sockfd, const char* id) {
  LOG_ERROR("sockfd %d buffer %p, size %d\n", sockfd, buffer, buffer_size);
  SendYuvData(sockfd, capture_frames_index++, buffer, buffer_size);
}

static int DoCaptureYuv(int sockfd) {
  LOG_ERROR("sockfd %d\n", sockfd);
#ifndef __ANDROID__
  if (video_dump_flow) {
    video_dump_flow->SetCaptureHandler(LinkCaptureCallBack);
    video_dump_flow->Control(kSocket_fd, sockfd);
    video_dump_flow->Control(kEnable_Link, capture_frames);
  }
#endif
  return 0;
}

static void ReplyStatus(int sockfd, CommandData_t* cmd, int ret_status) {
  LOG_INFO("enter\n");
  char send_data[MAXPACKETSIZE];
  strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
  cmd->cmdType = CMD_TYPE_CAPTURE;
  cmd->cmdID = CMD_ID_CAPTURE_YUV_CAPTURE;
  cmd->datLen = 2;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = DATA_ID_CAPTURE_RAW_STATUS;  // ProcessID
  cmd->dat[1] = ret_status;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++) {
    cmd->checkSum += cmd->dat[i];
  }

  LOG_INFO("cmd->checkSum %d\n", cmd->checkSum);
  memcpy(send_data, cmd, sizeof(CommandData_t));
  send(sockfd, send_data, sizeof(CommandData_t), 0);
  LOG_INFO("exit\n");
}

static void ReplySensorPara(int sockfd, CommandData_t* cmd) {
  LOG_INFO("enter\n");
  char send_data[MAXPACKETSIZE];
  memset(cmd, 0, sizeof(CommandData_t));
  strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
  cmd->cmdType = CMD_TYPE_CAPTURE;
  cmd->cmdID = CMD_ID_CAPTURE_YUV_CAPTURE;
  cmd->datLen = 3;
  LOG_ERROR("g_width  %d g_height %d\n", g_width, g_height);
  Sensor_Yuv_Params_t* param = (Sensor_Yuv_Params_t*)(cmd->dat);
  param->data_id = DATA_ID_CAPTURE_RAW_GET_PARAM;
  param->width = g_width;
  param->height = g_height;
  param->format = RKISP_FORMAT_NV12;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++) {
    cmd->checkSum += cmd->dat[i];
  }

  LOG_INFO("cmd->checkSum %d\n", cmd->checkSum);
  memcpy(send_data, cmd, sizeof(CommandData_t));
  send(sockfd, send_data, sizeof(CommandData_t), 0);
  LOG_INFO("exit\n");
}

static void SetSensorPara(int sockfd, CommandData_t* recv_cmd, CommandData_t* cmd) {
  LOG_INFO("enter\n");
  char send_data[MAXPACKETSIZE];

  Capture_Yuv_Params_t* CapParam = (Capture_Yuv_Params_t*)(recv_cmd->dat + 1);
  LOG_INFO(" set gain        : %d\n", CapParam->gain);
  LOG_INFO(" set exposure    : %d\n", CapParam->time);
  LOG_INFO(" set fmt        : %d\n", CapParam->fmt);
  LOG_INFO(" set framenumber : %d\n", CapParam->framenumber);

  capture_frames = CapParam->framenumber;
  capture_frames_index = 0;
  capture_check_sum = 0;

  memset(cmd, 0, sizeof(CommandData_t));
  strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
  cmd->cmdType = PC_TO_DEVICE;
  cmd->cmdID = CMD_ID_CAPTURE_YUV_CAPTURE;
  cmd->datLen = 2;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = DATA_ID_CAPTURE_RAW_SET_PARAM;
  cmd->dat[1] = RES_SUCCESS;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++) {
    cmd->checkSum += cmd->dat[i];
  }
  LOG_INFO("cmd->checkSum %d\n", cmd->checkSum);
  memcpy(send_data, cmd, sizeof(CommandData_t));
  send(sockfd, send_data, sizeof(CommandData_t), 0);
  LOG_INFO("exit\n");
}

void RKAiqOLProtocol::HandlerOnLineMessage(int sockfd, char* buffer, int size) {
  CommandData_t* common_cmd = (CommandData_t*)buffer;
  CommandData_t send_cmd;
  int ret_val, ret;

  LOG_INFO("HandlerOnLineMessage:\n");
  LOG_INFO("DATA datLen: 0x%x\n", common_cmd->datLen);

  if (strcmp((char*)common_cmd->RKID, TAG_OL_PC_TO_DEVICE) == 0) {
    LOG_INFO("RKID: %s\n", common_cmd->RKID);
  } else {
    LOG_INFO("RKID: Unknow\n");
    return;
  }

  LOG_INFO("cmdID: 0x%x cmdType: 0x%x\n", common_cmd->cmdID, common_cmd->cmdType);

  switch (common_cmd->cmdType) {
    case CMD_TYPE_STREAMING:
      RKAiqProtocol::DoChangeAppMode(APP_RUN_STATUS_TUNRING);
      if (common_cmd->cmdID == 0xffff){
          uint16_t check_sum;
          uint32_t result;
          DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, READY);
          OnLineSet(sockfd, common_cmd, check_sum, result);
          DoAnswer2(sockfd, &send_cmd, common_cmd->cmdID, check_sum, result ? RES_FAILED : RES_SUCCESS);
      }
      break;
    case CMD_TYPE_STATUS:
      DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, READY);
      break;
    case CMD_TYPE_UAPI_SET: {
      uint16_t check_sum;
      uint32_t result;
      DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, READY);
      OnLineSet(sockfd, common_cmd, check_sum, result);
      DoAnswer2(sockfd, &send_cmd, common_cmd->cmdID, check_sum, result ? RES_FAILED : RES_SUCCESS);
    } break;
    case CMD_TYPE_UAPI_GET: {
      ret = OnLineGet(sockfd, common_cmd);
      if (ret == 0) {
        DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, RES_SUCCESS);
      } else {
        DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, RES_FAILED);
      }
    } break;
    case CMD_TYPE_CAPTURE: {
      LOG_INFO("CMD_TYPE_CAPTURE in\n");
      DoCaptureYuv(sockfd);
      char* datBuf = (char*)(common_cmd->dat);
      switch (datBuf[0]) {
        case DATA_ID_CAPTURE_YUV_STATUS:
          LOG_INFO("ProcID DATA_ID_CAPTURE_YUV_STATUS in\n");
          ReplyStatus(sockfd, &send_cmd, READY);
          LOG_INFO("ProcID DATA_ID_CAPTURE_YUV_STATUS out\n");
          break;
        case DATA_ID_CAPTURE_YUV_GET_PARAM:
          LOG_INFO("ProcID DATA_ID_CAPTURE_YUV_GET_PARAM in\n");
          ReplySensorPara(sockfd, &send_cmd);
          LOG_INFO("ProcID DATA_ID_CAPTURE_YUV_GET_PARAM out\n");
          break;
        case DATA_ID_CAPTURE_YUV_SET_PARAM:
          LOG_INFO("ProcID DATA_ID_CAPTURE_YUV_SET_PARAM in\n");
          SetSensorPara(sockfd, common_cmd, &send_cmd);
          LOG_INFO("ProcID DATA_ID_CAPTURE_YUV_SET_PARAM out\n");
          break;
        case DATA_ID_CAPTURE_YUV_START: {
          LOG_INFO("ProcID DATA_ID_CAPTURE_YUV_START in\n");
          capture_status = BUSY;
          DoCaptureYuv(sockfd);
          capture_status = READY;
          LOG_INFO("ProcID DATA_ID_CAPTURE_YUV_START out\n");
          break;
        }
        case DATA_ID_CAPTURE_YUV_CHECKSUM:
          LOG_INFO("ProcID DATA_ID_CAPTURE_YUV_CHECKSUM in\n");
          SendYuvDataResult(sockfd, &send_cmd, common_cmd);
          LOG_INFO("ProcID DATA_ID_CAPTURE_YUV_CHECKSUM out\n");
          break;
        default:
          break;
      }
      LOG_INFO("CMD_TYPE_CAPTURE out\n\n");
    } break;
    default:
      LOG_INFO("cmdID: Unknow\n");
      break;
  }
}
