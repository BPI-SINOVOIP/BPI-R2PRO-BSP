// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <getopt.h>
#include <memory>

#include "flow_export.h"
#include "mediaserver.h"

#ifdef LINK_VENDOR
#include "vendor_storage.h"
#define VENDOR_LINKKIT_LICENSE_ID 255 // max 255
#define VENDOR_TUYA_LICENSE_ID 254
#endif

#ifdef ENABLE_CY43438
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/serial.h>
extern "C"{
#include  "utils/CY_WL_API/wifi.h"
}
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "mediaserver.cpp"

static bool quit = false;
static bool need_dbus = true;
static bool is_session_bus = false;
static std::string media_config;
static bool need_dbserver = true;
#ifdef LINK_API_ENABLE
static rockchip::mediaserver::LicenseKey license;
#endif

#ifdef ENABLE_SHM_SERVER
#include "shm_control.h"
shmc::ShmQueue<shmc::SVIPC> queue_w_;
#endif

static void parse_args(int argc, char **argv);

#ifdef ENABLE_CY43438
static void *CY43438WifiInit(void *arg) {
  LOG_INFO("WIFI_Init begin\n");
  int ret = WIFI_Init();
	if (ret) {
		LOG_ERROR("WIFI_Init, err = %d\n", ret);
		return NULL;
	}
  LOG_INFO("WIFI_Init success\n");

  ret = WIFI_Resume();
	if (ret) {
		LOG_ERROR("resume_enter, err = %d\n", ret);
		return NULL;
	}
  LOG_INFO("WIFI_Resume success\n");

  ret = WIFI_GetStatus();
	if (ret) { //if (wl_is_associated())
		LOG_INFO("Already joined AP.\n");
		rk_obtain_ip_from_vendor((char *)"wlan0");
    LOG_INFO("rk_obtain_ip_from_vendor over.\n");
    system("aplay /etc/connect_success.wav &");
    // system("ping 111.231.160.125 -c 3 > /dev/kmsg 2>&1 & ");
	} else {
    LOG_INFO("Not joined AP.\n");
    wifi_info_s wifi_info;
    LOG_INFO("get dhcp info from vendor\n");
    ret = rkvendor_read(VENDOR_WIFI_INFO_ID, (char *)&wifi_info, sizeof(wifi_info));
    if (ret) {
      system("aplay /etc/no_connect_net.wav &");
      return NULL;
    }

	  LOG_INFO("ssid:%s, psk:%s, ip_addr: %s, netmask: %s, gateway: %s, dns: %s\n",
		          wifi_info.ssid, wifi_info.psk, wifi_info.ip_addr, wifi_info.netmask, wifi_info.gateway, wifi_info.dns);
    if (strlen(wifi_info.ssid) && strlen(wifi_info.psk)) {
      if (WIFI_Connect(wifi_info.ssid, wifi_info.psk, 0)) {
        LOG_ERROR("WIFI_Connect fail\n");
        system("aplay /etc/connect_fail.wav &");
        return NULL;
      }
      LOG_INFO("WIFI_Connect success\n");
      if (rk_obtain_ip_from_udhcpc((char *)"wlan0")) {
        LOG_ERROR("obtain_ip fail\n");
        system("aplay /etc/connect_fail.wav &");
      } else {
        LOG_INFO("obtain_ip success\n");
        system("aplay /etc/connect_success.wav &");
      }
    }
	}

  return NULL;
}

static void uart_to_mcu_poweroff() {
  int fd;
  int baud = B115200;
  struct termios newtio;
	struct serial_rs485 rs485;

	fd = open("/dev/ttyS5", O_RDWR | O_NONBLOCK);
	if (fd < 0) {
		LOG_ERROR("Error opening serial port\n");
		return;
	}

	bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */

	/* man termios get more info on below settings */
	newtio.c_cflag = baud | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = 0;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	// block for up till 128 characters
	newtio.c_cc[VMIN] = 128;
	// 0.5 seconds read timeout
	newtio.c_cc[VTIME] = 5;
	/* now clean the modem line and activate the settings for the port */
	tcflush(fd, TCIOFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);
  if(ioctl(fd, TIOCGRS485, &rs485) >= 0) {
    /* disable RS485 */
    rs485.flags &= ~(SER_RS485_ENABLED | SER_RS485_RTS_ON_SEND | SER_RS485_RTS_AFTER_SEND);
    rs485.delay_rts_after_send = 0;
    rs485.delay_rts_before_send = 0;
    if(ioctl(fd, TIOCSRS485, &rs485) < 0) {
      perror("Error setting RS-232 mode");
    }
	}

  /*
  * The flag ASYNC_SPD_CUST might have already been set, so
  * clear it to avoid confusing the kernel uart dirver.
  */
  struct serial_struct ss;
	if (ioctl(fd, TIOCGSERIAL, &ss) < 0) {
		// return silently as some devices do not support TIOCGSERIAL
    LOG_INFO("return silently as some devices do not support TIOCGSERIAL\n");
		return;
	}
	// if ((ss.flags & ASYNC_SPD_MASK) != ASYNC_SPD_CUST)
	// 	return;
	ss.flags &= ~ASYNC_SPD_MASK;
	if (ioctl(fd, TIOCSSERIAL, &ss) < 0) {
		LOG_ERROR("TIOCSSERIAL failed");
		return;
	}

  // write
  int written = write(fd, "\x11\x26", 4);
  if (written < 0)
    LOG_ERROR("write()");
  else
    LOG_INFO("written is %d\n", written);
}
#endif // ENABLE_CY43438

namespace rockchip {
namespace mediaserver {

#ifdef LINK_API_ENABLE
int MediaServer::InitMediaLink() {
  LinkManagerPtr &link_manager = LinkManager::GetInstance();
  if (link_manager) {
    auto link_flow_unit = link_manager->GetLinkFlowUnit(0, "video:");
    if (link_flow_unit) {
#ifdef LINK_API_ENABLE_LINKKIT
#ifdef LINK_VENDOR
      char vendor_data[256] = {0};
      while (rkvendor_read(VENDOR_LINKKIT_LICENSE_ID, vendor_data,
                           sizeof(vendor_data) / sizeof(vendor_data[0]))) {
        LOG_INFO("rkvendor_read fail, retry\n");
        usleep(100000);
      }
      LOG_INFO("vendor_data is %s\n", vendor_data);
      nlohmann::json license_js = nlohmann::json::parse(vendor_data);
      license.product_key = license_js.at("product_key");
      license.product_secret = license_js.at("product_secret");
      license.device_name = license_js.at("device_name");
      license.device_secret = license_js.at("device_secret");
#else  // LINK_VENDOR
      license.product_key = link_flow_unit->GetProductKey();
      license.product_secret = link_flow_unit->GetProductSecret();
      license.device_name = link_flow_unit->GetDeviceName();
      license.device_secret = link_flow_unit->GetDeviceSecret();
#endif // LINK_VENDOR
      LOG_INFO("license.product_key %s\n", license.product_key.c_str());
      LOG_INFO("license.product_secret %s\n", license.product_secret.c_str());
      LOG_INFO("license.device_name %s\n", license.device_name.c_str());
      LOG_INFO("license.device_secret %s\n", license.device_secret.c_str());
#elif defined LINK_API_ENABLE_TUYA
#ifdef LINK_VENDOR
      char vendor_data[256] = {0};
      if (rkvendor_read(VENDOR_TUYA_LICENSE_ID, vendor_data,
                           sizeof(vendor_data) / sizeof(vendor_data[0]))) {
        LOG_INFO("rkvendor_read fail\n");
        system("aplay /etc/no_key.wav &");
        return -1;
      }
      LOG_INFO("vendor_data is %s\n", vendor_data);
      nlohmann::json license_js = nlohmann::json::parse(vendor_data);
      license.pid = license_js.at("pid");
      license.uuid = license_js.at("uuid");
      license.authkey = license_js.at("authkey");
#else  // LINK_VENDOR
      license.pid = link_flow_unit->GetPID();
      license.uuid = link_flow_unit->GetUUID();
      license.authkey = link_flow_unit->GetAuthkey();
#endif // LINK_VENDOR
      LOG_INFO("license.pid %s\n", license.pid.c_str());
      LOG_INFO("license.uuid %s\n", license.uuid.c_str());
      LOG_INFO("license.authkey %s\n", license.authkey.c_str());
#endif
    } else {
      LOG_ERROR("can't find link flow\n");
      return -1;
    }
    link_manager->FillLicenseKey(&license);
    link_manager->InitDevice();
    link_manager->StartLink();
  }
  return 0;
}

int MediaServer::DeInitMediaLink() {
  LinkManagerPtr &link_manager = LinkManager::GetInstance();
  if (link_manager) {
    link_manager->StopLink();
    link_manager->DeInitDevice();
  }
  return 0;
}
#endif // LINK_API_ENABLE

#ifdef ENABLE_DBUS
int MediaServer::InitDbusServer() {
  if (need_dbus) {
    dbus_server_.reset(new DBusServer(is_session_bus, need_dbserver));
    assert(dbus_server_);
    dbus_server_->start();
  }
  return 0;
}

int MediaServer::DeInitDbusServer() {
  if (need_dbus) {
    if (dbus_server_ != nullptr)
      dbus_server_->stop();
  }
  return 0;
}

int MediaServer::RegisterDbusProxy(FlowManagerPtr &flow_manager) {
  if (need_dbus) {
    flow_manager->RegisterDBserverProxy(dbus_server_->GetDBserverProxy());
    flow_manager->RegisterDBEventProxy(dbus_server_->GetDBEventProxy());
    flow_manager->RegisterStorageManagerProxy(dbus_server_->GetStorageProxy());
    flow_manager->RegisterIspserverProxy(dbus_server_->GetIspserverProxy());
  }
  return 0;
}
#endif // ENABLE_DBUS

MediaServer::MediaServer() {
  LOG_DEBUG("media servers setup ...\n");
#ifdef ENABLE_CY43438
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, CY43438WifiInit, NULL);
#endif

#ifdef ENABLE_DBUS
  InitDbusServer();
#endif
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  assert(flow_manager);
#ifdef ENABLE_DBUS
  RegisterDbusProxy(flow_manager);
#endif
  flow_manager->ConfigParse(media_config);
  flow_manager->CreatePipes();
#ifdef ENABLE_DBUS
#ifdef ENABLE_SCHEDULES_SERVER
  if (need_dbserver) {
    flow_manager->InitScheduleMutex();
    flow_manager->CreateSchedules();
  }
#endif
#endif

#ifdef ENABLE_ZBAR
  flow_manager->CreateScanImage();
#endif

#ifdef ENABLE_SHM_SERVER
  auto camera_pipe = GetFlowPipe(0, StreamType::CAMERA);
  if (camera_pipe) {
    auto link_flow = camera_pipe->GetFlow(StreamType::LINK);
    if (link_flow)
      link_flow->SetUserCallBack(nullptr, ShmControl::PushUserHandler);
    auto rockx_filter =
        camera_pipe->GetFlow(StreamType::FILTER, RKMEDIA_FILTER_ROCKX_FILTER);
    LOG("RegisterCallBack ***********rockx_filter=%p\n", rockx_filter);
    if (rockx_filter)
      rockx_filter->Control(easymedia::S_NN_CALLBACK,
                            ShmControl::PushUserHandler);
  }
  auto file_pipe = GetFlowPipe(0, StreamType::FILE);
  if (file_pipe) {
    auto link_flow = file_pipe->GetFlow(StreamType::LINK);
    if (link_flow)
      link_flow->SetUserCallBack(nullptr, ShmControl::PushUserHandler);
  }
#endif

#ifdef LINK_API_ENABLE
  InitMediaLink(); // will block until connecting to aliyun
#endif

  LOG_DEBUG("media servers setup ok\n");
}

MediaServer::~MediaServer() {
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();

#ifdef LINK_API_ENABLE
  DeInitMediaLink();
#endif

#ifdef ENABLE_ZBAR
  flow_manager->DestoryScanImage();
#endif

#ifdef ENABLE_DBUS
#ifdef ENABLE_SCHEDULES_SERVER
  if (need_dbserver)
    flow_manager->DestorySchedules();
#endif
  DeInitDbusServer();
#endif

  flow_manager->SaveConfig(FLOWS_CONF_BAK);
  flow_manager->DestoryPipes();
  flow_manager.reset();
  LOG_DEBUG("media servers upload finish\n");
}

} // namespace mediaserver
} // namespace rockchip

static void sigterm_handler(int sig) {
  fprintf(stderr, "signal %d\n", sig);
  LOG_DEINIT();

#ifdef LINK_API_ENABLE
#ifdef THUNDER_BOOT
  // StopRecord(0);
  system("echo timer > /sys/class/leds/red/trigger");
  auto &link_manager = rockchip::mediaserver::LinkManager::GetInstance();
  if (link_manager) {
    link_manager->ReportWakeUpData1();
#ifdef ENABLE_CY43438
    if (sig == SIGQUIT)
      uart_to_mcu_poweroff();
#endif
    link_manager->StopLink();
    link_manager->DeInitDevice();
  }

#ifdef LINK_API_ENABLE_LINKKIT
  auto link_flow_unit = link_manager->GetLinkFlowUnit(0, "video:");
  char cmd[1024];
  sprintf(cmd, "mqtt-rrpc-demo -p %s -n %s -s %s 2>&1 &",
          license.product_key.c_str(), license.device_name.c_str(),
          license.device_secret.c_str());
  printf("cmd is %s\n", cmd);
  system(cmd);
#endif // LINK_API_ENABLE_LINKKIT

#endif // THUNDER_BOOT
#endif // LINK_API_ENABLE
  quit = true;
}

int main(int argc, char *argv[]) {
#ifdef ENABLE_SHM_SERVER
  shmc::SetLogHandler(shmc::kDebug, [](shmc::LogLevel lv, const char *s) {
    fprintf(stderr, "[%d] %s", lv, s);
  });
  queue_w_.InitForWrite(kShmKey, kQueueBufSize);
#endif
  parse_args(argc, argv);
  setenv("RKMEDIA_LOG_METHOD", "MINILOG", 0);
  setenv("RKMEDIA_LOG_LEVEL", "INFO", 0);
  LOG_INIT();
  __minilog_log_init(argv[0], NULL, false, enable_minilog_backtrace, argv[0],
                     "1.0.0");

  fprintf(stderr, "mediaserver[%d]: start config: %s\n", __LINE__,
          (char *)media_config.c_str());
  signal(SIGQUIT, sigterm_handler);
  signal(SIGINT, sigterm_handler);
  signal(SIGTERM, sigterm_handler);
  signal(SIGXCPU, sigterm_handler);
  signal(SIGPIPE, SIG_IGN);
  std::unique_ptr<rockchip::mediaserver::MediaServer> MS =
      std::unique_ptr<rockchip::mediaserver::MediaServer>(
          new rockchip::mediaserver::MediaServer());
  while (!quit) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  MS.reset();
  return 0;
}

static void usage_tip(FILE *fp, int argc, char **argv) {
  fprintf(
      fp,
      "Usage: %s [options]\n"
      "Version %s\n"
      "Options:\n"
      "-c | --config      mediaserver confg file \n"
      "-S | --system      Use system bus \n"
      "-s | --session     Use session bus \n"
      "-D | --database    Depend dbserver app \n"
      "-d | --no_database Not depend dbserver app \n"
      "-a | --stand_alone Not depend dbus server \n"
      "-h | --help        For help \n\n"
      "Environment variable:\n"
      "mediaserver_log_level        [0/1/2/3],   loglevel: "
      "error/warn/info/debug\n"
      "enable_minilog_backtrace     [0/1],       enable minilog backtrace \n"
      "enable_encoder_debug         [0/1],       enable encoder statistics \n"
      "\n",
      argv[0], "V1.1");
}

static const char short_options[] = "c:SsDdah";
static const struct option long_options[] = {
    {"config", required_argument, NULL, 'c'},
    {"system", no_argument, NULL, 'S'},
    {"session", no_argument, NULL, 's'},
    {"database", no_argument, NULL, 'D'},
    {"no_database", no_argument, NULL, 'd'},
    {"stand_alone", no_argument, NULL, 'a'},
    {"help", no_argument, NULL, 'h'},
    {0, 0, 0, 0}};

static int get_env(const char *name, int *value, int default_value) {
  char *ptr = getenv(name);
  if (NULL == ptr) {
    *value = default_value;
  } else {
    char *endptr;
    int base = (ptr[0] == '0' && ptr[1] == 'x') ? (16) : (10);
    errno = 0;
    *value = strtoul(ptr, &endptr, base);
    if (errno || (ptr == endptr)) {
      errno = 0;
      *value = default_value;
    }
  }
  return 0;
}

static void parse_args(int argc, char **argv) {
  media_config = "";
#ifdef MEDIASERVER_CONF_PREFIX
  media_config.append(MEDIASERVER_CONF_PREFIX).append(FLOWS_CONF);
#else
  media_config.append(FLOWS_CONF);
#endif

#ifdef ENABLE_MINILOGGER
  enable_minilog = 1;
#else
  enable_minilog = 0;
#endif

  get_env("enable_minilog_backtrace", &enable_minilog_backtrace, 0);
  get_env("enable_encoder_debug", &enable_encoder_debug, 0);
  get_env("mediaserver_log_level", &mediaserver_log_level, LOG_LEVEL_INFO);
  LOG_INFO("mediaserver_log_level is %d\n", mediaserver_log_level);

  for (;;) {
    int idx;
    int c;
    c = getopt_long(argc, argv, short_options, long_options, &idx);
    if (-1 == c)
      break;
    switch (c) {
    case 0: /* getopt_long() flag */
      break;
    case 'c':
      media_config = optarg;
      break;
    case 'S':
      is_session_bus = false;
      break;
    case 's':
      is_session_bus = true;
      break;
    case 'D':
      need_dbserver = true;
      break;
    case 'd':
      need_dbserver = false;
      break;
    case 'a':
      need_dbus = false;
      break;
    case 'h':
      usage_tip(stdout, argc, argv);
      exit(EXIT_SUCCESS);
    default:
      usage_tip(stderr, argc, argv);
      exit(EXIT_FAILURE);
    }
  }
  if (media_config.empty()) {
    usage_tip(stderr, argc, argv);
    exit(EXIT_FAILURE);
  }
}
