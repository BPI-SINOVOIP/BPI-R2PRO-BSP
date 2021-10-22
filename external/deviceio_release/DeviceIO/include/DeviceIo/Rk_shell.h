#ifndef __RK_SHELL_H__
#define __RK_SHELL_H__

#ifdef __cplusplus
extern "C" {
#endif

int RK_shell_exec(const char* cmd, char* result, size_t size);
int RK_shell_system(const char *cmd);

#ifdef __cplusplus
}
#endif

#endif
