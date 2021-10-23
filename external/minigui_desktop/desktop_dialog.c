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
#include <sys/ioctl.h>
#include <sys/prctl.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "common.h"

static int menu_sel = -1;
static int line_sel = 1;
static int batt = 0;
int res_loaded = 0;

#define MENU_NUM        5
#define MENU_ICON_NUM   2

#define PHOTO_NUM        6
#define PHOTO_ICON_NUM   1

#define SLIDE_DISTANCE 100
#define WHOLE_BUTTON_NUM 5

static BITMAP menu_bmap[MENU_NUM][MENU_ICON_NUM];

static RECT msg_rcMusic = {MUSIC_PINT_X, MUSIC_PINT_Y, MUSIC_PINT_X + MUSIC_PINT_W, MUSIC_PINT_Y + MUSIC_PINT_H};
static RECT msg_rcPhoto = {PHOTO_PINT_X, PHOTO_PINT_Y, PHOTO_PINT_X + PHOTO_PINT_W, PHOTO_PINT_Y + PHOTO_PINT_H};
static RECT msg_rcVideo = {VIDEO_PINT_X, VIDEO_PINT_Y, VIDEO_PINT_X + VIDEO_PINT_W, VIDEO_PINT_Y + VIDEO_PINT_H};
static RECT msg_rcFolde = {FOLDE_PINT_X, FOLDE_PINT_Y, FOLDE_PINT_X + FOLDE_PINT_W, FOLDE_PINT_Y + FOLDE_PINT_H};
static RECT msg_rcSetting = {SETTING_PINT_X, SETTING_PINT_Y, SETTING_PINT_X + SETTING_PINT_W, SETTING_PINT_Y + SETTING_PINT_H};

static const GAL_Rect msg_galrcMenu[] =
{
    {MUSIC_PINT_X, MUSIC_PINT_Y, MUSIC_PINT_W, MUSIC_PINT_H},
    {PHOTO_PINT_X, PHOTO_PINT_Y, PHOTO_PINT_W, PHOTO_PINT_H},
    {VIDEO_PINT_X, VIDEO_PINT_Y, VIDEO_PINT_W, VIDEO_PINT_H},
    {FOLDE_PINT_X, FOLDE_PINT_Y, FOLDE_PINT_W, FOLDE_PINT_H},
    {SETTING_PINT_X, SETTING_PINT_Y, SETTING_PINT_W, SETTING_PINT_H},
    {PHOTO_PREVIEW_LEFT_X, PHOTO_PREVIEW_LEFT_Y, PHOTO_PREVIEW_LEFT_W, PHOTO_PREVIEW_LEFT_H},
    {PHOTO_PREVIEW_CENTER_X, PHOTO_PREVIEW_CENTER_Y, PHOTO_PREVIEW_CENTER_W, PHOTO_PREVIEW_CENTER_H},
    {PHOTO_PREVIEW_RIGHT_X, PHOTO_PREVIEW_RIGHT_Y, PHOTO_PREVIEW_RIGHT_W, PHOTO_PREVIEW_RIGHT_H}
};

static int full_screen = 0, double_click_timer = 0;
static touch_pos touch_pos_down, touch_pos_up, touch_pos_old;

static int is_button(int x, int y, GAL_Rect rect)
{
    return ((x <= rect.x + rect.w) && (x >= rect.x) && (y <= rect.y + rect.h) && (y >= rect.y));
}

static int check_button(int x, int y)
{
    int rect_num;
    for (rect_num = 0; rect_num < WHOLE_BUTTON_NUM; rect_num++)
    {
        if (is_button(x, y, msg_galrcMenu[rect_num]))
        {
            return rect_num;
        }
    }
    return -1;
}

static int loadres(void)
{
    int i, j;
    char img[128];
    char *respath = get_ui_image_path();

    for (i = 0; i < MENU_ICON_NUM; i++)
    {
        snprintf(img, sizeof(img), "%smusic%d.png", respath, i);
        //printf("%s\n", img);
        if (LoadBitmap(HDC_SCREEN, &menu_bmap[0][i], img))
            return -1;

        snprintf(img, sizeof(img), "%sphoto%d.png", respath, i);
        //printf("%s\n", img);
        if (LoadBitmap(HDC_SCREEN, &menu_bmap[1][i], img))
            return -1;

        snprintf(img, sizeof(img), "%svideo%d.png", respath, i);
        //printf("%s\n", img);
        if (LoadBitmap(HDC_SCREEN, &menu_bmap[2][i], img))
            return -1;

        snprintf(img, sizeof(img), "%sfolde%d.png", respath, i);
        //printf("%s\n", img);
        if (LoadBitmap(HDC_SCREEN, &menu_bmap[3][i], img))
            return -1;

        snprintf(img, sizeof(img), "%ssetting%d.png", respath, i);
        //printf("%s\n", img);
        if (LoadBitmap(HDC_SCREEN, &menu_bmap[4][i], img))
            return -1;
    }
    return 0;
}

static void unloadres(void)
{
    int i, j;

    for (j = 0; j < MENU_NUM; j++)
        for (i = 0; i < MENU_ICON_NUM; i ++)
            UnloadBitmap(&menu_bmap[j][i]);
}

static void desktop_enter(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch (lParam)
    {
    case 0:
        creat_browser_dialog(hWnd, FILTER_FILE_MUSIC, res_str[RES_STR_TITLE_MUSIC]);
        break;
    case 1:
        creat_browser_dialog(hWnd, FILTER_FILE_PIC, res_str[RES_STR_TITLE_PIC]);
        break;
    case 2:
        creat_browser_dialog(hWnd, FILTER_FILE_VIDEO, res_str[RES_STR_TITLE_VIDEO]);
        break;
    case 3:
        creat_browser_dialog(hWnd, FILTER_FILE_NO, res_str[RES_STR_TITLE_BROWSER]);
        break;
    case 4:
    {
        int oldstyle = get_themestyle();
        creat_setting_dialog(hWnd);
        if (oldstyle != get_themestyle())
        {
            unloadres();
            loadres();
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
        }
        break;
    }
    }
}

static LRESULT desktop_dialog_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;

    //printf("%s message = 0x%x, 0x%x, 0x%x\n", __func__, message, wParam, lParam);
    switch (message)
    {
    case MSG_INITDIALOG:
    {
        DWORD bkcolor;
        HWND hFocus = GetDlgDefPushButton(hWnd);
        loadres();
        batt = battery;
        bkcolor = GetWindowElementPixel(hWnd, WE_BGC_WINDOW);
        SetWindowBkColor(hWnd, bkcolor);
        SetTimer(hWnd, _ID_TIMER_DESKTOP, TIMER_DESKTOP);
        if (hFocus)
            SetFocus(hFocus);
        nhWnd = hWnd;
        return 0;
    }
    case MSG_TIMER:
    {
        time_flush();
        if (now_time->tm_min != last_time->tm_min || now_time->tm_hour != last_time->tm_hour)
        {
            if (timing_power_off[TIMING_NUM].status == 1 && (timing_power_off[TIMING_NUM].timing == (now_time->tm_hour * 100 + now_time->tm_min)))
            {
                printf("%s:timing for power off\n", __func__);
                creat_poweroff_dialog(hWnd, TYPE_TIMING);
            }
            InvalidateRect(hWnd, &msg_rcBg, FALSE);
        }
        if (double_click_timer > 0) double_click_timer = 0;
        if (wParam == _ID_TIMER_DESKTOP)
        {
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
    case MSG_PAINT:
    {
        int i, j;
        int page;
        gal_pixel old_brush;
        gal_pixel pixle = 0xff000000;

        hdc = BeginPaint(hWnd);
        SelectFont(hdc, logfont);
        old_brush = SetBrushColor(hdc, pixle);
        FillBoxWithBitmap(hdc, BG_PINT_X,
                          BG_PINT_Y, BG_PINT_W,
                          BG_PINT_H, &background_bmap);
        if (res_loaded)
        {
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
        }


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

//==================display volume icon============================
        if (res_loaded)
        {
            BITMAP *volume_display;

            if (get_volume() == 0) volume_display = &volume_0;
            else if (get_volume() > 0  && get_volume() <= 32)  volume_display = &volume_1;
            else if (get_volume() > 32  && get_volume() <= 66)  volume_display = &volume_2;
            else volume_display = &volume_3;

            FillBoxWithBitmap(hdc, VOLUME_PINT_X - status_bar_offset, VOLUME_PINT_Y,
                              VOLUME_PINT_W, VOLUME_PINT_H,
                              volume_display);
        }

        for (i = 0; i < MENU_NUM; i++)
        {
            if (i == menu_sel && line_sel == 1)
                FillBoxWithBitmap(hdc, msg_galrcMenu[i].x,
                                  msg_galrcMenu[i].y, msg_galrcMenu[i].w,
                                  msg_galrcMenu[i].h, &menu_bmap[i][1]);
            else
                FillBoxWithBitmap(hdc, msg_galrcMenu[i].x + MENU_ICON_ZOOM_W / 2,
                                  msg_galrcMenu[i].y + MENU_ICON_ZOOM_H, msg_galrcMenu[i].w - MENU_ICON_ZOOM_W,
                                  msg_galrcMenu[i].h - MENU_ICON_ZOOM_H, &menu_bmap[i][0]);
        }
        SetBrushColor(hdc, old_brush);
        EndPaint(hWnd, hdc);
        if (!res_loaded)
        {
            main_loadres();
            res_loaded = 1;
            InvalidateRect(hWnd, &msg_rcStatusBar, FALSE);
        }
        break;
    }
    case MSG_KEYDOWN:
        printf("%s message = 0x%x, 0x%x, 0x%x\n", __func__, message, wParam, lParam);
        switch (wParam)
        {
        case KEY_DOWN_FUNC:
        case KEY_UP_FUNC:
            line_sel = line_sel ? 0 : 1;
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            break;
        case KEY_RIGHT_FUNC:
            if (line_sel)
            {
                if (menu_sel < MENU_NUM - 1)
                    menu_sel++;
                else
                    menu_sel = 0;
            }
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            break;
        case KEY_LEFT_FUNC:
            if (line_sel)
            {
                if (menu_sel > 0)
                    menu_sel--;
                else
                    menu_sel = MENU_NUM - 1;
            }
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            break;
        case KEY_ENTER_FUNC:
            if (line_sel == 1)
            {
                switch (menu_sel)
                {
                case 0:
                    creat_browser_dialog(hWnd, FILTER_FILE_MUSIC, res_str[RES_STR_TITLE_MUSIC]);
                    break;
                case 1:
                    creat_browser_dialog(hWnd, FILTER_FILE_PIC, res_str[RES_STR_TITLE_PIC]);
                    break;
                case 2:
                    creat_browser_dialog(hWnd, FILTER_FILE_VIDEO, res_str[RES_STR_TITLE_VIDEO]);
                    break;
                case 3:
                    creat_browser_dialog(hWnd, FILTER_FILE_NO, res_str[RES_STR_TITLE_BROWSER]);
                    break;
                case 4:
                {
                    int oldstyle = get_themestyle();
                    creat_setting_dialog(hWnd);
                    if (oldstyle != get_themestyle())
                    {
                        unloadres();
                        loadres();
                        InvalidateRect(hWnd, &msg_rcBg, TRUE);
                    }
                    break;
                }
                }
            }
            break;
        }
        break;
    case MSG_COMMAND:
        break;
    case MSG_DESTROY:
        KillTimer(hWnd, _ID_TIMER_DESKTOP);
        unloadres();
        break;
    case MSG_LBUTTONDOWN:
        touch_pos_down.x = LOSWORD(lParam);
        touch_pos_down.y = HISWORD(lParam);
        printf("%s MSG_LBUTTONDOWN x %d, y %d\n", __func__, touch_pos_down.x, touch_pos_down.y);
        break;
    case MSG_LBUTTONUP:
    {
        if (get_bl_brightness() == 0)
        {
            screenon();
            break;
        }
        DisableScreenAutoOff();
        touch_pos_up.x = LOSWORD(lParam);
        touch_pos_up.y = HISWORD(lParam);
        printf("%s MSG_LBUTTONUP x %d, y %d\n", __func__, touch_pos_up.x, touch_pos_up.y);
        if (touch_pos_down.x - touch_pos_up.x > SLIDE_DISTANCE)
        {
            //printf("slide left\n");
            //InvalidateRect(hWnd, &msg_rcBg, TRUE);
        }
        else if (touch_pos_up.x - touch_pos_down.x > SLIDE_DISTANCE)
        {
            //printf("slide right\n");
            //InvalidateRect(hWnd, &msg_rcBg, TRUE);
        }
        else
        {
            int witch_button = check_button(touch_pos_up.x, touch_pos_up.y);
            if (witch_button >= 0 && witch_button < WHOLE_BUTTON_NUM)
            {
                line_sel = 1;
                menu_sel = witch_button;
                desktop_enter(hWnd, wParam, witch_button);
            }
        }
        touch_pos_old.x = touch_pos_up.x;
        touch_pos_old.y = touch_pos_up.y;
        EnableScreenAutoOff();
        break;
    }
    }

    return DefaultDialogProc(hWnd, message, wParam, lParam);
}

void creat_desktop_dialog(HWND hWnd)
{
    DLGTEMPLATE DesktopDlg = {WS_VISIBLE, WS_EX_NONE | WS_EX_AUTOSECONDARYDC,
                              0, 0,
                              LCD_W, LCD_H,
                              DESKTOP_DLG_STRING, 0, 0, 0, NULL, 0
                             };
    DialogBoxIndirectParam(&DesktopDlg, HWND_DESKTOP, desktop_dialog_proc, 0L);
}
