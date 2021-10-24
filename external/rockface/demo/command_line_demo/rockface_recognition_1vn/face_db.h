/****************************************************************************
*
*    Copyright (c) 2017 - 2019 by Rockchip Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Rockchip Corporation. This is proprietary information owned by
*    Rockchip Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Rockchip Corporation.
*
*****************************************************************************/

#ifndef _FACE_DB_H
#define _FACE_DB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sqlite3.h"
#include "rockface.h"
#include "rockface_recognition.h"

int open_db(const char *db_path, sqlite3 **db);

int close_db(sqlite3 *db);

int get_all_face(sqlite3* db, face_data* face_array, int face_num);

int get_face_count(sqlite3* db, int *face_num);

int insert_face(sqlite3* db, face_data* face);

int get_all_mask_face(sqlite3* db, mask_face_data* face_array, int face_num);

int insert_mask_face(sqlite3* db, mask_face_data* face);

#ifdef __cplusplus
} //extern "C"
#endif

#endif