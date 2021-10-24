#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/ctrl/static.h>
#include "rknn_msg.h"
#include "ui_res.h"
#include "joint.h"

#define MODEL_OUT         96
#define DST_W             192
#define DST_H             192
#define DISP_W            1280
#define DISP_H            720
#define CIRCLE_R          12

#define CAPTION_H         70
#define RECT_EDGE_SIZE    20
#define TEXT_SIZE_CAPTION 36
#define TEXT_COLOR        0xffffeb06 //real rgb is 0xff06ebff
#define CAPTION_BKCOLOR   0x80311600 //real rgb is 0x80001631
#define _ID_CAPTION_ID    100

static HWND g_main_hwnd;
static HWND g_caption_hwnd;
static PLOGFONT g_caption_font = NULL;
static WNDPROC old_caption_proc;

RECT main_rect = {0, 70, 1280, 720};
const int match_list[14][2] = {{0, 1}, {1, 2}, {1, 5}, {2, 3}, {2, 8},
                               {3, 4},{5, 6}, {5, 11}, {6, 7}, {8, 9},
                               {8, 11}, {9, 10}, {11, 12}, {12, 13}};
void joint_paint_object_msg()
{
    InvalidateRect(g_main_hwnd, &main_rect, TRUE);
}

static int mDrawText(HDC hdc, const char *buf, int n, int left, int top, int right,
                int bottom, UINT format)
{
    RECT rc;

    rc.left = left;
    rc.top = top;
    rc.right = right;
    rc.bottom = bottom;
    return DrawText (hdc, buf, n, &rc,  format);
}

static void paint_object(HDC hdc)
{
    int front_w = DST_W;
    int front_h = DST_H;
    int ui_w = DISP_W;
    int ui_h = DISP_H;
    int edge = RECT_EDGE_SIZE;
    int keypoint[14 * 2];

    float *cpm_result = joint_get_joint_group();
    for (int i = 0; i < 14; i++) {
        int x = *(cpm_result + i*2) * ui_w / MODEL_OUT;
        int y = *(cpm_result + i*2 + 1) * ui_h / MODEL_OUT;
        keypoint[i*2] = x;
        keypoint[i*2+1] = y;
    }

    for (int i = 0; i < 14; i++) {
        int point_index_1 = match_list[i][0];
        int point_index_2 = match_list[i][1];
        int x1 = keypoint[point_index_1 * 2];
        int y1 = keypoint[point_index_1 * 2 + 1];
        int x2 = keypoint[point_index_2 * 2];
        int y2 = keypoint[point_index_2 * 2 + 1];
        if ((x1 == 0 && y1 ==0) || (x2 ==0 && y2 ==0)) {
            continue;
        }
        SetPenColor(hdc, PIXEL_lightwhite);
        SetPenWidth(hdc, 5);
        LineEx(hdc, x1, y1, x2, y2);
    }

    for (int i = 0; i < 14; i++) {
        int x = *(cpm_result + i*2) * ui_w * 2 / front_w;
        int y = *(cpm_result + i*2 + 1) * ui_h * 2 / front_h;
        if (x != 0 && y != 0) {
            POINT p;
            p.x = x;
            p.y = y;
            FillBoxWithBitmap(hdc, x - CIRCLE_R, y - CIRCLE_R,
                              CIRCLE_R * 2, CIRCLE_R * 2, &dot_mobilenet_bmap);
        }
    }
}

void joint_paint_object(HWND hwnd)
{
    HDC hdc;
    hdc = BeginPaint(hwnd);
  //  SetBkColor (hdc, g_bkcolor);
    SetTextColor(hdc, TEXT_COLOR);
    paint_object(hdc);
    EndPaint(hwnd, hdc);
}

static LRESULT caption_wnd_proc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{

    HDC hdc;
    switch(message){
    case MSG_CREATE:
        hdc = BeginPaint (hwnd);
        FillBoxWithBitmap(hdc, 1050, 11, 200, 48, &img_logo_bmap);
        SelectFont (hdc, g_caption_font);
        SetBkColor (hdc, CAPTION_BKCOLOR);
        SetTextColor(hdc, TEXT_COLOR);
        mDrawText(hdc, "Rockchip Joint Demo", -1, 40, 0, g_rcScr.right, CAPTION_H,
                  DT_NOCLIP | DT_SINGLELINE | DT_LEFT | DT_VCENTER);
        EndPaint (hwnd, hdc);
        break;
    }
    return (*old_caption_proc)(hwnd, message, w_param, l_param);
}

HWND caption_create(HWND mainhwnd)
{
    HWND hwnd;

    hwnd = CreateWindow(CTRL_STATIC, NULL, WS_CHILD | WS_VISIBLE | SS_SIMPLE,
                        _ID_CAPTION_ID, 0, 0, g_rcScr.right, CAPTION_H, mainhwnd, 0);
    if (hwnd <= HWND_NULL)
        return hwnd;
    SetWindowBkColor(hwnd, CAPTION_BKCOLOR);
    g_caption_font = CreateLogFont(FONT_TYPE_NAME_SCALE_TTF,
                                   "ubuntuMono", "ISO8859-1",
                                   FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN,
                                   FONT_FLIP_NIL, FONT_OTHER_NIL,
                                   FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,
                                   TEXT_SIZE_CAPTION, 0);
   SetWindowFont(hwnd, g_caption_font);
   SetWindowElementAttr(hwnd, WE_FGC_WINDOW, TEXT_COLOR);
   old_caption_proc = SetWindowCallbackProc(hwnd, caption_wnd_proc);
   PostMessage (hwnd, MSG_CREATE, 0, 0);
   return hwnd;
}

void caption_destory(HWND hwnd)
{
    DestroyLogFont(g_caption_font);
    DestroyWindow(hwnd);
}

int joint_ui_init(HWND hwnd)
{
    g_main_hwnd = hwnd;
    g_caption_hwnd = caption_create(hwnd);
    if (g_caption_hwnd <= HWND_NULL)
        return -1;
    else
        return 0;
}

int joint_ui_deinit(HWND hwnd)
{
    if (g_caption_hwnd > HWND_NULL)
        caption_destory(g_caption_hwnd);
    g_main_hwnd = 0;
    return 0;
}