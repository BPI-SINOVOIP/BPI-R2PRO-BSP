/****************************************************************************
*   RKNN Runtime Test
****************************************************************************/

/*-------------------------------------------
                Includes
-------------------------------------------*/
#include "rk_face_recognize_wrapper.h"
#include <time.h>
#define _BASETSD_H

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

/*-------------------------------------------
        Macros and Variables
-------------------------------------------*/
#define CHECK_STATUS(func) do { \
    status = func; \
    if (status < 0)  { \
        goto final; \
    }   \
} while(0)

/*-------------------------------------------
                  Functions
-------------------------------------------*/

static char load_model(const char *filename,rknn_context* ctx)
{
    int ret;
    FILE *fp = fopen(filename, "rb");
    if(fp == NULL) {
        printf("fopen %s fail!\n", filename);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    int model_len = ftell(fp);
    unsigned char *model = (unsigned char*)malloc(model_len);
    fseek(fp, 0, SEEK_SET);
    if(model_len != fread(model, 1, model_len, fp)) {
        printf("fread %s fail!\n", filename);
        free(model);
        return NULL;
    }

    ret = rknn_init(ctx, model, model_len, 0);
    if(ret < 0) {
        printf("rknn_init fail! ret=%d\n", ret);
        return -1;
    }

    if(fp) {
        fclose(fp);
    }
    if(model) {
        free(model);
    }
    return 0;
}

/*-------------------------------------------
                  Main Functions
-------------------------------------------*/
int main
    (
    int argc,
    char **argv
    )
{
    int status = 0;
    char detect_model_name[100] = "./model/detect.rknn";;
    char align_model_name[100]="./model/align.rknn";
    char recognize_model_name[100]="./model/recognize.rknn";
    char *image_name = NULL;
    const char persons_path[100] = "persondata.txt";
    rknn_context ctx_detect;
    rknn_context ctx_align;
    rknn_context ctx_recognize;
	rknn_context ctx_ga;
    char *in_data = NULL;
    int in_size;
	struct face_group group;
    int width_in,height_in;
    struct RkFace face;

    RkImage img_src,img_align;
    int img_size;

    unsigned char *detemodel;

    width_in = 300;
    height_in = 300;
	
    if(argc != 2)
    {
        printf("Usage:%s image_file_name\n", argv[0]);
        return -1;
    }
    image_name = (char *)argv[1];

    //detect
    load_model(detect_model_name, &ctx_detect);

    //align
    load_model(align_model_name, &ctx_align);

    //recognize
    load_model(recognize_model_name, &ctx_recognize);

    //original input data
    cv::Mat orig_img = cv::imread(image_name, 1);
    in_data = (char*)orig_img.data;
    img_src.width = width_in;
    img_src.height = height_in;
    img_src.type = PIXEL_RGB;
    img_size =  img_src.width *  img_src.height * 3;
    img_src.pixels = in_data;
    if(img_src.pixels == NULL){
        printf("malloc error\n");
    }

	img_align.width = 112;
	img_align.height = 112;
	img_align.pixels = (char *) malloc(img_align.width * img_align.height * 3 * sizeof(char));
    if(img_align.pixels == NULL){
        printf("img_align.pixels malloc error\n");
    }

    struct timespec t1,t2;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&t1);
    //face detect
    rkFaceDetect(ctx_detect, &img_src);
    rkFacePostProcess(ctx_detect, &group);
    if(group.count > 0){
        face = group.faces[0];

        rkFaceAlign(ctx_align,&img_src,&img_align,&face);    
        //person test
        rkLoadPersons(persons_path);
        //face feature
        rkFaceFeature(ctx_recognize, &img_align, &face);

        rkFaceRecognizetest(ctx_recognize);

        clock_gettime(CLOCK_THREAD_CPUTIME_ID,&t2);
        printf("face recognize time:%ld us\n", ((t2.tv_sec - t1.tv_sec) * 1000000000 - t1.tv_nsec + t2.tv_nsec)/1000);
        int time_valu = ((t2.tv_sec - t1.tv_sec) * 1000000000 - t1.tv_nsec + t2.tv_nsec)/1000;
    }

/*   
    //person register
    in_data = vnn_ReadJpegImage(image_name, 112, 112, 3);
    strcpy( face.name,"usr1");
    rkFaceRegister(ctx_align, ctx_recognize, in_data, &face);

    in_data = vnn_ReadJpegImage("Princess_Masako_0001.jpg", 112, 112, 3);
    strcpy( face.name,"usr2");
    rkFaceRegister(ctx_align, ctx_recognize, in_data, &face);

    rkSavePersons( persons_path);
*/

    // Release
    if(ctx_detect >= 0) {
        rknn_destroy(ctx_detect);
    }
    if(ctx_align >= 0) {
        rknn_destroy(ctx_align);
    }
    if(ctx_recognize >= 0) {
        rknn_destroy(ctx_recognize);
    }

    if (img_align.pixels != NULL)
        free(img_align.pixels);

    return status;
}
