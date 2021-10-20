// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_FLOW_NODE_H_
#define _RK_FLOW_NODE_H_

#include <iterator>
#include <list>
#include <vector>

#include "key_string.h"
#include "media_type.h"

#include "logger/log.h"

namespace rockchip {
namespace mediaserver {

#define FLOWS_CONF "/usr/share/mediaserver/mediaserver.conf"
#define FLOWS_CONF_BAK "/data/mediaserver.bak"

#define NODE_PIPE_ID "Pipe_"
#define NODE_FLOW_ID "Flow_"

#define NODE_FLOW_INDEX "flow_index"
#define NODE_FLOW_INDEX_FLOW_TYPE "flow_type"
#define NODE_FLOW_INDEX_STREAM_ID "stream_id"
#define NODE_FLOW_INDEX_STREAM_TYPE "stream_type"
#define NODE_FLOW_INDEX_FLOW_INDEX_NAME "flow_index_name"
#define NODE_FLOW_INDEX_UPFLOW_INDEX_NAME "upflow_index_name"
#define NODE_FLOW_INDEX_OUT_SLOT_INDEX "out_slot_index"
#define NODE_FLOW_INDEX_IN_SLOT_INDEX "in_slot_index_of_down"
#define NODE_FLOW_INDEX_OPEN_PIEP_ID "actual_open_pipe_id"
#define NODE_FLOW_INDEX_OPEN_FLOW_ID "actual_open_flow_id"
#define NODE_FLOW_INDEX_FIX_RESO "fix_resolution"
#define NODE_FLOW_INDEX_AJUST_RESO "ajust_resolution"

#define NODE_FLOW_NAME "flow_name"

#define NODE_FLOW_PARAM "flow_param"
#define NODE_FLOW_PARAM_STREAM_NAME "name"
#define NODE_FLOW_PARAM_PORT_NUM KEY_PORT_NUM
#define NODE_FLOW_PARAM_KEY_EVENT_PATH "key_event_path"
#define NODE_FLOW_PARAM_KEY_EVENT_CODE "key_event_code"

#define NODE_STREAM_PARAM "stream_param"
#define NODE_STREAM_PARAM_DEVICE KEY_DEVICE
#define NODE_STREAM_PARAM_INPUT_DATA_TYPE KEY_INPUTDATATYPE
#define NODE_STREAM_PARAM_OUTPUT_DATA_TYPE KEY_OUTPUTDATATYPE
#define NODE_STREAM_PARAM_SAMPLE_FORMAT KEY_SAMPLE_FMT
#define NODE_STREAM_PARAM_NEED_HW_DRAW KEY_NEED_HW_DRAW
#define NODE_STREAM_PARAM_ROCKX_MODEL KEY_ROCKX_MODEL

/*video param node*/
#define NODE_STREAM_PARAM_GOP KEY_VIDEO_GOP
#define NODE_STREAM_PARAM_BITRATE_MAX KEY_COMPRESS_BITRATE_MAX
#define NODE_STREAM_PARAM_BITRATE_MIN KEY_COMPRESS_BITRATE_MIN
#define NODE_STREAM_PARAM_BITRATE KEY_COMPRESS_BITRATE
#define NODE_STREAM_PARAM_STREAMSMOOTH "no_define"
#define NODE_STREAM_PARAM_FRAMERATE KEY_FPS
#define NODE_STREAM_PARAM_FRAMERATE_IN KEY_FPS_IN
#define NODE_STREAM_PARAM_WIDTH KEY_BUFFER_WIDTH
#define NODE_STREAM_PARAM_HEIGHT KEY_BUFFER_HEIGHT
#define NODE_STREAM_PARAM_VIR_WIDTH KEY_BUFFER_VIR_WIDTH
#define NODE_STREAM_PARAM_VIR_HEIGHT KEY_BUFFER_VIR_HEIGHT
#define NODE_STREAM_PARAM_IMAGE_QUALITY_INIT KEY_COMPRESS_QP_INIT
#define NODE_STREAM_PARAM_IMAGE_QUALITY_STEP KEY_COMPRESS_QP_STEP
#define NODE_STREAM_PARAM_IMAGE_QUALITY_MIN KEY_COMPRESS_QP_MIN
#define NODE_STREAM_PARAM_IMAGE_QUALITY_MAX KEY_COMPRESS_QP_MAX
#define NODE_STREAM_PARAM_IMAGE_QUALITY_MIN_I_QP KEY_COMPRESS_QP_MIN_I
#define NODE_STREAM_PARAM_IMAGE_QUALITY_MAX_I_QP KEY_COMPRESS_QP_MAX_I
#define NODE_STREAM_PARAM_RC_MODE KEY_COMPRESS_RC_MODE
#define NODE_STREAM_PARAM_RC_QUALITY KEY_COMPRESS_RC_QUALITY
#define NODE_STREAM_PARAM_H264_PROFILE KEY_PROFILE
#define NODE_STREAM_PARAM_CODECTYPE KEY_CODECTYPE
#define NODE_STREAM_PARAM_SMART "smart"
#define NODE_STREAM_PARAM_SVC "no_define"
#define NODE_STREAM_PARAM_VIDEO_TYPE "no_define"
#define NODE_STREAM_ROI_REGIONS KEY_ROI_REGIONS
#define NODE_STREAM_RI_REGIONS "ri_regions"
#define NODE_STREAM_PARAM_ENABLE KEY_ENABLE
#define NODE_STREAM_PARAM_REG_ENABLE KEY_ENABLE_FACE_REG
#define NODE_STREAM_PARAM_INTERVAL KEY_FRAME_INTERVAL
#define NODE_STREAM_PARAM_DURATION KEY_BODY_DURATION
#define NODE_STREAM_PARAM_PERCENTAGE KEY_BODY_PERCENTAGE
#define NODE_STREAM_PARAM_SENSITIVITY "Sensitivity"
#define NODE_STREAM_PARAM_DETECT_RECT "detect_rect"
#define NODE_STREAM_PARAM_FULL_RANGE KEY_FULL_RANGE

/*move detection param node*/
#define NODE_STREAM_PARAM_MD_SINGLE_REF KEY_MD_SINGLE_REF
#define NODE_STREAM_PARAM_MD_ORI_WIDTH KEY_MD_ORI_WIDTH
#define NODE_STREAM_PARAM_MD_ORI_HEIGHT KEY_MD_ORI_HEIGHT
#define NODE_STREAM_PARAM_MD_DS_WIDTH KEY_MD_DS_WIDTH
#define NODE_STREAM_PARAM_MD_DS_HEIGHT KEY_MD_DS_HEIGHT
#define NODE_STREAM_PARAM_MD_ROI_CNT KEY_MD_ROI_CNT
#define NODE_STREAM_PARAM_MD_ROI_RECT KEY_MD_ROI_RECT
/*audio param node*/
#define NODE_STREAM_PARAM_CHANNEL KEY_CHANNELS
#define NODE_STREAM_PARAM_SAMPLE_RATE KEY_SAMPLE_RATE
#define NODE_STREAM_PARAM_VOLUME "no_define"
#define NODE_STREAM_PARAM_AUDIO_SOURCE "no_define"
#define NODE_STREAM_PARAM_ENCODE_TYPE "no_define"
#define NODE_STREAM_PARAM_ANS "no_define"
/*rga param node*/
#define NODE_STREAM_PARAM_RECT KEY_BUFFER_RECT

/*muxer param node*/
#define NODE_FLOW_PARAM_FILE_PATH KEY_PATH
#define NODE_FLOW_PARAM_FILE_PREFIX "file_prefix"

/*muxer -> rstp type */
#define RTSP_DATA_TYPE_MPEGTS MUXER_MPEG_TS

/*flow name*/
#define RKMEDIA_FLOW_NAME_LIVE555_RTSP "live555_rtsp_server"

/*stream name*/
#define RKMEDIA_STREAM_NAME_RKMPP "rkmpp"

/*filter name*/
#define RKMEDIA_FILTER_NAME_RGA "rkrga"
#define RKMEDIA_FILTER_NAME_THROUGH "through_guard"
#define RKMEDIA_FILTER_ROCKFACE_DETECT "rockface_detect"
#define RKMEDIA_FILTER_ROCKFACE_BODYDETECT "rockface_bodydetect"
#define RKMEDIA_FILTER_ROCKFACE_RECOGNIZE "rockface_recognize"
#define RKMEDIA_FILTER_DRAW_FILTER "draw_filter"
#define RKMEDIA_FILTER_ROCKX_FILTER "rockx_filter"
#define RKMEDIA_FILTER_NN_INPUT "nn_result_input"
#define RKMEDIA_FILTER_FACE_CAPTURE "face_capture"

#define DB_VALUE_LEVEL_HIGHEST "highest"
#define DB_VALUE_LEVEL_HIGHER "higher"
#define DB_VALUE_LEVEL_HIGH "high"
#define DB_VALUE_LEVEL_MEDIUM "medium"
#define DB_VALUE_LEVEL_LOW "low"
#define DB_VALUE_LEVEL_LOWER "lower"
#define DB_VALUE_LEVEL_LOWEST "lowest"

#define DB_VALUE_H264_BASELINE "baseline"
#define DB_VALUE_H264_MAIN "main"
#define DB_VALUE_H264_HIGH "high"

#define DB_RC_MODE_CBR "CBR"
#define DB_RC_MODE_VBR "VBR"

#define DB_ENCORDE_TYPE_H264 "H.264"
#define DB_ENCORDE_TYPE_H265 "H.265"
#define DB_ENCORDE_TYPE_MJPEG "MJPEG"
#define DB_ENCORDE_TYPE_MPEG4 "MPEG4"

#define DB_SMART_VALUE_OPEN "open"
#define DB_SMART_VALUE_CLOSE "close"
#define DB_SMART_ENABLE "1"
#define DB_SMART_DISABLE "0"

#define PARAM_STRING_APPEND2(s, s1, s2)                                        \
  s.append(s1).append("=").append(s2).append("\n")

enum class FlowType { UNKNOW, SOURCE, IO, SINK };
enum class StreamType {
  UNKNOW,
  CAMERA,
  VIDEO_ENCODER,
  VIDEO_DECODER,
  JPEG_THROUGH_GUARD,
  JPEG_ENCODER,
  JPEG_DECODER,
  FILE,
  AUDIO,
  AUDIO_ENCODER,
  RTSP,
  LINK,
  FILTER,
  MUXER,
  MOVE_DETEC,
  GENERAL
};

#define MAX_CAM_NUM 4
#define MAX_SOUND_NUM 3
#define MAX_OSD_REGION_NUM 15
#define ROI_REGION_NUM 4
#define WEB_VIEW_RECT_W 704
#define WEB_VIEW_RECT_H 480
#define MAX_DAY_SCHEDULE_NUM 8

enum class StreamId {
  MAIN_STREAM = 0,
  SUB_STREAM,
  THIRD_STREAM,
  FOURTH_STREAM,
};

enum class VideoStreamType { UNKNOW, VIDEO_STREAM, VIDEO_COMPOSITE_STREAM };

typedef std::pair<std::string, std::string> prop_p;
typedef std::vector<std::pair<std::string, std::string>> props_v;

} // namespace mediaserver
} // namespace rockchip

#endif
