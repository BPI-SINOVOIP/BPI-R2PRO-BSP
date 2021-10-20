#ifndef _LINK_VISUAL_IPC_H
#define _LINK_VISUAL_IPC_H

#include "link_visual_def.h"

#ifdef __cplusplus
extern "C" {
#endif
  
/*--------------------——-- SDK业务相关的消息通知 ----------------------------*/
/**
 * @brief 通知设备开始推流
 * 
 * @param [IN] service_id: 推流服务ID 
 * @param [IN] type: 推流服务的类型
 * @param [IN] param: 可变参数，根据type会有不同
 *                    type == LV_STREAM_CMD_LIVE,param->pre_time为预录时间，单位：s
 *                    type == LV_STREAM_CMD_STORAGE_RECORD_BY_FILE,
 *                    param->file_name/param->timestamp_offset/param->timestamp_base为请求播放的录像参数
 *
 * @retval < 0 : Fail.
 * @retval  0 : Success.
 * 
 * @notice: type == LV_STREAM_CMD_LIVE时，需要调用lv_stream_send_video()/lv_stream_send_audio()直接开始推流；
 *          type == LV_STREAM_CMD_STORAGE_RECORD_BY_FILE/LV_STREAM_CMD_STORAGE_RECORD_BY_UTC_TIME时，
 *          需要等待lv_on_push_streaming_cmd_cb()中的消息通知;当lv_on_push_streaming_cmd_cb()中通知LV_STORAGE_RECORD_START时，
 *          调用lv_stream_send_video()/lv_stream_send_audio()开始推流
 * @see lv_stream_start_service() lv_on_push_streaming_cmd_cb()
 */
typedef int (*lv_start_push_streaming_cb)(int service_id, lv_stream_type_e type, const lv_stream_param_s *param);
   
/**
 * @brief 通知设备停止推流
 * 
 * @param [IN] service_id: 推流服务ID
 * 
 * @retval < 0 : Fail.
 * @retval  0 : Success.
 */
typedef int (*lv_stop_push_streaming_cb)(int service_id);

/**
 * @brief 推送直播/存储录像流的过程中，需要支持的命令
 * 
 * @param [IN] service_id: 推流服务ID 
 * @param [IN] cmd: 命令类型 
 * @param [IN] timestamp_ms: 时间戳参数，单位为毫秒
 *                        cmd == LV_STORAGE_RECORD_SEEK时可用，其他命令无意义
 * 
 * @retval < 0 : Fail.
 * @retval  0 : Success.
 */
typedef int (*lv_on_push_streaming_cmd_cb)(int service_id, lv_on_push_streaming_cmd_type_e cmd, const lv_on_push_streaming_cmd_param_s *param);

/**
 * @brief 通知设备开始语音对讲服务
 * 
 * @param [IN] service_id: 语音对讲服务ID 
 * @param [IN] audio_param:  设备接收到的音频数据的参数
 * 
 * @retval < 0 : Fail.
 * @retval  0 : Success.
 */
typedef int (*lv_start_voice_intercom_cb)(int service_id);

/**
 * @brief 通知设备停止语音对讲服务
 * 
 * @param [IN] service_id: 语音对讲服务ID 
 * 
 * @retval < 0 : Fail.
 * @retval  0 : Success.
 */
typedef int (*lv_stop_voice_intercom_cb)(int service_id);

/**
 * @brief 语音对讲服务，设备端接收的语音数据的格式
 * 
 * @param [IN] service_id: 语音对讲服务ID 
 * @param [IN] audio_param: 设备接收的语音数据的格式
 * 
 * @retval < 0 : Fail.
 * @retval  0 : Success.
 */
typedef int (*lv_voice_intercom_receive_metadata_cb)(int service_id, const lv_audio_param_s *audio_param);

/**  
 * @brief 接收到语音数据
 * 
 * @param [IN] buffer: 语音数据指针
 * @param [IN] buffer_len: 语音数据大小
 * 
 * @return void
 */
typedef void (*lv_voice_intercom_receive_data_cb)(const char *buffer, unsigned int buffer_len);

/**
 * @brief 触发设备抓图
 * 
 * @param [IN] id: 触发ID，需回传 
 * 
 * @retval < 0 : Fail.
 * @retval  0 : Success.
 * @see lv_post_alarm_image()
 */
typedef int (*lv_trigger_pic_capture_cb)(const char *id);

/**
 * @brief 触发设备录像
 * 
 * @param [IN] type: 录像类型 
 * @param [IN] duration: 录像时间
 * @param [IN] pre_duration: 预录时间
 * 
 * @retval < 0 : Fail.
 * @retval  0 : Success.
 */
typedef int (*lv_trigger_record_cb)(int type, int duration, int pre_duration);

/**
 * @brief 查询存储录像列表
 * 
 * @param [IN] start_time  : 查询的开始时间，UTC时间，秒数
 * @param [IN] stop_time   : 查询的结束时间，UTC时间，秒数
 * @param [IN] num         : 录像查询的数量 小于等于0的时候 请求时间范围内的全部录像
 * @param [IN] id          : 该次查询的会话标识符，需要回传到on_complete
 * @param [IN] on_complete : 返回录像列表的函数，目前仅支持同步调用，否则会超时；
 *                          num返回实际所查询到的数量，id回传，lv_query_storage_record_response_s里放置录像列表信息。
 * 
 * @return void
 */
typedef void (*lv_query_storage_record_cb)(unsigned int start_time,
                                          unsigned int stop_time,
                                          int num,
                                          lv_storage_record_type_e type,
                                          const char *id,
                                          int (*on_complete)(int num, const char *id,
                                              const lv_query_storage_record_response_s *response));

/**
 * @brief 按月查询存储录像列表
 *
 * @param [IN] month  : 查询的年月份，如201806
 * @param [IN] id  : 该次查询的会话标识符，需要回传到on_complete
 * @param [IN] on_complete : 返回月份列表的函数，目前仅支持同步调用，否则会超时；
 *                          response为int数组，长度为31.int值为0表示无录像，非0表示有录像,当前月份不足31天部分也置为0（如2月30）
 *
 * @return void
 */
typedef void (*lv_query_storage_record_by_month_cb)(const char* month, const char *id,
                                                int (*on_complete)(const char *id, const int *response));

/**
 * @brief 云端事件通知
 *
 * @param [IN] type: 事件类型
 * @param [IN] param: 事件附加参数
 *
 * @retval < 0 : Fail.
 * @retval  0 : Success.
 */
typedef int (*lv_cloud_event_cb)(lv_cloud_event_type_e type, const lv_cloud_event_param_s *param);

/**
 * @brief 日志信息回调
 *
 * @param [IN] level  : 日志级别
 * @param [IN] file_name  : 日志的源文件名称
 * @param [IN] line  : 日志的源文件行数
 * @param [IN] fmt  : 日志打印可变参数
 *
 * @return void
 */
typedef void (*lv_log_cb)(lv_log_level_e level, const char *file_name, int line, const char *fmt, ...);

/*-------------------------SDK配置相关内容----------------------------*/
/* 产品三元组长度限制 */
#define PRODUCT_KEY_LEN     (20)
#define DEVICE_NAME_LEN     (32)
#define DEVICE_SECRET_LEN   (64)
#define PRODUCT_SECRET_LEN  (64)

#define PATH_NAME_LEN       (32)
/* 配置参数结构体 */
typedef struct lv_config_s {
    /* 三元组信息 */
    char product_key[PRODUCT_KEY_LEN + 1];
    char device_name[DEVICE_NAME_LEN + 1];
    char device_secret[DEVICE_SECRET_LEN + 1];

    /* SDK的日志配置 */
    lv_log_level_e log_level;
    lv_log_destination log_dest;
    lv_log_cb log_cb;

    /* 点播的模式,只能设置为LV_STREAM_CMD_STORAGE_RECORD_BY_FILE或LV_STREAM_CMD_STORAGE_RECORD_BY_UTC_TIME */
    lv_stream_type_e storage_record_mode;

    /* 码流数量配置 */
    /* 直播源目前支持主、子码流共2路，多客户端观看同一类型的码流时时，客户端观看的码流是一致的。
     * 若码流通过云端转发，云端会进行分发，分发无路数限制；
     * 若通过P2P分发，live_p2p_num为设备可支持的最大P2P路数
     * live_p2p_num值范围为1~8，默认值为4 */
    unsigned int live_p2p_num;
    /* 点播源在多客户端观看时，无论是云端转发还是P2P，客户端观看的码流都是独立的，不会进行分发
     * storage_record_source_num为支持的点播最大路数
     * 值范围1~8，默认值为1 */
    unsigned int storage_record_source_num;

    /* 码流自检查功能，能帮助开发者发现码流本身问题,调试过程中请打开。
     * 0 - 关闭， >0 - 打开 */
    unsigned int stream_auto_check;
    /* 码流数据自动保存功能，需要排查码流兼容性等问题才需要打开,在stream_auto_check打开后可使用
     * 0 - 关闭， >0 - 打开 */
    unsigned int stream_auto_save;
    /* 码流数据自动保存的路径，路径需保证存在且可写，路径名末尾需要含有"/"，如 "/tmp/" */
    char stream_auto_save_path[PATH_NAME_LEN + 1];

    /* 设备取证服务功能（Device Attestation Service, 缩写为das)，0-开启，1-关闭 */
    unsigned int das_close;

    /* 设备支持带预录的事件录像功能，0-不支持，1-支持（若未实现 LV_STREAM_CMD_PRE_EVENT_RECORD ，请设置为不支持） */
    unsigned int pre_event_record_support;

    /* 音视频推流服务 */
    lv_start_push_streaming_cb start_push_streaming_cb;
    lv_stop_push_streaming_cb stop_push_streaming_cb;
    lv_on_push_streaming_cmd_cb on_push_streaming_cmd_cb;

    /* 语音对讲服务 */
    lv_start_voice_intercom_cb start_voice_intercom_cb;
    lv_stop_voice_intercom_cb stop_voice_intercom_cb;
    lv_voice_intercom_receive_metadata_cb voice_intercom_receive_metadata_cb;
    lv_voice_intercom_receive_data_cb voice_intercom_receive_data_cb;

    /* 存储录像查询命令 */
    lv_query_storage_record_cb query_storage_record_cb;
    lv_query_storage_record_by_month_cb query_storage_record_by_month_cb;

    /* 触发设备抓图 */
    lv_trigger_pic_capture_cb trigger_pic_capture_cb;

    /* 触发设备录像 */
    lv_trigger_record_cb trigger_record_cb;

    /* 云端事件通知 */
    lv_cloud_event_cb cloud_event_cb;

} lv_config_s;

/*------------------------- SDK功能接口 ----------------------------*/
/**
 * @brief SDK初始化
 * 
 * @param [IN] config: SDK配置参数集合 
 * 
 * @return lv_error_e
 */
int lv_init(const lv_config_s *config);

/**
 * @brief SDK销毁
 * 
 * @param [IN] void
 * 
 * @return lv_error_e
 */
int lv_destroy(void);

/**
 * @brief linkkit适配器，将物模型中的服务类消息注入该函数中，由LinkVisual代为处理
 *
 * @param [IN] lv_linkkit_adapter_type_s: 消息类型入参
 * @param [IN] lv_linkkit_adapter_property_s: 消息内容入参
 *
 * @return lv_error_e
 */
int lv_ipc_linkkit_adapter(lv_linkkit_adapter_type_s type, const lv_linkkit_adapter_property_s *in);

/**
 * @brief 在发送实际视音频数据前发送视音频相关配置,用于直播推流和存储录像播放
 * 
 * @param [IN] service_id: 服务ID，来自回调 lv_start_push_streaming_cb
 * @param [IN] bitrate_kbps: 目标码率，单位为kbps，0~8000;
 *              SDK根据码流来预设定内部视音频缓冲区，设置过大会导致弱网下延迟增大，设置过小会导致弱网时丢帧频繁，画面卡顿
 * @param [IN] duration: 回放的文件长度，单位：s。
 *          LV_STREAM_CMD_LIVE置为0.
 *          LV_STREAM_CMD_STORAGE_RECORD_BY_FILE置为单文件的时长.
 *          LV_STREAM_CMD_STORAGE_RECORD_BY_UTC_TIME置为 lv_stream_param_s里的stop_time - start_time.
 * @param [IN] video_param: 视频的相关参数配置
 * @param [IN] audio_param: 音频的相关参数配置
 * @notice 通知开始推流、强制I帧请求后，需要调用此API。
 * 
 * @return lv_error_e
 * @see lv_start_push_streaming_cb()
 */
int lv_stream_send_config(int service_id,
                          unsigned int bitrate_kbps,
                          double duration,
                          const lv_video_param_s *video_param,
                          const lv_audio_param_s *audio_param);

/**
 * @brief 发送视频数据
 * 
 * @param [IN] service_id: 服务ID
 * @param [IN] format: 视频数据编码格式
 * @param [IN] buffer: 视频数据指针
 * @param [IN] buffer_len: 视频数据长度
 * @param [IN] key_frame: 视频帧是否为关键帧,0为非关键帧，非0为关键帧，一般关键帧可设置为1.
 * @param [IN] timestamp_ms: 视频帧时间戳，单位：ms
 * 
 * @return lv_error_e
 */
int lv_stream_send_video(int service_id,
                         lv_video_format_e formaft,
                         unsigned char* buffer,
                         unsigned int buffer_len,
                         int key_frame,
                         unsigned int timestamp_ms);

/**
 * @brief 发送音频数据
 * 
 * @param [IN] service_id: 服务ID
 * @param [IN] format: 视频数据编码格式
 * @param [IN] buffer: 音频数据指针
 * @param [IN] buffer_len: 音频数据长度
 * @param [IN] timestamp_ms: 音频帧时间戳，单位：ms
 * 
 * @return lv_error_e
 */
int lv_stream_send_audio(int service_id,
                         lv_audio_format_e format,
                         unsigned char* buffer,
                         unsigned int buffer_len,
                         unsigned int timestamp_ms);

/**
 * @brief 开始语音对讲服务
 * 
 * @param [IN] service_id: 服务ID
 * @param [IN] audio_param: 对讲音频的相关参数配置
 * 
 * @return lv_error_e
 * @see lv_start_voice_intercom_cb()
 */
int lv_voice_intercom_start_service(int service_id, const lv_audio_param_s *audio_param);

/**
 * @brief 发送语音对讲的音频数据
 * 
 * @param [IN] service_id: 服务ID
 * @param [IN] buffer: 音频数据指针
 * @param [IN] buffer_len: 音频数据长度
 * @param [IN] timestamp:  音频帧时间戳，单位：ms
 * 
 * @return lv_error_e
 */
int lv_voice_intercom_send_audio(int service_id, const char* buffer, int buffer_len , unsigned int timestamp_ms);

/**
 * @brief 停止语音对讲服务
 * 
 * @param [IN] service_id: 服务ID
 * 
 * @return lv_error_e
 * @see lv_stop_voice_intercom_cb()
 */
int lv_voice_intercom_stop_service(int service_id);

/**
 * @brief 报警事件图片上传或抓图上传
 * @notice 报警事件上传时，推荐使用lv_post_intelligent_alarm，该接口将逐渐废弃
 *
 * @param [IN] buffer: 事件图片数据指针
 * @param [IN] buffer_len: 事件图片数据长度，图片大小最大为5M（含）
 * @param [IN] type: 事件类型，lv_event_type_e
 * @param [IN] id: 当type=LV_TRIGGER_PIC时，需要回传lv_trigger_pic_capture_cb中的id，其他为空字符串
 * @param [iN] event_payload： 告警内容，格式为字符串；不大于1024个字节，超过会被截断；可为空
 * @notice: 该接口会先上报lv已定义好的事件，再上传图片，此时图片和事件是绑定在一起的；
 *          如果无图片数据，则只上传事件；
 *          该图片不能与开发者自定义的事件进行绑定
 * @notice: 若物模型里AlarmEvent里无data字段，event_payload传NULL即可；若有data字段但不需要使用，event_payload则传空字符串
 *
 * @return lv_error_e
 * @see lv_trigger_pic_capture_cb()
 */
int lv_post_alarm_image(const char* buffer,
                        int buffer_len,
                        lv_event_type_e type,
                        const char *id, 
                        const char *event_payload);

/**
 * @brief 智能报警事件图片上传
 *
 * @param [IN] param: 结构体lv_intelligent_alarm_param_s指针
 *
 * @notice:
 * 接口调用：
 * 1. 本接口为异步接口，存在图片数据时，SDK会进行图片数据的拷贝；事件也会进行拷贝
 * 2. 图片数据需最大为1MB（含），超过则返回失败
 * 3. 附加字符串大于2048B时会被截断，但不会返回失败
 * 4. 调用间隔短于云端设定值，返回失败
 * SDK内部逻辑：
 * 1. 图片数据拷贝后，为防止内存过量增加，会限制带图片的事件总数，最大值默认为10；
 * 2. 图片数量超过最大限制时，自动删除最旧的图片数据，但事件会保留
 * 3. 不带图片的事件不限制最大数量
 * 4. 图片上传失败也会尝试上报事件
 * 5. 图片、事件上报失败都不会重传
 *
 * @return lv_error_e
 */
int lv_post_intelligent_alarm(const lv_intelligent_alarm_param_s *param);


/**
 * @brief 点播功能，点播模式为LV_STREAM_CMD_STORAGE_RECORD_BY_FILE时，在文件回放结束后调用
 * 点播模式为LV_STREAM_CMD_STORAGE_RECORD_BY_UTC_TIME，在最后一个文件播放结束后调用
 *
 * @param [IN] service_id: 服务ID
 * @param [IN] filename: 文件名
 *
 * @return lv_error_e
 *
 */
int lv_post_file_close(int service_id, const char *filename);

/**
 * @brief 事件录像（含预录）功能，即模式为LV_STREAM_CMD_PRE_EVENT_RECORD时，
 * 在预录缓冲数据消耗完第一次切换为实时码流时调用
 *
 * @param [IN] service_id: 服务ID
 *
 * @return lv_error_e
 *
 */
int lv_post_pre_complete(int service_id);

/*
 * @brief 用于动态调节SDK的某些功能，如日志级别，方便调试。
 *
 * @param [IN] type: 功能类型,见enum lv_control_type_e.
 * @param [IN] ...: 可变长参数，参数类型与功能类型有关
 *              type:LV_CONTROL_LOG_LEVEL, 参数：int log_level;实时生效
 *              type:LV_CONTROL_STREAM_AUTO_CHECK，参数：unsigned int flag；已启动的流不会生效，新启动的流会生效
 *              type:LV_CONTROL_STREAM_AUTO_SAVE，参数：unsigned int flag，char *save_path；需要关闭时，
 *                          save_path参数不会被使用。已启动的流不会生效，新启动的流会生效
 *
 * @return lv_error_e
 */
int lv_control(lv_control_type_e type, ...);


#ifdef __cplusplus
}
#endif
#endif /* _LINK_VISUAL_IPC_H */
