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
#define WHOLE_BUTTON_NUM 2

static int batt = 0;
static int autopoweroff_cnt = 50;
static int run_lowpower = 0, run_poweroff = 0;
static int create_type = 0;

static touch_pos touch_pos_down, touch_pos_up, touch_pos_old;

static const GAL_Rect msg_rcArrow[] =
{
    {TIME_INPUT_OK_X, TIME_INPUT_OK_Y, TIME_INPUT_OK_W, TIME_INPUT_OK_H},
    {TIME_INPUT_CANCEL_X, TIME_INPUT_CANCEL_Y, TIME_INPUT_CANCEL_W, TIME_INPUT_CANCEL_H}
};

static int is_button(int x, int y, GAL_Rect rect)
{
    return ((x <= rect.x + rect.w) && (x >= rect.x) && (y <= rect.y + rect.h) && (y >= rect.y));
}

static int check_button(int x, int y)
{
    if (is_button(x, y, msg_rcArrow[0]))
        return 0;
    if (is_button(x, y, msg_rcArrow[1]))
        return 1;
    return -1;
}

static int loadres(void)
{
    return 0;
}

static void unloadres(void)
{
}

static void poweroff(void)
{
    printf("%s\n", __func__);
    if (create_type == TYPE_LOWPOWER)
        system("halt");
    else
        system("poweroff");
}

static void menu_back(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    EndDialog(hWnd, wParam);
}

static LRESULT dialog_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
        SetTimer(hWnd, _ID_TIMER_LOWPOWER, TIMER_LOWPOWER);
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
        if (wParam == _ID_TIMER_LOWPOWER)
        {
#ifdef ENABLE_BATT
            if (batt != battery)
            {
                batt = battery;
                InvalidateRect(hWnd, &msg_rcBatt, TRUE);
            }
#endif
            if (create_type != TYPE_POWEROFF)
            {
                if (autopoweroff_cnt > 0)
                {
                    autopoweroff_cnt --;
                    InvalidateRect(hWnd, &msg_rcBatt, TRUE);
                }
                else
                {
                    poweroff();
                }
            }
        }
        break;
    }
    case MSG_PAINT:
    {
        int i;
        int page;
        int cur_page;
        struct file_node *file_node_temp;
        gal_pixel old_brush;
        gal_pixel pixle = 0xffffffff;
        gal_pixel pixle2 = 0xff5e5e5e;

        hdc = BeginPaint(hWnd);
        old_brush = SetBrushColor(hdc, pixle2);
        FillBox(hdc, 0, 0, TIME_INPUT_W, TIME_INPUT_H);
        old_brush = SetBrushColor(hdc, pixle);
        SetBkColor(hdc, COLOR_lightwhite);
        SetBkMode(hdc, BM_TRANSPARENT);
        SetTextColor(hdc, RGB2Pixel(hdc, 0xff, 0xff, 0xff));
        SelectFont(hdc, logfont);

        int arrow_i;
        char text_buf[10];
        RECT msg_rcButton;

        MoveTo(hdc, 0, TIME_INPUT_OK_Y - TIME_INPUT_OK_PADDING);
        LineTo(hdc, TIME_INPUT_W, TIME_INPUT_OK_Y - TIME_INPUT_OK_PADDING);
        MoveTo(hdc, TIME_INPUT_W / 2, TIME_INPUT_OK_Y - TIME_INPUT_OK_PADDING);
        LineTo(hdc, TIME_INPUT_W / 2, TIME_INPUT_H);

        SetBkColor(hdc, COLOR_transparent);

        msg_rcButton.left = 0;
        msg_rcButton.right = POWER_OFF_W;
        msg_rcButton.top = POWER_OFF_H / 3;
        msg_rcButton.bottom = POWER_OFF_H;

        if (create_type == TYPE_LOWPOWER)
        {
            DrawText(hdc, res_str[RES_STR_LOWPOWER], -1, &msg_rcButton, DT_CENTER);
        }
        else
        {
            DrawText(hdc, res_str[RES_STR_POWEROFF], -1, &msg_rcButton, DT_CENTER);
        }

        char cnt_buf[5];
        sprintf(cnt_buf, "%d\0", autopoweroff_cnt / 5);
        msg_rcButton.top = POWER_OFF_H / 2;
        if (create_type != TYPE_POWEROFF)
        {
            DrawText(hdc, cnt_buf, -1, &msg_rcButton, DT_CENTER);
        }

        msg_rcButton.left = TIME_INPUT_OK_X;
        msg_rcButton.right = msg_rcButton.left + TIME_INPUT_OK_W;
        msg_rcButton.top = TIME_INPUT_OK_Y;
        msg_rcButton.bottom = msg_rcButton.top + TIME_INPUT_OK_H;
        DrawText(hdc, res_str[RES_STR_OK], -1, &msg_rcButton, DT_CENTER);

        msg_rcButton.left = TIME_INPUT_CANCEL_X;
        msg_rcButton.right = msg_rcButton.left + TIME_INPUT_CANCEL_W;
        DrawText(hdc, res_str[RES_STR_CANCEL], -1, &msg_rcButton, DT_CENTER);
        SetBkColor(hdc, COLOR_transparent);

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
        case KEY_UP_FUNC:
            break;
        case KEY_DOWN_FUNC:
            break;
        case KEY_ENTER_FUNC:
            break;
        }
        break;
    case MSG_COMMAND:
    {
        break;
    }
    case MSG_DESTROY:
        KillTimer(hWnd, _ID_TIMER_LOWPOWER);
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
        int witch_button = check_button(touch_pos_up.x, touch_pos_up.y);
        if (witch_button == 0) poweroff();
        if (witch_button == 1) menu_back(hWnd, wParam, lParam);
        EnableScreenAutoOff();
        break;
    }
    }

    return DefaultDialogProc(hWnd, message, wParam, lParam);
}

void creat_poweroff_dialog(HWND hWnd, int type)
{
    DLGTEMPLATE DesktopDlg = {WS_VISIBLE, WS_EX_NONE | WS_EX_AUTOSECONDARYDC,
                              POWER_OFF_X, POWER_OFF_Y,
                              POWER_OFF_W, POWER_OFF_H,
                              DESKTOP_DLG_STRING, 0, 0, 0, NULL, 0
                             };
    create_type = type;
    switch (create_type)
    {
    case TYPE_LOWPOWER:
        if (run_lowpower)
            return;
        run_lowpower = 1;
        break;
    case TYPE_TIMING:
        if (run_poweroff)
            return;
        run_poweroff = 1;
        break;
    case TYPE_POWEROFF:
        break;
    default:
        return;
    }

    DialogBoxIndirectParam(&DesktopDlg, hWnd, dialog_proc, 0L);
}
