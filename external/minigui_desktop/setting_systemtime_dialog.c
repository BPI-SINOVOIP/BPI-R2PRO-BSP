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

#define SLIDE_DISTANCE 100
#define SYSTEMTIME_WHOLE_BUTTON_NUM 7
#define SYSTEMTIME_NUM    (RES_STR_SYSTEMTIME_OFF2 - RES_STR_SYNC_NET_TIME + 1)

static BITMAP list_sel_bmap;
static BITMAP seldot_bmap[2];
static int list_sel = 0;
static int batt = 0;

static int on_1 = 0;
static int off_1 = 0;
static int on_2 = 0;
static int off_2 = 0;
static int on_3 = 0;
static int off_3 = 0;

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

    snprintf(img, sizeof(img), "%slist_sel.png", respath);
    if (LoadBitmap(HDC_SCREEN, &list_sel_bmap, img))
        return -1;

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

    UnloadBitmap(&list_sel_bmap);
    for (i = 0; i < 2; i++)
    {
        UnloadBitmap(&seldot_bmap[i]);
    }
}

static void systemtime_enter(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    struct tm tar_time;
    tar_time.tm_year = 0;
    tar_time.tm_mon = 0;
    tar_time.tm_mday = 0;
    tar_time.tm_hour = 0;
    tar_time.tm_min = 0;
    FILE *fp;
    char time_fm_buf[100];
    switch (lParam)
    {
    case 0:
        set_if_sync_net_time(!get_if_sync_net_time());
        InvalidateRect(hWnd, &msg_rcBg, TRUE);
        break;
    case 1:
        if (get_if_sync_net_time()) break;
        InvalidateRect(hWnd, &msg_rcBg, TRUE);
        creat_time_input_dialog(hWnd, INPUT_DATE, now_time);
        break;
    case 2:
        if (get_if_sync_net_time()) break;
        InvalidateRect(hWnd, &msg_rcBg, TRUE);
        if (get_time_format() == USE_24_HOUR_FORMAT)
            creat_time_input_dialog(hWnd, INPUT_TIME_24, now_time);
        else
            creat_time_input_dialog(hWnd, INPUT_TIME_12, now_time);
        break;
    case 3:
        set_time_format(!get_time_format());
        status_bar_offset = (get_time_format() == USE_24_HOUR_FORMAT) ? 0 : STATUS_BAR_ICO_OFFSET;
        time_flush();
        InvalidateRect(hWnd, &msg_rcBg, TRUE);
        break;
    case 4:
        if (touch_pos_up.x < (LCD_W * 3 / 4))
        {
            on_1 = !on_1;
            timing_power_on[0].status = on_1;
            timing_power_on_set();
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
        }
        else
        {
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            tar_time.tm_hour = (int)timing_power_on[0].timing / 100;
            tar_time.tm_min = (int)timing_power_on[0].timing % 100;
            if (get_time_format())
                creat_time_input_dialog(hWnd, INPUT_TIME_TIMING_24, &tar_time);
            else
                creat_time_input_dialog(hWnd, INPUT_TIME_TIMING_12, &tar_time);
        }
        break;
    case 5:
        if (touch_pos_up.x < (LCD_W * 3 / 4))
        {
            off_1 = !off_1;
            timing_power_off[0].status = off_1;
            timing_power_off_set();
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
        }
        else
        {
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            tar_time.tm_hour = (int)timing_power_off[0].timing / 100;
            tar_time.tm_min = (int)timing_power_off[0].timing % 100;
            if (get_time_format())
                creat_time_input_dialog(hWnd, INPUT_TIME_TIMING_24 + 3, &tar_time);
            else
                creat_time_input_dialog(hWnd, INPUT_TIME_TIMING_12 + 3, &tar_time);
        }
        break;
    case 6:
        if (touch_pos_up.x < (LCD_W * 3 / 4))
        {
            on_2 = !on_2;
            timing_power_on[1].status = on_2;
            timing_power_on_set();
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
        }
        else
        {
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            tar_time.tm_hour = (int)timing_power_on[1].timing / 100;
            tar_time.tm_min = (int)timing_power_on[1].timing % 100;
            if (get_time_format())
                creat_time_input_dialog(hWnd, INPUT_TIME_TIMING_24 + 1, &tar_time);
            else
                creat_time_input_dialog(hWnd, INPUT_TIME_TIMING_12 + 1, &tar_time);
        }
        break;
    case 7:
        if (touch_pos_up.x < (LCD_W * 3 / 4))
        {
            off_2 = !off_2;
            timing_power_off[1].status = off_2;
            timing_power_off_set();
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
        }
        else
        {
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            tar_time.tm_hour = (int)timing_power_off[1].timing / 100;
            tar_time.tm_min = (int)timing_power_off[1].timing % 100;
            if (get_time_format())
                creat_time_input_dialog(hWnd, INPUT_TIME_TIMING_24 + 4, &tar_time);
            else
                creat_time_input_dialog(hWnd, INPUT_TIME_TIMING_12 + 4, &tar_time);
        }
        break;
    default:
        break;
    }
}

static void menu_back(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    EndDialog(hWnd, wParam);
}

static LRESULT setting_systemtime_dialog_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
        list_sel = 0;
        on_1 = timing_power_on[0].status;
        on_2 = timing_power_on[1].status;
        on_3 = timing_power_on[2].status;
        off_1 = timing_power_off[0].status;
        off_2 = timing_power_off[1].status;
        off_3 = timing_power_off[2].status;
        SetTimer(hWnd, _ID_TIMER_SETTING_SYSTEMTIME, TIMER_SETTING_SYSTEMTIME);
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
        if (wParam == TIMER_SETTING_SYSTEMTIME)
        {
#ifdef ENABLE_BATT
            if (batt != battery)
            {
                batt = battery;
                InvalidateRect(hWnd, &msg_rcStatusBar, TRUE);
            }
#endif
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
        DrawText(hdc, res_str[RES_STR_TITLE_SYSTEMTIME], -1, &msg_rcTitle, DT_TOP);
        FillBox(hdc, TITLE_LINE_PINT_X, TITLE_LINE_PINT_Y, TITLE_LINE_PINT_W, TITLE_LINE_PINT_H);

        page = (SYSTEMTIME_NUM + SETTING_NUM_PERPAGE - 1) / SETTING_NUM_PERPAGE;
        cur_page = list_sel / SETTING_NUM_PERPAGE;

        char time_temp[6];

        for (i = 0; i < SYSTEMTIME_NUM; i++)
        {
            RECT msg_rcText;
            RECT msg_rcTimeformat;

            if ((cur_page * SETTING_NUM_PERPAGE + i) >= SYSTEMTIME_NUM)
                break;

            msg_rcText.left = SETTING_LIST_STR_PINT_X;
            msg_rcText.top = SETTING_LIST_STR_PINT_Y + SETTING_LIST_STR_PINT_SPAC * i;
            msg_rcText.right = LCD_W - msg_rcText.left;
            msg_rcText.bottom = msg_rcText.top + SETTING_LIST_STR_PINT_H;

            if (i == list_sel % SETTING_NUM_PERPAGE)
                FillBoxWithBitmap(hdc, 0, msg_rcText.top - 9, LCD_W, SETTING_LIST_SEL_PINT_H, &list_sel_bmap);

            switch (RES_STR_SYNC_NET_TIME + cur_page * SETTING_NUM_PERPAGE + i)
            {
            case RES_STR_SYNC_NET_TIME:
                DrawText(hdc, res_str[RES_STR_SYNC_NET_TIME], -1, &msg_rcText, DT_TOP);
                if (get_if_sync_net_time()) DrawText(hdc, res_str[RES_STR_ENABLE], -1, &msg_rcText, DT_RIGHT);
                else DrawText(hdc, res_str[RES_STR_DISABLE], -1, &msg_rcText, DT_RIGHT);
                break;
            case RES_STR_SYSTEMTIME_DATA:
                if (get_if_sync_net_time()) SetTextColor(hdc, RGB2Pixel(hdc, 0x5e, 0x5e, 0x5e));
                DrawText(hdc, res_str[RES_STR_SYSTEMTIME_DATA], -1, &msg_rcText, DT_TOP);
                DrawText(hdc, status_bar_date_str, -1, &msg_rcText, DT_RIGHT);
                SetTextColor(hdc, RGB2Pixel(hdc, 0xff, 0xff, 0xff));
                break;
            case RES_STR_SYSTEMTIME_TIME:
                if (get_if_sync_net_time()) SetTextColor(hdc, RGB2Pixel(hdc, 0x5e, 0x5e, 0x5e));
                DrawText(hdc, res_str[RES_STR_SYSTEMTIME_TIME], -1, &msg_rcText, DT_TOP);
                DrawText(hdc, status_bar_time_str, -1, &msg_rcText, DT_RIGHT);
                SetTextColor(hdc, RGB2Pixel(hdc, 0xff, 0xff, 0xff));
                break;
            case RES_STR_SYSTEMTIME_FORMAT:
                DrawText(hdc, res_str[RES_STR_SYSTEMTIME_FORMAT], -1, &msg_rcText, DT_TOP);
                msg_rcTimeformat = msg_rcText;
                msg_rcTimeformat.left = msg_rcTimeformat.right - 2 * REALTIME_PINT_W;
                if (get_time_format() == USE_24_HOUR_FORMAT)
                {
                    DrawText(hdc, "<", -1, &msg_rcTimeformat, DT_LEFT);
                    DrawText(hdc, res_str[RES_STR_SYSTEMTIME_FORMAT_24], -1, &msg_rcTimeformat, DT_CENTER);
                    DrawText(hdc, ">", -1, &msg_rcTimeformat, DT_RIGHT);
                }
                else
                {
                    DrawText(hdc, "<", -1, &msg_rcTimeformat, DT_LEFT);
                    DrawText(hdc, res_str[RES_STR_SYSTEMTIME_FORMAT_12], -1, &msg_rcTimeformat, DT_CENTER);
                    DrawText(hdc, ">", -1, &msg_rcTimeformat, DT_RIGHT);
                }
                break;
            case RES_STR_SYSTEMTIME_ON1:
                DrawText(hdc, res_str[RES_STR_SYSTEMTIME_ON1], -1, &msg_rcText, DT_TOP);
                if (on_1) DrawText(hdc, res_str[RES_STR_ENABLE], -1, &msg_rcText, DT_CENTER);
                else DrawText(hdc, res_str[RES_STR_DISABLE], -1, &msg_rcText, DT_CENTER);
                if (get_time_format() == USE_24_HOUR_FORMAT)
                    sprintf(time_temp, "%02d:%02d", (int)(timing_power_on[0].timing / 100), timing_power_on[0].timing % 100);
                else
                {
                    if (timing_power_on[0].timing > 1200)
                        sprintf(time_temp, "%02d:%02d PM", (int)(timing_power_on[0].timing / 100) % 12, timing_power_on[0].timing % 100);
                    else
                        sprintf(time_temp, "%02d:%02d AM", (int)(timing_power_on[0].timing / 100), timing_power_on[0].timing % 100);
                }
                DrawText(hdc, time_temp, -1, &msg_rcText, DT_RIGHT);
                break;
            case RES_STR_SYSTEMTIME_ON2:
                break;
                DrawText(hdc, res_str[RES_STR_SYSTEMTIME_ON2], -1, &msg_rcText, DT_TOP);
                if (on_2) DrawText(hdc, res_str[RES_STR_ENABLE], -1, &msg_rcText, DT_CENTER);
                else DrawText(hdc, res_str[RES_STR_DISABLE], -1, &msg_rcText, DT_CENTER);
                if (get_time_format() == USE_24_HOUR_FORMAT)
                    sprintf(time_temp, "%02d:%02d", (int)(timing_power_on[1].timing / 100), timing_power_on[1].timing % 100);
                else
                {
                    if (timing_power_on[1].timing > 1200)
                        sprintf(time_temp, "%02d:%02d PM", (int)(timing_power_on[1].timing / 100) % 12, timing_power_on[1].timing % 100);
                    else
                        sprintf(time_temp, "%02d:%02d AM", (int)(timing_power_on[1].timing / 100), timing_power_on[1].timing % 100);
                }
                DrawText(hdc, time_temp, -1, &msg_rcText, DT_RIGHT);
                break;

            case RES_STR_SYSTEMTIME_OFF1:
                DrawText(hdc, res_str[RES_STR_SYSTEMTIME_OFF1], -1, &msg_rcText, DT_TOP);
                if (off_1) DrawText(hdc, res_str[RES_STR_ENABLE], -1, &msg_rcText, DT_CENTER);
                else DrawText(hdc, res_str[RES_STR_DISABLE], -1, &msg_rcText, DT_CENTER);
                if (get_time_format() == USE_24_HOUR_FORMAT)
                    sprintf(time_temp, "%02d:%02d", (int)(timing_power_off[0].timing / 100), timing_power_off[0].timing % 100);
                else
                {
                    if (timing_power_off[0].timing > 1200)
                        sprintf(time_temp, "%02d:%02d PM", (int)(timing_power_off[0].timing / 100) % 12, timing_power_off[0].timing % 100);
                    else
                        sprintf(time_temp, "%02d:%02d AM", (int)(timing_power_off[0].timing / 100), timing_power_off[0].timing % 100);
                }
                DrawText(hdc, time_temp, -1, &msg_rcText, DT_RIGHT);
                break;

            case RES_STR_SYSTEMTIME_OFF2:
                break;
                DrawText(hdc, res_str[RES_STR_SYSTEMTIME_OFF2], -1, &msg_rcText, DT_TOP);
                if (off_2) DrawText(hdc, res_str[RES_STR_ENABLE], -1, &msg_rcText, DT_CENTER);
                else DrawText(hdc, res_str[RES_STR_DISABLE], -1, &msg_rcText, DT_CENTER);
                if (get_time_format() == USE_24_HOUR_FORMAT)
                    sprintf(time_temp, "%02d:%02d", (int)(timing_power_off[1].timing / 100), timing_power_off[1].timing % 100);
                else
                {
                    if (timing_power_off[1].timing > 1200)
                        sprintf(time_temp, "%02d:%02d PM", (int)(timing_power_off[1].timing / 100) % 12, timing_power_off[1].timing % 100);
                    else
                        sprintf(time_temp, "%02d:%02d AM", (int)(timing_power_off[1].timing / 100), timing_power_off[1].timing % 100);
                }
                DrawText(hdc, time_temp, -1, &msg_rcText, DT_RIGHT);
                break;
            }
        }

        if (page > 1)
        {
            for (i = 0; i < page; i++)
            {
                int x;
                if (page == 1)
                    x =  SETTING_PAGE_DOT_X;
                else if (page % 2)
                    x =  SETTING_PAGE_DOT_X - page / 2 * SETTING_PAGE_DOT_SPAC;
                else
                    x =  SETTING_PAGE_DOT_X - page / 2 * SETTING_PAGE_DOT_SPAC + SETTING_PAGE_DOT_SPAC / 2;

                if (i == cur_page)
                    FillCircle(hdc, x + i * SETTING_PAGE_DOT_SPAC, SETTING_PAGE_DOT_Y, SETTING_PAGE_DOT_DIA);
                else
                    Circle(hdc, x + i * SETTING_PAGE_DOT_SPAC, SETTING_PAGE_DOT_Y, SETTING_PAGE_DOT_DIA);
            }
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
            if (list_sel < (SYSTEMTIME_NUM - 1))
                list_sel++;
            else
                list_sel = 0;
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            break;
        case KEY_UP_FUNC:
            if (list_sel > 0)
                list_sel--;
            else
                list_sel = SYSTEMTIME_NUM - 1;
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            break;
        case KEY_ENTER_FUNC:
            break;
        }
        break;
    case MSG_COMMAND:
        break;
    case MSG_DESTROY:
        KillTimer(hWnd, _ID_TIMER_SETTING_SYSTEMTIME);
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
        if (witch_button == 0) menu_back(hWnd, wParam, lParam);
        if (witch_button > 0 && witch_button < SYSTEMTIME_WHOLE_BUTTON_NUM)
        {
            list_sel = witch_button - 1;
            systemtime_enter(hWnd, wParam, list_sel);
        }

        touch_pos_old.x = touch_pos_up.x;
        touch_pos_old.y = touch_pos_up.y;
        EnableScreenAutoOff();
        break;
    }
    }
    return DefaultDialogProc(hWnd, message, wParam, lParam);
}

void creat_setting_systemtime_dialog(HWND hWnd)
{
    DLGTEMPLATE DesktopDlg = {WS_VISIBLE, WS_EX_NONE | WS_EX_AUTOSECONDARYDC,
                              0, 0,
                              LCD_W, LCD_H,
                              DESKTOP_DLG_STRING, 0, 0, 0, NULL, 0
                             };
    //DesktopDlg.controls = DesktopCtrl;

    DialogBoxIndirectParam(&DesktopDlg, hWnd, setting_systemtime_dialog_proc, 0L);
}
