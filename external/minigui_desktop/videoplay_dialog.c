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

//编码
#include <libavcodec/avcodec.h>
//封装格式处理
#include <libavformat/avformat.h>
//像素处理
#include <libswscale/swscale.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "common.h"

#define SLIDE_DISTANCE 100
#define WHOLE_BUTTON_NUM 1

static pthread_t videoplay_t = NULL;
static BITMAP mBitMap;
static unsigned char rgb_buff[LCD_W * LCD_H * 4];
static HWND hMainWnd;
static int videoplay_exit = 0;
pthread_mutex_t mutex;
static int file_total;
static int list_select = 0;
static struct directory_node *dir_node = 0;

static touch_pos touch_pos_down, touch_pos_up, touch_pos_old;

static const GAL_Rect msg_galrcMenu[] =
{
    {BACK_PINT_X, BACK_PINT_Y, BACK_PINT_W, BACK_PINT_H},
};

static int is_button(int x, int y, GAL_Rect rect)
{
    return ((x <= rect.x + rect.w) && (x >= rect.x) && (y <= rect.y + rect.h) && (y >= rect.y));
}

static int check_button(int x, int y)
{
    int i;
    for (i = 0; i < WHOLE_BUTTON_NUM; i++)
    {
        if (is_button(x, y, msg_galrcMenu[i])) return i;
    }
    return -1;
}

static void decodeYUV420SP(int *rgba, unsigned char *yuv420sp, int width, int height)
{
    int frameSize = width * height;
    for (int j = 0, yp = 0; j < height; j++)
    {
        int uvp = frameSize + (j >> 1) * width, u = 0, v = 0;
        for (int i = 0; i < width; i++, yp++)
        {
            int y = (0xff & ((int) yuv420sp[yp])) - 16;
            if (y < 0)
                y = 0;
            if ((i & 1) == 0)
            {
                v = (0xff & yuv420sp[uvp++]) - 128;
                u = (0xff & yuv420sp[uvp++]) - 128;
            }
            int y1192 = 1192 * y;
            int r = (y1192 + 1634 * v);
            int g = (y1192 - 833 * v - 400 * u);
            int b = (y1192 + 2066 * u);

            if (r < 0)
                r = 0;
            else if (r > 262143)
                r = 262143;
            if (g < 0)
                g = 0;
            else if (g > 262143)
                g = 262143;
            if (b < 0)
                b = 0;
            else if (b > 262143)
                b = 262143;

            rgba[yp] = 0xff000000 | ((b << 6) & 0xff0000)
                       | ((g >> 2) & 0xff00) | ((r >> 10) & 0xff);
//              rgba[yp] = 0xff000000 | ((r << 6) & 0xff0000)
//                      | ((g >> 2) & 0xff00) | ((b >> 10) & 0xff);
        }
    }
}

static void decodeYUV420P(unsigned char *yuv420_y, unsigned char *yuv420_u, unsigned char *yuv420_v, int *rgba, int width, int height)
{
    int frameSize = width * height;
    for (int j = 0, yp = 0; j < height; j++)
    {
        int up = (j >> 2) * width;
        int vp = (j >> 2) * width;
        //int up = 0;
        //int vp = 0;
        int u = 0, v = 0;
        for (int i = 0; i < width; i++, yp++)
        {
            int y = (0xff & ((int) yuv420_y[yp])) - 16;
            if (y < 0)
                y = 0;
            if ((i & 1) == 0)
            {
                u = (0xff & yuv420_u[up++]) - 128;
                v = (0xff & yuv420_v[vp++]) - 128;
            }
            int y1192 = 1192 * y;
            int r = (y1192 + 1634 * v);
            int g = (y1192 - 833 * v - 400 * u);
            int b = (y1192 + 2066 * u);

            if (r < 0)
                r = 0;
            else if (r > 262143)
                r = 262143;
            if (g < 0)
                g = 0;
            else if (g > 262143)
                g = 262143;
            if (b < 0)
                b = 0;
            else if (b > 262143)
                b = 262143;

            rgba[yp] = 0xff000000 | ((b << 6) & 0xff0000)
                       | ((g >> 2) & 0xff00) | ((r >> 10) & 0xff);
//              rgba[yp] = 0xff000000 | ((r << 6) & 0xff0000)
//                      | ((g >> 2) & 0xff00) | ((b >> 10) & 0xff);
        }
    }
}

static int Table_fv1[256] = { -180, -179, -177, -176, -174, -173, -172, -170, -169, -167, -166, -165, -163, -162, -160, -159, -158, -156, -155, -153, -152, -151, -149, -148, -146, -145, -144, -142, -141, -139, -138, -137,  -135, -134, -132, -131, -130, -128, -127, -125, -124, -123, -121, -120, -118, -117, -115, -114, -113, -111, -110, -108, -107, -106, -104, -103, -101, -100, -99, -97, -96, -94, -93, -92, -90,  -89, -87, -86, -85, -83, -82, -80, -79, -78, -76, -75, -73, -72, -71, -69, -68, -66, -65, -64, -62, -61, -59, -58, -57, -55, -54, -52, -51, -50, -48, -47, -45, -44, -43, -41, -40, -38, -37,  -36, -34, -33, -31, -30, -29, -27, -26, -24, -23, -22, -20, -19, -17, -16, -15, -13, -12, -10, -9, -8, -6, -5, -3, -2, 0, 1, 2, 4, 5, 7, 8, 9, 11, 12, 14, 15, 16, 18, 19, 21, 22, 23, 25, 26, 28, 29, 30, 32, 33, 35, 36, 37, 39, 40, 42, 43, 44, 46, 47, 49, 50, 51, 53, 54, 56, 57, 58, 60, 61, 63, 64, 65, 67, 68, 70, 71, 72, 74, 75, 77, 78, 79, 81, 82, 84, 85, 86, 88, 89, 91, 92, 93, 95, 96, 98, 99, 100, 102, 103, 105, 106, 107, 109, 110, 112, 113, 114, 116, 117, 119, 120, 122, 123, 124, 126, 127, 129, 130, 131, 133, 134, 136, 137, 138, 140, 141, 143, 144, 145, 147, 148,  150, 151, 152, 154, 155, 157, 158, 159, 161, 162, 164, 165, 166, 168, 169, 171, 172, 173, 175, 176, 178 };
static int Table_fv2[256] = { -92, -91, -91, -90, -89, -88, -88, -87, -86, -86, -85, -84, -83, -83, -82, -81, -81, -80, -79, -78, -78, -77, -76, -76, -75, -74, -73, -73, -72, -71, -71, -70, -69, -68, -68, -67, -66, -66, -65, -64, -63, -63, -62, -61, -61, -60, -59, -58, -58, -57, -56, -56, -55, -54, -53, -53, -52, -51, -51, -50, -49, -48, -48, -47, -46, -46, -45, -44, -43, -43, -42, -41, -41, -40, -39, -38, -38, -37, -36, -36, -35, -34, -33, -33, -32, -31, -31, -30, -29, -28, -28, -27, -26, -26, -25, -24, -23, -23, -22, -21, -21, -20, -19, -18, -18, -17, -16, -16, -15, -14, -13, -13, -12, -11, -11, -10, -9, -8, -8, -7, -6, -6, -5, -4, -3, -3, -2, -1, 0, 0, 1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 8, 9, 10, 10, 11, 12, 12, 13, 14, 15, 15, 16, 17, 17, 18, 19, 20, 20, 21, 22, 22, 23, 24, 25, 25, 26, 27, 27, 28, 29, 30, 30, 31, 32, 32, 33, 34, 35, 35, 36, 37, 37, 38, 39, 40, 40, 41, 42, 42, 43, 44, 45, 45, 46, 47, 47, 48, 49, 50, 50, 51, 52, 52, 53, 54, 55, 55, 56, 57, 57, 58, 59, 60, 60, 61, 62, 62, 63, 64, 65, 65, 66, 67, 67, 68, 69, 70, 70, 71, 72, 72, 73, 74, 75, 75, 76, 77, 77, 78, 79, 80, 80, 81, 82, 82, 83, 84, 85, 85, 86, 87, 87, 88, 89, 90, 90 };
static int Table_fu1[256] = { -44, -44, -44, -43, -43, -43, -42, -42, -42, -41, -41, -41, -40, -40, -40, -39, -39, -39, -38, -38, -38, -37, -37, -37, -36, -36, -36, -35, -35, -35, -34, -34, -33, -33, -33, -32, -32, -32, -31, -31, -31, -30, -30, -30, -29, -29, -29, -28, -28, -28, -27, -27, -27, -26, -26, -26, -25, -25, -25, -24, -24, -24, -23, -23, -22, -22, -22, -21, -21, -21, -20, -20, -20, -19, -19, -19, -18, -18, -18, -17, -17, -17, -16, -16, -16, -15, -15, -15, -14, -14, -14, -13, -13, -13, -12, -12, -11, -11, -11, -10, -10, -10, -9, -9, -9, -8, -8, -8, -7, -7, -7, -6, -6, -6, -5, -5, -5, -4, -4, -4, -3, -3, -3, -2, -2, -2, -1, -1, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 20, 20, 20, 21, 21, 22, 22, 22, 23, 23, 23, 24, 24, 24, 25, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31, 31, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35, 36, 36, 36, 37, 37, 37, 38, 38, 38, 39, 39, 39, 40, 40, 40, 41, 41, 41, 42, 42, 42, 43, 43 };
static int Table_fu2[256] = { -227, -226, -224, -222, -220, -219, -217, -215, -213, -212, -210, -208, -206, -204, -203, -201, -199, -197, -196, -194, -192, -190, -188, -187, -185, -183, -181, -180, -178, -176, -174, -173, -171, -169, -167, -165, -164, -162, -160, -158, -157, -155, -153, -151, -149, -148, -146, -144, -142, -141, -139, -137, -135, -134, -132, -130, -128, -126, -125, -123, -121, -119, -118, -116, -114, -112, -110, -109, -107, -105, -103, -102, -100, -98, -96, -94, -93, -91, -89, -87, -86, -84, -82, -80, -79, -77, -75, -73, -71, -70, -68, -66, -64, -63, -61, -59, -57, -55, -54, -52, -50, -48, -47, -45, -43, -41, -40, -38, -36, -34, -32, -31, -29, -27, -25, -24, -22, -20, -18, -16, -15, -13, -11, -9, -8, -6, -4, -2, 0, 1, 3, 5, 7, 8, 10, 12, 14, 15, 17, 19, 21, 23, 24, 26, 28, 30, 31, 33, 35, 37, 39, 40, 42, 44, 46, 47, 49, 51, 53, 54, 56, 58, 60, 62, 63, 65, 67, 69, 70, 72, 74, 76, 78, 79, 81, 83, 85, 86, 88, 90, 92, 93, 95, 97, 99, 101, 102, 104, 106, 108, 109, 111, 113, 115, 117, 118, 120, 122, 124, 125, 127, 129, 131, 133, 134, 136, 138, 140, 141, 143, 145, 147, 148, 150, 152, 154, 156, 157, 159, 161, 163, 164, 166, 168, 170, 172, 173, 175, 177, 179, 180, 182, 184, 186, 187, 189, 191, 193, 195, 196, 198, 200, 202, 203, 205, 207, 209, 211, 212, 214, 216, 218, 219, 221, 223, 225 };

int YV12ToBGR24_Table(unsigned char *pY, unsigned char *pU, unsigned char *pV, unsigned char *pBGR24, int width, int height)
{
    if (width < 1 || height < 1 || pBGR24 == NULL)
        return -1;
    const long len = width * height;
    unsigned char *yData = pY;
    unsigned char *vData = pU;//&yData[len];
    unsigned char *uData = pV;//&vData[len >> 2];

    int bgr[4];
    int yIdx, uIdx, vIdx, idx;
    int rdif, invgdif, bdif;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            yIdx = i * width + j;
            vIdx = (i / 2) * (width / 2) + (j / 2);
            uIdx = vIdx;

            rdif = Table_fv1[vData[vIdx]];
            invgdif = Table_fu1[uData[uIdx]] + Table_fv2[vData[vIdx]];
            bdif = Table_fu2[uData[uIdx]];

            bgr[0] = yData[yIdx] + bdif;
            bgr[1] = yData[yIdx] - invgdif;
            bgr[2] = yData[yIdx] + rdif;
            bgr[3] = 255;

            for (int k = 0; k < 4; k++)
            {
                idx = (i * width + j) * 4 + k;
                if (bgr[k] >= 0 && bgr[k] <= 255)
                    pBGR24[idx] = bgr[k];
                else
                    pBGR24[idx] = (bgr[k] < 0) ? 0 : 255;
            }
        }
    }
    return 0;
}

void print_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    //printf("second:%ld\n",tv.tv_sec);
    //printf("millisecond:%ld\n",tv.tv_sec*1000 + tv.tv_usec/1000);
    printf("microsecond:%ld\n", tv.tv_sec * 1000000 + tv.tv_usec);
}

void *videoplay(void *arg)
{
    //const char *input = "/usr/local/share/3.mp4";
    char input[256];
    struct file_node *file_node;
    int i;
    //const char *output = "test.yuv";

    file_node = dir_node->file_node_list;

    for (i = 0; i < dir_node->file_sel; i++)
    {
        if (file_node)
            file_node = file_node->next_node;
    }
    snprintf(input, sizeof(input), "%s/%s", dir_node->patch, file_node->name);
    printf("%s video file = %s\n", __func__, input);
    av_register_all();
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx, input, NULL, NULL) != 0)
    {
        printf("%s", "无法打开输入视频文件");
        return;
    }

    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        printf("%s", "无法获取视频文件信息");
        return;
    }

    int v_stream_idx = -1;
    i = 0;
    //number of streams
    for (; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            v_stream_idx = i;
            break;
        }
    }

    if (v_stream_idx == -1)
    {
        printf("%s", "找不到视频流\n");
        return;
    }

    AVCodecContext *pCodecCtx = pFormatCtx->streams[v_stream_idx]->codec;
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL)
    {
        printf("%s", "找不到解码器\n");
        return;
    }

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        printf("%s", "解码器无法打开\n");
        return;
    }

    printf("视频的文件格式：%s\n", pFormatCtx->iformat->name);
    printf("视频时长：%d\n", (pFormatCtx->duration) / 1000000);
    printf("视频的宽高：%d,%d\n", pCodecCtx->width, pCodecCtx->height);
    printf("解码器的名称：%s\n", pCodec->name);

    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));

    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFrameYUV = av_frame_alloc();
    uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, LCD_W, LCD_H));
    avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, LCD_W, LCD_H);

    struct SwsContext *sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                 LCD_W, LCD_H, AV_PIX_FMT_YUV420P,
                                 SWS_BICUBIC, NULL, NULL, NULL);
    int got_picture, ret;

    //FILE *fp_yuv = fopen(output, "wb+");

    int frame_count = 0;

    while ((av_read_frame(pFormatCtx, packet) >= 0) && (videoplay_exit == 0))
    {
        if (packet->stream_index == v_stream_idx)
        {
            //printf("1\n");
            //print_time();
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            if (ret < 0)
            {
                printf("%s", "解码错误");
                return;
            }
            //printf("2\n");
            //print_time();
            if (got_picture)
            {
                //printf("3\n");
                //print_time();
#if 0
                sws_scale(sws_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
                          pFrameYUV->data, pFrameYUV->linesize);
                //printf("4\n");
                //print_time();
                decodeYUV420P(pFrameYUV->data[0], pFrameYUV->data[1], pFrameYUV->data[2], rgb_buff, LCD_W, LCD_H);
#else
                pthread_mutex_lock(&mutex);
                decodeYUV420P(pFrame->data[0], pFrame->data[1], pFrame->data[2], rgb_buff, LCD_W, LCD_H);
                pthread_mutex_unlock(&mutex);
#endif
                //printf("5\n");
                //print_time();
                InvalidateRect(hMainWnd, &msg_rcDialog, TRUE);
                int y_size = pCodecCtx->width * pCodecCtx->height;
                //fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);
                //fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);
                //fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);

                frame_count++;
                //printf("解码第%d帧\n",frame_count);
            }
        }

        av_free_packet(packet);
    }

    //fclose(fp_yuv);

    av_frame_free(&pFrame);

    avcodec_close(pCodecCtx);

    avformat_free_context(pFormatCtx);
    videoplay_t = NULL;
    SendMessage(hMainWnd, MSG_VIDEOPLAY_END, 0, 0);
    return 0;
}

static int loadres(void)
{
    return 0;
}

static void unloadres(void)
{

}

static void videoplay_enter(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    //videoplay();
    if (videoplay_t == NULL)
    {
        videoplay_exit = 0;
        pthread_create(&videoplay_t, NULL, videoplay, (void *)"videoplay");
    }
}

static void menu_back(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    videoplay_exit = 1;
    //if (videoplay_t != NULL)
    //    pthread_join(videoplay_t, NULL);
    EndDialog(hWnd, wParam);
}

static LRESULT videoplay_dialog_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
        loadres();
        memset(rgb_buff, 0xff, LCD_W * LCD_H * 4);
        //SetTimer(hWnd, _ID_TIMER_VIDEOPLAY, TIMER_VIDEOPLAY);
        //InvalidateRect(hWnd, &msg_rcDialog, TRUE);
        hMainWnd = hWnd;
        pthread_mutex_init(&mutex, NULL);
        if (videoplay_t == NULL)
        {
            videoplay_exit = 0;
            pthread_create(&videoplay_t, NULL, videoplay, (void *)"videoplay");
        }
        return 0;
    }
    case MSG_TIMER:
        if (wParam == _ID_TIMER_VIDEOPLAY)
        {
            //InvalidateRect(hWnd, &msg_rcDialog, TRUE);
        }
        break;
    case MSG_KEYDOWN:
        switch (wParam)
        {
        case KEY_ENTER_FUNC:
            //videoplay();
            if (videoplay_t == NULL)
            {
                videoplay_exit = 0;
                pthread_create(&videoplay_t, NULL, videoplay, (void *)"videoplay");
            }
            break;
        case KEY_DOWN_FUNC:
            break;
        case KEY_UP_FUNC:
            break;
        case KEY_EXIT_FUNC:
            videoplay_exit = 1;
            //if (videoplay_t != NULL)
            //    pthread_join(videoplay_t, NULL);
            break;
        }
        break;
    case MSG_COMMAND:
        break;
    case MSG_PAINT:
    {
        int i;
        pthread_mutex_lock(&mutex);
        FillBoxWithBitmap(hdc, BACK_PINT_X, BACK_PINT_Y,
                          BACK_PINT_W, BACK_PINT_H,
                          &back_bmap);
        hdc = BeginPaint(hWnd);
        SelectFont(hdc, logfont);
        InitBitmap(HDC_SCREEN, LCD_W, LCD_H, LCD_W * 4, rgb_buff, &mBitMap);
        FillBoxWithBitmap(hdc, 0, 0, LCD_W, LCD_H, &mBitMap);
        EndPaint(hWnd, hdc);
        pthread_mutex_unlock(&mutex);
        break;
    }
    case MSG_VIDEOPLAY_END:
        EndDialog(hWnd, wParam);
        break;
    case MSG_DESTROY:
        unloadres();
        pthread_mutex_destroy(&mutex);
        return 0;
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
        if (witch_button > 0 && witch_button < WHOLE_BUTTON_NUM)
        {
            videoplay_enter(hWnd, wParam, lParam);
        }
        touch_pos_old.x = touch_pos_up.x;
        touch_pos_old.y = touch_pos_up.y;
        EnableScreenAutoOff();
        break;
    }
    return DefaultDialogProc(hWnd, message, wParam, lParam);
}

void creat_videoplay_dialog(HWND hWnd, struct directory_node *node)
{
    DLGTEMPLATE PicPreViewDlg = {WS_VISIBLE, WS_EX_NONE | WS_EX_AUTOSECONDARYDC,
                                 0, 0,
                                 LCD_W, LCD_H,
                                 DESKTOP_DLG_STRING, 0, 0, 0, NULL, 0
                                };
    list_select = node->file_sel;
    file_total = node->total;
    dir_node = node;
    DialogBoxIndirectParam(&PicPreViewDlg, hWnd, videoplay_dialog_proc, 0L);
}
