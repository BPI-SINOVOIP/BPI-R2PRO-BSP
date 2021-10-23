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

#include<sys/stat.h>
#include<sys/types.h>
#include<dirent.h>
#include <unistd.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "common.h"

#define SLIDE_DISTANCE 100
#define WHOLE_BUTTON_NUM 3

static BITMAP seldot_bmap[2];
static int batt = 0;
#define VOLUME_NUM    2

static touch_pos touch_pos_down, touch_pos_up, touch_pos_old;

static int check_button(int x, int y)
{
    if ((x <= BACK_PINT_X + BACK_PINT_W) &&
            (x >= BACK_PINT_X) &&
            (y <= BACK_PINT_Y + BACK_PINT_H) &&
            (y >= BACK_PINT_Y))
        return 0;
    if (y > SETTING_LIST_STR_PINT_Y)
        return (((y - SETTING_LIST_STR_PINT_Y) / SETTING_LIST_STR_PINT_SPAC) + 1);
    return -1;
}

static int loadres(void)
{
    int i;
    char img[128];
    char *respath = get_ui_image_path();

    for (i = 0; i < 2; i++)
    {
        snprintf(img, sizeof(img), "%sdot%d.png", respath, i);
        if (LoadBitmap(HDC_SCREEN, &seldot_bmap[i], img))
            return -1;
    }
    return 0;
}

static void unloadres(void)
{
    int i;

    for (i = 0; i < 2; i++)
    {
        UnloadBitmap(&seldot_bmap[i]);
    }
}

static void volume_enter(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    InvalidateRect(hWnd, &msg_rcBg, TRUE);
}

static void menu_back(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    EndDialog(hWnd, wParam);
}

static LRESULT setting_volume_dialog_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
        bkcolor = GetWindowElementPixel(hWnd, WE_BGC_WINDOW);
        SetWindowBkColor(hWnd, bkcolor);
        if (hFocus)
            SetFocus(hFocus);
        batt = battery;
        SetTimer(hWnd, _ID_TIMER_SETTING_VOLUME, TIMER_SETTING_VOLUME);
        nhWnd = hWnd;
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
        if (wParam == _ID_TIMER_SETTING_VOLUME)
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
        int i;
        gal_pixel old_brush;
        gal_pixel pixle = 0xffffffff;

        hdc = BeginPaint(hWnd);
        old_brush = SetBrushColor(hdc, pixle);
        FillBoxWithBitmap(hdc, BG_PINT_X,
                          BG_PINT_Y, BG_PINT_W,
                          BG_PINT_H, &background_bmap);
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

//==================display volume icon============================
        BITMAP *volume_display;

        if (get_volume() == 0) volume_display = &volume_0;
        else if (get_volume() > 0  && get_volume() <= 32)    volume_display = &volume_1;
        else if (get_volume() > 32  && get_volume() <= 66)  volume_display = &volume_2;
        else volume_display = &volume_3;

        FillBoxWithBitmap(hdc, VOLUME_PINT_X - status_bar_offset, VOLUME_PINT_Y,
                          VOLUME_PINT_W, VOLUME_PINT_H,
                          volume_display);

        SetBkColor(hdc, COLOR_transparent);
        SetBkMode(hdc, BM_TRANSPARENT);
        SetTextColor(hdc, RGB2Pixel(hdc, 0xff, 0xff, 0xff));
        SelectFont(hdc, logfont);
        DrawText(hdc, res_str[RES_STR_TITLE_VOLUME], -1, &msg_rcTitle, DT_TOP);
        FillBox(hdc, TITLE_LINE_PINT_X, TITLE_LINE_PINT_Y, TITLE_LINE_PINT_W, TITLE_LINE_PINT_H);

        FillBox(hdc, VOLUME_LINE_X, VOLUME_LINE_Y, VOLUME_LINE_W, VOLUME_LINE_H);
        FillBoxWithBitmap(hdc, VOLUME_LINE_X + ((get_volume())*VOLUME_LINE_W / 100) - (DOT_PINT_W / 2),
                               VOLUME_LINE_Y - (DOT_PINT_H / 2) + 2,
                               DOT_PINT_W, DOT_PINT_H,
                               &seldot_bmap[1]);

        char *volume_str[4];
        snprintf(volume_str, sizeof(volume_str), "%d", get_volume());

        RECT msg_rcFilename;
        msg_rcFilename.left = VOLUME_DATA_X;
        msg_rcFilename.top = VOLUME_DATA_Y;
        msg_rcFilename.right = VOLUME_DATA_X + VOLUME_DATA_W;
        msg_rcFilename.bottom = VOLUME_DATA_Y + VOLUME_DATA_H;

        SetBkColor(hdc, COLOR_transparent);
        SetBkMode(hdc, BM_TRANSPARENT);
        SetTextColor(hdc, RGB2Pixel(hdc, 0xff, 0xff, 0xff));
        DrawText(hdc, volume_str, -1, &msg_rcFilename, DT_TOP);

        SetBrushColor(hdc, old_brush);
        EndPaint(hWnd, hdc);
        break;
    }
    case MSG_KEYDOWN:
        //printf("%s message = 0x%x, 0x%x, 0x%x\n", __func__, message, wParam, lParam);
        switch (wParam)
        {
        case KEY_EXIT_FUNC:
            EndDialog(hWnd, wParam);
            break;
        case KEY_LEFT_FUNC:
        {
            int vol_val;
            vol_val = get_volume();
            if (vol_val >= 10)
                vol_val -= 10;
            set_volume(vol_val);
            break;
        }
        case KEY_RIGHT_FUNC:
        {
            int vol_val;
            vol_val = get_volume();
            if (vol_val <= 90)
                vol_val += 10;
            set_volume(vol_val);
            break;
        }
        }
        break;
    case MSG_COMMAND:
        break;
    case MSG_DESTROY:
        KillTimer(hWnd, _ID_TIMER_SETTING_VOLUME);
        unloadres();
        break;
    case MSG_LBUTTONDOWN:
        if (get_bl_brightness() == 0)
        {
            screenon();
            break;
        }
        DisableScreenAutoOff();
        touch_pos_down.x = LOSWORD(lParam);
        touch_pos_down.y = HISWORD(lParam);
        printf("%s MSG_LBUTTONDOWN x %d, y %d\n", __func__, touch_pos_down.x, touch_pos_down.y);

        int button = check_button(touch_pos_down.x, touch_pos_down.y);
        if (button != 0)
        {
            if (touch_pos_down.x <= VOLUME_LINE_X) set_volume(0);
            else if (touch_pos_down.x >= VOLUME_LINE_X + VOLUME_LINE_W) set_volume(100);
            else set_volume((touch_pos_down.x - VOLUME_LINE_X) * 100 / VOLUME_LINE_W);
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
        }
        break;
    case MSG_LBUTTONUP:
    {
        touch_pos_up.x = LOSWORD(lParam);
        touch_pos_up.y = HISWORD(lParam);
        int button = check_button(touch_pos_down.x, touch_pos_down.y);
        if (button == 0)
            menu_back(hWnd, wParam, lParam);
        touch_pos_old.x = touch_pos_up.x;
        touch_pos_old.y = touch_pos_up.y;
        EnableScreenAutoOff();
        break;
    }
    }
    return DefaultDialogProc(hWnd, message, wParam, lParam);
}

void creat_setting_volume_dialog(HWND hWnd)
{
    DLGTEMPLATE DesktopDlg = {WS_VISIBLE, WS_EX_NONE | WS_EX_AUTOSECONDARYDC,
                              0, 0,
                              LCD_W, LCD_H,
                              DESKTOP_DLG_STRING, 0, 0, 0, NULL, 0
                             };
    //DesktopDlg.controls = DesktopCtrl;

    DialogBoxIndirectParam(&DesktopDlg, hWnd, setting_volume_dialog_proc, 0L);
}

