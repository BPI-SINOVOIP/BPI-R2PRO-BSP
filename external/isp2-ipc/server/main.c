#include <ctype.h>
#include <errno.h>
#include <fcntl.h>  /* low-level i/o */
#include <fcntl.h>  /* low-level i/o */
#include <getopt.h> /* getopt_long() */
#include <inttypes.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#include "../utils/log.h"
#include "config.h"

#include <linux/videodev2.h>
#include "mediactl/mediactl.h"
#include "rk_aiq_user_api_sysctl.h"
#include "rk_aiq_user_api_sysctl_ipc.h"

#if CONFIG_DBUS
#if CONFIG_CALLFUNC
#include "call_fun_ipc.h"
#include "fun_map.h"
#endif
#include <gdbus.h>
#endif

#if CONFIG_DBSERVER
#include "db_monitor.h"
#include "isp_func.h"
#include "isp_n2d_ctl.h"
#include "manage.h"
#endif

enum { LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG };

int enable_minilog = 0;
#define LOG_TAG "ispserver"
int ispserver_log_level = LOG_INFO;

#define CLEAR(x) memset(&(x), 0, sizeof(x))

/* Private v4l2 event */
#define CIFISP_V4L2_EVENT_STREAM_START (V4L2_EVENT_PRIVATE_START + 1)
#define CIFISP_V4L2_EVENT_STREAM_STOP (V4L2_EVENT_PRIVATE_START + 2)

#define RKAIQ_FILE_PATH_LEN 64
#define RKAIQ_CAMS_NUM_MAX 2
#define RKAIQ_FLASH_NUM_MAX 2

rk_aiq_sys_ctx_t *aiq_ctx[RKAIQ_CAMS_NUM_MAX] = {NULL, NULL};
rk_aiq_working_mode_t mode[RKAIQ_CAMS_NUM_MAX] = {RK_AIQ_WORKING_MODE_ISP_HDR2,
                                                  RK_AIQ_WORKING_MODE_ISP_HDR2};
rk_aiq_sys_ctx_t *db_aiq_ctx = NULL;
static int silent = 0;
static int width = 2688;
static int height = 1520;
static int fixfps = -1;
static bool need_sync_db = true;

#if SYS_START
static bool is_quick_start = true;
#else
static bool is_quick_start = false;
#endif

#if SYS_START
const char *iq_file_dir = "/etc/iqfiles/";
#elif CONFIG_OEM
const char *iq_file_dir = "/oem/etc/iqfiles/";
#else
const char *iq_file_dir = "/etc/iqfiles/";
#endif

#define MAX_MEDIA_DEV_NUM 10

struct rkaiq_media_info {
  char sd_isp_path[RKAIQ_FILE_PATH_LEN];
  char vd_params_path[RKAIQ_FILE_PATH_LEN];
  char vd_stats_path[RKAIQ_FILE_PATH_LEN];

  struct {
    char sd_sensor_path[RKAIQ_FILE_PATH_LEN];
    char sd_lens_path[RKAIQ_FILE_PATH_LEN];
    char sd_flash_path[RKAIQ_FLASH_NUM_MAX][RKAIQ_FILE_PATH_LEN];
    bool link_enabled;
    char sensor_entity_name[32];
  } cams[RKAIQ_FLASH_NUM_MAX];
  int is_exist;
  int fix_nohdr_mode;
};

static struct rkaiq_media_info media_info[RKAIQ_CAMS_NUM_MAX];

static void errno_exit(const char *s) {
  LOG_ERROR("%s error %d, %s\n", s, errno, strerror(errno));
  exit(EXIT_FAILURE);
}

static int xioctl(int fh, int request, void *arg) {
  int r;

  do {
    r = ioctl(fh, request, arg);
  } while (-1 == r && EINTR == errno);

  return r;
}

static int save_prepare_status(int cam_id, int status) {
  char pipe_name[128];
  char data[32];
  sprintf(pipe_name, "/tmp/.ispserver_cam%d", cam_id);
  int fd = open(pipe_name, O_RDWR | O_CREAT | O_TRUNC | O_SYNC);
  if (fd > 0) {
    snprintf(data, sizeof(data), "%d", status);
    write(fd, data, strlen(data));
    close(fd);
    sync();
  } else {
    printf("save_prepare_status open err\n");
  }
}

static int rkaiq_get_devname(struct media_device *device, const char *name,
                             char *dev_name) {
  const char *devname;
  struct media_entity *entity = NULL;

  entity = media_get_entity_by_name(device, name, strlen(name));
  if (!entity) return -1;

  devname = media_entity_get_devname(entity);

  if (!devname) {
    fprintf(stderr, "can't find %s device path!", name);
    return -1;
  }

  strncpy(dev_name, devname, RKAIQ_FILE_PATH_LEN);

  LOG_INFO("get %s devname: %s\n", name, dev_name);

  return 0;
}

static int rkaiq_enumrate_modules(struct media_device *device,
                                  struct rkaiq_media_info *media_info) {
  uint32_t nents, i;
  const char *dev_name = NULL;
  int active_sensor = -1;

  nents = media_get_entities_count(device);
  for (i = 0; i < nents; ++i) {
    int module_idx = -1;
    struct media_entity *e;
    const struct media_entity_desc *ef;
    const struct media_link *link;

    e = media_get_entity(device, i);
    ef = media_entity_get_info(e);

    if (ef->type != MEDIA_ENT_T_V4L2_SUBDEV_SENSOR &&
        ef->type != MEDIA_ENT_T_V4L2_SUBDEV_FLASH &&
        ef->type != MEDIA_ENT_T_V4L2_SUBDEV_LENS)
      continue;

    if (ef->name[0] != 'm' && ef->name[3] != '_') {
      fprintf(stderr,
              "sensor/lens/flash entity name format is incorrect,"
              "pls check driver version !\n");
      return -1;
    }

    /* Retrive the sensor index from sensor name,
* which is indicated by two characters after 'm',
*     e.g.  m00_b_ov13850 1-0010
*            ^^, 00 is the module index
*/
    module_idx = atoi(ef->name + 1);
    if (module_idx >= RKAIQ_CAMS_NUM_MAX) {
      fprintf(stderr, "sensors more than two not supported, %s\n", ef->name);
      continue;
    }

    dev_name = media_entity_get_devname(e);

    switch (ef->type) {
      case MEDIA_ENT_T_V4L2_SUBDEV_SENSOR:
        strncpy(media_info->cams[module_idx].sd_sensor_path, dev_name,
                RKAIQ_FILE_PATH_LEN);

        link = media_entity_get_link(e, 0);
        if (link && (link->flags & MEDIA_LNK_FL_ENABLED)) {
          media_info->cams[module_idx].link_enabled = true;
          active_sensor = module_idx;
          strcpy(media_info->cams[module_idx].sensor_entity_name, ef->name);
        }
        break;
      case MEDIA_ENT_T_V4L2_SUBDEV_FLASH:
        // TODO, support multiple flashes attached to one module
        strncpy(media_info->cams[module_idx].sd_flash_path[0], dev_name,
                RKAIQ_FILE_PATH_LEN);
        break;
      case MEDIA_ENT_T_V4L2_SUBDEV_LENS:
        strncpy(media_info->cams[module_idx].sd_lens_path, dev_name,
                RKAIQ_FILE_PATH_LEN);
        break;
      default:
        break;
    }
  }

  if (active_sensor < 0) {
    LOG_ERROR("Not sensor link is enabled, does sensor probe correctly?\n");
    return -1;
  }

  return 0;
}

int rkaiq_get_media_info() {
  int ret = 0;
  unsigned int i, index = 0, cam_id, cam_num;
  char sys_path[64];
  int isp0_link_sensor = 0;
  int find_sensor[RKAIQ_CAMS_NUM_MAX] = {0};
  int find_isp[RKAIQ_CAMS_NUM_MAX] = {0};
  int find_ispp[RKAIQ_CAMS_NUM_MAX] = {0};
  struct media_device *device = NULL;
  while (index < RKAIQ_CAMS_NUM_MAX) {
    rk_aiq_static_info_t static_info;
    ret = rk_aiq_uapi_sysctl_enumStaticMetas(index, &static_info);
    if (ret) break;
    LOG_INFO("get sensor name %s\n", static_info.sensor_info.sensor_name);
    index++;
  }
  cam_num = index;
  LOG_INFO("get cam_num %d\n", cam_num);

  index = 0;

  while (index < MAX_MEDIA_DEV_NUM) {
    cam_id = index;
    snprintf(sys_path, 64, "/dev/media%d", index++);
    if (access(sys_path, F_OK)) continue;
    LOG_INFO("media get sys_path: %s\n", sys_path);

    device = media_device_new(sys_path);
    if (device == NULL) {
      LOG_ERROR("Failed to create media %s\n", sys_path);
      continue;
    }
    /* Enumerate entities, pads and links. */
    ret = media_device_enumerate(device);
    if (ret) {
      LOG_INFO("media_device_enumerate success");
      media_info[cam_id].is_exist = 1;
      continue;
    }
    if (!ret) {
      /* Try rkisp */
      ret = rkaiq_get_devname(device, "rkisp-isp-subdev",
                              media_info[cam_id].sd_isp_path);
      ret |= rkaiq_get_devname(device, "rkisp-input-params",
                               media_info[cam_id].vd_params_path);
      ret |= rkaiq_get_devname(device, "rkisp-statistics",
                               media_info[cam_id].vd_stats_path);
      LOG_INFO(
          "cam_id = %d, rkisp-isp-subdev = %s, rkisp-input-params = %s, "
          "rkisp-statistic = %s\n",
          cam_id, media_info[cam_id].sd_isp_path,
          media_info[cam_id].vd_params_path, media_info[cam_id].vd_stats_path);
    }
    if (ret) {
      media_info[cam_id].is_exist = 1;
      media_device_unref(device);
      continue;
    }

    ret = rkaiq_enumrate_modules(device, media_info);
    media_info[cam_id].is_exist = 1;
    media_device_unref(device);
  }

  return ret;
}

static void db_config_sync(char *hdr_mode) {
#if CONFIG_DBSERVER
  if (need_sync_db && !is_quick_start) {
    /* IMAGE_ADJUSTMENT */
    int brightness = 50;
    int contrast = 50;
    int saturation = 50;
    int sharpness = 50;
    int hue = 50;
    hash_image_adjustment_get(&brightness, &contrast, &saturation, &sharpness,
                              &hue);
    LOG_INFO(
        "brightness:%d, contrast:%d, saturation:%d, sharpness:%d, hue:%d\n\n",
        brightness, contrast, saturation, sharpness, hue);
    brightness_set(brightness);
    contrast_set(contrast);
    saturation_set(saturation);
    sharpness_set(sharpness);
    hue_set(hue);
    /* EXPOSURE */
    exposure_para_set_by_hash();
    /* BLC */
    if (0 != atoi(hdr_mode)) {
      int hdr_level = 0;
      hash_image_blc_get(NULL, &hdr_level, NULL, NULL, NULL);
      blc_hdr_level_enum_set(hdr_level);
    } else {
      blc_normal_mode_para_set_by_hash();
    }
    /* IMAGE_ENHANCEMENT */
    nr_mode_t hash_nr_mode;
    dc_mode_t hash_dc_mode;
    work_mode_2_t dehaze_mode;
    int spatial_level = 0;
    int temporal_level = 0;
    int fec_level = 0;
    int dehaze_level = 0;
    int ldch_level = 0;
    int rotation_angle = 0;
    hash_image_enhancement_get(&hash_nr_mode, &hash_dc_mode, &dehaze_mode,
                               &spatial_level, &temporal_level, &dehaze_level,
                               &ldch_level, &fec_level, &rotation_angle);
    LOG_INFO(
        "nr_mode:%d, distortion_correction_mode:%d, dehaze_mode:%d, "
        "spatial_level:%d, temporal_level:%d, dehaze_level:%d, "
        "ldch_level:%d, fec_level:%d, rotation_angle: %d\n\n",
        hash_nr_mode, hash_dc_mode, dehaze_mode, spatial_level, temporal_level,
        dehaze_level, ldch_level, fec_level, rotation_angle);

    nr_para_set(hash_nr_mode, spatial_level, temporal_level);

    /* IMAGE_ADJUSTMENT */
    expPwrLineFreq_t frequency_mode;
    flip_mode_t mirror_mode;
    hash_image_video_adjustment_get(&frequency_mode, &mirror_mode);
    LOG_INFO("frequency_mode is %d, mirror_mode is %d\n\n", frequency_mode,
             mirror_mode);
    frequency_mode_set(frequency_mode);

    dehaze_para_set(dehaze_mode, dehaze_level);
    dc_para_set(hash_dc_mode, ldch_level, fec_level);

    /* WHITE_BALANCE */
    white_balance_set_by_hash_table();

    mirror_mode_set(mirror_mode);
    /* only for fbc*/
    bypass_stream_rotation_set(rotation_angle);
    /* NIGHT_TO_DAY*/
    night_to_day_para_cap_set_db();
    night2day_loop_run();
  }
#endif
}
static void init_engine(int cam_id) {
  int index;

#if CONFIG_DBSERVER
  if (need_sync_db && !is_quick_start) {
    rk_aiq_working_mode_t hdr_mode_db = RK_AIQ_WORKING_MODE_NORMAL;
    while (hash_image_hdr_mode_get4init(&hdr_mode_db)) {
      LOG_INFO("Get data is empty, please start dbserver\n");
      sleep(1);
    }
    LOG_INFO("hdr_mode_db: %d \n", hdr_mode_db);
    switch (hdr_mode_db) {
      case RK_AIQ_WORKING_MODE_NORMAL: {
        setenv("HDR_MODE", "0", 1);
        LOG_DEBUG("set hdr normal\n");
        break;
      }
      case RK_AIQ_WORKING_MODE_ISP_HDR2: {
        setenv("HDR_MODE", "1", 1);
        break;
      }
      case RK_AIQ_WORKING_MODE_ISP_HDR3: {
        setenv("HDR_MODE", "2", 1);
        break;
      }
    }
  }
#endif

  char *hdr_mode = getenv("HDR_MODE");
  if (hdr_mode) {
    LOG_INFO("hdr mode: %s\n", hdr_mode);
    if (0 == atoi(hdr_mode))
      mode[cam_id] = RK_AIQ_WORKING_MODE_NORMAL;
    else if (1 == atoi(hdr_mode))
      mode[cam_id] = RK_AIQ_WORKING_MODE_ISP_HDR2;
    else if (2 == atoi(hdr_mode))
      mode[cam_id] = RK_AIQ_WORKING_MODE_ISP_HDR3;
  } else {
    mode[cam_id] = RK_AIQ_WORKING_MODE_NORMAL;
  }

  /* if (media_info[cam_id].fix_nohdr_mode)
    mode[cam_id] = RK_AIQ_WORKING_MODE_NORMAL;
  for (index = 0; index < RKAIQ_CAMS_NUM_MAX; index++)
    if (media_info[cam_id].cams[index].link_enabled) break; */

  aiq_ctx[cam_id] =
      rk_aiq_uapi_sysctl_init(media_info[cam_id].cams[index].sensor_entity_name,
                              iq_file_dir, NULL, NULL);

#if CONFIG_DBSERVER
  /* sync fec from db*/
  if (need_sync_db && !is_quick_start) {
    int fec_en;
    hash_image_fec_enable_get4init(&fec_en, NULL);
    int fec_ret = rk_aiq_uapi_setFecEn(aiq_ctx[cam_id], fec_en);
    LOG_INFO("set fec_en: %d, ret is %d\n", fec_en, fec_ret);
  }
  /* sync hdr mode for quick_start */
  if (!need_sync_db || is_quick_start) {
    hdr_global_value_set(mode[cam_id]);
  }
#endif
  if (rk_aiq_uapi_sysctl_prepare(
          aiq_ctx[cam_id], width, height,
          /* RK_AIQ_WORKING_MODE_NORMAL */ mode[cam_id])) {
    LOG_ERROR("rkaiq engine prepare failed !\n");
    exit(-1);
  }
  db_aiq_ctx = aiq_ctx[cam_id];
  save_prepare_status(cam_id, 1);

#if CONFIG_DBSERVER
  set_stream_on();
#endif

  if (fixfps > 0) {
#if CONFIG_DBSERVER
    isp_fix_fps_set(fixfps);
#else
    frameRateInfo_t fps_info;
    memset(&fps_info, 0, sizeof(fps_info));
    fps_info.fps = fixfps;
    fps_info.mode = OP_MANUAL;
    rk_aiq_uapi_setFrameRate(db_aiq_ctx, fps_info);
#endif
  }
  db_config_sync(hdr_mode);
}

static void start_engine(int cam_id) {
  LOG_INFO("device manager start\n");
  rk_aiq_uapi_sysctl_start(aiq_ctx[cam_id]);
  if (aiq_ctx[cam_id] == NULL) {
    LOG_ERROR("rkisp_init engine failed\n");
    exit(-1);
  } else {
#if CONFIG_DBSERVER
#if SYS_START
    send_stream_on_signal();
#endif
#endif
    LOG_INFO("rkisp_init engine succeed\n");
  }
}

static void stop_engine(int cam_id) {
  rk_aiq_uapi_sysctl_stop(aiq_ctx[cam_id], false);
}

static void deinit_engine(int cam_id) {
#if CONFIG_DBSERVER
  if (need_sync_db) {
    set_stream_off();
    night2day_loop_stop();
  }
#endif
  save_prepare_status(cam_id, 0);
  rk_aiq_uapi_sysctl_deinit(aiq_ctx[cam_id]);
}

// blocked func
static int wait_stream_event(int fd, unsigned int event_type, int time_out_ms) {
  int ret;
  struct v4l2_event event;

  CLEAR(event);

  do {
    /*
* xioctl instead of poll.
* Since poll() cannot wait for input before stream on,
* it will return an error directly. So, use ioctl to
* dequeue event and block until sucess.
*/
    ret = xioctl(fd, VIDIOC_DQEVENT, &event);
    if (ret == 0 && event.type == event_type) {
      return 0;
    }
  } while (true);

  return -1;
}

static int subscrible_stream_event(int cam_id, int fd, bool subs) {
  struct v4l2_event_subscription sub;
  int ret = 0;

  CLEAR(sub);
  sub.type = CIFISP_V4L2_EVENT_STREAM_START;
  ret = xioctl(fd, subs ? VIDIOC_SUBSCRIBE_EVENT : VIDIOC_UNSUBSCRIBE_EVENT,
               &sub);
  if (ret) {
    LOG_ERROR("can't subscribe %s start event!\n",
              media_info[cam_id].vd_params_path);
    exit(EXIT_FAILURE);
  }

  CLEAR(sub);
  sub.type = CIFISP_V4L2_EVENT_STREAM_STOP;
  ret = xioctl(fd, subs ? VIDIOC_SUBSCRIBE_EVENT : VIDIOC_UNSUBSCRIBE_EVENT,
               &sub);
  if (ret) {
    LOG_ERROR("can't subscribe %s stop event!\n",
              media_info[cam_id].vd_params_path);
  }

  LOG_INFO("subscribe events from %s success !\n",
           media_info[cam_id].vd_params_path);

  return 0;
}

void *wait_thread_func() {
  LOG_DEBUG("wait_thread_func...q: %d, db: %d\n", is_quick_start, need_sync_db);
#if CONFIG_DBSERVER
  database_hash_init();
  while (!wait_dbus_init_func()) {
    if (dbus_warn_log_status_get()) {
      dbus_warn_log_close();
    }
    LOG_DEBUG("wait for dbus init\n");
    usleep(100000);
  }
  if (need_sync_db) {
    dbus_warn_log_open();
  }
  database_init();
  manage_init();
  rk_aiq_working_mode_t hdr_mode_db = RK_AIQ_WORKING_MODE_NORMAL;
  hash_image_hdr_mode_get4init(&hdr_mode_db);
  hdr_mode_set4db(hdr_mode_db);
  switch (hdr_mode_db) {
    case RK_AIQ_WORKING_MODE_NORMAL:
      is_quick_start = false;
      db_config_sync("0");
      break;
    case RK_AIQ_WORKING_MODE_ISP_HDR2:
      is_quick_start = false;
      db_config_sync("1");
      break;
    case RK_AIQ_WORKING_MODE_ISP_HDR3:
      is_quick_start = false;
      db_config_sync("2");
      break;
    default:
      LOG_ERROR("undefine hdr mode: %d, fail to sync db config\n", hdr_mode_db);
      break;
  }
#endif
}

void *thread_func(void *arg) {
  int ret = 0;
  int isp_fd;
  unsigned int stream_event = -1;
  int cam_id = *(int *)arg;
  if (cam_id < 0) cam_id = 0;
  LOG_INFO("thread_func cam_id %d...\n", cam_id);

  setlinebuf(stdout);
#if CONFIG_DBSERVER
  if (!is_quick_start && (need_sync_db || wait_dbus_init_func())) {
    LOG_INFO("init for db\n");
    database_hash_init();
    database_init();
    manage_init();
  }
#endif
  for (;;) {
    /* Refresh media info so that sensor link status updated */

    isp_fd = open(media_info[cam_id].vd_params_path, O_RDWR);
    if (isp_fd < 0) {
      LOG_ERROR("open %s failed %s\n", media_info[cam_id].vd_params_path,
                strerror(errno));
      pthread_exit(-1);
    }
    subscrible_stream_event(cam_id, isp_fd, true);
    init_engine(cam_id);

    while (1) {
      LOG_INFO("wait stream start event...\n");
      wait_stream_event(isp_fd, CIFISP_V4L2_EVENT_STREAM_START, -1);
      LOG_INFO("wait stream start event success ...\n");
      rk_aiq_state_t aiq_state = rk_aiq_get_state();
      LOG_INFO("state=%d\n", aiq_state);
      if (aiq_state == AIQ_STATE_INVALID) {
        LOG_INFO("start engine...\n");
        start_engine(cam_id);
      }

      LOG_INFO("wait stream stop event...\n");
      wait_stream_event(isp_fd, CIFISP_V4L2_EVENT_STREAM_STOP, -1);
      LOG_INFO("wait stream stop event success ...\n");
      if (aiq_state == AIQ_STATE_INVALID) {
        LOG_INFO("stop engine...\n");
        stop_engine(cam_id);
      }

#if CONFIG_DBSERVER
      if (!check_stream_status()) break;
#endif
    }

    deinit_engine(cam_id);
    subscrible_stream_event(cam_id, isp_fd, false);
    close(isp_fd);
    usleep(100 * 1000);
    LOG_INFO("----------------------------------------------\n\n");
  }
}

static void main_exit(void) {
  LOG_INFO("server %s\n", __func__);
  for (int id = 0; id < RKAIQ_CAMS_NUM_MAX; id++) {
    if (aiq_ctx[id]) {
      LOG_INFO("stop engine camera index %d...\n", id);
      stop_engine(id);
      LOG_INFO("deinit engine camera index %d...\n", id);
      deinit_engine(id);
      aiq_ctx[id] = NULL;
    }
  }
#if CONFIG_DBUS
#if CONFIG_CALLFUNC
  LOG_INFO("deinit call_fun_ipc_server...\n");
  call_fun_ipc_server_deinit();
#endif
#endif
}

void signal_crash_handler(int sig) {
#if CONFIG_DBUS
#if CONFIG_CALLFUNC
  call_fun_ipc_server_deinit();
#endif
#endif
  exit(-1);
}

void signal_exit_handler(int sig) {
#if CONFIG_DBUS
#if CONFIG_CALLFUNC
  call_fun_ipc_server_deinit();
#endif
#endif
  exit(0);
}

static const char short_options[] = "nf:";
static const struct option long_options[] = {
    {"nsd", no_argument, NULL, 'n'},
    {"fps", required_argument, NULL, 'f'},
    {0, 0, 0, 0}};
void parse_args(int argc, char **argv) {
  for (;;) {
    int idx;
    int c;
    c = getopt_long(argc, argv, short_options, long_options, &idx);
    if (-1 == c) break;
    switch (c) {
      case 0: /* getopt_long() flag */
        break;
      case 'f':
        fixfps = atoi(optarg);
        LOG_INFO("## fixfps: %d\n", fixfps);
        break;
      case 'n':
        need_sync_db = false;
        LOG_INFO("## need_sync_db: %d\n", need_sync_db);
        break;
      default:
        break;
    }
  }
}

int main(int argc, char **argv) {
#ifdef ENABLE_MINILOGGER
  enable_minilog = 1;
  __minilog_log_init(argv[0], NULL, false, false, "ispserver", "1.0.0");
#endif

  parse_args(argc, argv);
  argv += 1;
  if (*argv) {
    if (strcmp(*argv, "-no-sync-db") == 0) need_sync_db = false;
  }
  pthread_t tidp[RKAIQ_CAMS_NUM_MAX];
  memset(media_info, 0, sizeof(media_info));
  if (rkaiq_get_media_info()) errno_exit("Bad media topology...\n");

  int cam_num = 0;
  for (int id = 0; id < RKAIQ_CAMS_NUM_MAX; id++) {
    int camid[RKAIQ_CAMS_NUM_MAX];
    camid[id] = id;
    if (media_info[id].is_exist) {
      cam_num++;
    }
  }

#if CONFIG_DBSERVER
  if (!need_sync_db) dbus_warn_log_close();

  if (wait_dbus_init_func()) {
    LOG_INFO("dbus init complete, alter quick start false\n");
    is_quick_start = false;
  }
#endif
  for (int id = 0; id < RKAIQ_CAMS_NUM_MAX; id++) {
    int camid[RKAIQ_CAMS_NUM_MAX];
    camid[id] = id;
    if (media_info[id].is_exist) {
      LOG_INFO("enter wait thread for cam index %d\n", id);
      if (pthread_create(&tidp[camid[id]], NULL, thread_func, &camid[id]) !=
          0) {
        LOG_INFO("enter wait thread for cam index %d error\n", id);
      }
    } else {
      LOG_INFO("is_exist = %d\n", media_info[id].is_exist);
    }
  }

#if CONFIG_DBSERVER
  LOG_DEBUG("before enter wait thread q: %d, db: %d\n", is_quick_start,
            need_sync_db);
  if (is_quick_start || !need_sync_db) {
    pthread_t wait_db_p;
    if (pthread_create(&wait_db_p, NULL, wait_thread_func, NULL) != 0) {
      LOG_INFO("enter wait thread for db error\n");
    }
  }
#endif

#if CONFIG_DBUS
  pthread_detach(pthread_self());
  GMainLoop *main_loop;
  atexit(main_exit);
  signal(SIGTERM, signal_exit_handler);
  signal(SIGINT, signal_exit_handler);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGBUS, signal_crash_handler);
  signal(SIGSEGV, signal_crash_handler);
  signal(SIGFPE, signal_crash_handler);
  signal(SIGABRT, signal_crash_handler);

#if CONFIG_CALLFUNC
  call_fun_ipc_server_init(map, sizeof(map) / sizeof(struct FunMap), DBUS_NAME,
                           DBUS_IF, DBUS_PATH, 0);
  LOG_INFO("call_fun_ipc_demo_server init\n");
#endif

  main_loop = g_main_loop_new(NULL, FALSE);

  g_main_loop_run(main_loop);
  if (main_loop) g_main_loop_unref(main_loop);
#else
  void *ret;
  for (int id = 0; id < RKAIQ_CAMS_NUM_MAX; id++) {
    if (media_info[id].is_exist) {
      pthread_join(tidp[id], &ret);
    }
  }
#endif

  return 0;
}
