#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "common.h"

static BITMAP arrow_up, arrow_down;
static int button_sel = 0, input_type;
static struct tm *res_time;
static struct tm temp_time;

static touch_pos touch_pos_down, touch_pos_up, touch_pos_old;

static int day_of_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static int day_of_month_leap[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
HWND last_hWnd;

static const GAL_Rect msg_rcArrow[] =
{
    {TIME_INPUT_OK_X, TIME_INPUT_OK_Y, TIME_INPUT_OK_W, TIME_INPUT_OK_H},
    {TIME_INPUT_CANCEL_X, TIME_INPUT_CANCEL_Y, TIME_INPUT_CANCEL_W, TIME_INPUT_CANCEL_H},
    {TIME_INPUT_ARROW_X_12_1, TIME_INPUT_ARROW_Y_1, TIME_INPUT_ARROW_W, TIME_INPUT_ARROW_H},
    {TIME_INPUT_ARROW_X_12_2, TIME_INPUT_ARROW_Y_1, TIME_INPUT_ARROW_W, TIME_INPUT_ARROW_H},
    {TIME_INPUT_ARROW_X_12_3, TIME_INPUT_ARROW_Y_1, TIME_INPUT_ARROW_W, TIME_INPUT_ARROW_H},
    {TIME_INPUT_ARROW_X_12_1, TIME_INPUT_ARROW_Y_2, TIME_INPUT_ARROW_W, TIME_INPUT_ARROW_H},
    {TIME_INPUT_ARROW_X_12_2, TIME_INPUT_ARROW_Y_2, TIME_INPUT_ARROW_W, TIME_INPUT_ARROW_H},
    {TIME_INPUT_ARROW_X_12_3, TIME_INPUT_ARROW_Y_2, TIME_INPUT_ARROW_W, TIME_INPUT_ARROW_H},
    {TIME_INPUT_ARROW_X_24_1, TIME_INPUT_ARROW_Y_1, TIME_INPUT_ARROW_W, TIME_INPUT_ARROW_H},
    {TIME_INPUT_ARROW_X_24_2, TIME_INPUT_ARROW_Y_1, TIME_INPUT_ARROW_W, TIME_INPUT_ARROW_H},
    {TIME_INPUT_ARROW_X_24_1, TIME_INPUT_ARROW_Y_2, TIME_INPUT_ARROW_W, TIME_INPUT_ARROW_H},
    {TIME_INPUT_ARROW_X_24_2, TIME_INPUT_ARROW_Y_2, TIME_INPUT_ARROW_W, TIME_INPUT_ARROW_H}
};

static int is_button(int x, int y, GAL_Rect rect)
{
    return ((x <= rect.x + rect.w) && (x >= rect.x) && (y <= rect.y + rect.h) && (y >= rect.y));
}

static int check_button(int x, int y)
{
    int i;
    for (i = 0; i < 2; i++)
    {
        if (is_button(x, y, msg_rcArrow[i]))
            return i;
    }
    switch (input_type & 0xf0)
    {
    case INPUT_DATE:
    case INPUT_TIME_12:
    case INPUT_TIME_TIMING_12:
        for (i = 2; i < 8; i++)
        {
            if (is_button(x, y, msg_rcArrow[i]))
                return i;
        }
        break;
    case INPUT_TIME_24:
    case INPUT_TIME_TIMING_24:
        for (i = 8; i < 12; i++)
        {
            if (is_button(x, y, msg_rcArrow[i]))
                return i;
        }
        break;
    }
    return -1;
}

static int loadres(void)
{
    int i;
    char img[128];
    char *respath = get_ui_image_path();

    snprintf(img, sizeof(img), "%sarrow_up.png", respath);
    if (LoadBitmap(HDC_SCREEN, &arrow_up, img))
        return -1;

    snprintf(img, sizeof(img), "%sarrow_down.png", respath);
    if (LoadBitmap(HDC_SCREEN, &arrow_down, img))
        return -1;

    return 0;
}

static void unloadres(void)
{
    UnloadBitmap(&arrow_up);
    UnloadBitmap(&arrow_down);
}

static void time_input_enter(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    struct tm tar_time;
    switch (input_type & 0xf0)
    {
    case INPUT_DATE:
        switch (lParam)
        {
        case 0:
            printf("ok\n");
            time_set(temp_time);
            printf("%s:return time:%d-%d-%d %d:%d\n", __func__, temp_time.tm_year, temp_time.tm_mon, temp_time.tm_mday,
                   temp_time.tm_hour, temp_time.tm_min);
            EndDialog(hWnd, wParam);
            break;
        case 1:
            printf("cancel\n");
            EndDialog(hWnd, wParam);
            break;
        case 2:
            printf("year up\n");
            if (temp_time.tm_year < 200)
                temp_time.tm_year++;
            break;
        case 5:
            printf("year down\n");
            if (temp_time.tm_year > 0)
                temp_time.tm_year--;
            break;
        case 3:
            printf("month up\n");
            if (temp_time.tm_mon < 11)
                temp_time.tm_mon++;
            if (temp_time.tm_mday > day_of_month[temp_time.tm_mon])
                temp_time.tm_mday = day_of_month[temp_time.tm_mon];
            break;
        case 6:
            printf("month down\n");
            if (temp_time.tm_mon > 0)
                temp_time.tm_mon--;
            if (temp_time.tm_mday > day_of_month[temp_time.tm_mon])
                temp_time.tm_mday = day_of_month[temp_time.tm_mon];
            break;
        case 4:
            printf("day up\n");
            if ((temp_time.tm_year % 4) == 0)
            {
                if (temp_time.tm_mday < day_of_month_leap[temp_time.tm_mon])
                    temp_time.tm_mday++;
            }
            else
            {
                if (temp_time.tm_mday < day_of_month[temp_time.tm_mon])
                    temp_time.tm_mday++;
            }
            break;
        case 7:
            printf("day down\n");
            if (temp_time.tm_mday > 1)
                temp_time.tm_mday--;
            break;
        }
        break;
    case INPUT_TIME_TIMING_12:
    case INPUT_TIME_12:
        switch (lParam)
        {
        case 0:
            printf("ok\n");
            if ((input_type & 0xf0) == INPUT_TIME_12)
                time_set(temp_time);
            else
                timing_set(temp_time, input_type & 0x0f);
            printf("%s:return time:%d-%d-%d %d:%d\n", __func__, temp_time.tm_year, temp_time.tm_mon, temp_time.tm_mday,
                   temp_time.tm_hour, temp_time.tm_min);
            EndDialog(hWnd, wParam);
            break;
        case 1:
            printf("cancel\n");
            EndDialog(hWnd, wParam);
            break;
        case 2:
            printf("hour up\n");
            if (temp_time.tm_hour < 23)
                temp_time.tm_hour++;
            break;
        case 5:
            printf("hour down\n");
            if (temp_time.tm_hour > 0)
                temp_time.tm_hour--;
            break;
        case 3:
            printf("minute up\n");
            if (temp_time.tm_min < 59)
                temp_time.tm_min++;
            break;
        case 6:
            printf("minute down\n");
            if (temp_time.tm_min > 0)
                temp_time.tm_min--;
            break;
        case 4:
            printf("am\n");
            if (temp_time.tm_hour > 12)
                temp_time.tm_hour -= 12;
            break;
        case 7:
            printf("pm\n");
            if (temp_time.tm_hour < 12)
                temp_time.tm_hour += 12;
            break;
        }
        break;
    case INPUT_TIME_TIMING_24:
    case INPUT_TIME_24:
        switch (lParam)
        {
        case 0:
            printf("ok\n");
            if ((input_type & 0xf0) == INPUT_TIME_24)
                time_set(temp_time);
            else
                timing_set(temp_time, input_type & 0x0f);
            printf("%s:return time:%d-%d-%d %d:%d\n", __func__, temp_time.tm_year, temp_time.tm_mon, temp_time.tm_mday,
                   temp_time.tm_hour, temp_time.tm_min);
            EndDialog(hWnd, wParam);
            break;
        case 1:
            printf("cancel\n");
            EndDialog(hWnd, wParam);
            break;
        case 8:
            printf("hour up\n");
            if (temp_time.tm_hour < 23)
                temp_time.tm_hour++;
            break;
        case 10:
            printf("hour down\n");
            if (temp_time.tm_hour > 0)
                temp_time.tm_hour--;
            break;
        case 9:
            printf("minute up\n");
            if (temp_time.tm_min < 59)
                temp_time.tm_min++;
            break;
        case 11:
            printf("minute down\n");
            if (temp_time.tm_min > 0)
                temp_time.tm_min--;
            break;
        }
        break;
    }
    InvalidateRect(last_hWnd, &msg_rcBg, TRUE);
    InvalidateRect(hWnd, &msg_rcBg, TRUE);
}

static LRESULT time_input_dialog_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
        button_sel = 0;
        //SetTimer(hWnd, _ID_TIMER_SETTING_SYSTEMTIME, TIMER_SETTING_SYSTEMTIME);
        nhWnd = hWnd;
        return 0;
    }
    case MSG_TIMER:
    {
        static int dialog_last_time = 60;
        if (now_time->tm_min != dialog_last_time)
        {
            dialog_last_time = now_time->tm_min;
            //InvalidateRect(hWnd, &msg_rcBg, FALSE);
            InvalidateRect(last_hWnd, &msg_rcBg, FALSE);
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
        msg_rcButton.left = TIME_INPUT_OK_X;
        msg_rcButton.right = msg_rcButton.left + TIME_INPUT_OK_W;
        msg_rcButton.top = TIME_INPUT_OK_Y;
        msg_rcButton.bottom = msg_rcButton.top + TIME_INPUT_OK_H;
        DrawText(hdc, res_str[RES_STR_OK], -1, &msg_rcButton, DT_CENTER);

        msg_rcButton.left = TIME_INPUT_CANCEL_X;
        msg_rcButton.right = msg_rcButton.left + TIME_INPUT_CANCEL_W;
        DrawText(hdc, res_str[RES_STR_CANCEL], -1, &msg_rcButton, DT_CENTER);
        SetBkColor(hdc, COLOR_transparent);
        switch (input_type & 0xf0)
        {
        case INPUT_DATE:
        {
            for (arrow_i = 2; arrow_i < 5; arrow_i++)
            {
                FillBoxWithBitmap(hdc, msg_rcArrow[arrow_i].x, msg_rcArrow[arrow_i].y,
                                  msg_rcArrow[arrow_i].w, msg_rcArrow[arrow_i].h, &arrow_up);
            }
            for (arrow_i = 5; arrow_i < 8; arrow_i++)
            {
                FillBoxWithBitmap(hdc, msg_rcArrow[arrow_i].x, msg_rcArrow[arrow_i].y,
                                  msg_rcArrow[arrow_i].w, msg_rcArrow[arrow_i].h, &arrow_down);
            }
            msg_rcButton.left = TIME_TIME_X_12_1;
            msg_rcButton.right = msg_rcButton.left + TIME_TEXT_W;
            msg_rcButton.top = TIME_TEXT_Y;
            msg_rcButton.bottom = msg_rcButton.top + TIME_TEXT_H;
            sprintf(text_buf, "%d", temp_time.tm_year + 1900);
            DrawText(hdc, text_buf, -1, &msg_rcButton, DT_CENTER);

            msg_rcButton.left = TIME_TIME_X_12_2;
            msg_rcButton.right = msg_rcButton.left + TIME_TEXT_W;
            sprintf(text_buf, "%d", temp_time.tm_mon + 1);
            DrawText(hdc, text_buf, -1, &msg_rcButton, DT_CENTER);

            msg_rcButton.left = TIME_TIME_X_12_3;
            msg_rcButton.right = msg_rcButton.left + TIME_TEXT_W;
            sprintf(text_buf, "%d", temp_time.tm_mday);
            DrawText(hdc, text_buf, -1, &msg_rcButton, DT_CENTER);
            break;
        }
        case INPUT_TIME_TIMING_12:
        case INPUT_TIME_12:
            for (arrow_i = 2; arrow_i < 5; arrow_i++)
            {
                FillBoxWithBitmap(hdc, msg_rcArrow[arrow_i].x, msg_rcArrow[arrow_i].y,
                                  msg_rcArrow[arrow_i].w, msg_rcArrow[arrow_i].h, &arrow_up);
            }
            for (arrow_i = 5; arrow_i < 8; arrow_i++)
            {
                FillBoxWithBitmap(hdc, msg_rcArrow[arrow_i].x, msg_rcArrow[arrow_i].y,
                                  msg_rcArrow[arrow_i].w, msg_rcArrow[arrow_i].h, &arrow_down);
            }
            msg_rcButton.left = TIME_TIME_X_12_1;
            msg_rcButton.right = msg_rcButton.left + TIME_TEXT_W;
            msg_rcButton.top = TIME_TEXT_Y;
            msg_rcButton.bottom = msg_rcButton.top + TIME_TEXT_H;
            sprintf(text_buf, "%d", temp_time.tm_hour % 12);
            DrawText(hdc, text_buf, -1, &msg_rcButton, DT_CENTER);

            msg_rcButton.left = TIME_TIME_X_12_2;
            msg_rcButton.right = msg_rcButton.left + TIME_TEXT_W;
            sprintf(text_buf, "%d", temp_time.tm_min);
            DrawText(hdc, text_buf, -1, &msg_rcButton, DT_CENTER);

            msg_rcButton.left = TIME_TIME_X_12_3;
            msg_rcButton.right = msg_rcButton.left + TIME_TEXT_W;
            if (temp_time.tm_hour < 12)
                DrawText(hdc, "AM", -1, &msg_rcButton, DT_CENTER);
            else
                DrawText(hdc, "PM", -1, &msg_rcButton, DT_CENTER);
            break;
        case INPUT_TIME_TIMING_24:
        case INPUT_TIME_24:
            for (arrow_i = 8; arrow_i < 10; arrow_i++)
            {
                FillBoxWithBitmap(hdc, msg_rcArrow[arrow_i].x, msg_rcArrow[arrow_i].y,
                                  msg_rcArrow[arrow_i].w, msg_rcArrow[arrow_i].h, &arrow_up);
            }
            for (arrow_i = 10; arrow_i < 12; arrow_i++)
            {
                FillBoxWithBitmap(hdc, msg_rcArrow[arrow_i].x, msg_rcArrow[arrow_i].y,
                                  msg_rcArrow[arrow_i].w, msg_rcArrow[arrow_i].h, &arrow_down);
            }
            msg_rcButton.left = TIME_TIME_X_24_1;
            msg_rcButton.right = msg_rcButton.left + TIME_TEXT_W_24;
            msg_rcButton.top = TIME_TEXT_Y;
            msg_rcButton.bottom = msg_rcButton.top + TIME_TEXT_H;
            sprintf(text_buf, "%d", temp_time.tm_hour);
            DrawText(hdc, text_buf, -1, &msg_rcButton, DT_CENTER);
            msg_rcButton.left = TIME_TIME_X_24_2;
            msg_rcButton.right = msg_rcButton.left + TIME_TEXT_W_24;
            sprintf(text_buf, "%d", temp_time.tm_min);
            DrawText(hdc, text_buf, -1, &msg_rcButton, DT_CENTER);
            break;
        }
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
        case KEY_DOWN_FUNC:
            break;
        case KEY_UP_FUNC:
            break;
        case KEY_ENTER_FUNC:
            break;
        }
        break;
    case MSG_COMMAND:
        break;
    case MSG_DESTROY:
        //KillTimer(hWnd, _ID_TIMER_SETTING_SYSTEMTIME);
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
        time_input_enter(hWnd, wParam, witch_button);

        touch_pos_old.x = touch_pos_up.x;
        touch_pos_old.y = touch_pos_up.y;
        EnableScreenAutoOff();
        break;
    }
    }
    return DefaultDialogProc(hWnd, message, wParam, lParam);
}

void creat_time_input_dialog(HWND hWnd, int type, struct tm *tar_time)
{
    DLGTEMPLATE DesktopDlg = {WS_VISIBLE, WS_EX_NONE | WS_EX_AUTOSECONDARYDC,
                              TIME_INPUT_X, TIME_INPUT_Y,
                              TIME_INPUT_W, TIME_INPUT_H,
                              DESKTOP_DLG_STRING, 0, 0, 0, NULL, 0
                             };
    //DesktopDlg.controls = DesktopCtrl;

    input_type = type;
    res_time = tar_time;
    temp_time.tm_year = res_time->tm_year;
    temp_time.tm_mon = res_time->tm_mon;
    temp_time.tm_mday = res_time->tm_mday;
    temp_time.tm_hour = res_time->tm_hour;
    temp_time.tm_min = res_time->tm_min;
    temp_time.tm_sec = 0;
    last_hWnd = hWnd;

    DialogBoxIndirectParam(&DesktopDlg, hWnd, time_input_dialog_proc, 0L);
}


