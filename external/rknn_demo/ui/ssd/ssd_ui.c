#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/ctrl/static.h>
#include "rknn_msg.h"
#include "ui_res.h"
#include "ssd.h"

#define DST_W             300
#define DST_H             300
#if NEED_RKNNAPI
#define DISP_W            2048
#define DISP_H            1536
#define TEXT_OUT_OFF      30
#define LOGO_OFF          1840
#else
#define DISP_W            1280
#define DISP_H            720
#define TEXT_OUT_OFF      12
#define LOGO_OFF          1050
#endif

#define CAPTION_H         70
#define RECT_EDGE_SIZE    20
#define TEXT_SIZE_FPS     36
#define TEXT_SIZE_FPSNUM  20
#define TEXT_SIZE_CAPTION 30
#define TEXT_COLOR        0xffffeb06 //real rgb is 0xff06ebff
#define CAPTION_BKCOLOR   0x80311600 //real rgb is 0x80001631
#define _ID_CAPTION_ID    100
#define _ID_FPS_ID        101

static HWND g_main_hwnd;
static HWND g_fps_hwnd;
static HWND g_caption_hwnd;
static PLOGFONT g_caption_font = NULL;
static PLOGFONT g_fps_font = NULL;

static WNDPROC old_fps_proc;
static WNDPROC old_caption_proc;

RECT fps_rect   = {30, 11, 153, 55};
#if NEED_RKNNAPI
RECT main_rect1 = {290, 70, 2048, 1536};
RECT main_rect2 = {0, 160, 290, 1536};
#else
RECT main_rect1 = {290, 70, 1280, 720};
RECT main_rect2 = {0, 160, 290, 720};
#endif

#define PaintRectBold(func, handle, rect) \
	            func(handle, ((RECT *)rect)->left, ((RECT *)rect)->top, \
						                         ((RECT *)rect)->right, ((RECT *)rect)->bottom); \
            func(handle, ((RECT *)rect)->left - 1, ((RECT *)rect)->top - 1, \
					                         ((RECT *)rect)->right + 1, ((RECT *)rect)->bottom + 1); \
            func(handle, ((RECT *)rect)->left + 1, ((RECT *)rect)->top + 1, \
					                         ((RECT *)rect)->right - 1, ((RECT *)rect)->bottom - 1)

void ssd_paint_object_msg()
{
    InvalidateRect(g_main_hwnd, &main_rect1, TRUE);
    InvalidateRect(g_main_hwnd, &main_rect2, TRUE);
}

void ssd_paint_fps_msg()
{
    InvalidateRect(g_fps_hwnd, &fps_rect, FALSE);
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

static void paint_single_object(HDC hdc, SSDRECT *select, const char *name)
{

    // FillBoxWithBitmap(hdc, select->left, select->top,
    //                   select->right - select->left,
    //                   select->bottom - select->top,
    //                   &mobilenet_box_bg_9_bmap);
    SetPenColor(hdc, TEXT_COLOR);
    PaintRectBold(Rectangle, hdc, select);
    FillBoxWithBitmap(hdc, select->left + RECT_EDGE_SIZE,
                      select->bottom - RECT_EDGE_SIZE -12,
                      8, 12, &dot_ssd_bmap);

    TextOut(hdc, select->left + RECT_EDGE_SIZE + 8 + 5,
            select->bottom - RECT_EDGE_SIZE - TEXT_OUT_OFF - 1, name);
}

static void paint_object(HDC hdc)
{
    int front_w = DST_W;
    int front_h = DST_H;
    int ui_w = DISP_W;
    int ui_h = DISP_H;
    int edge = RECT_EDGE_SIZE;

    struct ssd_group *mssd_group = ssd_get_ssd_group();
    if (mssd_group->count) {
        for (int i = 0; i < mssd_group->count; i++) {
            struct ssd_object *object = &mssd_group->objects[i];
            //  struct win *ui_win = rk_fb_getuiwin();
            SSDRECT select;
            select.left = object->select.left * ui_w / front_w;
            select.top = object->select.top * ui_h  / front_h;
            select.right = object->select.right * ui_w / front_w;
            select.bottom = object->select.bottom * ui_h / front_h;
            if (select.left < edge)
                select.left = edge;
            if (select.top < edge)
                select.top = edge;
            if (select.right > ui_w - edge)
                select.right = ui_w - edge;
            if (select.bottom > ui_h - edge)
                select.bottom = ui_h - edge;
            paint_single_object(hdc, &select, mssd_group->objects[i].name);
        }
    }
    // mssd_group->posted--;
    // mssd_group->count = 0;
}

void ssd_paint_object(HWND hwnd)
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
        FillBoxWithBitmap(hdc, LOGO_OFF, 11, 200, 48, &img_logo_bmap);
        SelectFont (hdc, g_caption_font);
        SetBkColor (hdc, CAPTION_BKCOLOR);
        SetTextColor(hdc, TEXT_COLOR);
        mDrawText(hdc, "Rockchip SSD Demo", -1, 40, 0, g_rcScr.right, CAPTION_H,
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
                                   TEXT_SIZE_FPS, 0);
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

static void fps_show(HDC hdc, float fps)
{
    char fps_buf[6] = {0};

    snprintf(fps_buf, 6, "%05.2f", fps);
    mDrawText(hdc, fps_buf,     1, 30, 11, 52, 55,
              DT_NOCLIP | DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    mDrawText(hdc, fps_buf + 1, 1, 60, 11, 82, 55,
              DT_NOCLIP | DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    mDrawText(hdc, fps_buf + 3, 1, 100, 11, 122, 55,
              DT_NOCLIP | DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    mDrawText(hdc, fps_buf + 4, 1, 130, 11, 152, 55,
              DT_NOCLIP | DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    SetPenColor(hdc, CAPTION_BKCOLOR);
    SetBrushColor(hdc, CAPTION_BKCOLOR);
    FillBox(hdc, 30, 31, 122, 3);
}

//typedef LRESULT (* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
static LRESULT fps_wnd_proc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
    HDC hdc;
    switch(message){
    case MSG_CREATE:
        hdc = BeginPaint (hwnd);
        FillBoxWithBitmap(hdc, 0, 0, 250, 70, &fps_bg_9_bmap);
        FillBoxWithBitmapPart(hdc, 0, 0, 250, 70, 20, 20, &fps_bg_9_bmap, 200, 50);
        FillBoxWithBitmap(hdc, 30, 11, 22, 44, &num_bg_bmap);
        FillBoxWithBitmap(hdc, 60, 11, 22, 44, &num_bg_bmap);
        FillBoxWithBitmap(hdc, 100, 11, 22, 44, &num_bg_bmap);
        FillBoxWithBitmap(hdc, 130, 11, 22, 44, &num_bg_bmap);
        FillBoxWithBitmap(hdc, 87, 47, 8, 8, &dot_mobilenet_bmap);
        SelectFont (hdc, g_fps_font);
        SetBkColor (hdc, CAPTION_BKCOLOR);
        SetTextColor(hdc, TEXT_COLOR);
        mDrawText(hdc, "FPS", -1, 0, 0, 230, 70,
                DT_NOCLIP | DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
        SetBkColor (hdc, TEXT_COLOR);
        SetTextColor(hdc, CAPTION_BKCOLOR);
        fps_show(hdc, ssd_get_fps());
        EndPaint (hwnd, hdc);
        break;
    case MSG_PAINT:
        hdc = BeginPaint (hwnd);
        FillBoxWithBitmapPart(hdc, 0, 0, 250, 70, 0, 0, &fps_bg_9_bmap, 250, 70);

        FillBoxWithBitmap(hdc, 0, 0, 250, 70, &fps_bg_9_bmap);
        FillBoxWithBitmap(hdc, 30, 11, 22, 44, &num_bg_bmap);
        FillBoxWithBitmap(hdc, 60, 11, 22, 44, &num_bg_bmap);
        FillBoxWithBitmap(hdc, 100, 11, 22, 44, &num_bg_bmap);
        FillBoxWithBitmap(hdc, 130, 11, 22, 44, &num_bg_bmap);
        FillBoxWithBitmap(hdc, 87, 47, 8, 8, &dot_mobilenet_bmap);
        SetBkColor (hdc, CAPTION_BKCOLOR);
        SetTextColor(hdc, TEXT_COLOR);
        mDrawText(hdc, "FPS", -1, 0, 0, 230, 70,
                DT_NOCLIP | DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
        SetBkColor (hdc, TEXT_COLOR);
        SetTextColor(hdc, CAPTION_BKCOLOR);
        fps_show(hdc, ssd_get_fps());

        EndPaint (hwnd, hdc);
        break;
    }
    return (*old_fps_proc)(hwnd, message, w_param, l_param);
}

HWND fps_create(HWND mainhwnd)
{
    HWND hwnd;

    hwnd = CreateWindow(CTRL_STATIC, NULL, WS_CHILD | WS_VISIBLE | SS_SIMPLE,
                        _ID_FPS_ID, 40, 90, 250, 70, mainhwnd, 0);
    if (hwnd <= HWND_NULL)
        return hwnd;
    SetWindowBkColor(hwnd, CAPTION_BKCOLOR);
    g_fps_font = CreateLogFont(FONT_TYPE_NAME_SCALE_TTF,
                               "ubuntuMono", "ISO8859-1",
                               FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN,
                               FONT_FLIP_NIL, FONT_OTHER_NIL,
                               FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,
                               TEXT_SIZE_FPS, 0);
    SetWindowFont(hwnd, g_fps_font);
    SetWindowElementAttr(hwnd, WE_FGC_WINDOW, TEXT_COLOR);
    old_fps_proc = SetWindowCallbackProc(hwnd, fps_wnd_proc);
    PostMessage (hwnd, MSG_CREATE, 0, 0);
    return hwnd;
}

void fps_destory(HWND hwnd)
{
    DestroyLogFont(g_fps_font);
    DestroyWindow(hwnd);
}

int ssd_ui_init(HWND hwnd)
{
    g_main_hwnd = hwnd;
    g_caption_hwnd = caption_create(hwnd);
    g_fps_hwnd = fps_create(hwnd);
    if (g_caption_hwnd <= HWND_NULL
        || g_fps_hwnd <= HWND_NULL)
        return -1;
    else
        return 0;
}

int ssd_ui_deinit(HWND hwnd)
{
    if (g_fps_hwnd > HWND_NULL)
        fps_destory(g_fps_hwnd);
    if (g_caption_hwnd > HWND_NULL)
        caption_destory(g_caption_hwnd);
    g_main_hwnd = 0;
    return 0;
}
