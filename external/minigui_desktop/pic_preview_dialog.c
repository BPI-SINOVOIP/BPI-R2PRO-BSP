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

#define SLIDE_DISTANCE 100
#define WHOLE_BUTTON_NUM 1
#define PHOTO_MOVE_SMOOTH  1

#define MOVE_STOP     0
#define MOVE_NEXT     1
#define MOVE_PRE      2
static int file_total;
static int list_select = 0;
static int move_mode = 0;
static int move_cnt = 0;
static struct directory_node *dir_node = 0;
static BITMAP *pic_bmap_pre;
static BITMAP *pic_bmap_cur;
static BITMAP *pic_bmap_next;
static BITMAP *pic_bmap_temp;
static touch_pos touch_pos_down, touch_pos_up, touch_pos_old;
static int double_click_timer = 0;
/*
static const GAL_Rect msg_galrcMenu[] = {
    {BACK_PINT_X,BACK_PINT_Y,BACK_PINT_W,BACK_PINT_H},
};

static int is_button(int x,int y,GAL_Rect rect)
{
    return ((x <= rect.x + rect.w ) && (x >= rect.x) && (y <= rect.y + rect.h ) && (y >= rect.y));
}

static int check_button(int x,int y)
{
    int i;
    for(i = 0;i < WHOLE_BUTTON_NUM;i++)
    {
        if(is_button(x,y,msg_galrcMenu[i])) return i;
    }
    return -1;
}
*/
static int loadpicinit(struct directory_node *node)
{
    int i;
    struct file_node *file_node;
    char img[200];

    if (node == 0)
        return -1;

    file_node = node->file_node_list;

    for (i = 0; i < list_select; i++)
    {
        if (file_node)
            file_node = file_node->next_node;
    }

    if (pic_bmap_cur == 0)
    {
        pic_bmap_cur = malloc(sizeof(BITMAP));
        memset(pic_bmap_cur, 0, sizeof(BITMAP));
    }
    if (pic_bmap_pre == 0)
    {
        pic_bmap_pre = malloc(sizeof(BITMAP));
        memset(pic_bmap_pre, 0, sizeof(BITMAP));
    }
    if (pic_bmap_next == 0)
    {
        pic_bmap_next = malloc(sizeof(BITMAP));
        memset(pic_bmap_next, 0, sizeof(BITMAP));
    }
    if (pic_bmap_temp == 0)
    {
        pic_bmap_temp = malloc(sizeof(BITMAP));
        memset(pic_bmap_temp, 0, sizeof(BITMAP));
    }

    snprintf(img, sizeof(img), "%s/%s", node->patch, file_node->name);
    //printf("%s\n", img);
    if (LoadBitmap(HDC_SCREEN, pic_bmap_cur, img))
        return -1;

    if (file_node->pre_node)
    {
        snprintf(img, sizeof(img), "%s/%s", node->patch, file_node->pre_node->name);
        //printf("%s\n", img);
        if (LoadBitmap(HDC_SCREEN, pic_bmap_pre, img))
            return -1;
    }
    if (file_node->next_node)
    {
        snprintf(img, sizeof(img), "%s/%s", node->patch, file_node->next_node->name);
        //printf("%s\n", img);
        if (LoadBitmap(HDC_SCREEN, pic_bmap_next, img))
            return -1;
    }
    return 0;
}

static int loadpic(struct directory_node *node, BITMAP *pic_bmap, int id)
{
    int i;
    struct file_node *file_node;
    char img[200];

    if (node == 0)
        return -1;

    if (id < 0 || id >=  file_total)
        return -1;

    file_node = node->file_node_list;

    for (i = 0; i < id; i++)
    {
        if (file_node)
            file_node = file_node->next_node;
    }
    if (pic_bmap)
    {
        snprintf(img, sizeof(img), "%s/%s", node->patch, file_node->name);
        //printf("%s\n", img);
        UnloadBitmap(pic_bmap);
        if (LoadBitmap(HDC_SCREEN, pic_bmap, img))
            return -1;
    }
    return 0;
}

static void unloadpic(void)
{
    if (pic_bmap_cur)
    {
        UnloadBitmap(pic_bmap_cur);
        free(pic_bmap_cur);
    }
    if (pic_bmap_pre)
    {
        UnloadBitmap(pic_bmap_pre);
        free(pic_bmap_pre);
    }
    if (pic_bmap_next)
    {
        UnloadBitmap(pic_bmap_next);
        free(pic_bmap_next);
    }
    if (pic_bmap_temp)
    {
        UnloadBitmap(pic_bmap_temp);
        free(pic_bmap_temp);
    }
}

static void picture_enter(HWND hWnd, WPARAM wParam, LPARAM lParam)
{

}

static void menu_back(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    EndDialog(hWnd, wParam);
}

static LRESULT picpreview_dialog_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;

    //printf("%s message = 0x%x, 0x%x, 0x%x\n", __func__, message, wParam, lParam);
    switch (message)
    {
    case MSG_INITDIALOG:
    {
        DWORD bkcolor;
        HWND hFocus = GetDlgDefPushButton(hWnd);
        bkcolor = GetWindowElementPixel(hWnd, WE_BGC_WINDOW);
        SetWindowBkColor(hWnd, bkcolor);
        if (hFocus)
            SetFocus(hFocus);
        loadpicinit(dir_node);
        SetTimer(hWnd, _ID_TIMER_PICPREVIEW, TIMER_PICPREVIEW);
        nhWnd = hWnd;
        return 0;
    }
    case MSG_TIMER:
        if (double_click_timer > 0)
            double_click_timer--;
#if PHOTO_MOVE_SMOOTH
        if (wParam == _ID_TIMER_PICPREVIEW)
        {
            if (move_mode != 0)
            {
                //printf("%s MSG_TIMER\n", __func__);
                move_cnt ++;
                if (move_cnt >= 10)
                {
                    if (move_mode == MOVE_NEXT)
                    {
                        BITMAP *bmap_temp = pic_bmap_pre;
                        pic_bmap_pre = pic_bmap_cur;
                        pic_bmap_cur = pic_bmap_next;
                        pic_bmap_next = pic_bmap_temp;
                        pic_bmap_temp = bmap_temp;
                    }
                    else if (move_mode == MOVE_PRE)
                    {
                        BITMAP *bmap_temp = pic_bmap_next;
                        pic_bmap_next = pic_bmap_cur;
                        pic_bmap_cur = pic_bmap_pre;
                        pic_bmap_pre = pic_bmap_temp;
                        pic_bmap_temp = bmap_temp;
                    }
                    move_mode = MOVE_STOP;
                    move_cnt = 0;
                }
                InvalidateRect(hWnd, &msg_rcDialog, TRUE);
            }
        }
#endif
        break;
    case MSG_KEYDOWN:
        switch (wParam)
        {
        case KEY_UP_FUNC:
            if (move_mode != 0)
                break;
            if (list_select != 0)
            {
                list_select--;
                loadpic(dir_node, pic_bmap_temp, list_select - 1);
                InvalidateRect(hWnd, &msg_rcDialog, TRUE);
                move_mode = MOVE_PRE;
            }
            break;
        case KEY_DOWN_FUNC:
            if (move_mode != 0)
                break;
            if (list_select < file_total - 1)
            {
                list_select++;
                loadpic(dir_node, pic_bmap_temp, list_select + 1);
                InvalidateRect(hWnd, &msg_rcDialog, TRUE);
                move_mode = MOVE_NEXT;
            }
            break;
        case KEY_EXIT_FUNC:
            EndDialog(hWnd, wParam);
            break;
        case KEY_ENTER_FUNC:
            break;
        }
        break;
    case MSG_COMMAND:
        break;
    case MSG_PAINT:
    {
        int i;
        hdc = BeginPaint(hWnd);
        SelectFont(hdc, logfont);
#if PHOTO_MOVE_SMOOTH
        if (move_mode == MOVE_NEXT)
        {
            FillBoxWithBitmap(hdc, LCD_W - (LCD_W * move_cnt / 10), 0, LCD_W, LCD_H, pic_bmap_next);
            FillBoxWithBitmap(hdc, -(LCD_W * move_cnt / 10), 0, LCD_W, LCD_H, pic_bmap_cur);
        }
        else if (move_mode == MOVE_PRE)
        {
            FillBoxWithBitmap(hdc, (LCD_W * move_cnt / 10), 0, LCD_W, LCD_H, pic_bmap_cur);
            FillBoxWithBitmap(hdc, (LCD_W * move_cnt / 10) - LCD_W, 0, LCD_W, LCD_H, pic_bmap_pre);
        }
        else
        {
            FillBoxWithBitmap(hdc, 0, 0, LCD_W, LCD_H, pic_bmap_cur);
        }
#else
        if (move_mode == MOVE_NEXT)
        {
            FillBoxWithBitmap(hdc, 0, 0, LCD_W, LCD_H, pic_bmap_next);
            BITMAP *bmap_temp = pic_bmap_pre;
            pic_bmap_pre = pic_bmap_cur;
            pic_bmap_cur = pic_bmap_next;
            pic_bmap_next = pic_bmap_temp;
            pic_bmap_temp = bmap_temp;
        }
        else if (move_mode == MOVE_PRE)
        {
            FillBoxWithBitmap(hdc, 0, 0, LCD_W, LCD_H, pic_bmap_pre);
            BITMAP *bmap_temp = pic_bmap_next;
            pic_bmap_next = pic_bmap_cur;
            pic_bmap_cur = pic_bmap_pre;
            pic_bmap_pre = pic_bmap_temp;
            pic_bmap_temp = bmap_temp;
        }
        else
        {
            FillBoxWithBitmap(hdc, 0, 0, LCD_W, LCD_H, pic_bmap_cur);
        }
        move_mode = MOVE_STOP;
#endif
        EndPaint(hWnd, hdc);
        break;
    }
    case MSG_DESTROY:
        KillTimer(hWnd, _ID_TIMER_PICPREVIEW);
        unloadpic();
        return 0;
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
            if (move_mode != 0)
                break;
            if (list_select < file_total - 1)
            {
                list_select++;
                loadpic(dir_node, pic_bmap_temp, list_select + 1);
                InvalidateRect(hWnd, &msg_rcDialog, TRUE);
                move_mode = MOVE_NEXT;
            }
        }
        else if (touch_pos_up.x - touch_pos_down.x > SLIDE_DISTANCE)
        {
            //printf("slide right\n");
            if (move_mode != 0)
                break;
            if (list_select != 0)
            {
                list_select--;
                loadpic(dir_node, pic_bmap_temp, list_select - 1);
                InvalidateRect(hWnd, &msg_rcDialog, TRUE);
                move_mode = MOVE_PRE;
            }
        }
        else
        {
            if (double_click_timer > 0 &&
                    abs(touch_pos_old.x - touch_pos_up.x) < 50 &&
                    abs(touch_pos_old.y - touch_pos_up.y) < 50)
                menu_back(hWnd, wParam, lParam);
            else
                double_click_timer = 5;
            /*
            int witch_button = check_button(touch_pos_up.x,touch_pos_up.y);
            if(witch_button == 0) menu_back(hWnd,wParam,lParam);
            if(witch_button > 0 && witch_button < WHOLE_BUTTON_NUM)
            {
                picture_enter(hWnd,wParam,witch_button);
            }*/
        }
        touch_pos_old.x = touch_pos_up.x;
        touch_pos_old.y = touch_pos_up.y;
        EnableScreenAutoOff();
        break;
    }
    }

    return DefaultDialogProc(hWnd, message, wParam, lParam);
}

void creat_picpreview_dialog(HWND hWnd, struct directory_node *node)
{
    DLGTEMPLATE PicPreViewDlg = {WS_VISIBLE, WS_EX_NONE | WS_EX_AUTOSECONDARYDC,
                                 0, 0,
                                 LCD_W, LCD_H,
                                 DESKTOP_DLG_STRING, 0, 0, 0, NULL, 0
                                };
    if (node == NULL)
        return;

    list_select = node->file_sel;
    file_total = node->total;
    dir_node = node;
    pic_bmap_pre = 0;
    pic_bmap_cur = 0;
    pic_bmap_next = 0;
    pic_bmap_temp = 0;
    DialogBoxIndirectParam(&PicPreViewDlg, hWnd, picpreview_dialog_proc, 0L);
}
