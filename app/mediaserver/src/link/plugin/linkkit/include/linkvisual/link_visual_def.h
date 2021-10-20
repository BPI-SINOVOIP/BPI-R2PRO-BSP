#ifndef LINK_VISUAL_DEF_H_
#define LINK_VISUAL_DEF_H_

#include <unistd.h>
#include <stdint.h>

typedef enum {
    LV_STORAGE_RECORD_PLAN = 0, //计划录像
    LV_STORAGE_RECORD_ALARM = 1, //报警录像
    LV_STORAGE_RECORD_INITIATIVE = 2, // 主动录像
    LV_STORAGE_RECORD_ANY = 99,       //所有类型。
} lv_storage_record_type_e; //如果不在这个范围内，则为用户自定义录像类型，由APP和设备侧自主协商含义，SDK不再明确列出具体类型.

/* 流可变参数  */
typedef struct {
    //当LV_STREAM_CMD_LIVE有效
    int stream_type;//主、子码流等

    //当LV_STREAM_CMD_PRE_EVENT_RECORD有效
    int pre_time;//预录事件录像的预录时间

    //当LV_STREAM_CMD_STORAGE_RECORD_BY_FILE有效，
    // 使用lv_stream_send_video 和lv_stream_send_audio 接口时，参数timestamp_ms 应该是timestamp_base*1000+ 文件的毫秒数（从0开始的相对时间）
    char file_name[256];//要点播的文件名
    int timestamp_offset;//文件的时间偏移值（相对于文件头的相对偏移时间），单位：s
    int timestamp_base;  //基准时间戳，单位：s
    unsigned int speed;//倍速信息
    unsigned int key_only;//倍速信息

    //当LV_STREAM_CMD_STORAGE_RECORD_BY_UTC_TIME有效
    unsigned int start_time;//播放当天0点的UTC时间,单位：s
    unsigned int stop_time;//播放当天24点的UTC时间,单位：s
    unsigned int seek_time;//播放的UTC时间相对于start_time的相对时间，即 seek_time + start_time = 播放的utc时间,单位：s
    lv_storage_record_type_e record_type;//录像类型
} lv_stream_param_s;

typedef enum {
    LV_STREAM_CMD_LIVE = 0, //直播
    //点播类型中，不建议使用LV_STREAM_CMD_STORAGE_RECORD_BY_FILE，推荐使用LV_STREAM_CMD_STORAGE_RECORD_BY_UTC_TIME；
    LV_STREAM_CMD_STORAGE_RECORD_BY_FILE = 1,//按文件名播放设备存储录像
    LV_STREAM_CMD_STORAGE_RECORD_BY_UTC_TIME = 2,//按UTC时间播放设备存储录像
    LV_STREAM_CMD_PRE_EVENT_RECORD = 3,//事件录像（预录）
    LV_STREAM_CMD_MAX,
} lv_stream_type_e;

typedef enum {
    LV_STORAGE_RECORD_START = 0,//开始播放，对于录像点播有效
    LV_STORAGE_RECORD_PAUSE,//暂停，对于录像点播有效
    LV_STORAGE_RECORD_UNPAUSE,// 继续播放，对于录像点播有效
    LV_STORAGE_RECORD_SEEK,// 定位，对于录像点播有效
    LV_STORAGE_RECORD_STOP,//停止，对于录像点播有效
    LV_STORAGE_RECORD_SET_PARAM,//设置点播倍速等参数信息
    LV_LIVE_REQUEST_I_FRAME,//强制编码I帧，对于直播有效
} lv_on_push_streaming_cmd_type_e;

/* 视频格式  */
typedef enum {
    // 编码类型切换后需保证首帧为I帧
    LV_VIDEO_FORMAT_H264 = 0, //AVC
    LV_VIDEO_FORMAT_H265 = 1, //HEVC
} lv_video_format_e;

/* 音频格式  */
typedef enum {
    //不支持同一设备切换音频编码类型，不支持切换编码参数
    LV_AUDIO_FORMAT_PCM = 0,//不支持，请转换为G711A
    LV_AUDIO_FORMAT_G711A = 1,
    LV_AUDIO_FORMAT_MP3 = 2,
    LV_AUDIO_FORMAT_G711U = 3,
} lv_audio_format_e;

/* 音频采样率  */
typedef enum {
    LV_AUDIO_SAMPLE_RATE_96000 = 0,
    LV_AUDIO_SAMPLE_RATE_88200 = 1,
    LV_AUDIO_SAMPLE_RATE_64000 = 2,
    LV_AUDIO_SAMPLE_RATE_48000 = 3,
    LV_AUDIO_SAMPLE_RATE_44100 = 4,
    LV_AUDIO_SAMPLE_RATE_32000 = 5,
    LV_AUDIO_SAMPLE_RATE_24000 = 6,
    LV_AUDIO_SAMPLE_RATE_22050 = 7,
    LV_AUDIO_SAMPLE_RATE_16000 = 8,
    LV_AUDIO_SAMPLE_RATE_12000 = 9,
    LV_AUDIO_SAMPLE_RATE_11025 = 10,
    LV_AUDIO_SAMPLE_RATE_8000 = 11,
    LV_AUDIO_SAMPLE_RATE_7350 = 12,
} lv_audio_sample_rate_e;

/* 音频位宽  */
typedef enum {
    LV_AUDIO_SAMPLE_BITS_8BIT  = 0,
    LV_AUDIO_SAMPLE_BITS_16BIT = 1,
} lv_audio_sample_bits_e;

/* 音频声道  */
typedef enum {
    LV_AUDIO_CHANNEL_MONO = 0,
    LV_AUDIO_CHANNEL_STEREO = 1,
} lv_audio_channel_e;

/* 视频参数结构体 */
typedef struct lv_video_param_s {
    lv_video_format_e format; //视频编码格式
    unsigned int fps;  //帧率
    unsigned int bitrate_kbps;      //波特率
    unsigned int key_frame_interval_ms;//最大关键帧间隔时间，单位：毫秒
} lv_video_param_s;


/* 音频参数结构体 */
typedef struct lv_audio_param_s {
    lv_audio_format_e format;
    lv_audio_sample_rate_e sample_rate;
    lv_audio_sample_bits_e sample_bits;
    lv_audio_channel_e channel;
} lv_audio_param_s;

/* 媒体封装或编码类型 */
typedef enum {
    LV_MEDIA_JPEG = 0,
    LV_MEDIA_PNG,
} lv_media_format;

/* 事件类型 */
typedef enum {
    /* 普通事件 */
    LV_TRIGGER_PIC = 0, //远程触发设备被动抓图上报
    LV_EVENT_MOVEMENT = 1, //移动侦测
    LV_EVENT_SOUND = 2, //声音侦测
    LV_EVENT_HUMAN = 3,  //人形侦测
    LV_EVENT_PET = 4, //宠物侦测
    LV_EVENT_CROSS_LINE = 5, //越界侦测
    LV_EVENT_REGIONAL_INVASION = 6, //区域入侵侦测
    LV_EVENT_FALL = 7, //跌倒检测
    LV_EVENT_FACE = 8, //人脸检测
    LV_EVENT_SMILING = 9, //笑脸检测
    LV_EVENT_ABNORMAL_SOUND = 10, //异响侦测
    LV_EVENT_CRY = 11, //哭声侦测
    LV_EVENT_LAUGH = 12, //笑声侦测
    /* AI事件 */
    LV_AI_EVENT_TYPE_ILLEGAL_PARKING = 10001,       //违章停车
    LV_AI_EVENT_TYPE_ROAD_BIZ,                      //占道经营
    LV_AI_EVENT_TYPE_REG_MOTORBIKE,                 //摩托车识别
    LV_AI_EVENT_TYPE_REG_PEDESTRIAN,                //行人识别
    LV_AI_EVENT_TYPE_REG_VEHICLE,                   //车辆识别
    LV_AI_EVENT_TYPE_STORE_BIZ,                     //店外经营
    LV_AI_EVENT_TYPE_REG_FACE,                      //人脸识别
    LV_AI_EVENT_TYPE_DETECT_FACE,                   //人脸检测
    LV_AI_EVENT_TYPE_DETECT_FACE_VEHICLE,           //人车检测
    LV_AI_EVENT_TYPE_DETECT_AREA,                   //区域入侵
    LV_AI_EVENT_TYPE_DETECT_CLIMB, //攀高检测
    LV_AI_EVENT_TYPE_DETECT_UP, //起身检测
    LV_AI_EVENT_TYPE_DETECT_OFF_POST, //离岗检测
    LV_AI_EVENT_TYPE_THINGS_LOST, //物品遗留
    LV_AI_EVENT_TYPE_FALL_DOWN, //跌倒检测
    LV_AI_EVENT_TYPE_ILLEGAL_BIKE, //非机动车乱停
    LV_AI_EVENT_TYPE_RUBBISH, //垃圾暴露
    LV_AI_EVENT_TYPE_HANG_OUT, //沿街晾挂
    LV_AI_EVENT_TYPE_DETECT_FIRE, //火灾检测
    LV_AI_EVENT_TYPE_FIRE_ACCESS, //消防通道占用
    LV_AI_EVENT_TYPE_IPC_SHELTER, //摄像头遮挡检测
    LV_AI_EVENT_TYPE_IPC_MOVE, //摄像头移动检测
    LV_AI_EVENT_TYPE_KEY_AREA, //重点区域占用
    LV_AI_EVENT_TYPE_CHILD_LOST, //小孩防走失
    LV_AI_EVENT_TYPE_DETECT_SMOKE, //吸烟检测
    LV_AI_EVENT_TYPE_PASSENGER_STATISTICC = 14001, //客流统计
    LV_EVENT_MAX
} lv_event_type_e;

/* 智能事件类型 */
typedef enum {
    LV_INTELLIGENT_EVENT_MOVING_CHECK = 1,//移动侦测
    LV_INTELLIGENT_EVENT_SOUND_CHECK = 2,//声音侦测
    LV_INTELLIGENT_EVENT_HUMAN_CHECK = 3,//人形侦测
    LV_INTELLIGENT_EVENT_PET_CHECK = 4,//宠物侦测
    LV_INTELLIGENT_EVENT_CROSS_LINE_CHECK = 5,//越界侦测
    LV_INTELLIGENT_EVENT_REGIONAL_INVASION = 6,//区域入侵侦测
    LV_INTELLIGENT_EVENT_FALL_CHECK = 7,//跌倒检测
    LV_INTELLIGENT_EVENT_FACE_CHECK = 8,//人脸检测
    LV_INTELLIGENT_EVENT_SMILING_CHECK = 9,//笑脸检测
    LV_INTELLIGENT_EVENT_ABNORMAL_SOUND_CHECK = 10,//异响侦测
    LV_INTELLIGENT_EVENT_CRY_CHECK = 11,//哭声侦测
    LV_INTELLIGENT_EVENT_LAUGH_CHECK = 12,//笑声侦测
    LV_INTELLIGENT_EVENT_ILLEGAL_PARKING = 10001,//违章停车
    LV_INTELLIGENT_EVENT_ILLEGAL_SALE = 10002,//占道经营
    LV_INTELLIGENT_EVENT_MOTORCYCLE_RECOGNITION = 10003,//摩托车识别
    LV_INTELLIGENT_EVENT_PEDESTRIAN_RECOGNITION = 10004,//行人识别
    LV_INTELLIGENT_EVENT_VEHICLES_RECOGNITION = 10005,//车辆识别
    LV_INTELLIGENT_EVENT_DELIVER_SALE = 10006,//到店经营
    LV_INTELLIGENT_EVENT_FACE_RECOGNITION = 10007,//人脸识别
    LV_INTELLIGENT_EVENT_FACE_DETECT = 10008,//人脸检测
    LV_INTELLIGENT_EVENT_PERSON_VEHICLE_DETECTION = 10009,//人车检测
    LV_INTELLIGENT_EVENT_IPC_OCCLUSION_DETECTION = 10010,//摄像头遮挡检测
    LV_INTELLIGENT_EVENT_IPC_MOVE_DETECTION = 10011,//摄像头移动检测
    LV_INTELLIGENT_EVENT_KEY_AREA_OCCUPY = 10012,//重点区域占用
    LV_INTELLIGENT_EVENT_REGIONAL_INVASION_GW = 11001,//区域入侵
    LV_INTELLIGENT_EVENT_CLIMBING_DETECT = 11002,//攀高检测
    LV_INTELLIGENT_EVENT_ARISE_DETECT = 11003,//起身检测
    LV_INTELLIGENT_EVENT_ABSENT_DETECT = 11004,//离岗检测
    LV_INTELLIGENT_EVENT_LOITERING_DETECT = 11005,//人员逗留检测
    LV_INTELLIGENT_EVENT_CROSS_LINE_DETECT = 11006,//拌线检测
    LV_INTELLIGENT_EVENT_RETROGRADE_DETECT = 11007,//逆行检测
    LV_INTELLIGENT_EVENT_QUICKLY_MOVING = 11008,//快速移动
    LV_INTELLIGENT_EVENT_GOODS_MOVED = 11009,//物品移动
    LV_INTELLIGENT_EVENT_GOODS_LEFT = 11010,//物品遗留
    LV_INTELLIGENT_EVENT_CROWD_DENSITY = 11011,//人群密度估计
    LV_INTELLIGENT_EVENT_CROWD_GATHERED = 11012,//人群聚集
    LV_INTELLIGENT_EVENT_CROWD_DISPERSED = 11013,//人群发散
    LV_INTELLIGENT_EVENT_STRENUOUS_EXERCISE = 11014,//剧烈运动
    LV_INTELLIGENT_EVENT_FALL_DETECT = 11015,//跌倒检测
    LV_INTELLIGENT_EVENT_KID_TRACK = 11016,//小孩防走失
    LV_INTELLIGENT_EVENT_MASK_DETECT = 11017,//口罩识别
    LV_INTELLIGENT_EVENT_PET_DETECT = 11018,//宠物检测
    LV_INTELLIGENT_EVENT_ILLEGAL_NON_MOTOR_VEHICLE_PARKING = 12001,//非机动车乱停
    LV_INTELLIGENT_EVENT_GARBAGE_EXPOSURE = 12002,//垃圾暴露
    LV_INTELLIGENT_EVENT_HANGING_ALONG_THE_STREET = 12003,//沿街晾挂
    LV_INTELLIGENT_EVENT_FIRE_DETECT = 13001,//火灾检测
    LV_INTELLIGENT_EVENT_FIRE_CHANNEL_OCCUPANCY = 13002,//消防通道占用
    LV_INTELLIGENT_EVENT_SMOKE_DETECT = 13003,//吸烟检测
    LV_INTELLIGENT_EVENT_PASSENGER_FLOW = 14001,//客流统计
} lv_intelligent_event_type_e;

/* 云端事件类型 */
typedef enum {
    LV_CLOUD_EVENT_MASK = 0,//检测口罩
    LV_CLOUD_EVENT_DOWNLOAD_FILE = 1,//下载文件
    LV_CLOUD_EVENT_UPLOAD_FILE = 2,//下载文件
} lv_cloud_event_type_e;

/* SDK日志等级 */
typedef enum {
    LV_LOG_ERROR = 2,
    LV_LOG_WARN = 3,
    LV_LOG_INFO = 4,
    LV_LOG_DEBUG = 5,
    LV_LOG_VERBOSE = 6,
    LV_LOG_MAX = 7,
} lv_log_level_e;

/* SDK日志输出定向*/
typedef enum {
    LV_LOG_DESTINATION_FILE,//写文件，未实现；需写文件请使用 LV_LOG_DESTINATION_USER_DEFINE
    LV_LOG_DESTINATION_STDOUT,//直接向stdout输出日志
    LV_LOG_DESTINATION_USER_DEFINE,//将日志消息放入回调函数 lv_log_cb 中。可在回调函数中实现写文件功能。
    LV_LOG_DESTINATION_MAX
} lv_log_destination;

/* SDK的函数返回值枚举量 */
typedef enum {
    LV_WARN_BUF_FULL = 1,
    LV_ERROR_NONE = 0,
    LV_ERROR_DEFAULT = -1,
    LV_ERROR_ILLEGAL_INPUT = -2,
} lv_error_e;

typedef enum {
    LV_REMOTE_FILE_OTHERS = 0,
    LV_REMOTE_FILE_VOICE_RECORD = 1,
    LV_REMOTE_FILE_FACE_PICTURE = 2,
} lv_remote_file_type_e ;

/* 录像查询列表结构体 */
typedef struct {
    long start_time;                     // 录像开始时间，UTC时间，单位为秒
    long stop_time;                      // 录像结束时间，UTC时间，单位为秒
    int file_size;                         // 录像的文件大小，单位字节
    char file_name[64 + 1];                // 录像的文件名
    lv_storage_record_type_e record_type;  //录像类型
} lv_query_storage_record_response_s;

/* linkkit适配器内容 */
typedef struct {
    int dev_id;
    const char *msg_id;
    int msg_id_len;
    const char *service_name;
    int service_name_len;
    const char *request;
    int request_len;
} lv_linkkit_adapter_property_s;

/* linkkit适配器类型 */
typedef enum {
    LV_LINKKIT_ADAPTER_TYPE_TSL_PROPERTY = 0, //物模型属性消息
    LV_LINKKIT_ADAPTER_TYPE_TSL_SERVICE, //物模型服务消息
    LV_LINKKIT_ADAPTER_TYPE_LINK_VISUAL, // LinkVisual自定义消息
    LV_LINKKIT_ADAPTER_TYPE_CONNECTED, //上线信息
} lv_linkkit_adapter_type_s;

/* SDK控制类型 */
typedef enum {
    LV_CONTROL_LOG_LEVEL = 0,
    LV_CONTROL_STREAM_AUTO_CHECK,
    LV_CONTROL_STREAM_AUTO_SAVE,
} lv_control_type_e;

typedef struct {
    struct {
        unsigned int timestamp;
    } seek;
    struct {
        unsigned int speed;
        unsigned int key_only;
    } set_param;
} lv_on_push_streaming_cmd_param_s;

typedef struct {
    /* 口罩识别事件 */
    struct {
        void *reserved;
    } mask;
    /* 文件下载事件 */
    struct {
        lv_remote_file_type_e file_type;
        const char *file_name;
        struct {
            const char *url;
        };
        struct {
            const char *buf;
        };//buf形式未实现
    } file_download;
    struct {
        lv_remote_file_type_e file_type;
        const char *file_name;
        struct {
            const char *url;
        };
        struct {
            char *id;
            void (*buf_upload)(const char *id, const char *buf, unsigned int buf_len);
        };//buf形式未实现
    } file_upload;
} lv_cloud_event_param_s ;

/* 智能事件参数集 */
typedef struct {
    lv_intelligent_event_type_e type;//智能事件类型
    lv_media_format format;//智能事件的媒体数据类型，当前只支持图片类
    struct {
        char *p;
        uint32_t len;
    } media;//智能事件的媒体数据,不大于1MB（大于时会返回错误），为空时仅上传其他信息
    struct {
        char *p;
        uint32_t len;
    } addition_string;//智能事件的附加字符串信息，不大于2048B(大于时会截断），为空时仅上传其他信息
} lv_intelligent_alarm_param_s;

#endif // LINK_VISUAL_DEF_H_
