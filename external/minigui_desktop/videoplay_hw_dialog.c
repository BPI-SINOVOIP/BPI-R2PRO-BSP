/*
 * This is a every simple sample for MiniGUI.
 * It will create a main window and display a string of "Hello, world!" in it.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "common.h"

#include "ffplay_ipc.h"

#define SLIDE_DISTANCE 100
#define WHOLE_BUTTON_NUM 4

static RECT msg_rcFilename = {VIDEO_FILENAME_PINT_X, VIDEO_FILENAME_PINT_Y, VIDEO_FILENAME_PINT_X + VIDEO_FILENAME_PINT_W, VIDEO_FILENAME_PINT_Y + VIDEO_FILENAME_PINT_H};
static RECT msg_rcTime = {VIDEO_TIME_PINT_X, VIDEO_TIME_PINT_Y, VIDEO_TIME_PINT_X + VIDEO_TIME_PINT_W, VIDEO_TIME_PINT_Y + VIDEO_TIME_PINT_H};
static RECT msg_rcProBar = {VIDEO_PROGRESSBAR_PINT_X, VIDEO_PROGRESSBAR_PINT_Y, VIDEO_PROGRESSBAR_PINT_X + VIDEO_PROGRESSBAR_PINT_W, VIDEO_PROGRESSBAR_PINT_Y + VIDEO_PROGRESSBAR_PINT_H};
static RECT msg_rcPlayStatus = {VIDEO_PLAYSTATUS_PINT_X, VIDEO_PLAYSTATUS_PINT_Y, VIDEO_PLAYSTATUS_PINT_X + VIDEO_PLAYSTATUS_PINT_W, VIDEO_PLAYSTATUS_PINT_Y + VIDEO_PLAYSTATUS_PINT_H};
static RECT msg_rcFileNum = {VIDEO_FILENUM_PINT_X, VIDEO_FILENUM_PINT_Y, VIDEO_FILENUM_PINT_X + VIDEO_FILENUM_PINT_W, VIDEO_FILENUM_PINT_Y + VIDEO_FILENUM_PINT_H};

#define UI_HIDE_TIME    5

static char seek_time[100];
static int cur_time;
static int total_time;
static int file_total;
static int file_select = 0;
static char time_str[30];
static char filenum_str[30];
static int play_status;
static int batt;
static int ui_hide_cnt = 0;

static struct directory_node *dir_node = 0;
static struct file_node *cur_file_node;

static BITMAP playstatus_bmap[2];
static BITMAP playprev_bmap;
static BITMAP playnext_bmap;
static HWND hMainWnd;

static touch_pos touch_pos_down, touch_pos_up, touch_pos_old;

static const GAL_Rect msg_galrcMenu[] =
{
    {BACK_PINT_X, BACK_PINT_Y, BACK_PINT_W, BACK_PINT_H},
    {VIDEO_PLAYPREV_PINT_X, VIDEO_PLAYPREV_PINT_Y, VIDEO_PLAYPREV_PINT_W, VIDEO_PLAYPREV_PINT_H},
    {VIDEO_PLAYSTATUS_PINT_X, VIDEO_PLAYSTATUS_PINT_Y, VIDEO_PLAYSTATUS_PINT_W, VIDEO_PLAYSTATUS_PINT_H},
    {VIDEO_PLAYNEXT_PINT_X, VIDEO_PLAYNEXT_PINT_Y, VIDEO_PLAYNEXT_PINT_W, VIDEO_PLAYNEXT_PINT_H},
};

static int is_button(int x, int y, GAL_Rect rect)
{
    return ((x <= rect.x + rect.w) && (x >= rect.x) && (y <= rect.y + rect.h) && (y >= rect.y));
}

static int check_button(int x, int y)
{
    int i;
    for (i = 0; i < WHOLE_BUTTON_NUM; i++)
    {
        if (is_button(x, y, msg_galrcMenu[i])) return i;
    }
    return -1;
}

static int loadres(void)
{
    int i, j;
    char img[128];
    char *respath = get_ui_image_path();

    snprintf(img, sizeof(img), "%smusic_play.png", respath);
    //printf("%s\n", img);
    if (LoadBitmap(HDC_SCREEN, &playstatus_bmap[0], img))
        return -1;

    snprintf(img, sizeof(img), "%smusic_pause.png", respath);
    //printf("%s\n", img);
    if (LoadBitmap(HDC_SCREEN, &playstatus_bmap[1], img))
        return -1;

    snprintf(img, sizeof(img), "%smusic_playprev.png", respath);
    //printf("%s\n", img);
    if (LoadBitmap(HDC_SCREEN, &playprev_bmap, img))
        return -1;

    snprintf(img, sizeof(img), "%smusic_playnext.png", respath);
    //printf("%s\n", img);
    if (LoadBitmap(HDC_SCREEN, &playnext_bmap, img))
        return -1;
    return 0;
}

static void unloadres(void)
{
    int i;
    UnloadBitmap(&playprev_bmap);
    UnloadBitmap(&playnext_bmap);

    for (i = 0; i < 2; i++)
        UnloadBitmap(&playstatus_bmap[i]);
}

static struct file_node *get_cur_file_node(int id)
{
    struct file_node *file_node_temp;
    int i;

    file_node_temp = dir_node->file_node_list;
    for (i = 0; i < id; i++)
    {
        if (file_node_temp)
            file_node_temp = file_node_temp->next_node;
    }

    return file_node_temp;
}

static void video_play(HWND hWnd, int start_time)
{
    int len;
    char *file_path;
    len = strlen(dir_node->patch) + strlen(cur_file_node->name) + 4;
    file_path = malloc(len);
    snprintf(file_path, len, "%s/%s", dir_node->patch, cur_file_node->name);
    media_play(file_path, hWnd, start_time);
    free(file_path);
}

static void menu_back(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    media_exit();
    EndDialog(hWnd, wParam);
}

static void videoplayhw_enter(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch (lParam)
    {
    case 0:
        if (ui_hide_cnt != UI_HIDE_TIME) menu_back(hWnd, wParam, lParam);
        break;
    case 1:
        if (ui_hide_cnt != UI_HIDE_TIME)
        {
            printf("prev video\n");
            if (file_select != 0)
                file_select--;
            else
                file_select = file_total - 1;
            cur_time = 0;
            total_time = 0;
            ui_hide_cnt = 0;
            play_status = 1;
            cur_file_node = get_cur_file_node(file_select);
            media_exit();
            video_play(hWnd, 0);
            //InvalidateRect(hWnd, &msg_rcDialog, TRUE);
        }
        break;
    case 2:
        if (ui_hide_cnt != UI_HIDE_TIME)
        {
            printf("pause video\n");
            play_status = play_status ? 0 : 1;
            ui_hide_cnt = 0;
            if (play_status)
                media_restore();
            else
                media_pause();
            //InvalidateRect(hWnd, &msg_rcPlayStatus, TRUE);
        }
        break;
    case 3:
        if (ui_hide_cnt != UI_HIDE_TIME)
        {
            printf("next video\n");
            if (file_select < file_total - 1)
                file_select++;
            else
                file_select = 0;
            cur_time = 0;
            total_time = 0;
            ui_hide_cnt = 0;
            play_status = 1;
            cur_file_node = get_cur_file_node(file_select);
            media_exit();
            video_play(hWnd, 0);
            //InvalidateRect(hWnd, &msg_rcDialog, TRUE);
        }
        break;
    case 10:
        ui_hide_cnt = 0;
        if ((cur_time - wParam / 10) < 0)
            snprintf(seek_time, sizeof(seek_time), "%d", 0);
        else
            snprintf(seek_time, sizeof(seek_time), "%d", cur_time - wParam / 10);
        //printf("seektime:%s cur time:%d offset:-%d total time:%d\n",
        //                seek_time,cur_time,wParam / 10,total_time);
        media_seek_to((char *)seek_time);
        break;
    case 11:
        ui_hide_cnt = 0;
        if ((cur_time + wParam / 10) > total_time)
            snprintf(seek_time, sizeof(seek_time), "%d", total_time - 1);
        else
            snprintf(seek_time, sizeof(seek_time), "%d", cur_time + wParam / 10);
        //printf("seektime:%s cur time:%d offset:%d total time:%d\n",
        //                seek_time,cur_time,wParam / 10,total_time);
        media_seek_to((char *)seek_time);
        break;
    default:
        if (ui_hide_cnt == UI_HIDE_TIME)
            ui_hide_cnt = 0;
        else
            ui_hide_cnt = UI_HIDE_TIME;
        break;
    }
}

static LRESULT videoplay_hw_dialog_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;

    //printf("%s message = 0x%x, 0x%x, 0x%x\n", __func__, message, wParam, lParam);
    switch (message)
    {
    case MSG_INITDIALOG:
    {
        DWORD bkcolor;
        HDC sndHdc;
        char *file_path;
        int len;
        HWND hFocus = GetDlgDefPushButton(hWnd);
        bkcolor = GetWindowElementPixel(hWnd, WE_BGC_WINDOW);
        SetWindowBkColor(hWnd, bkcolor);
        if (hFocus)
            SetFocus(hFocus);
        loadres();
        batt = battery;
        SetTimer(hWnd, _ID_TIMER_VIDEOPLAY_HW, TIMER_VIDEOPLAY_HW);
        ui_hide_cnt = 0;
        hMainWnd = hWnd;
        sndHdc = GetSecondaryDC((HWND)hWnd);
        SetMemDCAlpha(sndHdc, MEMDC_FLAG_SWSURFACE, 0);
        InvalidateRect(hWnd, &msg_rcDialog, TRUE);
        len = strlen(dir_node->patch) + strlen(cur_file_node->name) + 4;
        file_path = malloc(len);
        snprintf(file_path, len, "%s/%s", dir_node->patch, cur_file_node->name);
        media_play(file_path, hWnd, 0);
        free(file_path);
        return 0;
    }
    case MSG_TIMER:
    {
        static int dialog_last_time = 60;
        if (now_time->tm_min != dialog_last_time)
        {
            dialog_last_time = now_time->tm_min;
            InvalidateRect(hWnd, &msg_rcBg, FALSE);
        }
        if (wParam == _ID_TIMER_VIDEOPLAY_HW)
        {
            if (ui_hide_cnt < UI_HIDE_TIME)
            {
                ui_hide_cnt++;
                if (ui_hide_cnt == UI_HIDE_TIME)
                    InvalidateRect(hWnd, &msg_rcDialog, TRUE);
            }
#ifdef ENABLE_BATT
            if (batt != battery)
            {
                batt = battery;
                InvalidateRect(hWnd, &msg_rcStatusBar, TRUE);
            }
#endif
#ifdef ENABLE_WIFI
            InvalidateRect(hWnd, &msg_rcWifi, TRUE);
#endif
        }
        break;
    }
    case MSG_KEYDOWN:
        if (ui_hide_cnt == UI_HIDE_TIME)
        {
            ui_hide_cnt = 0;
            InvalidateRect(hWnd, &msg_rcDialog, TRUE);
            break;
        }
        ui_hide_cnt = 0;
        switch (wParam)
        {
        case KEY_UP_FUNC:
            break;
        case KEY_DOWN_FUNC:
            break;
        case KEY_ENTER_FUNC:
            play_status = play_status ? 0 : 1;
            if (play_status)
                media_restore();
            else
                media_pause();
            InvalidateRect(hWnd, &msg_rcPlayStatus, TRUE);
            break;
        case KEY_EXIT_FUNC:
            media_exit();
            EndDialog(hWnd, wParam);
            break;
        }
        break;
    case MSG_COMMAND:
        break;
    case MSG_DISPLAY_CHANGED:
    {
        printf("videoplay MSG_DISPLAY_CHANGED\n");
        {
            int starttime = cur_time;
            char *file_path;
            int len = strlen(dir_node->patch) + strlen(cur_file_node->name) + 4;
            file_path = malloc(len);
            if (file_path)
            {
                snprintf(file_path, len, "%s/%s", dir_node->patch, cur_file_node->name);
                media_exit();
                media_play(file_path, hWnd, starttime);
                free(file_path);
            }
        }
        break;
    }
    case MSG_PAINT:
    {
        int i;
        hdc = BeginPaint(hWnd);

        if (ui_hide_cnt == UI_HIDE_TIME)
        {
            SetBrushColor(hdc, 0x00000000);
            FillBox(hdc, 0, 0, LCD_W, LCD_H);
        }
        else
        {
            SetBrushColor(hdc, 0x80000000);
            FillBox(hdc, 0, 0, LCD_W, VIDEO_TOPBAR_H);
            FillBox(hdc, 0, LCD_H - VIDEO_BOTTOMBAR_H, LCD_W, VIDEO_BOTTOMBAR_H);
            SetBrushColor(hdc, 0x00000000);
            FillBox(hdc, 0, VIDEO_TOPBAR_H, LCD_W, LCD_H - VIDEO_TOPBAR_H - VIDEO_BOTTOMBAR_H);

            FillBoxWithBitmap(hdc, BACK_PINT_X, BACK_PINT_Y,
                              BACK_PINT_W, BACK_PINT_H,
                              &back_bmap);
#ifdef ENABLE_BATT
            FillBoxWithBitmap(hdc, BATT_PINT_X - status_bar_offset, BATT_PINT_Y,
                              BATT_PINT_W, BATT_PINT_H,
                              &batt_bmap[batt]);
#endif
#ifdef ENABLE_WIFI
            if (get_wifi_state() == RK_WIFI_State_OFF)
            {
                FillBoxWithBitmap(hdc, WIFI_PINT_X - status_bar_offset, WIFI_PINT_Y,
                                  WIFI_PINT_W, WIFI_PINT_H,
                                  &wifi_disabled_bmap);
            }
            else if (get_wifi_state() == RK_WIFI_State_CONNECTED)
            {
                FillBoxWithBitmap(hdc, WIFI_PINT_X - status_bar_offset, WIFI_PINT_Y,
                                  WIFI_PINT_W, WIFI_PINT_H,
                                  &wifi_connected_bmap);
            }
            else
            {
                FillBoxWithBitmap(hdc, WIFI_PINT_X - status_bar_offset, WIFI_PINT_Y,
                                  WIFI_PINT_W, WIFI_PINT_H,
                                  &wifi_disconnected_bmap);
            }
#endif
            RECT msg_rcTime;
            msg_rcTime.left = REALTIME_PINT_X - status_bar_offset;
            msg_rcTime.top = REALTIME_PINT_Y;
            msg_rcTime.right = REALTIME_PINT_X + REALTIME_PINT_W;
            msg_rcTime.bottom = REALTIME_PINT_Y + REALTIME_PINT_H;
            SetBkColor(hdc, COLOR_transparent);
            SetBkMode(hdc, BM_TRANSPARENT);
            SetTextColor(hdc, RGB2Pixel(hdc, 0xff, 0xff, 0xff));
            SelectFont(hdc, logfont_title);
            DrawText(hdc, status_bar_time_str, -1, &msg_rcTime, DT_TOP);
            msg_rcTime.left = REALDATE_PINT_X - status_bar_offset;
            DrawText(hdc, status_bar_date_str, -1, &msg_rcTime, DT_TOP);

            BITMAP *volume_display;

            if (get_volume() == 0) volume_display = &volume_0;
            else if (get_volume() > 0  && get_volume() <= 32)    volume_display = &volume_1;
            else if (get_volume() > 32  && get_volume() <= 66)  volume_display = &volume_2;
            else volume_display = &volume_3;

            FillBoxWithBitmap(hdc, VOLUME_PINT_X - status_bar_offset, VOLUME_PINT_Y,
                              VOLUME_PINT_W, VOLUME_PINT_H,
                              volume_display);

            msg_rcTime.left = TIME_PINT_X;
            msg_rcTime.top = TIME_PINT_Y;
            msg_rcTime.right = TIME_PINT_X + TIME_PINT_W;
            msg_rcTime.bottom = TIME_PINT_Y + TIME_PINT_H;
            SetBkColor(hdc, COLOR_transparent);
            SetBkMode(hdc, BM_TRANSPARENT);
            SetBrushColor(hdc, 0xff4f4f4f);
            FillBox(hdc, VIDEO_PROGRESSBAR_PINT_X, VIDEO_PROGRESSBAR_PINT_Y, VIDEO_PROGRESSBAR_PINT_W, VIDEO_PROGRESSBAR_PINT_H); //progressbar
            SetBrushColor(hdc, 0xffffffff);
            SetTextColor(hdc, RGB2Pixel(hdc, 0xff, 0xff, 0xff));
            if (total_time)
                FillBox(hdc, VIDEO_PROGRESSBAR_PINT_X, VIDEO_PROGRESSBAR_PINT_Y, VIDEO_PROGRESSBAR_PINT_W * cur_time / total_time, VIDEO_PROGRESSBAR_PINT_H);  //progressbar
            if (cur_file_node)
                SelectFont(hdc, logfont_title);
            DrawText(hdc, cur_file_node->name, -1, &msg_rcFilename, DT_TOP | DT_CENTER);
            snprintf(time_str, sizeof(time_str), "%02d:%02d/%02d:%02d", cur_time / 60, cur_time % 60, total_time / 60, total_time % 60);
            SelectFont(hdc, logfont_title);
            DrawText(hdc, time_str, -1, &msg_rcTime, DT_TOP | DT_RIGHT); //progress time

            FillBoxWithBitmap(hdc, VIDEO_PLAYSTATUS_PINT_X, VIDEO_PLAYSTATUS_PINT_Y, VIDEO_PLAYSTATUS_PINT_W, VIDEO_PLAYSTATUS_PINT_H, &playstatus_bmap[play_status]);
            FillBoxWithBitmap(hdc, VIDEO_PLAYPREV_PINT_X, VIDEO_PLAYPREV_PINT_Y, VIDEO_PLAYPREV_PINT_W, VIDEO_PLAYPREV_PINT_H, &playprev_bmap);
            FillBoxWithBitmap(hdc, VIDEO_PLAYNEXT_PINT_X, VIDEO_PLAYNEXT_PINT_Y, VIDEO_PLAYNEXT_PINT_W, VIDEO_PLAYNEXT_PINT_H, &playnext_bmap);
            snprintf(filenum_str, sizeof(filenum_str), "%d/%d", file_select + 1, file_total);
            SelectFont(hdc, logfont_title);
            DrawText(hdc, filenum_str, -1, &msg_rcFileNum, DT_TOP | DT_CENTER);
        }
        EndPaint(hWnd, hdc);
        break;
    }
    case MSG_MEDIA_UPDATE:
        //printf("MSG_MEDIA_UPDATE cmd = %d, val = %d\n", wParam, lParam);
        if (wParam == MEDIA_CMD_TOTAL_TIME)
        {
            total_time = lParam;
            InvalidateRect(hWnd, &msg_rcTime, TRUE);
            InvalidateRect(hWnd, &msg_rcProBar, TRUE);
            InvalidateRect(hWnd, &msg_rcDialog, TRUE);
        }
        else if (wParam == MEDIA_CMD_CUR_TIME)
        {
            cur_time = lParam;
            InvalidateRect(hWnd, &msg_rcTime, TRUE);
            InvalidateRect(hWnd, &msg_rcProBar, TRUE);
            InvalidateRect(hWnd, &msg_rcDialog, TRUE);
        }
        else if (wParam == MEDIA_CMD_END)
        {
            media_exit();
            EndDialog(hWnd, wParam);
        }
        break;
    case MSG_DESTROY:
        KillTimer(hWnd, _ID_TIMER_VIDEOPLAY_HW);
        unloadres();
        EnableScreenAutoOff();
        return 0;
    case MSG_LBUTTONDOWN:
        touch_pos_down.x = LOSWORD(lParam);
        touch_pos_down.y = HISWORD(lParam);
        printf("%s MSG_LBUTTONDOWN x %d, y %d\n", __func__, touch_pos_down.x, touch_pos_down.y);
        break;
    case MSG_LBUTTONUP:
    {
        touch_pos_up.x = LOSWORD(lParam);
        touch_pos_up.y = HISWORD(lParam);
        printf("%s MSG_LBUTTONUP x %d, y %d\n", __func__, touch_pos_up.x, touch_pos_up.y);

        if ((touch_pos_down.x - touch_pos_up.x) > SLIDE_DISTANCE && ui_hide_cnt != UI_HIDE_TIME)
        {
            printf("seek back\n");
            videoplayhw_enter(hWnd, (touch_pos_down.x - touch_pos_up.x), 10);
        }
        else if ((touch_pos_up.x - touch_pos_down.x) > SLIDE_DISTANCE && ui_hide_cnt != UI_HIDE_TIME)
        {
            printf("seek forward\n");
            videoplayhw_enter(hWnd, (touch_pos_up.x - touch_pos_down.x), 11);
        }
        else
        {
            int witch_button = check_button(touch_pos_up.x, touch_pos_up.y);
            videoplayhw_enter(hWnd, wParam, witch_button);
        }
        touch_pos_old.x = touch_pos_up.x;
        touch_pos_old.y = touch_pos_up.y;
        InvalidateRect(hWnd, &msg_rcDialog, TRUE);
        break;
    }
    }
    return DefaultDialogProc(hWnd, message, wParam, lParam);
}

void creat_videoplay_hw_dialog(HWND hWnd, struct directory_node *node)
{
    DLGTEMPLATE PicPreViewDlg = {WS_VISIBLE, WS_EX_NONE | WS_EX_AUTOSECONDARYDC,
                                 0, 0,
                                 LCD_W, LCD_H,
                                 DESKTOP_DLG_STRING, 0, 0, 0, NULL, 0
                                };

    if (node == NULL)
        return;
    DisableScreenAutoOff();
    file_select = node->file_sel;
    file_total = node->total;
    dir_node = node;
    cur_time = 0;
    total_time = 0;
    play_status = 1;
    cur_file_node = get_cur_file_node(file_select);

    DialogBoxIndirectParam(&PicPreViewDlg, hWnd, videoplay_hw_dialog_proc, 0L);
}
