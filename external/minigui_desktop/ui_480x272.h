/*
 * Rockchip App
 *
 * Copyright (C) 2017 Rockchip Electronics Co., Ltd.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL), available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __UI_480X272_H__
#define __UI_480X272_H__

#define LCD_W    480
#define LCD_H    272

#define TTF_FONT_SIZE     16

#define STATUS_BAR_ICO_SPACE    15

#define BATT_PINT_X    LCD_W - 40
#define BATT_PINT_Y    10
#define BATT_PINT_W    28
#define BATT_PINT_H    15

#define WIFI_PINT_X    (BATT_PINT_X - STATUS_BAR_ICO_SPACE -20)
#define WIFI_PINT_Y    5
#define WIFI_PINT_W    28
#define WIFI_PINT_H    20

#define BACK_PINT_X    10
#define BACK_PINT_Y    10
#define BACK_PINT_W    40
#define BACK_PINT_H    20

#define BG_PINT_X    0
#define BG_PINT_Y    0
#define BG_PINT_W    LCD_W
#define BG_PINT_H    LCD_H

#define DESKTOP_DLG_X    0
#define DESKTOP_DLG_Y    80
#define DESKTOP_DLG_W    LCD_W - DESKTOP_DLG_X
#define DESKTOP_DLG_H    LCD_H - DESKTOP_DLG_Y
#define DESKTOP_DLG_STRING    "desktop"

#define TITLE_PINT_X    70
#define TITLE_PINT_Y    10
#define TITLE_PINT_W    130
#define TITLE_PINT_H    24

#define TITLE_LINE_PINT_X    0
#define TITLE_LINE_PINT_Y    36
#define TITLE_LINE_PINT_W    LCD_W
#define TITLE_LINE_PINT_H    2

//desktop_dialog
#define ICON_SPAC      91

#define MUSIC_PINT_X    25
#define MUSIC_PINT_Y    LCD_H - 80
#define MUSIC_PINT_W    64
#define MUSIC_PINT_H    64

#define PHOTO_PINT_X    MUSIC_PINT_X + ICON_SPAC
#define PHOTO_PINT_Y    MUSIC_PINT_Y
#define PHOTO_PINT_W    MUSIC_PINT_W
#define PHOTO_PINT_H    MUSIC_PINT_H

#define VIDEO_PINT_X    PHOTO_PINT_X + ICON_SPAC
#define VIDEO_PINT_Y    MUSIC_PINT_Y
#define VIDEO_PINT_W    MUSIC_PINT_W
#define VIDEO_PINT_H    MUSIC_PINT_H

#define FOLDE_PINT_X    VIDEO_PINT_X + ICON_SPAC
#define FOLDE_PINT_Y    MUSIC_PINT_Y
#define FOLDE_PINT_W    MUSIC_PINT_W
#define FOLDE_PINT_H    MUSIC_PINT_H

#define SETTING_PINT_X    FOLDE_PINT_X + ICON_SPAC
#define SETTING_PINT_Y    MUSIC_PINT_Y
#define SETTING_PINT_W    MUSIC_PINT_W
#define SETTING_PINT_H    MUSIC_PINT_H

#define MENU_ICON_ZOOM_W    (MUSIC_PINT_W / 5)
#define MENU_ICON_ZOOM_H    (MUSIC_PINT_H / 5)

#define PHOTO_ICON_PINT_X    50
#define PHOTO_ICON_PINT_Y    80
#define PHOTO_ICON_PINT_W    80
#define PHOTO_ICON_PINT_H    45
#define PHOTO_ICON_SPAC      140
#define PHOTO_ICON_ZOOM_W    (PHOTO_ICON_PINT_W / 5)
#define PHOTO_ICON_ZOOM_H    (PHOTO_ICON_PINT_H / 5)

#define PHOTO_ICON_NUM_PERPAGE  3

#define PHOTO_PREVIEW_CENTER_W  (LCD_W / 3)
#define PHOTO_PREVIEW_CENTER_H  (LCD_H / 3)
#define PHOTO_PREVIEW_CENTER_X  ((LCD_W - PHOTO_PREVIEW_CENTER_W)/ 2)
#define PHOTO_PREVIEW_CENTER_Y  ((LCD_H - PHOTO_PREVIEW_CENTER_H)/ 2)

#define PHOTO_PREVIEW_LEFT_W  (LCD_W / 6)
#define PHOTO_PREVIEW_LEFT_H  (LCD_H / 6)
#define PHOTO_PREVIEW_LEFT_X  90
#define PHOTO_PREVIEW_LEFT_Y  (PHOTO_PREVIEW_CENTER_Y + (PHOTO_PREVIEW_CENTER_H - PHOTO_PREVIEW_LEFT_H))

#define PHOTO_PREVIEW_RIGHT_W  (LCD_W / 6)
#define PHOTO_PREVIEW_RIGHT_H  (LCD_H / 6)
#define PHOTO_PREVIEW_RIGHT_X  (LCD_W - PHOTO_PREVIEW_RIGHT_W - 90)
#define PHOTO_PREVIEW_RIGHT_Y  PHOTO_PREVIEW_LEFT_Y

#define DESKTOP_PAGE_DOT_X    (LCD_W / 2)
#define DESKTOP_PAGE_DOT_Y    (LCD_H - 90)
#define DESKTOP_PAGE_DOT_DIA  4
#define DESKTOP_PAGE_DOT_SPAC  40

//audioplay_dialog
#define ALBUM_ICON_PINT_W    100
#define ALBUM_ICON_PINT_H    100
#define ALBUM_ICON_PINT_X    ((LCD_W - ALBUM_ICON_PINT_W) / 2)
#define ALBUM_ICON_PINT_Y    ((LCD_H - ALBUM_ICON_PINT_H) / 2 - 40)

#define FILENAME_PINT_W    LCD_W
#define FILENAME_PINT_H    24
#define FILENAME_PINT_X    0
#define FILENAME_PINT_Y    (ALBUM_ICON_PINT_Y + ALBUM_ICON_PINT_H + 20)

#define FILENUM_PINT_W    LCD_W
#define FILENUM_PINT_H    24
#define FILENUM_PINT_X    0
#define FILENUM_PINT_Y    (LCD_H - FILENUM_PINT_H - 10)

#define PROGRESSBAR_PINT_X    20
#define PROGRESSBAR_PINT_Y    (LCD_H - 50)
#define PROGRESSBAR_PINT_W    (LCD_W - PROGRESSBAR_PINT_X * 2)
#define PROGRESSBAR_PINT_H    4

#define TIME_PINT_W    100
#define TIME_PINT_H    24
#define TIME_PINT_X    (LCD_W - PROGRESSBAR_PINT_X - TIME_PINT_W)
#define TIME_PINT_Y    (PROGRESSBAR_PINT_Y - TIME_PINT_H)

#define PLAY_BUTTON_SPAC   20

#define PLAYPREV_PINT_W    12
#define PLAYPREV_PINT_H    16
#define PLAYPREV_PINT_X    (PROGRESSBAR_PINT_X)
#define PLAYPREV_PINT_Y    (PROGRESSBAR_PINT_Y - PLAYSTATUS_PINT_H - 8)

#define PLAYSTATUS_PINT_W    12
#define PLAYSTATUS_PINT_H    16
#define PLAYSTATUS_PINT_X    (PLAYPREV_PINT_X + PLAY_BUTTON_SPAC)
#define PLAYSTATUS_PINT_Y    PLAYPREV_PINT_Y

#define PLAYNEXT_PINT_W    12
#define PLAYNEXT_PINT_H    16
#define PLAYNEXT_PINT_X    (PLAYSTATUS_PINT_X + PLAY_BUTTON_SPAC)
#define PLAYNEXT_PINT_Y    PLAYPREV_PINT_Y


//browser_dialog
#define BROWSER_LIST_STR_PINT_X    70
#define BROWSER_LIST_STR_PINT_Y    47
#define BROWSER_LIST_STR_PINT_W    24
#define BROWSER_LIST_STR_PINT_H    24
#define BROWSER_LIST_STR_PINT_SPAC      36

#define BROWSER_LIST_PIC_PINT_W    32
#define BROWSER_LIST_PIC_PINT_H    32

#define BROWSER_LIST_SEL_PINT_H    36

#define FILE_NUM_PERPAGE  6

#define BROWSER_PAGE_DOT_X    (LCD_W / 2)
#define BROWSER_PAGE_DOT_Y    (LCD_H - 10)
#define BROWSER_PAGE_DOT_DIA  4
#define BROWSER_PAGE_DOT_SPAC  40
//setting
#define SETTING_NUM_PERPAGE    6

#define SETTING_LIST_STR_PINT_X    30
#define SETTING_LIST_STR_PINT_Y    45
#define SETTING_LIST_STR_PINT_W    24
#define SETTING_LIST_STR_PINT_H    24
#define SETTING_LIST_STR_PINT_SPAC      36

#define SETTING_LIST_SEL_PINT_H    36

#define SETTING_LIST_DOT_PINT_X    LCD_W - 40
#define SETTING_LIST_DOT_PINT_W    16
#define SETTING_LIST_DOT_PINT_H    16

#define SETTING_INFO_PINT_X    30
#define SETTING_INFO_PINT_Y    70
#define SETTING_INFO_PINT_W    LCD_W - SETTING_INFO_PINT_X *2
#define SETTING_INFO_PINT_H    24
#define SETTING_INFO_PINT_SPAC      30

#define SETTING_PAGE_DOT_X    (LCD_W / 2)
#define SETTING_PAGE_DOT_Y    (LCD_H - 10)
#define SETTING_PAGE_DOT_DIA  4
#define SETTING_PAGE_DOT_SPAC  40
//videoplay_hw_dialog
#define VIDEO_TOPBAR_H           38
#define VIDEO_BOTTOMBAR_H        70

#define VIDEO_FILENAME_PINT_W    LCD_W
#define VIDEO_FILENAME_PINT_H    24
#define VIDEO_FILENAME_PINT_X    0
#define VIDEO_FILENAME_PINT_Y    10

#define VIDEO_FILENUM_PINT_W    LCD_W
#define VIDEO_FILENUM_PINT_H    24
#define VIDEO_FILENUM_PINT_X    0
#define VIDEO_FILENUM_PINT_Y    (LCD_H - VIDEO_FILENUM_PINT_H - 10)

#define VIDEO_PROGRESSBAR_PINT_X    20
#define VIDEO_PROGRESSBAR_PINT_Y    (LCD_H - 40)
#define VIDEO_PROGRESSBAR_PINT_W    (LCD_W - VIDEO_PROGRESSBAR_PINT_X * 2)
#define VIDEO_PROGRESSBAR_PINT_H    4

#define VIDEO_TIME_PINT_W    100
#define VIDEO_TIME_PINT_H    24
#define VIDEO_TIME_PINT_X    (LCD_W - VIDEO_PROGRESSBAR_PINT_X - VIDEO_TIME_PINT_W)
#define VIDEO_TIME_PINT_Y    (VIDEO_PROGRESSBAR_PINT_Y - VIDEO_TIME_PINT_H)

#define VIDEO_BUTTON_SPAC   20

#define VIDEO_PLAYPREV_PINT_W    12
#define VIDEO_PLAYPREV_PINT_H    16
#define VIDEO_PLAYPREV_PINT_X    (VIDEO_PROGRESSBAR_PINT_X)
#define VIDEO_PLAYPREV_PINT_Y    (VIDEO_PROGRESSBAR_PINT_Y - VIDEO_PLAYSTATUS_PINT_H - 8)

#define VIDEO_PLAYSTATUS_PINT_W    12
#define VIDEO_PLAYSTATUS_PINT_H    16
#define VIDEO_PLAYSTATUS_PINT_X    (VIDEO_PLAYPREV_PINT_X + VIDEO_BUTTON_SPAC)
#define VIDEO_PLAYSTATUS_PINT_Y    VIDEO_PLAYPREV_PINT_Y

#define VIDEO_PLAYNEXT_PINT_W    12
#define VIDEO_PLAYNEXT_PINT_H    16
#define VIDEO_PLAYNEXT_PINT_X    (VIDEO_PLAYSTATUS_PINT_X + VIDEO_BUTTON_SPAC)
#define VIDEO_PLAYNEXT_PINT_Y    VIDEO_PLAYPREV_PINT_Y
#endif
