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

#include <DeviceIo/Rk_wifi.h>
#include "cJSON.h"

#define SLIDE_DISTANCE 100
#define WIFI_WHOLE_BUTTON_NUM 10

static BITMAP list_sel_bmap;
static BITMAP wifi_signal_3;
static BITMAP wifi_signal_2;
static BITMAP wifi_signal_1;
static BITMAP wifi_key_bmap;
static BITMAP list_sel1_bmap;

static BITMAP seldot_bmap[2];
static int list_sel = 0;
static int batt = 0;
#define WIFI_NUM    1

static touch_pos touch_pos_down, touch_pos_up, touch_pos_old;

struct wifi_info input_wifi_date = {"", ""};
struct wifi_info connect_wifi_date = {"", ""};


struct wifi_avaiable wifiavaiable_list[50];
int wifiavaiable_size;

int cur_page = 1;
int avaiable_wifi_display_mode = 0;

int wifijson_parse(char *wifijson)
{
    if (wifijson == NULL)
    {
        return 0;
    }

    cJSON *root, *item_rssi, *item_ssid, *object;
    int i, size;

    root = cJSON_Parse(wifijson);

    if (NULL == root)
    {
        return 0;
    }

    size = cJSON_GetArraySize(root);

    if (size)
    {
        for (i = 0; i < size; i++)
        {
            if (i >= 50)
            {
                cJSON_Delete(root);
                return 50;
            }

            object = cJSON_GetArrayItem(root, i);
            if (NULL == object)
            {
                cJSON_Delete(root);
                return 0;
            }

            item_rssi = cJSON_GetObjectItem(object, "rssi");
            if (item_rssi == NULL)
            {
                cJSON_Delete(root);
                return 0;
            }
            printf("cJSON_GetObjectItem: type=%d, string is %s, value=%d\n", item_rssi->type, item_rssi->string, item_rssi->valueint);
            wifiavaiable_list[i].rssi = item_rssi->valueint;
            item_ssid = cJSON_GetObjectItem(object, "ssid");
            if (item_ssid == NULL)
            {
                cJSON_Delete(root);
                return 0;
            }
            printf("cJSON_GetObjectItem: type=%d, string is %s, value=%s\n", item_ssid->type, item_ssid->string, item_ssid->valuestring);
            snprintf(wifiavaiable_list[i].ssid, 64, "%s", item_ssid->valuestring);
        }
        for (i = 0; i < size; i++)
        {
            printf("i=%d, ssid=%s,rssi=%d\n",
                   i,
                   wifiavaiable_list[i].ssid,
                   wifiavaiable_list[i].rssi);
        }
    }

    cJSON_Delete(root);
    return size;
}

pthread_t thread1;
char *message1 = "get_available_wifi";

void *get_available_wifi(void *ptr)
{
    RK_wifi_enable(1);
    RK_wifi_scan();
    wifiavaiable_size = wifijson_parse(RK_wifi_scan_r_sec(0x1F));

    while (wifiavaiable_size < 2) // wait for scan result
    {
        wifiavaiable_size = wifijson_parse(RK_wifi_scan_r_sec(0x1F));
    }

    avaiable_wifi_display_mode = 0;
    //  _print_wifi();
    if (get_wifi_state() == RK_WIFI_State_DISCONNECTED) // auto connect when pwd is exsited
    {
        int i;
        for (i = 0; i < wifiavaiable_size; i++)
        {
            if (get_wifi_psk(wifiavaiable_list[i].ssid) != NULL)
            {
                snprintf(connect_wifi_date.ssid, 64, "%s", wifiavaiable_list[i].ssid);
                //set_wifi_state(RK_WIFI_State_CONNECTING);  // to display faster
                wifi_connect_flag = 2;
                break;
            }
        }
    }
    pthread_exit((void *)0);
}

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

    snprintf(img, sizeof(img), "%skey.png", respath);
    if (LoadBitmap(HDC_SCREEN, &wifi_key_bmap, img))
        return -1;

    snprintf(img, sizeof(img), "%swifi_signal_3.png", respath);
    if (LoadBitmap(HDC_SCREEN, &wifi_signal_3, img))
        return -1;

    snprintf(img, sizeof(img), "%swifi_signal_2.png", respath);
    if (LoadBitmap(HDC_SCREEN, &wifi_signal_2, img))
        return -1;

    snprintf(img, sizeof(img), "%swifi_signal_1.png", respath);
    if (LoadBitmap(HDC_SCREEN, &wifi_signal_1, img))
        return -1;

    snprintf(img, sizeof(img), "%slist_sel1.png", respath);
    if (LoadBitmap(HDC_SCREEN, &list_sel1_bmap, img))
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
    UnloadBitmap(&wifi_key_bmap);
    UnloadBitmap(&wifi_signal_3);
    UnloadBitmap(&wifi_signal_2);
    UnloadBitmap(&wifi_signal_1);
    UnloadBitmap(&list_sel1_bmap);
    for (i = 0; i < 2; i++)
    {
        UnloadBitmap(&seldot_bmap[i]);
    }
}



static void wifi_enter(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    if (get_wifi_state() != RK_WIFI_State_OFF  &&  lParam < wifiavaiable_size)
    {
        if (!((get_wifi_state() == RK_WIFI_State_CONNECTED  ||  get_wifi_state() == RK_WIFI_State_CONNECTING) && ((lParam == 0) || (lParam == 1))))
        {
            if (avaiable_wifi_display_mode) lParam -= 2;
            snprintf(input_wifi_date.ssid, sizeof(input_wifi_date.ssid), "%s", wifiavaiable_list[lParam].ssid);
            input_dialog_type = WIFI_PWD ;
            creat_input_dialog(hWnd);
        }
    }
}

static void menu_back(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    EndDialog(hWnd, wParam);
}

static LRESULT setting_wifi_dialog_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    int page;
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
        SetTimer(hWnd, _ID_TIMER_SETTING_WIFI, TIMER_SETTING_WIFI);
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
        if (wParam == _ID_TIMER_SETTING_WIFI)
        {
#ifdef ENABLE_BATT
            if (batt != battery)
            {
                batt = battery;
            }
#endif
            if (wifi_connect_flag)
            {
                if (wifi_connect_flag == 2) //use local ssid and pwd to connect
                {
                    snprintf(connect_wifi_date.psk, 64, "%s", get_wifi_psk(connect_wifi_date.ssid));
                }
                else  //use input ssid and pwd to connect
                {
                    //      snprintf(connect_wifi_date.ssid, 128, "%s", input_wifi_date.ssid);   do this in input_dialog.c to display faster
                    //      snprintf(connect_wifi_date.psk, 128, "%s", input_wifi_date.psk);
                }
                RK_wifi_connect(connect_wifi_date.ssid, connect_wifi_date.psk);
                wifi_connect_flag = 0 ;
            }
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
        }
        break;
    }
    case MSG_PAINT:
    {
        int i, j;
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
        DrawText(hdc, res_str[RES_STR_TITLE_WIFI], -1, &msg_rcTitle, DT_TOP);
        FillBox(hdc, TITLE_LINE_PINT_X, TITLE_LINE_PINT_Y, TITLE_LINE_PINT_W, TITLE_LINE_PINT_H);

        page = (wifiavaiable_size / WIFI_NUM_PERPAGE) + 1;

        RECT msg_rcFilename;
        msg_rcFilename.left = SETTING_LIST_STR_PINT_X;
        msg_rcFilename.top = SETTING_LIST_STR_PINT_Y;
        msg_rcFilename.right = LCD_W - msg_rcFilename.left;
        msg_rcFilename.bottom = msg_rcFilename.top + SETTING_LIST_STR_PINT_H;

        SelectFont(hdc, logfont);
        DrawText(hdc, res_str[RES_STR_WIFI_CONNECTION], -1, &msg_rcFilename, DT_TOP);

        if (get_wifi_state() != RK_WIFI_State_OFF)
        {
            FillBoxWithBitmap(hdc, 0, msg_rcFilename.top - 9, LCD_W, SETTING_LIST_SEL_PINT_H, &list_sel_bmap);
            msg_rcFilename.left = LCD_W - 100;
            msg_rcFilename.top = SETTING_LIST_STR_PINT_Y;
            msg_rcFilename.right = LCD_W - SETTING_LIST_STR_PINT_X;
            msg_rcFilename.bottom = msg_rcFilename.top + SETTING_LIST_STR_PINT_H;
            SelectFont(hdc, logfont);
            DrawText(hdc, res_str[RES_STR_ENABLE], -1, &msg_rcFilename, DT_TOP);

            if (pthread_kill(thread1, 0) != ESRCH && pthread_kill(thread1, 0) !=  EINVAL) // the thread is running
            {
                msg_rcFilename.left = LCD_W / 2 - SETTING_LIST_STR_PINT_X * 2;
                msg_rcFilename.top = SETTING_LIST_STR_PINT_Y + SETTING_LIST_STR_PINT_SPAC * (2);
                msg_rcFilename.right = LCD_W;
                msg_rcFilename.bottom = msg_rcFilename.top + SETTING_LIST_STR_PINT_H;
                SelectFont(hdc, logfont);
                DrawText(hdc, res_str[RES_STR_SCANING], -1, &msg_rcFilename, DT_TOP);
            }
            else // display all avaiable wifi
            {
                if (wifiavaiable_size > 0)
                {
                    int state;
                    state = get_wifi_state();

                    int display_rssi;

                    if
                    (get_wifi_state() != RK_WIFI_State_DISCONNECTED && cur_page == 1)
                    {
                        avaiable_wifi_display_mode = 1;

                        //=============  display connected wifi info in first page =========
                        msg_rcFilename.left = SETTING_LIST_STR_PINT_X;
                        msg_rcFilename.top = SETTING_LIST_STR_PINT_Y + SETTING_LIST_STR_PINT_SPAC ;
                        msg_rcFilename.right = LCD_W - msg_rcFilename.left;
                        msg_rcFilename.bottom = msg_rcFilename.top + SETTING_LIST_STR_PINT_H;
                        FillBoxWithBitmap(hdc, 0, msg_rcFilename.top - 9, LCD_W, SETTING_LIST_SEL_PINT_H, &list_sel_bmap);

                        SelectFont(hdc, logfont);
                        DrawText(hdc, connect_wifi_date.ssid, -1, &msg_rcFilename, DT_TOP);



                        msg_rcFilename.left += 500;
                        if (get_wifi_state() == RK_WIFI_State_CONNECTED)
                            DrawText(hdc, res_str[RES_STR_CONNECTED], -1, &msg_rcFilename, DT_TOP);
                        else if (get_wifi_state() == RK_WIFI_State_CONNECTING)
                            DrawText(hdc, res_str[RES_STR_CONNECTING], -1, &msg_rcFilename, DT_TOP);
                        else if (get_wifi_state() == RK_WIFI_State_CONNECTFAILED_WRONG_KEY)
                            DrawText(hdc, res_str[RES_STR_WRONG_PWD], -1, &msg_rcFilename, DT_TOP);

                        int k;
                        for (k = 0; k < wifiavaiable_size; k++) // get wifi rssi
                        {
                            if (strcmp(wifiavaiable_list[k].ssid, connect_wifi_date.ssid) == 0)
                            {
                                display_rssi = wifiavaiable_list[k].rssi;
                                break;
                            }
                        }

                        if (display_rssi > -90)
                        {
                            FillBoxWithBitmap(hdc, LCD_W - 100, msg_rcFilename.top, WIFI_SIGNAL_PINT_W, WIFI_SIGNAL_PINT_H, &wifi_signal_3);
                        }
                        else if (display_rssi < -95)
                        {
                            FillBoxWithBitmap(hdc, LCD_W - 100, msg_rcFilename.top, WIFI_SIGNAL_PINT_W, WIFI_SIGNAL_PINT_H, &wifi_signal_1);
                        }
                        else
                        {
                            FillBoxWithBitmap(hdc, LCD_W - 100, msg_rcFilename.top, WIFI_SIGNAL_PINT_W, WIFI_SIGNAL_PINT_H, &wifi_signal_2);
                        }

                        //  FillBoxWithBitmap(hdc, LCD_W - 70, msg_rcFilename.top, WIFI_KEY_PINT_W, WIFI_KEY_PINT_H, &wifi_key_bmap);

                        msg_rcFilename.left = SETTING_LIST_STR_PINT_X;
                        msg_rcFilename.top = SETTING_LIST_STR_PINT_Y + SETTING_LIST_STR_PINT_SPAC * 2 ;
                        msg_rcFilename.right = LCD_W - msg_rcFilename.left;
                        msg_rcFilename.bottom = msg_rcFilename.top + SETTING_LIST_STR_PINT_H;
                        FillBoxWithBitmap(hdc, 0, msg_rcFilename.top - 9, LCD_W, SETTING_LIST_SEL_PINT_H, &list_sel_bmap);

                        FillBox(hdc, SETTING_LIST_STR_PINT_X, SETTING_LIST_STR_PINT_Y + SETTING_LIST_STR_PINT_SPAC * 2.2, LCD_W - SETTING_LIST_STR_PINT_X * 2, TITLE_LINE_PINT_H);
                        FillBox(hdc, SETTING_LIST_STR_PINT_X, SETTING_LIST_STR_PINT_Y + SETTING_LIST_STR_PINT_SPAC * 2.4, LCD_W - SETTING_LIST_STR_PINT_X * 2, TITLE_LINE_PINT_H);
                    }

                    int wifi_avaiable_list_head;

                    for (; j < WIFI_NUM_PERPAGE; j++)
                    {

                        if (avaiable_wifi_display_mode)
                        {
                            if (cur_page == 1)
                            {
                                if (!j) j = 2;
                            }
                            wifi_avaiable_list_head = j + (cur_page - 1) * WIFI_NUM_PERPAGE - 2;
                        }
                        else wifi_avaiable_list_head = j + (cur_page - 1) * WIFI_NUM_PERPAGE;


                        if (j + (cur_page - 1)*WIFI_NUM_PERPAGE >= wifiavaiable_size)
                            break;

                        state = get_wifi_state();

                        msg_rcFilename.left = SETTING_LIST_STR_PINT_X;
                        msg_rcFilename.top = SETTING_LIST_STR_PINT_Y + SETTING_LIST_STR_PINT_SPAC * (j + 1);
                        msg_rcFilename.right = LCD_W - msg_rcFilename.left;
                        msg_rcFilename.bottom = msg_rcFilename.top + SETTING_LIST_STR_PINT_H;
                        FillBoxWithBitmap(hdc, 0, msg_rcFilename.top - 9, LCD_W, SETTING_LIST_SEL_PINT_H, &list_sel_bmap);


                        if (j == (list_sel % WIFI_NUM_PERPAGE))
                        {
                            if (!((get_wifi_state() == RK_WIFI_State_CONNECTED  ||  get_wifi_state() == RK_WIFI_State_CONNECTING) && (cur_page == 1) && ((j == 0) || (j == 1))))
                                FillBoxWithBitmap(hdc, 0, msg_rcFilename.top - 9, LCD_W, SETTING_LIST_SEL_PINT_H, &list_sel1_bmap);
                        }
                        if (wifiavaiable_list[wifi_avaiable_list_head].rssi > -90)
                        {
                            FillBoxWithBitmap(hdc, LCD_W - 100, msg_rcFilename.top, WIFI_SIGNAL_PINT_W, WIFI_SIGNAL_PINT_H, &wifi_signal_3);
                        }
                        else if (wifiavaiable_list[wifi_avaiable_list_head].rssi < -95)
                        {
                            FillBoxWithBitmap(hdc, LCD_W - 100, msg_rcFilename.top, WIFI_SIGNAL_PINT_W, WIFI_SIGNAL_PINT_H, &wifi_signal_1);
                        }
                        else
                        {
                            FillBoxWithBitmap(hdc, LCD_W - 100, msg_rcFilename.top, WIFI_SIGNAL_PINT_W, WIFI_SIGNAL_PINT_H, &wifi_signal_2);
                        }

                        if (get_wifi_psk(wifiavaiable_list[wifi_avaiable_list_head].ssid) != NULL)
                        {
                            FillBoxWithBitmap(hdc, LCD_W - 70, msg_rcFilename.top, WIFI_KEY_PINT_W, WIFI_KEY_PINT_H, &wifi_key_bmap);
                        }

                        SelectFont(hdc, logfont);
                        DrawText(hdc, wifiavaiable_list[wifi_avaiable_list_head].ssid, -1, &msg_rcFilename, DT_TOP);
                    }
                }
            }
        }
        else
        {
            msg_rcFilename.left = LCD_W - 100;
            msg_rcFilename.top = SETTING_LIST_STR_PINT_Y;
            msg_rcFilename.right = LCD_W - SETTING_LIST_STR_PINT_X;
            msg_rcFilename.bottom = msg_rcFilename.top + SETTING_LIST_STR_PINT_H;

            DrawText(hdc, res_str[RES_STR_DISABLE], -1, &msg_rcFilename, DT_TOP);
        }
        if (get_wifi_state() != RK_WIFI_State_OFF)
        {
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

                    if (i == (cur_page - 1))
                        FillCircle(hdc, x + i * SETTING_PAGE_DOT_SPAC, SETTING_PAGE_DOT_Y, SETTING_PAGE_DOT_DIA);
                    else
                        Circle(hdc, x + i * SETTING_PAGE_DOT_SPAC, SETTING_PAGE_DOT_Y, SETTING_PAGE_DOT_DIA);
                }
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
            if (list_sel < (WIFI_NUM - 1))
                list_sel++;
            else
                list_sel = 0;
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            break;
        case KEY_UP_FUNC:
            if (list_sel > 0)
                list_sel--;
            else
                list_sel = WIFI_NUM - 1;
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            break;
        case KEY_ENTER_FUNC:
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            break;
        }
        break;
    case MSG_COMMAND:
        break;
    case MSG_DESTROY:
        KillTimer(hWnd, _ID_TIMER_SETTING_WIFI);
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
        if ((touch_pos_down.y - touch_pos_up.y) > SLIDE_DISTANCE)
        {
            printf("seek forward\n");
            page = (wifiavaiable_size / WIFI_NUM_PERPAGE) + 1;

            if (cur_page < page) cur_page++;
        }
        else if ((touch_pos_up.y - touch_pos_down.y) > SLIDE_DISTANCE)
        {
            printf("seek back\n");
            if (cur_page > 1) cur_page--;
        }
        else
        {
            int witch_button = check_button(touch_pos_up.x, touch_pos_up.y);

            if (witch_button == 0)
            {
                menu_back(hWnd, wParam, lParam);
                cur_page = 1;
            }
            else if (witch_button == 1)
            {

                if (get_wifi_state() != RK_WIFI_State_OFF)
                {
                    set_wifi_state(RK_WIFI_State_OFF);
                    RK_wifi_enable(0);
                    cur_page = 1;
                }
                else
                {
                    set_wifi_state(RK_WIFI_State_DISCONNECTED);

//                        if(pthread_kill(thread1,0) != ESRCH && pthread_kill(thread1,0) !=  EINVAL)
                    {
                        int iret1;
                        iret1 = pthread_create(&thread1, NULL, get_available_wifi, (void *) message1);
//                            pthread_detach(&thread1);
                        printf("Thread 1 returns: %d\n", iret1); // return 0 if seccuess
                    }
                }
            }
            else if ((witch_button > 1) && (witch_button < WIFI_WHOLE_BUTTON_NUM))
            {
                list_sel = witch_button - 2 + ((cur_page - 1) * WIFI_NUM_PERPAGE);
                wifi_enter(hWnd, wParam, list_sel);
            }
        }
        InvalidateRect(hWnd, &msg_rcBg, TRUE);
        break;
    }
    }
    return DefaultDialogProc(hWnd, message, wParam, lParam);
}

void creat_setting_wifi_dialog(HWND hWnd)
{
    DLGTEMPLATE DesktopDlg = {WS_VISIBLE, WS_EX_NONE | WS_EX_AUTOSECONDARYDC,
                              0, 0,
                              LCD_W, LCD_H,
                              DESKTOP_DLG_STRING, 0, 0, 0, NULL, 0
                             };
    //DesktopDlg.controls = DesktopCtrl;

    pwd_short_flag = 0;
    DialogBoxIndirectParam(&DesktopDlg, hWnd, setting_wifi_dialog_proc, 0L);
}
