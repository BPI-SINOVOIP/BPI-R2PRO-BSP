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
#define WHOLE_BUTTON_NUM 1

static BITMAP list_sel_bmap;
static BITMAP type_bmap[FILE_TYPE_MAX];
static int file_sel = 0;
static struct directory_node *dir_node = 0;
static struct directory_node *cur_dir_node = 0;
static int filter_type = 0;
static char *pTitle = 0;
static int batt = 0;
static int file_page, page_choice_dir, choice_dir;
static touch_pos touch_pos_down, touch_pos_up, touch_pos_old;

static int check_button(int x, int y)
{
    if ((x <= BACK_PINT_X + BACK_PINT_W) &&
            (x >= BACK_PINT_X) &&
            (y <= BACK_PINT_Y + BACK_PINT_H) &&
            (y >= BACK_PINT_Y))
        return 0;
    if (y > BROWSER_LIST_STR_PINT_Y)
        return (file_page * FILE_NUM_PERPAGE + ((y - BROWSER_LIST_STR_PINT_Y) / BROWSER_LIST_STR_PINT_SPAC) + 1);
    return -1;
}

static char music_file_ext_name[][6] =
{
    ".mp3",
    ".wav"
};

static char pic_file_ext_name[][6] =
{
    ".jpg",
    ".bmp",
    ".png"
};

static char game_file_ext_name[][17] =
{
    ".gbc",
    ".sav",
    ".smc",
    ".nes",
    ".fba",
    ".bin",
    ".smd",
    ".gg",
    ".zip",
    ".gba",
    ".gb",
    ".z64",
    ".n64",
    ".v64",
    ".ccd",
    ".gen",
    ".img"
};

static char zip_file_ext_name[][6] =
{
    ".zip"
};

static char video_file_ext_name[][6] =
{
    ".mp4"
};

static int check_file_type(char *name)
{
    int i;

    for (i = 0; i < sizeof(music_file_ext_name) / sizeof(music_file_ext_name[0]); i++)
    {
        if (strstr(name, music_file_ext_name[i]) != 0)
            return FILE_MUSIC;
    }

    for (i = 0; i < sizeof(pic_file_ext_name) / sizeof(pic_file_ext_name[0]); i++)
    {
        if (strstr(name, pic_file_ext_name[i]) != 0)
            return FILE_PIC;
    }

    for (i = 0; i < sizeof(game_file_ext_name) / sizeof(game_file_ext_name[0]); i++)
    {
        if (strstr(name, game_file_ext_name[i]) != 0)
            return FILE_GAME;
    }

    for (i = 0; i < sizeof(zip_file_ext_name) / sizeof(zip_file_ext_name[0]); i++)
    {
        if (strstr(name, zip_file_ext_name[i]) != 0)
            return FILE_ZIP;
    }

    for (i = 0; i < sizeof(video_file_ext_name) / sizeof(video_file_ext_name[0]); i++)
    {
        if (strstr(name, video_file_ext_name[i]) != 0)
            return FILE_VIDEO;
    }

    return FILE_OTHER;
}

static void get_file_list(struct directory_node *node)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    struct file_node *file_node;
    int len;
    char *dir = node->patch;

    file_node = node->file_node_list;
    if ((dp = opendir(dir)) == NULL)
    {
        printf("open %s error\n", dir);
        return ;
    }

    chdir(dir);
    while ((entry = readdir(dp)) != NULL)
    {
        lstat(entry->d_name, &statbuf);
        if (S_ISDIR(statbuf.st_mode))
        {
            struct file_node *file_node_temp;
            char *name;

            //printf("name:%s/  len = %d  type:%d  inode:%d\n", entry->d_name, strlen(entry->d_name),
            //       statbuf.st_mode,statbuf.st_size,entry->d_ino);
            if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
                continue;
            file_node_temp = malloc(sizeof(struct file_node));
            memset(file_node_temp, 0, sizeof(struct file_node));
            len = strlen(entry->d_name) + 2;
            name = malloc(len);
            memset(name, 0, len);
            sprintf(name, "%s", entry->d_name);
            file_node_temp->pre_node = file_node;
            file_node_temp->type = FILE_FOLDER;
            file_node_temp->name = name;
            if (file_node)
            {
                file_node->next_node = file_node_temp;
                file_node_temp->pre_node = file_node;
            }
            else
            {
                node->file_node_list = file_node_temp;
            }
            file_node = file_node_temp;
            //printf("file_node_temp->pre_node = 0x%x\n", file_node_temp->pre_node);
            //printf("file_node_temp->name = %s\n", file_node_temp->name);
            node->total ++;
        }
    }
    chdir("..");
    closedir(dp);
    if ((dp = opendir(dir)) == NULL)
    {
        printf("open %s error\n", dir);
        return;
    }
    chdir(dir);
    while ((entry = readdir(dp)) != NULL)
    {
        lstat(entry->d_name, &statbuf);
        if (!S_ISDIR(statbuf.st_mode))
        {
            struct file_node *file_node_temp;
            char *name;
            int type;

            //printf("name:%s/  len = %d  type:%d  inode:%d\n",entry->d_name, strlen(entry->d_name),
            //       statbuf.st_mode,statbuf.st_size,entry->d_ino);
            type = check_file_type(entry->d_name);
            if ((filter_type != FILTER_FILE_NO) && (filter_type != type))
                continue;

            file_node_temp = malloc(sizeof(struct file_node));
            memset(file_node_temp, 0, sizeof(struct file_node));
            len = strlen(entry->d_name) + 2;
            name = malloc(len);
            memset(name, 0, len);
            sprintf(name, "%s", entry->d_name);
            file_node_temp->pre_node = file_node;
            file_node_temp->name = name;
            file_node_temp->type = type;//check_file_type(name);
            if (file_node)
            {
                file_node->next_node = file_node_temp;
                file_node_temp->pre_node = file_node;
            }
            else
            {
                node->file_node_list = file_node_temp;
            }
            file_node = file_node_temp;

            node->total ++;
        }
    }
    chdir("..");
    closedir(dp);
    //printf("node->total = %d\n", node->total);
}

static struct directory_node *new_dir_node(char *dir)
{
    int len;
    struct directory_node *dir_node_temp;

    dir_node_temp = malloc(sizeof(struct directory_node));
    memset(dir_node_temp, 0, sizeof(struct directory_node));
    len = strlen(dir) + 2;
    dir_node_temp->patch = malloc(len);
    memset(dir_node_temp->patch, 0, len);
    sprintf(dir_node_temp->patch, "%s", dir);
    //printf("dir_node_temp->patch = %s\n", dir_node_temp->patch);
    get_file_list(dir_node_temp);

    return dir_node_temp;
}

static struct directory_node *free_dir_node(struct directory_node *node)
{
    struct directory_node *dir_node_temp;
    struct file_node *file_node_temp;

    if (node == 0)
        return 0;

    dir_node_temp = node->pre_node;

    if (node->patch)
    {
        free(node->patch);
        node->patch = 0;
    }
    if (node->pre_node)
        node->pre_node->next_node = 0;
    file_node_temp = node->file_node_list;
    while (file_node_temp)
    {
        struct file_node *file_node_next = file_node_temp->next_node;

        if (file_node_temp->name)
            free(file_node_temp->name);

        file_node_temp = file_node_next;
    }
    free(node);

    return dir_node_temp;
}

static void enter_folder(HWND hWnd, struct directory_node *node)
{
    int i;
    struct file_node *file_node_temp = node->file_node_list;

    for (int i = 0; i < node->file_sel; i++)
    {
        if (file_node_temp)
            file_node_temp = file_node_temp->next_node;
        else
            return;
    }
    if (file_node_temp->type == FILE_FOLDER)
    {
        int len;
        char *dir;
        struct directory_node *dir_node_temp;

        len = strlen(node->patch);
        len += strlen(file_node_temp->name);
        len += 2;
        dir = malloc(len);
        memset(dir, 0, len);
        sprintf(dir, "%s/%s", node->patch, file_node_temp->name);
        dir_node_temp = new_dir_node(dir);
        cur_dir_node->next_node = dir_node_temp;
        dir_node_temp->pre_node = cur_dir_node;
        cur_dir_node = dir_node_temp;
        free(dir);
    }
    else if (file_node_temp->type == FILE_MUSIC)
    {
        creat_audioplay_dialog(hWnd, cur_dir_node);
    }
    else if (file_node_temp->type == FILE_GAME)
    {
        char cmd[512];
        int sel = 0;
        char path[256] = {0};
        char cfgfilename[256] = {0};
        char cfgfilepath[256] = {0};
        int i = 0;
        char *tmp = path;
        char *gamefilename = cfgfilename;
        char *gamefilecfg = cfgfilepath;
        sel = strlen(file_node_temp->name);
        for (i = sel - 1; i > 0; i--)
        {
            if (file_node_temp->name[i] != '.')
            {
                *tmp = file_node_temp->name[i];
                tmp++;
            }
            else
                break;
        }
        for (i = 0; i < sel - 1; i++)
        {
            if (file_node_temp->name[i] != '.')
            {
                *gamefilename = file_node_temp->name[i];
                gamefilename++;
            }
            else
                break;
        }

        sprintf(cfgfilepath, "%s/%s%s", node->patch, cfgfilename, DEFRETROARCHNAME);

        if ((access(cfgfilepath, F_OK)) != -1)
        {
            //cfgfilepath exists
            sprintf(cfgfilepath, "%s", DEFRETROARCH);
        }
        else
        {
            //cfgfilepath don't exists
            sprintf(cfgfilepath, "%s", DEFRETROARCH);
        }

        if (!strcasecmp(path, "piz"))
            sel = 0;
        else if (!strcasecmp(path, "dms") || !strcasecmp(path, "gg") || !strcasecmp(path, "nib") || !strcasecmp(path, "neg"))
            sel = 5;
        else if (!strcasecmp(path, "sen"))
            sel = 1;
        else if (!strcasecmp(path, "cms") || !strcasecmp(path, "cfs"))
            sel = 6;
        else if (!strcasecmp(path, "abg") || !strcasecmp(path, "cbg") || !strcasecmp(path, "bg"))
            sel = 7;
        else if (!strcasecmp(path, "46n") || !strcasecmp(path, "46z") || !strcasecmp(path, "46v"))
            sel = 8;
        else if (!strcasecmp(path, "dcc") || !strcasecmp(path, "gmi"))
            sel = 9;
        else sel = 0;

        DisableKeyMessage();
        DisableScreenAutoOff();
        sprintf(path, "%s/%s", node->patch, file_node_temp->name);
        sprintf(cmd, "/data/start_game.sh %d \"%s\" \"%s\"", sel, cfgfilepath, path);
        system("touch /tmp/.minigui_freeze");
        system(cmd);
        system("rm /tmp/.minigui_freeze");
        EnableKeyMessage();
        EnableScreenAutoOff();
        InvalidateRect(hWnd, &msg_rcBg, TRUE);
    }
    else if (file_node_temp->type == FILE_PIC)
    {
        creat_picpreview_dialog(hWnd, cur_dir_node);
    }
    else if (file_node_temp->type == FILE_VIDEO)
    {
#ifdef ENABLE_VIDEO
        creat_videoplay_hw_dialog(hWnd, cur_dir_node);
#endif
    }
    else if (file_node_temp->type == FILE_ZIP)
    {
    }
}

static void free_file_list(struct directory_node *node)
{
    struct file_node *file_node_temp;

    if (node == 0)
        return;

    if (node->next_node)
        free_file_list(node->next_node);

    if (node->patch)
    {
        free(node->patch);
        node->patch = 0;
    }
    if (node->pre_node)
        node->pre_node->next_node = 0;
    file_node_temp = node->file_node_list;
    while (file_node_temp)
    {
        struct file_node *file_node_next = file_node_temp->next_node;

        if (file_node_temp->name)
            free(file_node_temp->name);

        file_node_temp = file_node_next;
    }
    free(node);
}

static void file_list_init(void)
{
    int len;

    if (filter_type == FILTER_FILE_PIC)
        dir_node = new_dir_node(BROWSER_PATH_PIC);
    else if (filter_type == FILTER_FILE_MUSIC)
        dir_node = new_dir_node(BROWSER_PATH_MUSIC);
    else if (filter_type == FILTER_FILE_GAME)
        dir_node = new_dir_node(BROWSER_PATH_GAME);
    else if (filter_type == FILTER_FILE_VIDEO)
        dir_node = new_dir_node(BROWSER_PATH_VIDEO);
    else
    {
        dir_node = 0;
        page_choice_dir = 1;
    }
    cur_dir_node = dir_node;
}

static void file_list_deinit(void)
{
    cur_dir_node = 0;

    free_file_list(dir_node);

    dir_node = 0;
}

static int loadres(void)
{
    int i;
    char img[128];
    char *respath = get_ui_image_path();

    snprintf(img, sizeof(img), "%slist_sel.png", respath);
    //printf("%s\n", img);
    if (LoadBitmap(HDC_SCREEN, &list_sel_bmap, img))
        return -1;
    for (i = 0; i < FILE_TYPE_MAX; i++)
    {
        snprintf(img, sizeof(img), "%simg_filetype%d.png", respath, i);
        //printf("%s\n", img);
        if (LoadBitmap(HDC_SCREEN, &type_bmap[i], img))
            return -1;
    }
    return 0;
}

static void unloadres(void)
{
    int i;
    UnloadBitmap(&list_sel_bmap);
    for (i = 0; i < FILE_TYPE_MAX; i++)
    {
        UnloadBitmap(&type_bmap[i]);
    }
}

static void menu_back(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    if (filter_type)
    {
        cur_dir_node = free_dir_node(cur_dir_node);
        if (cur_dir_node)
        {
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
        }
        else
        {
            dir_node = 0;
            EndDialog(hWnd, wParam);
        }
    }
    else
    {
        if (page_choice_dir)
        {
            EndDialog(hWnd, wParam);
        }
        else
        {
            cur_dir_node = free_dir_node(cur_dir_node);
            if (!cur_dir_node)
                page_choice_dir = 1;
            dir_node = 0;
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
        }
    }
}

static void browser_enter(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    if (!page_choice_dir)
    {
        enter_folder(hWnd, cur_dir_node);
        InvalidateRect(hWnd, &msg_rcBg, TRUE);
    }
    else
    {
        switch (lParam)
        {
        case 0:
            menu_back(hWnd, wParam, lParam);
            break;
        case 1:
            printf("open local\n");
            page_choice_dir = 0;
            choice_dir = 0;
            dir_node = new_dir_node(BROWSER_PATH_ROOT);
            cur_dir_node = dir_node;
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            break;
        case 2:
            printf("open sdcard\n");
            page_choice_dir = 0;
            choice_dir = 1;
            dir_node = new_dir_node(SDCARD_PATH_ROOT);
            cur_dir_node = dir_node;
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            break;
        case 3:
            printf("open udisk\n");
            page_choice_dir = 0;
            choice_dir = 2;
            dir_node = new_dir_node(UDISK_PATH_ROOT);
            cur_dir_node = dir_node;
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            break;
        default:
            break;
        }
    }
}

static LRESULT browser_dialog_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
        page_choice_dir = 0;
        choice_dir = 0;
        file_page = 0;
        file_list_init();
        batt = battery;
        SetTimer(hWnd, _ID_TIMER_BROWSER, TIMER_BROWSER);
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
        if (wParam == _ID_TIMER_BROWSER)
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
        }

        break;
    }
    case MSG_PAINT:
    {
        int i;
        int page;
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
        DrawText(hdc, pTitle, -1, &msg_rcTitle, DT_TOP);
        FillBox(hdc, TITLE_LINE_PINT_X, TITLE_LINE_PINT_Y, TITLE_LINE_PINT_W, TITLE_LINE_PINT_H);
        if (!page_choice_dir)
        {
            RECT msg_rcFilename;
            if (cur_dir_node->total == 0)
            {
                msg_rcFilename.left = BROWSER_LIST_STR_PINT_X;
                msg_rcFilename.top = BROWSER_LIST_STR_PINT_Y + BROWSER_LIST_STR_PINT_SPAC;
                msg_rcFilename.right = LCD_W - msg_rcFilename.left;
                msg_rcFilename.bottom = msg_rcFilename.top + BROWSER_LIST_STR_PINT_H;
                DrawText(hdc, res_str[RES_STR_NO_CONTENT], -1, &msg_rcFilename, DT_TOP);
            }
            page = (cur_dir_node->total + FILE_NUM_PERPAGE - 1) / FILE_NUM_PERPAGE;

            file_node_temp = cur_dir_node->file_node_list;
            for (i = 0; i < (cur_dir_node->file_sel / FILE_NUM_PERPAGE) * FILE_NUM_PERPAGE; i++)
            {
                if (file_node_temp->next_node)
                    file_node_temp = file_node_temp->next_node;
            }

            for (i = 0; i < FILE_NUM_PERPAGE; i++)
            {
                if (((cur_dir_node->file_sel / FILE_NUM_PERPAGE) * FILE_NUM_PERPAGE + i) >= cur_dir_node->total)
                    break;
                msg_rcFilename.left = BROWSER_LIST_STR_PINT_X + BROWSER_LIST_PIC_PINT_W;
                msg_rcFilename.top = BROWSER_LIST_STR_PINT_Y + BROWSER_LIST_STR_PINT_SPAC * i;
                msg_rcFilename.right = LCD_W - msg_rcFilename.left;
                msg_rcFilename.bottom = msg_rcFilename.top + BROWSER_LIST_STR_PINT_H;
                FillBoxWithBitmap(hdc, 20, msg_rcFilename.top, BROWSER_LIST_PIC_PINT_W, BROWSER_LIST_PIC_PINT_H, &type_bmap[file_node_temp->type]);
                if (i == (cur_dir_node->file_sel % FILE_NUM_PERPAGE))
                    FillBoxWithBitmap(hdc, 0, msg_rcFilename.top - 9, LCD_W, BROWSER_LIST_SEL_PINT_H, &list_sel_bmap);
                DrawText(hdc, file_node_temp->name, -1, &msg_rcFilename, DT_TOP);
                if (file_node_temp->next_node)
                    file_node_temp = file_node_temp->next_node;
                else
                    break;
            }

            if (page > 1)
            {
                for (i = 0; i < page; i++)
                {
                    int x;
                    if (page == 1)
                        x =  BROWSER_PAGE_DOT_X;
                    else if (page % 2)
                        x =  BROWSER_PAGE_DOT_X - page / 2 * BROWSER_PAGE_DOT_SPAC;
                    else
                        x =  BROWSER_PAGE_DOT_X - page / 2 * BROWSER_PAGE_DOT_SPAC + BROWSER_PAGE_DOT_SPAC / 2;

                    if (i == cur_dir_node->file_sel / FILE_NUM_PERPAGE)
                        FillCircle(hdc, x + i * BROWSER_PAGE_DOT_SPAC, BROWSER_PAGE_DOT_Y, BROWSER_PAGE_DOT_DIA);
                    else
                        Circle(hdc, x + i * BROWSER_PAGE_DOT_SPAC, BROWSER_PAGE_DOT_Y, BROWSER_PAGE_DOT_DIA);
                }
            }
        }
        else
        {
            RECT msg_rcDirlist;
            for (i = 0; i < 3; i++)
            {
                msg_rcDirlist.left = BROWSER_LIST_STR_PINT_X;
                msg_rcDirlist.top = BROWSER_LIST_STR_PINT_Y + BROWSER_LIST_STR_PINT_SPAC * i;
                msg_rcDirlist.right = LCD_W - msg_rcDirlist.left;
                msg_rcDirlist.bottom = msg_rcDirlist.top + BROWSER_LIST_STR_PINT_H;
                if (i == choice_dir)
                    FillBoxWithBitmap(hdc, 0, msg_rcDirlist.top - 9, LCD_W, BROWSER_LIST_SEL_PINT_H, &list_sel_bmap);
                DrawText(hdc, res_str[RES_STR_LOCAL + i], -1, &msg_rcDirlist, DT_TOP);
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
            cur_dir_node = free_dir_node(cur_dir_node);
            if (cur_dir_node)
            {
                InvalidateRect(hWnd, &msg_rcBg, TRUE);
            }
            else
            {
                dir_node = 0;
                EndDialog(hWnd, wParam);
            }
            break;
        case KEY_DOWN_FUNC:
            if (cur_dir_node->file_sel < (cur_dir_node->total - 1))
                cur_dir_node->file_sel++;
            else
                cur_dir_node->file_sel = 0;
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            break;
        case KEY_UP_FUNC:
            if (cur_dir_node->file_sel > 0)
                cur_dir_node->file_sel--;
            else
                cur_dir_node->file_sel = cur_dir_node->total - 1;
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            break;
        case KEY_ENTER_FUNC:
            enter_folder(hWnd, cur_dir_node);
            InvalidateRect(hWnd, &msg_rcBg, TRUE);
            break;
        }
        break;
    case MSG_COMMAND:
        break;
    case MSG_DESTROY:
        KillTimer(hWnd, _ID_TIMER_BROWSER);
        file_list_deinit();
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
        int witch_button;
        if (!page_choice_dir)
        {
            if (touch_pos_down.y - touch_pos_up.y > SLIDE_DISTANCE)
            {
                //printf("slide up\n");
                cur_dir_node->file_sel += FILE_NUM_PERPAGE;
                if (cur_dir_node->file_sel > (cur_dir_node->total - 1))
                    cur_dir_node->file_sel = (cur_dir_node->total - 1);
                file_page++;
                if (file_page > ((cur_dir_node->total + FILE_NUM_PERPAGE - 1) / FILE_NUM_PERPAGE - 1))
                    file_page = ((cur_dir_node->total + FILE_NUM_PERPAGE - 1) / FILE_NUM_PERPAGE - 1);
                InvalidateRect(hWnd, &msg_rcBg, TRUE);
            }
            else if (touch_pos_up.y - touch_pos_down.y > SLIDE_DISTANCE)
            {
                //printf("slide down\n");
                cur_dir_node->file_sel -= FILE_NUM_PERPAGE;
                if (cur_dir_node->file_sel < 0)
                    cur_dir_node->file_sel = 0;
                file_page--;
                if (file_page < 0)
                    file_page = 0;
                InvalidateRect(hWnd, &msg_rcBg, TRUE);
            }
            else
            {
                witch_button = check_button(touch_pos_up.x, touch_pos_up.y);
                //printf("file %d of %d\n",witch_button,cur_dir_node->total);
                if (witch_button == 0) menu_back(hWnd, wParam, lParam);
                if (witch_button > 0 && witch_button <= (cur_dir_node->total))
                {
                    cur_dir_node->file_sel = witch_button - 1;
                    InvalidateRect(hWnd, &msg_rcBg, TRUE);
                    browser_enter(hWnd, wParam, lParam);
                }
            }
        }
        else
        {
            witch_button = check_button(touch_pos_up.x, touch_pos_up.y);
            browser_enter(hWnd, wParam, witch_button);
        }
        touch_pos_old.x = touch_pos_up.x;
        touch_pos_old.y = touch_pos_up.y;
        EnableScreenAutoOff();
        break;
    }
    }

    return DefaultDialogProc(hWnd, message, wParam, lParam);
}

void creat_browser_dialog(HWND hWnd, int type, char *title)
{
    DLGTEMPLATE DesktopDlg = {WS_VISIBLE, WS_EX_NONE | WS_EX_AUTOSECONDARYDC,
                              0, 0,
                              LCD_W, LCD_H,
                              DESKTOP_DLG_STRING, 0, 0, 0, NULL, 0
                             };
    //DesktopDlg.controls = DesktopCtrl;
    pTitle = title;
    filter_type = type;

    DialogBoxIndirectParam(&DesktopDlg, hWnd, browser_dialog_proc, 0L);
}
