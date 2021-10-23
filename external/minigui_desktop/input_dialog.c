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

static char input_content[64] = "";
int pwd_cnt = 0;
#define SLIDE_DISTANCE 100
#define WHOLE_BUTTON_NUM 4
#define INPUT_NUM   3

static BITMAP list_sel_bmap;
static int list_sel = 0;
static int batt = 0;
static int wifistate;
static touch_pos touch_pos_down, touch_pos_up, touch_pos_old;
int input_dialog_type = 0;
int pwd_short_flag;
int wifi_connect_flag; // =1 connect temporary ssid and password ; =2 connect paramenter ssid and password

int shift = 0;
int symbol_flag = 0;

#define WIFI_NAME_PINT_X    ((LCD_W - 72) / 2)
#define WIFI_NAME_PINT_Y    24
#define WIFI_NAME_PINT_W    72
#define WIFI_NAME_PINT_H    24

#define INPUT_PINT_X    ((LCD_W - 200) / 2)
#define INPUT_PINT_Y    WIFI_NAME_PINT_Y + 40
#define INPUT_PINT_W    LCD_W
#define INPUT_PINT_H    24

#define INPUT_LINE_X1    100
#define INPUT_LINE_Y1    (INPUT_PINT_Y+INPUT_PINT_H+5)
#define INPUT_LINE_X2    LCD_W - 100
#define INPUT_LINE_Y2    (INPUT_PINT_Y+INPUT_PINT_H+5)

#define INPUT_BT1_X    (LCD_W-INPUT_BT_LSP*10)/2
#define INPUT_BT1_Y    (LCD_H-INPUT_BT_CSP*5-10)
#define INPUT_BT1_W    65
#define INPUT_BT1_H    65
#define INPUT_BT1_STRING    "1"

#define IDC_INPUT_SET_BT_MIN   120
#define IDC_INPUT_SET_BT_MAX   160

#define INPUT_BT_LSP   (INPUT_BT1_W + 3)
#define INPUT_BT_CSP   (INPUT_BT1_H + 3)

RECT msg_rcWifipwd = {INPUT_PINT_X, INPUT_PINT_Y, INPUT_PINT_X + INPUT_PINT_W, INPUT_PINT_Y + INPUT_PINT_H};
RECT msg_rcInput = {SETTING_LIST_STR_PINT_X, SETTING_LIST_STR_PINT_Y + SETTING_LIST_STR_PINT_SPAC, LCD_W - SETTING_LIST_STR_PINT_X, SETTING_LIST_STR_PINT_Y + SETTING_LIST_STR_PINT_SPAC + SETTING_LIST_STR_PINT_H};

CTRLDATA KeyboardCtrl[BUTTON_MAXNUM] =
{
    {
        CTRL_BUTTON, WS_VISIBLE | BS_DEFPUSHBUTTON | BS_PUSHBUTTON | WS_TABSTOP,
        INPUT_BT1_X, INPUT_BT1_Y, INPUT_BT1_W, INPUT_BT1_H,
        IDC_INPUT_SET_BT_MIN, INPUT_BT1_STRING, 0
    },
};

char *keyboard_name[][BUTTON_MAXNUM] =
{
    {
        "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
        "q", "w", "e", "r", "t", "y", "u",  "i", "o", "p",
        "a", "s", "d", "f", "g", "h", "j", "k", "l",
        "Caps", "z", "x", "c", "v", "b", "n", "m",  "Del",
        "@*#", "Space",   "Enter"
    },
    {
        "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
        "Q", "W", "E", "R", "T", "Y", "U",  "I", "O", "P",
        "A", "S", "D", "F", "G", "H", "J", "K", "L",
        "Caps", "Z", "X", "C", "V", "B", "N", "M",    "Del",
        "@*#", "Space",    "Enter"

    },
    {
        "`", "~", "!", "@", "#", "$", "%", "^", "&", "*",
        "(", ")", "-", "_", "=", "+", "[",  "]", "{",
        "}", "\\", "|", ";", ":", "\'", "\"", ",", "<", ".",
        "Caps", ">", "/", "?",  "/", "*", "-", "+",  "Del",
        "a123", "Space", "Enter"
    }
};

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

    return 0;
}

static void unloadres(void)
{
    int i;

    UnloadBitmap(&list_sel_bmap);
}

static void menu_back(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    EndDialog(hWnd, wParam);
}

static void input_enter(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch (input_dialog_type)
    {
    case WIFI_PWD:
        switch (lParam)
        {
        case 2:
            if (pwd_cnt < 5)
            {
                pwd_short_flag = 1;
                InvalidateRect(hWnd, &msg_rcBg, FALSE);
            }
            else
            {
                snprintf(input_wifi_date.psk, 64, "%s", input_content);
                set_wifi_state(RK_WIFI_State_CONNECTING);  // to display faster
                snprintf(connect_wifi_date.ssid, 64, "%s", input_wifi_date.ssid);
                snprintf(connect_wifi_date.psk, 64, "%s", input_wifi_date.psk);
                wifi_connect_flag = 1;
                cur_page = 1;
                menu_back(hWnd, wParam, lParam);
            }
            break;
        }
        break;
    }
}



static LRESULT input_dialog_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
        wifistate = get_wifi_state();
        list_sel = 0;
        SetTimer(hWnd, _ID_TIMER_INPUT, TIMER_INPUT);
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
        if (wParam == _ID_TIMER_INPUT)
        {
#ifdef ENABLE_BATT
            if (batt != battery)
            {
                batt = battery;
                InvalidateRect(hWnd, &msg_rcStatusBar, FALSE);
            }
#endif
#ifdef ENABLE_WIFI
            if (get_wifi_state() != wifistate)
            {
                wifistate = get_wifi_state();
                InvalidateRect(hWnd, &msg_rcWifi, FALSE);
            }
#endif
        }
        break;
    }
    case MSG_COMMAND:
    {
        if (wParam == (IDC_INPUT_SET_BT_MIN + 29))
        {
            int i;
            //cap key down
            if (shift)
                shift = 0;
            else
                shift = 1;
            symbol_flag = 0;
            for (i = 0; i < BUTTON_MAXNUM; i++)
            {
                HWND hwnd_button = GetDlgItem(hWnd, IDC_INPUT_SET_BT_MIN + i);
                SendMessage(hwnd_button, MSG_SETTEXT, strlen(keyboard_name[shift][i]), (LPARAM)keyboard_name[shift][i]);
            }
        }
        else if (wParam == (IDC_INPUT_SET_BT_MIN + 38))
        {
            int j;
            //char key down

            if (symbol_flag)
            {
                symbol_flag = 0;
            }
            else
            {
                symbol_flag = 1;
            }

            for (j = 0; j < BUTTON_MAXNUM; j++)
            {
                HWND hwnd_button_symbol = GetDlgItem(hWnd, IDC_INPUT_SET_BT_MIN + j);
                if (symbol_flag)
                    SendMessage(hwnd_button_symbol, MSG_SETTEXT, strlen(keyboard_name[2][j]), (LPARAM)keyboard_name[2][j]);
                else
                    SendMessage(hwnd_button_symbol, MSG_SETTEXT, strlen(keyboard_name[shift][j]), (LPARAM)keyboard_name[shift][j]);
            }
        }
        else if (wParam == (IDC_INPUT_SET_BT_MIN + 37))
        {
            //del key down
            if (pwd_cnt >= 1)
            {
                pwd_cnt--;
                input_content[pwd_cnt] = '\0';
                InvalidateRect(hWnd, &msg_rcInput, FALSE);
            }
        }
        else if (wParam == (IDC_INPUT_SET_BT_MIN + 39))
        {
            //space key down
            input_content[pwd_cnt] = ' ';
            if (pwd_cnt < 127)
                pwd_cnt++;
            InvalidateRect(hWnd, &msg_rcInput, FALSE);
        }
        else if (wParam == (IDC_INPUT_SET_BT_MIN + 40))
        {
            //enter key down
            if (pwd_cnt < 5)
            {
                pwd_short_flag = 1;
                InvalidateRect(hWnd, &msg_rcBg, FALSE);
            }
            else
            {
                snprintf(input_wifi_date.psk, 64, "%s", input_content);
                set_wifi_state(RK_WIFI_State_CONNECTING);  // to display faster
                snprintf(connect_wifi_date.ssid, 64, "%s", input_wifi_date.ssid);
                snprintf(connect_wifi_date.psk, 64, "%s", input_wifi_date.psk);
                wifi_connect_flag = 1;
                cur_page = 1;
                menu_back(hWnd, wParam, lParam);
            }
        }
        else if (wParam >= IDC_INPUT_SET_BT_MIN && wParam <= IDC_INPUT_SET_BT_MAX)
        {
            if (symbol_flag)
                input_content[pwd_cnt] = keyboard_name[2][wParam - IDC_INPUT_SET_BT_MIN][0];
            else
                input_content[pwd_cnt] = keyboard_name[shift][wParam - IDC_INPUT_SET_BT_MIN][0];
            if (pwd_cnt < 127)
                pwd_cnt++;
            InvalidateRect(hWnd, &msg_rcInput, FALSE);
        }

        if (get_bl_brightness() == 0)
        {
            screenon();
            break;
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


        RECT msg_rcTime, msg_rcFilename;
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
        switch (input_dialog_type)
        {
        case WIFI_PWD:
            msg_rcTitle.right += 100;

            SetBkColor(hdc, COLOR_transparent);
            SetBkMode(hdc, BM_TRANSPARENT);
            SetTextColor(hdc, RGB2Pixel(hdc, 0xff, 0xff, 0xff));
            SelectFont(hdc, logfont);
            DrawText(hdc, res_str[RES_STR_WIFI_CONNECTION], -1, &msg_rcTitle, DT_TOP);
            FillBox(hdc, TITLE_LINE_PINT_X, TITLE_LINE_PINT_Y, TITLE_LINE_PINT_W, TITLE_LINE_PINT_H);

            page = (INPUT_NUM + SETTING_NUM_PERPAGE - 1) / SETTING_NUM_PERPAGE;
            cur_page = list_sel / SETTING_NUM_PERPAGE;

            msg_rcTitle.right -= 100;

            msg_rcFilename.left = SETTING_LIST_STR_PINT_X;
            msg_rcFilename.top = SETTING_LIST_STR_PINT_Y;
            msg_rcFilename.right = LCD_W - msg_rcFilename.left;
            msg_rcFilename.bottom = msg_rcFilename.top + SETTING_LIST_STR_PINT_H;
            SelectFont(hdc, logfont);
            DrawText(hdc, res_str[RES_STR_WIFI_SSID], -1, &msg_rcFilename, DT_TOP);
            msg_rcFilename.left = SETTING_LIST_STR_PINT_X + 150;
            DrawText(hdc, input_wifi_date.ssid, -1, &msg_rcFilename, DT_TOP);

            msg_rcFilename.left = SETTING_LIST_STR_PINT_X;
            msg_rcFilename.top = SETTING_LIST_STR_PINT_Y + SETTING_LIST_STR_PINT_SPAC;
            msg_rcFilename.bottom = msg_rcFilename.top + SETTING_LIST_STR_PINT_H;
            DrawText(hdc, res_str[RES_STR_WIFI_PWD], -1, &msg_rcFilename, DT_TOP);

            msg_rcFilename.top = SETTING_LIST_STR_PINT_Y + SETTING_LIST_STR_PINT_SPAC * 2;
            msg_rcFilename.bottom = msg_rcFilename.top + SETTING_LIST_STR_PINT_H;

            if (list_sel == 2)
            {
                FillBoxWithBitmap(hdc, 0, msg_rcFilename.top - 9, LCD_W, SETTING_LIST_SEL_PINT_H, &list_sel_bmap);
            }
            SelectFont(hdc, logfont);
            DrawText(hdc, res_str[RES_STR_OK], -1, &msg_rcFilename, DT_TOP);

            if (pwd_short_flag)
            {
                msg_rcFilename.top = SETTING_LIST_STR_PINT_Y + SETTING_LIST_STR_PINT_SPAC * 2;
                msg_rcFilename.left = LCD_W / 2 - SETTING_LIST_STR_PINT_X;
                msg_rcFilename.right = LCD_W - SETTING_LIST_STR_PINT_X;
                msg_rcFilename.bottom = msg_rcFilename.top + SETTING_LIST_STR_PINT_H;
                DrawText(hdc, res_str[RES_STR_TOO_SHORT], -1, &msg_rcFilename, DT_TOP);
            }

            msg_rcFilename.left = SETTING_LIST_STR_PINT_X + 150;
            msg_rcFilename.top = SETTING_LIST_STR_PINT_Y + SETTING_LIST_STR_PINT_SPAC;
            msg_rcFilename.right = LCD_W - SETTING_LIST_STR_PINT_X - 150;
            msg_rcFilename.bottom = msg_rcFilename.top + SETTING_LIST_STR_PINT_H;

            SelectFont(hdc, logfont);
            DrawText(hdc, input_content, -1, &msg_rcFilename, DT_TOP);

            SetTextColor(hdc, RGB2Pixel(hdc, 0xff, 0xff, 0xff));
            FillBox(hdc, msg_rcFilename.left, msg_rcFilename.bottom, LCD_W / 2 - SETTING_LIST_STR_PINT_X - 150, TITLE_LINE_PINT_H);
            break;
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
            if (list_sel < (INPUT_NUM - 1))
                list_sel++;
            else
                list_sel = 0;
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            break;
        case KEY_UP_FUNC:
            if (list_sel > 0)
                list_sel--;
            else
                list_sel = INPUT_NUM - 1;
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            break;
        case KEY_ENTER_FUNC:
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            break;
        }
        break;
    case MSG_DESTROY:
        KillTimer(hWnd, _ID_TIMER_INPUT);
        unloadres();
        break;
    case MSG_LBUTTONDOWN:
        touch_pos_down.x = LOSWORD(lParam);
        touch_pos_down.y = HISWORD(lParam);
        printf("%s MSG_LBUTTONDOWN x %d, y %d\n", __func__, touch_pos_down.x, touch_pos_down.y);
        touch_pos_old.x = touch_pos_up.x;
        touch_pos_old.y = touch_pos_up.y;
        EnableScreenAutoOff();
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
        int witch_button = check_button(touch_pos_down.x, touch_pos_down.y);
        if (witch_button == 0)
        {
            menu_back(hWnd, wParam, lParam);
        }
        else if (witch_button > 0 && witch_button < WHOLE_BUTTON_NUM)
        {
            list_sel = witch_button - 1;
            input_enter(hWnd, wParam, list_sel);
        }
        touch_pos_old.x = touch_pos_up.x;
        touch_pos_old.y = touch_pos_up.y;
        EnableScreenAutoOff();
        break;
    }
    }
    return DefaultDialogProc(hWnd, message, wParam, lParam);
}



void keyboard_init()
{

    int i;

    for (i = 0; i < BUTTON_MAXNUM; i++)
    {
        if (i == 0)
        {
            KeyboardCtrl[i].dwStyle = WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP;
            KeyboardCtrl[i].x = INPUT_BT1_X;
            KeyboardCtrl[i].y = INPUT_BT1_Y;
            KeyboardCtrl[i].w = INPUT_BT1_W;
            KeyboardCtrl[i].h = INPUT_BT1_H;
            KeyboardCtrl[i].id = IDC_INPUT_SET_BT_MIN;
        }
        else if (i < 10)
        {
            KeyboardCtrl[i].dwStyle = WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP;
            KeyboardCtrl[i].x = INPUT_BT1_X + i * INPUT_BT_LSP;
            KeyboardCtrl[i].y = INPUT_BT1_Y;
            KeyboardCtrl[i].w = INPUT_BT1_W;
            KeyboardCtrl[i].h = INPUT_BT1_H;
            KeyboardCtrl[i].id = IDC_INPUT_SET_BT_MIN + i;
        }
        else if (i == 10)
        {
            KeyboardCtrl[i].dwStyle = WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP;
            KeyboardCtrl[i].x = INPUT_BT1_X;
            KeyboardCtrl[i].y = INPUT_BT1_Y + INPUT_BT_CSP;
            KeyboardCtrl[i].w = INPUT_BT1_W ;
            KeyboardCtrl[i].h = INPUT_BT1_H;
            KeyboardCtrl[i].id = IDC_INPUT_SET_BT_MIN + i;
        }
        else if (i < 20)
        {
            KeyboardCtrl[i].dwStyle = WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP;
            KeyboardCtrl[i].x = INPUT_BT1_X + (i - 10) * INPUT_BT_LSP;
            KeyboardCtrl[i].y = INPUT_BT1_Y + INPUT_BT_CSP;
            KeyboardCtrl[i].w = INPUT_BT1_W;
            KeyboardCtrl[i].h = INPUT_BT1_H;
            KeyboardCtrl[i].id = IDC_INPUT_SET_BT_MIN + i;
        }
        else if (i == 20)
        {
            KeyboardCtrl[i].dwStyle = WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP;
            KeyboardCtrl[i].x = INPUT_BT1_X + INPUT_BT_CSP / 2;
            KeyboardCtrl[i].y = INPUT_BT1_Y + INPUT_BT_CSP * 2;
            KeyboardCtrl[i].w = INPUT_BT1_W ;
            KeyboardCtrl[i].h = INPUT_BT1_H;
            KeyboardCtrl[i].id = IDC_INPUT_SET_BT_MIN + i;
        }
        else if (i < 29)
        {
            KeyboardCtrl[i].dwStyle = WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP;
            KeyboardCtrl[i].x = INPUT_BT1_X + (i - 20) * INPUT_BT_LSP + INPUT_BT_CSP / 2;
            KeyboardCtrl[i].y = INPUT_BT1_Y + INPUT_BT_CSP * 2;
            KeyboardCtrl[i].w = INPUT_BT1_W;
            KeyboardCtrl[i].h = INPUT_BT1_H;
            KeyboardCtrl[i].id = IDC_INPUT_SET_BT_MIN + i;
        }
        else if (i == 29)
        {
            KeyboardCtrl[i].dwStyle = WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP;
            KeyboardCtrl[i].x = INPUT_BT1_X ;
            KeyboardCtrl[i].y = INPUT_BT1_Y + INPUT_BT_CSP * 3;
            KeyboardCtrl[i].w = INPUT_BT1_W + INPUT_BT_LSP;
            KeyboardCtrl[i].h = INPUT_BT1_H;
            KeyboardCtrl[i].id = IDC_INPUT_SET_BT_MIN + i;
        }
        else if (i < 37)
        {
            KeyboardCtrl[i].dwStyle = WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP;
            KeyboardCtrl[i].x = INPUT_BT1_X + (i - 28) * INPUT_BT_LSP;
            KeyboardCtrl[i].y = INPUT_BT1_Y + INPUT_BT_CSP * 3;
            KeyboardCtrl[i].w = INPUT_BT1_W;
            KeyboardCtrl[i].h = INPUT_BT1_H;
            KeyboardCtrl[i].id = IDC_INPUT_SET_BT_MIN + i;
        }
        else if (i == 37)
        {
            KeyboardCtrl[i].dwStyle = WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP;
            KeyboardCtrl[i].x = INPUT_BT1_X + + (i - 29) * INPUT_BT_LSP;
            KeyboardCtrl[i].y = INPUT_BT1_Y + INPUT_BT_CSP * 3;
            KeyboardCtrl[i].w = INPUT_BT1_W + INPUT_BT_LSP;
            KeyboardCtrl[i].h = INPUT_BT1_H;
            KeyboardCtrl[i].id = IDC_INPUT_SET_BT_MIN + i;

        }
        else if (i == 39)
        {
            KeyboardCtrl[i].dwStyle = WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP;
            KeyboardCtrl[i].x = INPUT_BT1_X + INPUT_BT_LSP * 2;
            KeyboardCtrl[i].y = INPUT_BT1_Y + INPUT_BT_CSP * 4;
            KeyboardCtrl[i].w = INPUT_BT1_W + INPUT_BT_LSP * 5;
            KeyboardCtrl[i].h = INPUT_BT1_H;
            KeyboardCtrl[i].id = IDC_INPUT_SET_BT_MIN + i;
        }
        else if (i == 38)
        {
            KeyboardCtrl[i].dwStyle = WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP;
            KeyboardCtrl[i].x = INPUT_BT1_X ;
            KeyboardCtrl[i].y = INPUT_BT1_Y + INPUT_BT_CSP * 4;
            KeyboardCtrl[i].w = INPUT_BT1_W + INPUT_BT_LSP;
            KeyboardCtrl[i].h = INPUT_BT1_H;
            KeyboardCtrl[i].id = IDC_INPUT_SET_BT_MIN + i;
        }
        else if (i == 40)
        {
            KeyboardCtrl[i].dwStyle = WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP;
            KeyboardCtrl[i].x = INPUT_BT1_X + INPUT_BT_LSP * 8;
            KeyboardCtrl[i].y = INPUT_BT1_Y + INPUT_BT_CSP * 4;
            KeyboardCtrl[i].w = INPUT_BT1_W + INPUT_BT_LSP;
            KeyboardCtrl[i].h = INPUT_BT1_H;
            KeyboardCtrl[i].id = IDC_INPUT_SET_BT_MIN + i;
        }
        KeyboardCtrl[i].class_name = CTRL_BUTTON;
        KeyboardCtrl[i].caption = keyboard_name[0][i];
        KeyboardCtrl[i].dwAddData = 0;
        KeyboardCtrl[i].dwExStyle = 0;
        KeyboardCtrl[i].werdr_name = 0;
        KeyboardCtrl[i].we_attrs = 0;
    }
}

void creat_input_dialog(HWND hWnd)
{
    DLGTEMPLATE DesktopDlg = {WS_VISIBLE, WS_EX_NONE | WS_EX_AUTOSECONDARYDC,
                              0, 0,
                              LCD_W, LCD_H,
                              DESKTOP_DLG_STRING, 0, 0, sizeof(KeyboardCtrl) / sizeof(KeyboardCtrl[0]), NULL, 0
                             };
    DesktopDlg.controls = KeyboardCtrl;

    memset(input_content, '\0', 64);
    pwd_cnt = 0;
    pwd_short_flag = 0;
    shift = 0;
    symbol_flag = 0;

    DialogBoxIndirectParam(&DesktopDlg, hWnd, input_dialog_proc, 0L);
}


