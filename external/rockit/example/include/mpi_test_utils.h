/*
 * Copyright 2020 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SRC_TESTS_RT_MPI_MPI_TEST_UTILS_H_
#define SRC_TESTS_RT_MPI_MPI_TEST_UTILS_H_

#include "rk_type.h"
#include "rk_debug.h"
#include "rk_comm_video.h"

#define RK_ALIGN(x, a)         (((x) + (a) - 1) & ~((a) - 1))
#define RK_ALIGN_16(x)         RK_ALIGN(x, 16)
#define RK_ALIGN_64(x)         RK_ALIGN(x, 64)
#define RK_ALIGN_256(x)        RK_ALIGN(x, 256)
#define RK_ALIGN_256_ODD(x)    (RK_ALIGN(x, 256) | 256)

typedef enum _RTUriSchemeType {
    RK_URI_SCHEME_NONE = 0,
    RK_URI_SCHEME_LOCAL,
    RK_URI_SCHEME_HTTP,
    RK_URI_SCHEME_RTSP,
    RK_URI_SCHEME_SAMBA,
    RK_URI_SCHEME_HLS,
    RK_URI_SCHEME_RTP,
    RK_URI_SCHEME_UDP,
    RK_URI_SCHEME_MMS,
    RK_URI_SCHEME_MMSH,
    RK_URI_SCHEME_MMST,
    RK_URI_SCHEME_TTS,
    RK_URI_SCHEME_IPTV,
} URI_SCHEME_TYPE;

RK_U64 mpi_test_utils_get_now_us();
URI_SCHEME_TYPE get_uri_scheme_type(const char* uri);
RK_S32 utils_av_to_rk_codec(RK_S32 s32Id);

#endif  // SRC_TESTS_RT_MPI_MPI_TEST_UTILS_H_

