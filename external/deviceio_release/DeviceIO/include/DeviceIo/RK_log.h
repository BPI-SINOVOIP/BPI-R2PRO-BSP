#ifndef __RK_LOG_H__
#define __RK_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

const int RK_LOG_TYPE_CONSOLE = 0x01;
const int RK_LOG_TYPE_FILE = 0x02;

int RK_LOG_set_type(const int type);
int RK_LOG_set_save_parameter(const char saveLevel, const char *dir, const int fileSize, const int fileNu);
int RK_LOGV(const char *format, ...);
int RK_LOGD(const char *format, ...);
int RK_LOGI(const char *format, ...);
int RK_LOGE(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif
