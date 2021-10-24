#include <stdio.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>

#define IMAGE_NAME_LEN 30
#define RES_PATH "/usr/local/share/minigui/res/images/"
char dot_joint[][IMAGE_NAME_LEN] =          {"dot_joint.png"};
char dot_mobilenet[][IMAGE_NAME_LEN] =      {"dot_mobilenet.png"};
char dot_ssd[][IMAGE_NAME_LEN] =            {"dot_ssd.png"};
char fps_bg_9[][IMAGE_NAME_LEN] =           {"fps_bg.9.png"};
char img_logo[][IMAGE_NAME_LEN] =           {"img_logo.png"};
char mobilenet_box_bg_9[][IMAGE_NAME_LEN] = {"mobilenet_box_bg.9.png"};
char num_bg[][IMAGE_NAME_LEN] =             {"num_bg.png"};
char ssd_box_bg_9[][IMAGE_NAME_LEN] =       {"ssd_box_bg.9.png"};

BITMAP dot_joint_bmap;
BITMAP dot_mobilenet_bmap;
BITMAP dot_ssd_bmap;
BITMAP fps_bg_9_bmap;
BITMAP img_logo_bmap;
BITMAP mobilenet_box_bg_9_bmap;
BITMAP num_bg_bmap;
BITMAP ssd_box_bg_9_bmap;

void *image_array[][2] = {{dot_joint, &dot_joint_bmap},
                        {dot_mobilenet, &dot_mobilenet_bmap},
                        {dot_ssd, &dot_ssd_bmap},
                        {fps_bg_9, &fps_bg_9_bmap},
				        {img_logo, &img_logo_bmap},
				        {mobilenet_box_bg_9, &mobilenet_box_bg_9_bmap},
				        {num_bg, &num_bg_bmap},
				        {ssd_box_bg_9, &ssd_box_bg_9_bmap}};
int loadres(void)
{
    char img[128];
    /*load dot_joint bmp*/
    for (int j = 0; j < sizeof(image_array) / sizeof(image_array[0]); j++) {
        snprintf(img, sizeof(img), "%s%s", RES_PATH, image_array[j][0]);
        if (LoadBitmap(HDC_SCREEN, image_array[j][1], img))
            return -1;
    }
    return 0;
}

void unloadres(void)
{
    for (int j = 0; j < sizeof(image_array) / sizeof(image_array[0]); j++) {
        UnloadBitmap(image_array[j][1]);
    }
}
