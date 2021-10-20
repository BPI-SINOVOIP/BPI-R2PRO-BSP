#include <signal.h>
#include <unistd.h>

#include <atomic>
#include <ctime>

#include "camera_infohw.h"
#include "domain_tcp_client.h"
#if 0
#include "rkaiq_manager.h"
#endif
#include "rkaiq_protocol.h"
#include "tcp_server.h"
#ifdef __ANDROID__
#include <rtspserver/RtspServer.h>
#include <cutils/properties.h>
#include "RtspServer.h"
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

DomainTCPClient g_tcpClient;
struct ucred* g_aiqCred = nullptr;
std::atomic_bool quit{false};
std::atomic<int> g_app_run_mode(APP_RUN_STATUS_INIT);
int g_width = 1920;
int g_height = 1080;
int g_device_id = 0;
int g_rtsp_en = 0;
int g_allow_killapp = 0;
int g_cam_count = 0;

std::string g_stream_dev_name;
std::string iqfile;
std::string g_sensor_name;
std::string g_sensor_name1;

std::shared_ptr<TCPServer> tcpServer = nullptr;
#if 0
std::shared_ptr<RKAiqToolManager> rkaiq_manager;
#endif
std::shared_ptr<RKAiqMedia> rkaiq_media;

void signal_handle(int sig) {
  quit.store(true, std::memory_order_release);
  if (tcpServer != nullptr) tcpServer->SaveExit();

  RKAiqProtocol::Exit();

  if (g_rtsp_en)
    deinit_rtsp();
}

static int get_env(const char* name, int* value, int default_value) {
  char* ptr = getenv(name);
  if (NULL == ptr) {
    *value = default_value;
  } else {
    char* endptr;
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

static const char short_options[] = "s:i:m:Dd:w:h:r:";
static const struct option long_options[] = {{"stream_dev", required_argument, NULL, 's'},
                                             {"iqfile", required_argument, NULL, 'i'},
                                             {"mode", required_argument, NULL, 'm'},
                                             {"width", no_argument, NULL, 'w'},
                                             {"height", no_argument, NULL, 'h'},
                                             {"device_id", required_argument, NULL, 'd'},
                                             {"help", no_argument, NULL, 'h'},
                                             {0, 0, 0, 0}};

static void parse_args(int argc, char** argv) {
  for (;;) {
    int idx;
    int c;
    c = getopt_long(argc, argv, short_options, long_options, &idx);
    if (-1 == c) {
      break;
    }
    switch (c) {
      case 0:
        break;
      case 's':
        g_stream_dev_name = optarg;
        break;
      case 'i':
        iqfile = optarg;
        break;
      case 'm':
        g_app_run_mode = atoi(optarg);
        break;
      case 'w':
        g_width = atoi(optarg);
        break;
      case 'h':
        g_height = atoi(optarg);
        break;
      case 'd':
        g_device_id = atoi(optarg);
        break;
      default:
        break;
    }
  }
  if (iqfile.empty()) {
#ifdef __ANDROID__
    iqfile = "/vendor/etc/camera/rkisp2";
#else
    iqfile = "/oem/etc/iqfiles";
#endif
  }
}

int main(int argc, char** argv) {
  int ret = -1;
  LOG_ERROR("#### AIQ tool server 20201222-0933 ####\n");

#ifdef _WIN32
  signal (SIGINT, signal_handle);
  signal (SIGQUIT, signal_handle);
  signal (SIGTERM, signal_handle);
#else
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGTERM);
  sigaddset(&mask, SIGQUIT);
  pthread_sigmask(SIG_BLOCK, &mask, NULL);

  struct sigaction new_action, old_action;
  new_action.sa_handler = signal_handle;
  sigemptyset (&new_action.sa_mask);
  new_action.sa_flags = 0;
  sigaction (SIGINT, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN)
	  sigaction (SIGINT, &new_action, NULL);
  sigaction (SIGQUIT, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN)
	  sigaction (SIGQUIT, &new_action, NULL);
  sigaction (SIGTERM, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN)
	  sigaction (SIGTERM, &new_action, NULL);
#endif

#ifdef __ANDROID__
  char property_value[PROPERTY_VALUE_MAX] = {0};
  property_get("persist.vendor.aiqtool.log", property_value, "5");
  log_level = strtoull(property_value, nullptr, 16);
  property_get("persist.vendor.aiqtool.killapp", property_value, "1");
  g_allow_killapp = strtoull(property_value, nullptr, 16);
  // property_get("persist.vendor.rkisp_no_read_back", property_value, "-1");
  // readback = strtoull(property_value, nullptr, 16);
#else
  get_env("rkaiq_tool_server_log_level", &log_level, 5);
  get_env("rkaiq_tool_server_kill_app", &g_allow_killapp, 0);
#endif

  parse_args(argc, argv);
  LOG_DEBUG("iqfile cmd_parser.get  %s\n", iqfile.c_str());
  LOG_DEBUG("g_mode cmd_parser.get  %d\n", g_app_run_mode.load());
  LOG_DEBUG("g_width cmd_parser.get  %d\n", g_width);
  LOG_DEBUG("g_height cmd_parser.get  %d\n", g_height);
  LOG_DEBUG("g_device_id cmd_parser.get  %d\n", g_device_id);

  rkaiq_media = std::make_shared<RKAiqMedia>();
  for (int i = 0; i < MAX_CAM_NUM; i++) rkaiq_media->GetMediaInfo();
  rkaiq_media->DumpMediaInfo();

  LOG_DEBUG("================== %d =====================", g_app_run_mode.load());

  if (g_stream_dev_name.length() > 0) {
    if (0 > access(g_stream_dev_name.c_str(), R_OK | W_OK)) {
      LOG_DEBUG("Could not access streaming device");
      g_rtsp_en = 0;
    } else {
      g_rtsp_en = 1;
    }
  }

  if (g_rtsp_en && g_stream_dev_name.length() > 0) {
    ret = RKAiqProtocol::DoChangeAppMode(APP_RUN_STATUS_STREAMING);
    if (ret != 0) {
      LOG_ERROR("Failed set mode to tunning mode, does app started?");
    }
  } else {
    ret = RKAiqProtocol::DoChangeAppMode(APP_RUN_STATUS_TUNRING);
    if (ret != 0) {
      LOG_ERROR("Failed set mode to tunning mode, does app started?");
    }
  }

  pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
  tcpServer = std::make_shared<TCPServer>();
  tcpServer->RegisterRecvCallBack(RKAiqProtocol::HandlerTCPMessage);
  tcpServer->Process(SERVER_PORT);
  while (!quit.load() && !tcpServer->Exited()) {
    pause();
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  tcpServer->SaveExit();

  if (g_aiqCred != nullptr) {
    delete g_aiqCred;
    g_aiqCred = nullptr;
  }

  if (g_rtsp_en) {
    system("pkill rkaiq_3A_server*");
    deinit_rtsp();
  }

#if 0
  rkaiq_manager.reset();
  rkaiq_manager = nullptr;
#endif
  return 0;
}
