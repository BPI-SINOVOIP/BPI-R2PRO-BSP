#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/ctrl/static.h>

#include "frg_ui.h"

#include "rknn_msg.h"
#include "ui_res.h"
#include "frg.h"

#define DISP_W            1280
#define DISP_H            720

#define CAPTION_H         70
#define RECT_EDGE_SIZE    20
#define TEXT_SIZE_FPS     36
#define TEXT_SIZE_FPSNUM  20
#define TEXT_SIZE_CAPTION 30
#define TEXT_COLOR        0xffffeb06 //real rgb is 0xff06ebff
#define CAPTION_BKCOLOR   0x80311600 //real rgb is 0x80001631
#define _ID_CAPTION_ID    100
#define _ID_FPS_ID        101
#define _ID_DATABASE_ID   102

#define DATA_BASE_RECT_W  240
#define DATA_BASE_RECT_H  600
#define DATA_BASE_RECT_X  (DISP_W - DATA_BASE_RECT_W - 16)
#define DATA_BASE_RECT_Y  (CAPTION_H)

static RECT g_database_rect =
    { DATA_BASE_RECT_X, DATA_BASE_RECT_Y,
      DATA_BASE_RECT_X + DATA_BASE_RECT_W,
      DATA_BASE_RECT_Y + DATA_BASE_RECT_H };

static HWND g_main_hwnd = 0;
static HWND g_fps_hwnd = 0;
static HWND g_caption_hwnd = 0;
static HWND g_database_hwnd = 0;
static PLOGFONT g_caption_font = NULL;
static PLOGFONT g_fps_font = NULL;

static WNDPROC old_fps_proc;
static WNDPROC old_caption_proc;
static WNDPROC old_database_proc;

RECT fps_rect   = {30, 11, 153, 55};
RECT main_rect1 = {290, 70, 1280, 720};
RECT main_rect2 = {0, 160, 290, 720};

#define PaintRectBold(func, handle, rect) \
	            func(handle, ((RECT *)rect)->left, ((RECT *)rect)->top, \
						                         ((RECT *)rect)->right, ((RECT *)rect)->bottom); \
            func(handle, ((RECT *)rect)->left - 1, ((RECT *)rect)->top - 1, \
					                         ((RECT *)rect)->right + 1, ((RECT *)rect)->bottom + 1); \
            func(handle, ((RECT *)rect)->left + 1, ((RECT *)rect)->top + 1, \
					                         ((RECT *)rect)->right - 1, ((RECT *)rect)->bottom - 1)

void frg_paint_object_msg()
{
    InvalidateRect(g_main_hwnd, &main_rect1, TRUE);
    InvalidateRect(g_main_hwnd, &main_rect2, TRUE);
}

void frg_paint_fps_msg()
{
    InvalidateRect(g_fps_hwnd, &fps_rect, FALSE);
}

void frg_post_quit_msg()
{
    PostQuitMessage(g_main_hwnd);
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

static void paint_single_db_object(HDC hdc, RECT *rect, void *rgb, int rgb_w,
                                   int rgb_h, const char *similarity)
{
    MYBITMAP my_bmp;
    BITMAP bmp;
    // printf("l, t, b, b: %d, %d, %d, %d\n",
    //        rect->left, rect->right, rect->top, rect->bottom);
    SetPenColor(hdc, TEXT_COLOR);
    PaintRectBold(Rectangle, hdc, rect);
    TextOut(hdc, rect->left + 2, rect->top + 2 , similarity);

    my_bmp.flags = MYBMP_TYPE_RGB | MYBMP_FLOW_DOWN | MYBMP_RGBSIZE_3;
    my_bmp.frames = 1;
    my_bmp.depth = 24;
    my_bmp.alpha = 0;
    my_bmp.transparent = 0;
    my_bmp.w = rgb_w;
    my_bmp.h = rgb_h;
    my_bmp.pitch = rgb_w * 3;
    my_bmp.size = rgb_w * rgb_h * 3;
    my_bmp.bits = rgb;

    if (!ExpandMyBitmap (hdc, &bmp, &my_bmp, NULL, 0)) {
        FillBoxWithBitmap(hdc, rect->left + 4, rect->top + 20 + 4, rgb_w, rgb_h, &bmp);
        UnloadBitmap(&bmp);
    }
}

static void paint_single_rect(HDC hdc, RECT *select) {
    SetPenColor(hdc, TEXT_COLOR);
    PaintRectBold(Rectangle, hdc, select);
}

static void paint_object(HDC hdc)
{
    struct frg_object *obj = NULL;
    float x_scale, y_scale;
    RECT rect;
    int i;
    char similarity[32];
    struct frg_group *group = frg_get_group();
    if (!group)
        return;
    obj = &group->objects[0];
    x_scale = (float)DISP_W / (float)group->rt_image_width;
    y_scale = (float)DISP_H / (float)group->rt_image_height;
    if (obj->x < 0) {
        obj->x = 0;
    }
    rect.left = obj->x * x_scale;
    rect.right = (obj->x + obj->w) * x_scale;
    rect.top = obj->y * y_scale;
    rect.bottom = (obj->y + obj->h) * y_scale;
    assert(!obj->rgb);
    paint_single_rect(hdc, &rect);

    for (i = 1; i < group->count; i++) {
        struct frg_object *object = &group->objects[i];
        RECT select;
        select.left = DATA_BASE_RECT_X;
        select.top = DATA_BASE_RECT_Y + object->index * (object->h + 8 + 20 + 16) + 16;
        select.right = select.left + object->w + 8;
        select.bottom = select.top + object->h + 8 + 20;
        snprintf(similarity, sizeof(similarity), "similar: %0.2f", object->match);
        paint_single_db_object(hdc, &select, object->rgb, object->w, object->h, similarity);
    }

    free_frg_group(group, 1);
}

int frg_paint_object(HWND hwnd)
{
    HDC hdc;
    hdc = BeginPaint(hwnd);
    paint_object(hdc);
    EndPaint(hwnd, hdc);
    return 0;
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
        mDrawText(hdc, "Rockchip FACE Recognition Demo", -1, 40, 0, g_rcScr.right, CAPTION_H,
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
        fps_show(hdc, frg_get_fps());
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
        fps_show(hdc, frg_get_fps());

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

static LRESULT database_wnd_proc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
    HDC hdc;
    switch(message){
    case MSG_CREATE:
        hdc = BeginPaint (hwnd);
        SetBkColor (hdc, CAPTION_BKCOLOR);
        SetPenColor(hdc, TEXT_COLOR);
        PaintRectBold(Rectangle, hdc, &g_database_rect);
        EndPaint (hwnd, hdc);
        break;
    case MSG_PAINT:
        hdc = BeginPaint (hwnd);
        EndPaint (hwnd, hdc);
        break;
    }
    return (*old_database_proc)(hwnd, message, w_param, l_param);
}

HWND database_create(HWND mainhwnd)
{
    HWND hwnd;

    hwnd = CreateWindow(CTRL_STATIC, NULL, WS_CHILD | WS_VISIBLE | SS_SIMPLE,
                        _ID_DATABASE_ID, DATA_BASE_RECT_X, DATA_BASE_RECT_Y,
                        DATA_BASE_RECT_W, DATA_BASE_RECT_H, mainhwnd, 0);
    if (hwnd <= HWND_NULL)
        return hwnd;
    SetWindowBkColor(hwnd, CAPTION_BKCOLOR);
    SetWindowElementAttr(hwnd, WE_FGC_WINDOW, TEXT_COLOR);
    old_database_proc = SetWindowCallbackProc(hwnd, database_wnd_proc);
    PostMessage (hwnd, MSG_CREATE, 0, 0);
    return hwnd;
}

void database_destory(HWND hwnd)
{
    DestroyWindow(hwnd);
}

int frg_ui_init(HWND hwnd)
{
    g_main_hwnd = hwnd;
    g_caption_hwnd = caption_create(hwnd);
    g_fps_hwnd = fps_create(hwnd);
    // g_database_hwnd = database_create(hwnd); // || g_database_hwnd <= HWND_NULL)
    if (g_caption_hwnd <= HWND_NULL
        || g_fps_hwnd <= HWND_NULL)
        return -1;
    else
        return 0;
}

int frg_ui_deinit(HWND hwnd)
{
    if (g_fps_hwnd > HWND_NULL)
        fps_destory(g_fps_hwnd);
    if (g_caption_hwnd > HWND_NULL)
        caption_destory(g_caption_hwnd);
    if (g_database_hwnd > HWND_NULL)
        database_destory(g_database_hwnd);
    g_main_hwnd = 0;
    return 0;
}
