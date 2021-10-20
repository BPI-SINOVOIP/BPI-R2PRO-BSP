#include "rkaiq_protocol.h"

#include <signal.h>
#include <sys/types.h>
#include <dirent.h>

#ifdef __ANDROID__
#include <cutils/properties.h>
#include <rtspserver/RtspServer.h>
#endif

#include "domain_tcp_client.h"
#include "tcp_client.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

extern int g_app_run_mode;
extern int g_width;
extern int g_height;
extern int g_rtsp_en;
extern int g_device_id;
extern int g_allow_killapp;
extern DomainTCPClient g_tcpClient;
extern struct ucred* g_aiqCred;
extern std::string iqfile;
extern std::string g_sensor_name;
extern std::shared_ptr<RKAiqMedia> rkaiq_media;
extern std::string g_stream_dev_name;

bool RKAiqProtocol::is_recv_running = false;
std::unique_ptr<std::thread> RKAiqProtocol::forward_thread = nullptr;
std::mutex RKAiqProtocol::mutex_;

#define MAX_PACKET_SIZE 8192
#pragma pack(1)
typedef struct FileTransferData_s {
    uint8_t RKID[8]; // "SendFile"
    unsigned long long packetSize;
    int commandID;
    int commandResult;
    int targetDirLen;
    uint8_t targetDir[256];
    int targetFileNameLen;
    uint8_t targetFileName[128];
    unsigned long long dataSize;
    char* data;
    unsigned int dataHash;
} FileTransferData;
#pragma pack()

static void HexDump(const unsigned char* data, size_t size)
{
    printf("####\n");
    int i;
    size_t offset = 0;
    while (offset < size)
    {
        printf("%04x  ", offset);
        for (i = 0; i < 16; i++)
        {
            if (i % 8 == 0)
            {
                putchar(' ');
            }
            if (offset + i < size)
            {
                printf("%02x ", data[offset + i]);
            }
            else
            {
                printf("   ");
            }
        }
        printf("   ");
        for (i = 0; i < 16 && offset + i < size; i++)
        {
            if (isprint(data[offset + i]))
            {
                printf("%c", data[offset + i]);
            }
            else
            {
                putchar('.');
            }
        }
        putchar('\n');
        offset += 16;
    }
    printf("####\n\n");
}

static unsigned int MurMurHash(const void* key, int len) {
  const unsigned int m = 0x5bd1e995;
  const int r = 24;
  const int seed = 97;
  unsigned int h = seed ^ len;
  // Mix 4 bytes at a time into the hash
  const unsigned char* data = (const unsigned char*)key;
  while (len >= 4) {
    unsigned int k = *(unsigned int*)data;
    k *= m;
    k ^= k >> r;
    k *= m;
    h *= m;
    h ^= k;
    data += 4;
    len -= 4;
  }
  // Handle the last few bytes of the input array
  switch (len) {
    case 3:
      h ^= data[2] << 16;
    case 2:
      h ^= data[1] << 8;
    case 1:
      h ^= data[0];
      h *= m;
  };
  // Do a few final mixes of the hash to ensure the last few
  // bytes are well-incorporated.
  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;
  return h;
}

static int ProcessExists(const char* process_name) {
  FILE* fp;
  char cmd[1024] = {0};
  char buf[1024] = {0};
  snprintf(cmd, sizeof(cmd), "ps -ef | grep %s | grep -v grep", process_name);
  fp = popen(cmd, "r");
  if (!fp) {
    LOG_INFO("popen ps | grep %s fail\n", process_name);
    return -1;
  }
  while (fgets(buf, sizeof(buf), fp)) {
    LOG_INFO("ProcessExists %s\n", buf);
    if (strstr(buf, process_name)) {
      fclose(fp);
      return 1;
    }
  }
  fclose(fp);
  return 0;
}

int StopProcess(const char* process, const char* str) {
  int count = 0;
  while (ProcessExists(process) > 0) {
    LOG_INFO("StopProcess %s... \n", process);
    system(str);
    sleep(1);
    count++;
    if (count > 3) {
      return -1;
    }
  }
  return 0;
}

int WaitProcessExit(const char* process, int sec) {
  int count = 0;
  LOG_INFO("WaitProcessExit %s... \n", process);
  while (ProcessExists(process) > 0) {
    LOG_INFO("WaitProcessExit %s... \n", process);
    sleep(1);
    count++;
    if (count > sec) {
      return -1;
    }
  }
  return 0;
}

int RKAiqProtocol::ConnectAiq() {
#ifdef __ANDROID__
    if (g_tcpClient.Setup(LOCAL_SOCKET_PATH) == false) {
      LOG_DEBUG("domain connect failed\n");
      g_tcpClient.Close();
      return -1;
    }
#else
    if (g_device_id == 0) {
      if (g_tcpClient.Setup("/tmp/UNIX.domain") == false) {
        LOG_DEBUG("domain connect failed\n");
        g_tcpClient.Close();
        return -1;
      }
    } else {
      if (g_tcpClient.Setup("/tmp/UNIX_1.domain") == false) {
        LOG_DEBUG("domain connect failed\n");
        g_tcpClient.Close();
        return -1;
      }
    }
#endif
    return 0;
}

void RKAiqProtocol::DisConnectAiq() { g_tcpClient.Close(); }

void RKAiqProtocol::KillApp() {
#ifdef __ANDROID__
  if (g_allow_killapp) {
    unlink(LOCAL_SOCKET_PATH);
    property_set("ctrl.stop", "cameraserver");
    property_set("ctrl.stop", "vendor.camera-provider-2-4");
    property_set("ctrl.stop", "vendor.camera-provider-2-4-ext");
    system("stop cameraserver");
    system("stop vendor.camera-provider-2-4");
    system("stop vendor.camera-provider-2-4-ext");
  }
#else
  if (g_allow_killapp) {
    if (g_aiqCred != nullptr) {
      kill(g_aiqCred->pid, SIGTERM);
      delete g_aiqCred;
      g_aiqCred = nullptr;
    }
  }
#endif
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

int RKAiqProtocol::StartApp() {
  int ret = -1;
#ifdef __ANDROID__
  if (g_allow_killapp) {
    property_set("ctrl.start", "cameraserver");
    system("start cameraserver");
    system("start vendor.camera-provider-2-4");
    system("start vendor.camera-provider-2-4-ext");
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
#endif
  return 0;
}

int RKAiqProtocol::StartRTSP() {
  int ret = -1;

  LOG_DEBUG("Starting RTSP !!!");
  KillApp();
  ret = rkaiq_media->LinkToIsp(true);
  if (ret) {
    LOG_ERROR("link isp failed!!!");
    return ret;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  media_info_t mi = rkaiq_media->GetMediaInfoT(g_device_id);
#ifdef __ANDROID__
  int readback = 0;
  std::string isp3a_server_cmd = "/vendor/bin/rkaiq_3A_server -mmedia= ";
  isp3a_server_cmd.append(mi.isp.media_dev_path);
  isp3a_server_cmd.append(" --sensor_index=");
  isp3a_server_cmd.append(std::to_string(g_device_id));
  isp3a_server_cmd.append(" &");
  system("pkill rkaiq_3A_server*");
  system(isp3a_server_cmd.c_str());
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
#endif
  if (g_stream_dev_name.empty()) {
    int isp_ver = rkaiq_media->GetIspVer();
    LOG_DEBUG(">>>>>>>> isp ver = %d\n", isp_ver);
    if (isp_ver == 4) {
      ret = init_rtsp(mi.ispp.pp_scale0_path.c_str(), g_width, g_height);
    } else if (isp_ver == 5) {
      ret = init_rtsp(mi.isp.main_path.c_str(), g_width, g_height);
    } else {
      ret = init_rtsp(mi.isp.main_path.c_str(), g_width, g_height);
    }
  } else {
    ret = init_rtsp(g_stream_dev_name.c_str(), g_width, g_height);
  }
  if (ret) {
    LOG_ERROR("init_rtsp failed!!");
    return ret;
  }

  LOG_DEBUG("Started RTSP !!!");
  return 0;
}

int RKAiqProtocol::StopRTSP() {
  LOG_DEBUG("Stopping RTSP !!!");
  deinit_rtsp();
#ifdef __ANDROID__
  system("pkill rkaiq_3A_server*");
#endif
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  LOG_DEBUG("Stopped RTSP !!!");
  return 0;
}

int RKAiqProtocol::DoChangeAppMode(appRunStatus mode) {
  std::lock_guard<std::mutex> lg(mutex_);
  int ret = -1;
  LOG_DEBUG("Switch to mode %d->%d\n", g_app_run_mode, mode);
  if (g_app_run_mode == mode) {
    return 0;
  }
  if (mode == APP_RUN_STATUS_CAPTURE) {
    LOG_DEBUG("Switch to APP_RUN_STATUS_CAPTURE\n");
    if (g_rtsp_en) {
      ret = StopRTSP();
      if (ret) {
        LOG_ERROR("stop RTSP failed!!!");
        g_app_run_mode = APP_RUN_STATUS_INIT;
        return ret;
      }
    }
    KillApp();
    ret = rkaiq_media->LinkToIsp(false);
    if (ret) {
      LOG_ERROR("unlink isp failed!!!");
      g_app_run_mode = APP_RUN_STATUS_INIT;
      return ret;
    }
  } else {
    LOG_DEBUG("Switch to APP_RUN_STATUS_TUNRING\n");
    ret = rkaiq_media->LinkToIsp(true);
    if (ret) {
      LOG_ERROR("link isp failed!!!");
      g_app_run_mode = APP_RUN_STATUS_INIT;
      return ret;
    }
    if (!g_rtsp_en) {
      ret = StartApp();
      if (ret) {
        LOG_ERROR("start app failed!!!");
        g_app_run_mode = APP_RUN_STATUS_INIT;
        return ret;
      }
    }
    int ret = ConnectAiq();
    if (ret) {
      LOG_ERROR("connect aiq failed!!!");
      is_recv_running = false;
      g_app_run_mode = APP_RUN_STATUS_INIT;
      return ret;
    }
  }
  g_app_run_mode = mode;
  LOG_DEBUG("Change mode to %d exit\n", g_app_run_mode);
  return 0;
}

static void InitCommandPingAns(CommandData_t* cmd, int ret_status) {
  strncpy((char*)cmd->RKID, RKID_CHECK, sizeof(cmd->RKID));
  cmd->cmdType = DEVICE_TO_PC;
  cmd->cmdID = CMD_ID_CAPTURE_STATUS;
  cmd->datLen = 1;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = ret_status;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++) {
    cmd->checkSum += cmd->dat[i];
  }
}

static void DoAnswer(int sockfd, CommandData_t* cmd, int cmd_id, int ret_status) {
  char send_data[MAXPACKETSIZE];
  LOG_INFO("enter\n");

  strncpy((char*)cmd->RKID, TAG_OL_DEVICE_TO_PC, sizeof(cmd->RKID));
  cmd->cmdType = DEVICE_TO_PC;
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

void RKAiqProtocol::HandlerCheckDevice(int sockfd, char* buffer, int size) {
  CommandData_t* common_cmd = (CommandData_t*)buffer;
  CommandData_t send_cmd;
  char send_data[MAXPACKETSIZE];
  int ret = -1;

  LOG_INFO("HandlerCheckDevice:\n");

  // for (int i = 0; i < common_cmd->datLen; i++) {
  //   LOG_INFO("DATA[%d]: 0x%x\n", i, common_cmd->dat[i]);
  // }

  if (strcmp((char*)common_cmd->RKID, RKID_CHECK) == 0) {
    LOG_INFO("RKID: %s\n", common_cmd->RKID);
  } else {
    LOG_INFO("RKID: Unknow\n");
    return;
  }

  LOG_INFO("cmdID: %d\n", common_cmd->cmdID);

  switch (common_cmd->cmdID) {
    case CMD_ID_CAPTURE_STATUS:
      LOG_INFO("CmdID CMD_ID_CAPTURE_STATUS in\n");
      if (common_cmd->dat[0] == KNOCK_KNOCK) {
        InitCommandPingAns(&send_cmd, READY);
        LOG_INFO("Device is READY\n");
      } else {
        LOG_ERROR("Unknow CMD_ID_CAPTURE_STATUS message\n");
      }
      memcpy(send_data, &send_cmd, sizeof(CommandData_t));
      send(sockfd, send_data, sizeof(CommandData_t), 0);
      LOG_INFO("cmdID CMD_ID_CAPTURE_STATUS out\n\n");
      break;
    case CMD_ID_GET_STATUS:
      DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, READY);
      break;
    case CMD_ID_GET_MODE:
      DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, g_app_run_mode);
      break;
    case CMD_ID_START_RTSP:
      g_rtsp_en = 1;
      ret = StartRTSP();
      if (ret) {
        LOG_ERROR("start RTSP failed!!!");
      }
      DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, g_app_run_mode);
      break;
    case CMD_ID_STOP_RTSP:
      g_rtsp_en = 0;
      ret = StopRTSP();
      if (ret) {
        LOG_ERROR("stop RTSP failed!!!");
      }
      g_app_run_mode = APP_RUN_STATUS_INIT;
      DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, g_app_run_mode);
      break;
    default:
      break;
  }
}



void RKAiqProtocol::HandlerReceiveFile(int sockfd, char* buffer, int size) {
  FileTransferData* recData = (FileTransferData*)buffer;
  LOG_INFO("HandlerReceiveFile begin\n");
  //HexDump((unsigned char*)buffer, size);
  //parse data
  unsigned long long packetSize = recData->packetSize;
  LOG_DEBUG("FILETRANS receive : sizeof(packetSize):%d\n", sizeof(packetSize));
  //HexDump((unsigned char*)&recData->packetSize, 8);
  LOG_DEBUG("FILETRANS receive : packetSize:%llu\n", packetSize);

  unsigned long long dataSize = recData->dataSize;
  LOG_DEBUG("FILETRANS receive : dataSize:%llu\n", dataSize);
  if (packetSize <= 0 || packetSize - dataSize > 500) {
    printf("FILETRANS no data received or packetSize error, return.\n");
    char tmpBuf[200]={0};
    snprintf(tmpBuf, sizeof(tmpBuf), "##StatusMessage##FileTransfer##Failed##TransferError##");
    std::string resultStr = tmpBuf;
    send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
    return;
  }

  char* receivedPacket = (char*)malloc(packetSize);
  memset(receivedPacket, 0, packetSize);
  memcpy(receivedPacket, buffer, size);

  unsigned long long remain_size = packetSize - size;
  int recv_size = 0;

  struct timespec startTime = {0, 0};
  struct timespec currentTime = {0, 0};
  clock_gettime(CLOCK_REALTIME, &startTime);
  printf("FILETRANS get, start receive:%ld\n", startTime.tv_sec);
  while (remain_size > 0) {
    clock_gettime(CLOCK_REALTIME, &currentTime);
    if (currentTime.tv_sec - startTime.tv_sec >= 20) {
      LOG_DEBUG("FILETRANS receive: receive data timeout, return\n");
      char tmpBuf[200]={0};
      snprintf(tmpBuf, sizeof(tmpBuf), "##StatusMessage##FileTransfer##Failed##Timeout##");
      std::string resultStr = tmpBuf;
      send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
      return;
    }

    unsigned long long offset = packetSize - remain_size;
    
    unsigned long long targetSize = 0;
    if (remain_size > MAX_PACKET_SIZE) {
      targetSize = MAX_PACKET_SIZE;
    } else {
      targetSize = remain_size;
    }
    recv_size = recv(sockfd, &receivedPacket[offset], targetSize, 0);
    remain_size = remain_size - recv_size;

    // LOG_DEBUG("FILETRANS receive,remain_size: %llu\n", remain_size);
  }
  LOG_DEBUG("FILETRANS receive: receive success, need check data\n");

  // HexDump((unsigned char*)receivedPacket, packetSize);
  // Send(receivedPacket, packetSize); //for debug use

  // parse data
  FileTransferData receivedData;
  memset((void*)&receivedData, 0, sizeof(FileTransferData));
  unsigned long long offset = 0;
  // magic
  memcpy(receivedData.RKID, receivedPacket, sizeof(receivedData.RKID));
  //HexDump((unsigned char*)receivedData.RKID, sizeof(receivedData.RKID));
  offset += sizeof(receivedData.RKID);
  // packetSize
  memcpy((void*)&receivedData.packetSize, receivedPacket + offset, sizeof(receivedData.packetSize));
  offset += sizeof(receivedData.packetSize);
  // command id
  memcpy((void*)&(receivedData.commandID), receivedPacket + offset, sizeof(int));
  offset += sizeof(int);
  // command result
  memcpy((void*)&(receivedData.commandResult), receivedPacket + offset, sizeof(int));
  offset += sizeof(int);
  // target dir len
  memcpy((void*)&(receivedData.targetDirLen), receivedPacket + offset, sizeof(receivedData.targetDirLen));
  offset += sizeof(receivedData.targetDirLen);
  LOG_DEBUG("FILETRANS receive: receivedData.targetDirLen:%d\n", receivedData.targetDirLen);
  // target dir
  memcpy((void*)&(receivedData.targetDir), receivedPacket + offset, sizeof(receivedData.targetDir));
  //HexDump((unsigned char*)receivedData.targetDir, sizeof(receivedData.targetDir));
  LOG_DEBUG("FILETRANS receive: receivedData.targetDir:%s\n", receivedData.targetDir);
  offset += sizeof(receivedData.targetDir);
  // target file name len
  memcpy((void*)&(receivedData.targetFileNameLen), receivedPacket + offset, sizeof(receivedData.targetFileNameLen));
  offset += sizeof(receivedData.targetFileNameLen);
  LOG_DEBUG("FILETRANS receive: receivedData.targetFileNameLen:%d\n", receivedData.targetFileNameLen);
  // target file name
  memcpy((void*)&(receivedData.targetFileName), receivedPacket + offset, sizeof(receivedData.targetFileName));
  //HexDump((unsigned char*)receivedData.targetFileName, sizeof(receivedData.targetFileName));
  LOG_DEBUG("FILETRANS receive: receivedData.targetFileName:%s\n", receivedData.targetFileName);
  offset += sizeof(receivedData.targetFileName);
  
  // data size
  memcpy((void*)&(receivedData.dataSize), receivedPacket + offset, sizeof(unsigned long long));
  LOG_DEBUG("FILETRANS receive: receivedData.dataSize:%u\n", receivedData.dataSize);
  offset += sizeof(unsigned long long);
  // data
  receivedData.data = (char*)malloc(receivedData.dataSize);
  memcpy(receivedData.data, receivedPacket + offset, receivedData.dataSize);
  offset += receivedData.dataSize;
  // data hash
  memcpy((void*)&(receivedData.dataHash), receivedPacket + offset, sizeof(unsigned int));

  if (receivedPacket != NULL) {
    free(receivedPacket);
    receivedPacket = NULL;
  }

  // size check
  if (receivedData.dataSize != dataSize) {
    LOG_DEBUG("FILETRANS receive: receivedData.dataSize != target data size, return\n");
    char tmpBuf[200]={0};
    snprintf(tmpBuf, sizeof(tmpBuf), "##StatusMessage##FileTransfer##Failed##DataSizeError##");
    std::string resultStr = tmpBuf;
    send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
    return;
  }

  // hash check
  unsigned int dataHash = MurMurHash(receivedData.data, receivedData.dataSize);
  LOG_DEBUG("FILETRANS receive 2: dataHash calculated:%x\n", dataHash);
  LOG_DEBUG("FILETRANS receive: receivedData.dataHash:%x\n", receivedData.dataHash);

  if (dataHash == receivedData.dataHash) {
    LOG_DEBUG("FILETRANS receive: data hash check pass\n");
  } else {
    LOG_DEBUG("FILETRANS receive: data hash check failed\n");
    char tmpBuf[200]={0};
    snprintf(tmpBuf, sizeof(tmpBuf), "##StatusMessage##FileTransfer##Failed##HashCheckFail##");
    std::string resultStr = tmpBuf;
    send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
    return;
  }

  //save to file
  std::string dstDir = (char*)receivedData.targetDir;
  std::string dstFileName = (char*)receivedData.targetFileName;
  std::string dstFilePath = dstDir + "/" + dstFileName;

  DIR *dirPtr = opendir(dstDir.c_str());
  if (dirPtr == NULL)
  {
    LOG_DEBUG("FILETRANS target dir %s not exist, return \n", dstDir.c_str());
    char tmpBuf[200]={0};
    snprintf(tmpBuf, sizeof(tmpBuf), "##StatusMessage##FileTransfer##Failed##DirError##");
    std::string resultStr = tmpBuf;
    send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
    return;
  }
  else
  {
    closedir(dirPtr);  
  }
  
  FILE *fWrite = fopen(dstFilePath.c_str(), "w");
  if(fWrite!=NULL)
  {
    fwrite(receivedData.data, receivedData.dataSize, 1, fWrite);
  }
  else
  {
    LOG_DEBUG("FILETRANS failed to create file %s, return\n",dstFilePath.c_str());
    char tmpBuf[200]={0};
    snprintf(tmpBuf, sizeof(tmpBuf), "##StatusMessage##FileTransfer##Failed##FileSaveError##");
    std::string resultStr = tmpBuf;
    send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
    return;
  }

  fclose(fWrite);
  if (receivedData.data != NULL) {
    free(receivedData.data);
    receivedData.data = NULL;
  }

  LOG_INFO("HandlerReceiveFile process finished.\n");

  char tmpBuf[200]={0};
  snprintf(tmpBuf, sizeof(tmpBuf), "##StatusMessage##FileTransfer##Success##%s##", dstFileName.c_str());
  std::string resultStr = tmpBuf;
  send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
}

void RKAiqProtocol::HandlerTCPMessage(int sockfd, char* buffer, int size) {
  CommandData_t* common_cmd = (CommandData_t*)buffer;
  LOG_INFO("HandlerTCPMessage:\n");
  LOG_INFO("HandlerTCPMessage CommandData_t: 0x%lx\n", sizeof(CommandData_t));
  LOG_INFO("HandlerTCPMessage RKID: %s\n", (char*)common_cmd->RKID);

  // TODO Check APP Mode

  if (strcmp((char*)common_cmd->RKID, TAG_PC_TO_DEVICE) == 0) {
    RKAiqRawProtocol::HandlerRawCapMessage(sockfd, buffer, size);
  } else if (strcmp((char*)common_cmd->RKID, TAG_OL_PC_TO_DEVICE) == 0) {
    RKAiqOLProtocol::HandlerOnLineMessage(sockfd, buffer, size);
  } else if (strcmp((char*)common_cmd->RKID, RKID_CHECK) == 0) {
    HandlerCheckDevice(sockfd, buffer, size);
  } else if (memcmp((char*)common_cmd->RKID, RKID_SEND_FILE, 8) == 0) {
    HandlerReceiveFile(sockfd, buffer, size);
  } else {
    if (!DoChangeAppMode(APP_RUN_STATUS_TUNRING)) MessageForward(sockfd, buffer, size);
  }
}

int RKAiqProtocol::doMessageForward(int sockfd) {
  is_recv_running = true;
  while (is_recv_running) {
    char recv_buffer[MAXPACKETSIZE] = {0};
    int recv_len = g_tcpClient.Receive(recv_buffer, MAXPACKETSIZE);
    if (recv_len > 0) {
      ssize_t ret = send(sockfd, recv_buffer, recv_len, 0);
      if (ret < 0) {
          LOG_ERROR("Forward socket %d failed\n", sockfd);
          close(sockfd);
          std::lock_guard<std::mutex> lk(mutex_);
          is_recv_running = false;
          return -1;
      }
    } else if (recv_len < 0 && errno != EAGAIN) {
      g_tcpClient.Close();
      close(sockfd);
      std::lock_guard<std::mutex> lk(mutex_);
      is_recv_running = false;
      return -1;
    }
  }

  std::lock_guard<std::mutex> lk(mutex_);
  is_recv_running = false;
  return 0;
}

int RKAiqProtocol::MessageForward(int sockfd, char* buffer, int size) {
  LOG_INFO("[%s]got data:%d!\n", __func__, size);
  int ret = g_tcpClient.Send((char*)buffer, size);
  if (ret < 0 && (errno != EAGAIN && errno != EINTR)) {
      g_tcpClient.Close();
      g_app_run_mode = APP_RUN_STATUS_INIT;
      LOG_ERROR("Forward to AIQ failed!");
      return -1;
  }

  std::lock_guard<std::mutex> lk(mutex_);
  if (is_recv_running) {
    return 0;
  }

#if 0
  if (forward_thread && forward_thread->joinable()) forward_thread->join();
#endif

  forward_thread = std::unique_ptr<std::thread>(new std::thread(&RKAiqProtocol::doMessageForward, sockfd));
  forward_thread->detach();

  return 0;
}

void RKAiqProtocol::Exit() {
  {
    std::lock_guard<std::mutex> lk(mutex_);
    is_recv_running = false;
  }
#if 0
  if (forward_thread && forward_thread->joinable()) {
    forward_thread->join();
  }
#endif
}
