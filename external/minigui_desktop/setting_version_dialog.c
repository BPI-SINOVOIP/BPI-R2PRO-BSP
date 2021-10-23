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

static int batt = 0;
static char *model = 0;
static char *verstion = 0;
static char *model_disp = 0;
static char *verstion_disp = 0;
static BITMAP list_sel_bmap;
static int list_sel = 0;

int update_flag = -1;
/* -1 means do not find update files yet
 *  0 means find no update file
 *  1 meas update file exist
 */

char update_cmd[128];

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
    int len;
    char img[128];
    char *respath = get_ui_image_path();

    loadversion(&model, &verstion);
    if (model)
    {
        len = strlen(model) + strlen(res_str[RES_STR_INFO_MODEL]) + 2;

        model_disp = malloc(len);
        snprintf(model_disp, len, "%s%s", res_str[RES_STR_INFO_MODEL], model);
        free(model);
        model = 0;
    }
    if (verstion)
    {
        len = strlen(verstion) + strlen(res_str[RES_STR_INFO_VERSION]) + 2;
        verstion_disp = malloc(len);
        snprintf(verstion_disp, len, "%s%s", res_str[RES_STR_INFO_VERSION], verstion);
        free(verstion);
        verstion = 0;
    }

    snprintf(img, sizeof(img), "%slist_sel.png", respath);
    if (LoadBitmap(HDC_SCREEN, &list_sel_bmap, img))
        return -1;
    return 0;
}

static void unloadres(void)
{
    UnloadBitmap(&list_sel_bmap);

    if (model_disp)
        free(model_disp);
    model_disp = 0;

    if (verstion_disp)
        free(verstion_disp);
    verstion_disp = 0;
}

static void version_enter(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch (lParam)
    {
        printf("1Param: %d\n", lParam);
    case 2:
    {
        int ret1, ret2, ret3;
        char cmd[128] = "ls /udisk/update.img" ;
        ret1 = system(cmd);
        if (ret1)
            fprintf(stderr, "no update file in udisk  ret1= %d  \n ", ret1);

        snprintf(cmd, sizeof(cmd), "ls /sdcard/update.img");
        ret2 = system(cmd);
        if (ret2)
            fprintf(stderr, "no update file in sdcard  ret2= %d  \n", ret2);

        snprintf(cmd, sizeof(cmd), "ls /userdata/update.img");
        ret3 = system(cmd);
        if (ret3)
            fprintf(stderr, "no update file in userdata  ret3= %d  \n", ret3);

        if ((ret1 == 512) && (ret2 == 512) && (ret3 == 512))
        {
            update_flag = 0;
        }
        else if (ret1 == 0 || ret2 == 0 || ret3 == 0)
        {
            update_flag = 1;
            if (!ret1)
                snprintf(update_cmd, sizeof(update_cmd), "update ota /udisk/update.img");
            else if (!ret2)
                snprintf(update_cmd, sizeof(update_cmd), "update ota /sdcard/update.img");
            else if (!ret3)
                snprintf(update_cmd, sizeof(update_cmd), "update ota /userdata/update.img");
        }
        printf("update_flag: %d \n", update_flag);
        break;
    }
    }
    InvalidateRect(hWnd, &msg_rcBg, TRUE);
}

static void menu_back(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    EndDialog(hWnd, wParam);
}

static LRESULT setting_version_dialog_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
        SetTimer(hWnd, _ID_TIMER_SETTING_VERSION, TIMER_SETTING_VERSION);
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
        if (wParam == _ID_TIMER_SETTING_VERSION)
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
            static int l;
            if (update_flag == 1)
            {
                l++;
                InvalidateRect(hWnd, &msg_rcBg, TRUE);
                if (l == 3) //delay 3s
                {
                    update_flag = -1;
                    l = 0;
                    system(update_cmd);
                }
            }
        }
        break;
    }
    case MSG_PAINT:
    {
        int i;
        gal_pixel old_brush;
        RECT msg_rcInfo;
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
        DrawText(hdc, res_str[RES_STR_TITLE_INFO], -1, &msg_rcTitle, DT_TOP);
        FillBox(hdc, TITLE_LINE_PINT_X, TITLE_LINE_PINT_Y, TITLE_LINE_PINT_W, TITLE_LINE_PINT_H);

        msg_rcInfo.left = SETTING_INFO_PINT_X;
        msg_rcInfo.top = SETTING_INFO_PINT_Y;
        msg_rcInfo.right = LCD_W - msg_rcInfo.left;
        msg_rcInfo.bottom = msg_rcInfo.top + SETTING_INFO_PINT_H;
        DrawText(hdc, model_disp, -1, &msg_rcInfo, DT_TOP);

        msg_rcInfo.top += SETTING_INFO_PINT_SPAC;
        msg_rcInfo.bottom = msg_rcInfo.top + SETTING_INFO_PINT_H;
        DrawText(hdc, verstion_disp, -1, &msg_rcInfo, DT_TOP);

        msg_rcInfo.top += SETTING_INFO_PINT_SPAC;
        msg_rcInfo.left = SETTING_INFO_PINT_X + 9;
        msg_rcInfo.bottom = msg_rcInfo.top + SETTING_INFO_PINT_H;
        DrawText(hdc, res_str[RES_STR_SYSTEM_UPGRAD], -1, &msg_rcInfo, DT_TOP);
        FillBoxWithBitmap(hdc, SETTING_INFO_PINT_X, msg_rcInfo.top - 9, 112, SETTING_LIST_SEL_PINT_H, &list_sel_bmap);

        msg_rcInfo.left = SETTING_INFO_PINT_X;
        msg_rcInfo.top = SETTING_INFO_PINT_Y + SETTING_INFO_PINT_SPAC * 4;
        msg_rcInfo.right = LCD_W ;
        msg_rcInfo.bottom = msg_rcInfo.top + SETTING_INFO_PINT_H;

        if (update_flag == 1)
        {
            DrawText(hdc, res_str[RES_STR_RECOVERY_SOON], -1, &msg_rcInfo, DT_TOP);
        }
        else if (update_flag == 0)
        {
            DrawText(hdc, res_str[RES_STR_NO_UPDATE_FILE], -1, &msg_rcInfo, DT_TOP);
            update_flag = -1;
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
        case KEY_UP_FUNC:
            break;
        case KEY_DOWN_FUNC:
            break;
        case KEY_ENTER_FUNC:
            break;
        }
        break;
    case MSG_COMMAND:
        break;
    case MSG_DESTROY:
        KillTimer(hWnd, _ID_TIMER_SETTING_VERSION);
        unloadres();
        break;
    case MSG_LBUTTONDOWN:
        touch_pos_down.x = LOSWORD(lParam);
        touch_pos_down.y = HISWORD(lParam);
        printf("%s MSG_LBUTTONDOWN x %d, y %d\n", __func__, touch_pos_down.x, touch_pos_down.y);
        break;
    case MSG_LBUTTONUP:
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
        if (witch_button == 0)
            menu_back(hWnd, wParam, lParam);
        if (witch_button == 3)
        {
            list_sel = witch_button - 1;
            version_enter(hWnd, wParam, list_sel);
            printf("system upgrade!\n");
        }
        touch_pos_old.x = touch_pos_up.x;
        touch_pos_old.y = touch_pos_up.y;
        EnableScreenAutoOff();
        InvalidateRect(hWnd, &msg_rcBg, TRUE);
        break;
    }
    return DefaultDialogProc(hWnd, message, wParam, lParam);
}

void creat_setting_version_dialog(HWND hWnd)
{
    DLGTEMPLATE DesktopDlg = {WS_VISIBLE, WS_EX_NONE | WS_EX_AUTOSECONDARYDC,
                              0, 0,
                              LCD_W, LCD_H,
                              DESKTOP_DLG_STRING, 0, 0, 0, NULL, 0
                             };
    //DesktopDlg.controls = DesktopCtrl;

    DialogBoxIndirectParam(&DesktopDlg, hWnd, setting_version_dialog_proc, 0L);
}
