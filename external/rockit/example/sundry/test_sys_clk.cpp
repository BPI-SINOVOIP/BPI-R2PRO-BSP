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
 *
 */

#include <unistd.h>

#include "rk_mpi_sys.h"
#include "rk_debug.h"
#include "argparse.h"

RK_S32 unit_test_mpi_sys_pts() {
    RK_MPI_SYS_InitPTSBase(0);

    RK_U64 curPts = 0ll;
    RK_U64 syncPts = 0ll;
    for (RK_S32 j = 0; j < 10; j++) {
        for (RK_S32 i = 0; i < 100; i++) {
            RK_MPI_SYS_GetCurPTS(&curPts);
            RK_LOGI("cur pts %lld", curPts);
            usleep(10 * 1000);
        }
        RK_LOGI("sync time.");
        RK_MPI_SYS_SyncPTS(10 * 1000 * j);
    }

    return RK_SUCCESS;
}

static const char *const usages[] = {
    "[options]: ",
    NULL,
};

RK_S32 main(int argc, const char **argv) {
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("basic options:"),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nselect a test case to run.",
                                 "\nuse --help for details.");

    argc = argparse_parse(&argparse, argc, argv);
    argparse_usage(&argparse);

    unit_test_mpi_sys_pts();

    return 0;
}

