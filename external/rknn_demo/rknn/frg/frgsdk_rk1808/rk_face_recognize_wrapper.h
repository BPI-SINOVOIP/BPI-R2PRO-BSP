#ifndef __RK_FACE_RECOGNIZE_WRAPPER_H_

#define __RK_FACE_RECOGNIZE_WRAPPER_H_

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
ctx : detect model context
img_orig : original image data,color format 300*300 RGBRGBRGB...
*/
__attribute__((visibility("default"))) int rkFaceDetect(rknn_context ctx, RkImage* img_orig);

/*
ctx : detect model context
group : the result of face detect
*/
__attribute__((visibility("default"))) int rkFacePostProcess(rknn_context ctx,struct face_group* group);


__attribute__((visibility("default"))) int rkFaceAlign(rknn_context ctx_align,RkImage* img_orig,RkImage* img_align,struct RkFace *face);


/*
ctx : recognize model context
img_orig : original image data,300*300 RGBRGBRGB...
face : face landmark ,locate ,score...
*/
__attribute__((visibility("default"))) int rkFaceFeature(rknn_context ctx_regc, RkImage* img_align,struct RkFace *face);

/*
feature1
feature2
len: feature dimension
return:Face feature similarity value
 */
__attribute__((visibility("default"))) float rkFeatureIdentify(float* feature1,float* feature2,int len);

/*
distance: Feature distance buffer,the buffer size must be equaled to "length"
enroll_feature_num : Number of Face Registration Features
length:Total Face Feature Number in Face Database
return:Recognize result,the index of person
 */
__attribute__((visibility("default"))) int rkRecognizeStrategy(float *distance, int length,int register_feature_num);


int rkFaceGa(rknn_context ctx_ga, RkImage* img_align);




__attribute__((visibility("default"))) int rkFaceRecognizetest(rknn_context ctx_regc);
//ctx : recognize model context
//img_orig : original image data
//face : face landmark ,locate ,score
//person : name ,feature
__attribute__((visibility("default"))) int rkFaceRegister(rknn_context ctx_align,rknn_context ctx_regc, RkImage* img_orig,struct RkFace *face);


__attribute__((visibility("default"))) int rkSavePersons(const char *persons_path);

__attribute__((visibility("default"))) int rkLoadPersons(const char *persons_path);

#ifdef __cplusplus
}
#endif


#endif