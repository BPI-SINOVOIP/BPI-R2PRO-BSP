#include <linux/input.h>
#include <nlohmann/json.hpp>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "flow_export.h"
#include "logger/log.h"
#include "rkbar_scan_api.h"
#include "scan_image.h"

#ifdef LINK_API_ENABLE
#include "link_manager.h"
#ifdef LINK_API_ENABLE_TUYA
#include "tuya_ipc_api.h"
#endif
#endif

#ifdef ENABLE_CY43438
extern "C"{
#include "../CY_WL_API/wifi.h"
}
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "scan_image.cpp"

#define LINK_SOURCE_PIPE_INDEX 1
#define SCAN_RETRY_TIME 150 // if fps is 30, then time is 5s
#define DEFAULT_KEY_EVENT_PATH "/dev/input/event1"
#define DEFAULT_KEY_EVENT_CODE 139
#define DEFAULT_SCAN_IMAGE_WIDTH 640
#define DEFAULT_SCAN_IMAGE_HEIGHT 480

namespace rockchip {
namespace mediaserver {

static int ready_scan_ = 0;
static int retry_time_ = SCAN_RETRY_TIME;
static int scan_image_width_ = DEFAULT_SCAN_IMAGE_WIDTH;
static int scan_image_height_ = DEFAULT_SCAN_IMAGE_HEIGHT;

ScanImage::ScanImage() {
  LOG_DEBUG("ScanImage init\n");
  ready_scan_ = 0;
  retry_time_ = 150;
  key_fd_ = 0;
}

ScanImage::~ScanImage() { LOG_DEBUG("ScanImage deinit\n"); }

static void nv12_bar_process(unsigned char *buffer, unsigned int buffer_size,
                             int null, const char *null_ptr) {
  if (ready_scan_) {
    LOG_INFO("%s: size %d\n", __func__, buffer_size);
    image_t *img = NULL;
    img = (image_t *)malloc(sizeof(image_t));
    img->width = scan_image_width_;
    img->height = scan_image_height_;
    img->crop_x = 0;
    img->crop_y = 0;
    img->crop_w = scan_image_width_;
    img->crop_h = scan_image_height_;
    img->bin = (unsigned char *)malloc(img->width * img->height);
    img->tmp = NULL;
    img->data = (uint8_t *)buffer;
    void *rkbar_hand = NULL;
    int ret = rkbar_init(&rkbar_hand);
    if (ret == -1) {
      LOG_INFO("rkbar init is err");
      goto out;
    }

    ret = rkbar_scan(rkbar_hand, img);
    if (ret > 0) {
      ready_scan_ = 0;
      retry_time_ = SCAN_RETRY_TIME;
      const char *test = rkbar_getresult(rkbar_hand);
      char *data;
      data = (char*)malloc(strlen(test));
      memcpy(data, test, strlen(test));
      LOG_INFO("rkbar the decoding result is \" %s \" \n", data);
      system("aplay /etc/qr_recognized.wav &");
#ifdef LINK_API_ENABLE_LINKKIT
#ifdef THUNDER_BOOT
      // connect wifi
      char ssid[20], psk[20], cmd[100];
      sscanf(data, "%[^;];%[^;];%[^;]",ssid, psk, NULL);
      system("killall tb_start_wifi.sh");
      sprintf(cmd, "tb_start_wifi.sh %s %s true &", ssid, psk);
      LOG_INFO("cmd is %s\n", cmd);
      system(cmd);
#endif // THUNDER_BOOT
#elif defined LINK_API_ENABLE_TUYA
      tuya_ipc_direct_connect(data, TUYA_IPC_DIRECT_CONNECT_QRCODE);
#ifdef THUNDER_BOOT
      // connect wifi
      char ssid[20], psk[20], cmd[100], wifi_ssid[20], wifi_psk[20];
      sscanf(data, "%*[^:]:%[^,],%*[^:]:%[^,]", psk, ssid);
      memset(wifi_ssid, 0, 20);
      memset(wifi_psk, 0, 20);
      memcpy(wifi_ssid, &ssid[1], strlen(ssid) - 2);
      memcpy(wifi_psk, &psk[1], strlen(psk) - 2);
#ifdef ENABLE_CY43438 //ENABLE_CY43438
      LOG_INFO("WIFI_Connect %s %s\n", wifi_ssid, wifi_psk);
      if (WIFI_Connect(wifi_ssid, wifi_psk, 0)) {
        LOG_ERROR("WIFI_Connect fail\n");
        system("aplay /etc/connect_fail.wav &");
        goto out;
      }
      if (rk_obtain_ip_from_udhcpc("wlan0")) {
        LOG_ERROR("obtain_ip fail\n");
        system("aplay /etc/connect_fail.wav &");
        goto out;
      }
      system("aplay /etc/connect_success.wav &");
#else
      system("cp /etc/wpa_supplicant.conf /tmp");
      sprintf(cmd, "s/SSID/%s/g", ssid);
      LOG_INFO("cmd is %s\n", cmd);
      system(cmd);
      sprintf(cmd, "s/PASSWORD/%s/g", psk);
      LOG_INFO("cmd is %s\n", cmd);
      system(cmd);
      system("wpa_supplicant -B -i wlan0 -c /tmp/wpa_supplicant.conf");
      usleep(1000 * 1000);
      system("udhcpc -i wlan0");
#endif

#endif // THUNDER_BOOT
#endif
      if (data)
        free(data);
    } else {
      retry_time_--;
      if (retry_time_ == 0) {
        ready_scan_ = 0;
        retry_time_ = SCAN_RETRY_TIME;
#ifdef THUNDER_BOOT
        system("echo 0 > /sys/class/leds/blue/brightness");
#endif // THUNDER_BOOT
      }
    }
out:
    rkbar_deinit(rkbar_hand);
    if (img->bin)
      free(img->bin);
    if (img)
      free(img);
  }

  return;
}

static void *WaitKeyEvent(void *arg) {
  auto os = reinterpret_cast<ScanImage *>(arg);
  char thread_name[40];
  snprintf(thread_name, sizeof(thread_name), "WaitKeyEvent");
  prctl(PR_SET_NAME, thread_name);

  LOG_INFO("key event path is %s\n", os->key_event_path_.c_str());
  os->key_fd_ = open(os->key_event_path_.c_str(), O_RDONLY);
  if (os->key_fd_ < 0) {
    LOG_ERROR("can't open %s\n", os->key_event_path_.c_str());
    return nullptr;
  }
  fd_set rfds;
  int nfds = os->key_fd_ + 1;
  struct timeval timeout;
  struct input_event key_event;

  while (os->status() == kThreadRunning) {
    // The rfds collection must be emptied every time,
    // otherwise the descriptor changes cannot be detected
    timeout.tv_sec = 1;
    FD_ZERO(&rfds);
    FD_SET(os->key_fd_, &rfds);
    select(nfds, &rfds, NULL, NULL, &timeout);
    // wait for the key event to occur
    if (FD_ISSET(os->key_fd_, &rfds)) {
      read(os->key_fd_, &key_event, sizeof(key_event));
      LOG_INFO("[timeval:sec:%d,usec:%d,type:%d,code:%d,value:%d]\n",
               key_event.time.tv_sec, key_event.time.tv_usec, key_event.type,
               key_event.code, key_event.value);
      if ((key_event.code == os->key_event_code_) && key_event.value) {
        ready_scan_ = 1;
#ifdef THUNDER_BOOT
        system("echo timer > /sys/class/leds/blue/trigger");
#endif // THUNDER_BOOT
      }
    }
  }

  if (os->key_fd_) {
    close(os->key_fd_);
    os->key_fd_ = 0;
  }
  LOG_DEBUG("wait key event out\n");
  return nullptr;
}

void ScanImage::start(void) {
  LOG_DEBUG("ScanImage start\n");
  // get scan image width and height
  auto scan_pipe = GetFlowPipe(LINK_SOURCE_PIPE_INDEX, StreamType::CAMERA);
  if (scan_pipe) {
    auto scan_source_flow_unit = scan_pipe->GetFlowunit(StreamType::CAMERA);
    std::string width_s;
    std::string height_s;
    scan_source_flow_unit->GetResolution(width_s, height_s);
    scan_image_width_ = stoi(width_s);
    scan_image_height_ = stoi(height_s);
  } else {
    scan_image_width_ = DEFAULT_SCAN_IMAGE_WIDTH;
    scan_image_height_ = DEFAULT_SCAN_IMAGE_HEIGHT;
  }
  LOG_INFO("scan image width is %d, height is %d\n", scan_image_width_, scan_image_height_);

  // get key event code
  auto link_flow_unit =
      GetLinkFlowUnit(LINK_SOURCE_PIPE_INDEX, StreamType::CAMERA, "image:");
  if (link_flow_unit) {
    key_event_path_ = link_flow_unit->GetKeyEventPath();
    LOG_INFO("link_flow_unit key event path is %s\n", key_event_path_.c_str());
    if (key_event_path_.empty())
      key_event_path_ = DEFAULT_KEY_EVENT_PATH;
    key_event_code_ = link_flow_unit->GetKeyEventCode();
    LOG_INFO("link_flow_unit key event code is %d\n", key_event_code_);
    if (!key_event_code_)
      key_event_code_ = DEFAULT_KEY_EVENT_CODE;
  }

  // get link flow
  auto link_flow =
      GetLinkFlow(LINK_SOURCE_PIPE_INDEX, StreamType::CAMERA, "image:");
  if (link_flow) {
    link_flow->SetCaptureHandler(nv12_bar_process);
    LOG_INFO("SetCaptureHandler success\n");
  } else {
    LOG_ERROR("Can't find source link flow\n");
    return;
  }

  wait_key_event_thread_.reset(new Thread(WaitKeyEvent, this));
  wait_key_event_thread_->set_status(kThreadRunning);
}

void ScanImage::stop(void) {
  if (wait_key_event_thread_) {
    wait_key_event_thread_->set_status(kThreadStopping);
    wait_key_event_thread_->join();
    if (key_fd_) {
      close(key_fd_);
      key_fd_ = 0;
    }
  }
  LOG_DEBUG("schedules manager stop\n");
}

ThreadStatus ScanImage::status(void) {
  if (wait_key_event_thread_)
    return wait_key_event_thread_->status();
  else
    return kThreadRunning;
}

} // namespace mediaserver
} // namespace rockchip
