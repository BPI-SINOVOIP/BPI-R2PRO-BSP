#ifndef _MSG_PROCESS_H_
#define _MSG_PROCESS_H_
#include <pthread.h>

typedef struct msg_element tmsg_element;

struct msg_element {
    tmsg_element* next;
    int msg;
    int ext;
    int sub0;
    int sub1;
    int dt_len;
    int dt1_len;
    char *dt;
    char *dt1;
};

typedef struct msg_buffer tmsg_buffer;

struct msg_buffer {
    tmsg_element* first;
    tmsg_element* last;
    int num;

    pthread_mutex_t mutex;
    pthread_cond_t  not_empty;

    void (*put)(tmsg_buffer* buf, tmsg_element* elm);
    tmsg_element* (*get)(tmsg_buffer* buf, int block);
    tmsg_element* (*get_timeout)(tmsg_buffer* buf, int block);

    tmsg_element* (*clear)(tmsg_buffer* buf);
    void (*sendmsg)(tmsg_buffer* buf, int msg, int ext, char* str, int len, char *str1, int len1);
    void (*sendmsgex)(tmsg_buffer* buf, int msg, int ext, int sub0, int sub1, char* str, int len);
    void (*dispose)(tmsg_buffer* buf);

    int (*getnum)(tmsg_buffer* buf) ;
};

tmsg_buffer* msg_buffer_init(void);
tmsg_element* dup_msg_element(tmsg_element* elm);
void free_tmsg_element(tmsg_element *msg_element);

#endif /* MESSAGE_MSG_CENTER_H_ */