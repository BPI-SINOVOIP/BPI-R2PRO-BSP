#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "msg_process.h"

#include "log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "msg_process.c"

static unsigned long timeout_ns = 0;

static void put_msg_to_buffer(tmsg_buffer* buf, tmsg_element* elm)
{
    struct timeval timenow;
    struct timespec timeout;
    int block = 1000;
    if (NULL == buf || NULL == elm) {
        return;
    }

    if (NULL != elm->next) {
        elm->next = NULL;
    }

    pthread_mutex_lock(&buf->mutex);
    if (buf->first) {
        tmsg_element* tmp = buf->first;
        while (tmp->next != NULL) {
            tmp = tmp->next;
        }
        tmp->next = elm;
    } else {
        buf->first = elm;
    }
    buf->num++;
    pthread_cond_signal(&buf->not_empty);
    pthread_mutex_unlock(&buf->mutex);
}

static tmsg_element* get_msg_from_buffer(tmsg_buffer* buf, int block)
{
    tmsg_element *elm = NULL;

    if (NULL == buf) {
        return NULL;
    }

    pthread_mutex_lock(&buf->mutex);

    while (0 == buf->num) {
        pthread_cond_wait(&buf->not_empty, &buf->mutex);
    }

    elm = buf->first;
    if (1 == buf->num) {
        buf->first = buf->last = NULL;
        buf->num = 0;
    } else {
        buf->first = buf->first->next;
        buf->num --;
    }

    pthread_mutex_unlock(&buf->mutex);

    return elm;
}

static tmsg_element* get_msg_from_buffer_timeout(tmsg_buffer* buf, int block/*ms*/)
{
    tmsg_element *elm = NULL;
    struct timeval timenow;
    struct timespec timeout;

    if (NULL == buf) {
        return NULL;
    }

    pthread_mutex_lock(&buf->mutex);
    if (0 == buf->num) {
        gettimeofday(&timenow, NULL);
        timeout.tv_sec = timenow.tv_sec + block / 1000;
        timeout.tv_nsec = (timenow.tv_usec + (block % 1000) * 1000) * 1000;
        pthread_cond_timedwait(&buf->not_empty, &buf->mutex, &timeout);
    }

    if (buf->num > 0) {
        elm = buf->first;
        if (1 == buf->num) {
            buf->first = buf->last = NULL;
            buf->num = 0;
        } else {
            buf->first = buf->first->next;
            buf->num --;
        }
    }

    pthread_mutex_unlock(&buf->mutex);

    return elm;
}

static tmsg_element* clear_msg_buffer(tmsg_buffer* buf)
{
    tmsg_element* elm = NULL;
    tmsg_element* elm_tmp = NULL;

    if (NULL == buf) {
        return NULL;
    }

    pthread_mutex_lock(&buf->mutex);
    if (buf->num > 0) {
        elm = buf->first;
        while (elm != NULL) {
            if (elm == buf->last) {
                buf->first = buf->last;
                if (buf->num != 1) {
                    buf->num = 1;
                }
                break;
            }

            elm_tmp = elm->next;
            free_tmsg_element(elm);
            buf->num --;
            elm = elm_tmp;
            buf->first = elm;
        }
    }

    pthread_mutex_unlock(&buf->mutex);

    return elm;
}

static void send_msg_to_buffer(tmsg_buffer* buf, int msg, int ext, char* str, int len, char *str1, int len1)
{
    tmsg_element *elm = NULL;

    elm = (tmsg_element *)malloc(sizeof(tmsg_element));
    if (NULL == elm) {
        LOG_ERROR("new msg element failed!!");
        return;
    }

    memset(elm, 0, sizeof(tmsg_element));
    elm->msg = msg;
    elm->ext = ext;
    elm->dt = NULL;
    elm->dt_len = len;
    elm->dt1 = NULL;
    elm->dt1_len = len1;
    if (str) {
        elm->dt = (char *)calloc(1, len + 1);
        memmove(elm->dt, str, len);
    }
    if (str1) {
        elm->dt1 = (char *)calloc(1, len1 + 1);
        memmove(elm->dt1, str1, len1);
    }

    elm->next = NULL;

    put_msg_to_buffer(buf, elm);
}

static void send_msg_to_buffer_ex(tmsg_buffer* buf, int msg, int ext, int sub0, int sub1, char* str, int len)
{
    tmsg_element *elm = NULL;

    elm = (tmsg_element *)malloc(sizeof(tmsg_element));
    if (NULL == elm) {
        LOG_ERROR("new msg element failed!!");
        return;
    }

    memset(elm, 0, sizeof(tmsg_element));
    elm->msg = msg;
    elm->ext = ext;
    elm->sub0 = sub0;
    elm->sub1 = sub1;
    elm->dt = NULL;
    elm->dt_len = len;
    if (str) {
        elm->dt = (char *)malloc(len);
        memmove(elm->dt, str, len);
    }
    elm->next = NULL;

    put_msg_to_buffer(buf, elm);
}

static void dispose_msg_buffer(tmsg_buffer* buf)
{
    tmsg_element* elm = NULL;

    if (NULL == buf) {
        return;
    }

    if (buf->first != buf->last
        && buf->num > 0) {
        elm = clear_msg_buffer(buf);
    } else {
        elm = buf->last;
    }

    if (NULL != elm) {
        free_tmsg_element(elm);
        buf->first = buf->last = NULL;
        buf->num = 0;
    }

    pthread_mutex_destroy(&buf->mutex);
    pthread_cond_destroy(&buf->not_empty);
    free(buf);

    buf = NULL;
}


static int get_msg_num(tmsg_buffer* buf)
{
    if (NULL == buf) {
        return 0;
    }

    return buf->num;
}

tmsg_buffer* msg_buffer_init(void)
{
    tmsg_buffer* msg_buffer = NULL;

    msg_buffer = (tmsg_buffer *)malloc(sizeof(tmsg_buffer));
    if (NULL == msg_buffer) {
        LOG_ERROR("init msg buffer failed!!");
        return NULL;
    }

    memset(msg_buffer, 0, sizeof(tmsg_buffer));
    msg_buffer->first = NULL;
    msg_buffer->last = NULL;
    msg_buffer->num = 0;

    pthread_mutex_init(&(msg_buffer->mutex), NULL);
    pthread_cond_init(&(msg_buffer->not_empty), NULL);

    msg_buffer->put = put_msg_to_buffer;
    msg_buffer->get = get_msg_from_buffer;
    msg_buffer->get_timeout = get_msg_from_buffer_timeout;
    msg_buffer->clear = clear_msg_buffer;
    msg_buffer->sendmsg = send_msg_to_buffer;
    msg_buffer->sendmsgex = send_msg_to_buffer_ex;
    msg_buffer->dispose = dispose_msg_buffer;
    msg_buffer->getnum = get_msg_num;

    return msg_buffer;
}

tmsg_element* dup_msg_element(tmsg_element* elm)
{
    tmsg_element* msg_element = NULL;
    if (NULL == elm) {
        LOG_ERROR("msg element is NULL!!");
        return NULL;
    }

    msg_element = (tmsg_element *)malloc(sizeof(tmsg_element));
    if (NULL == msg_element) {
        LOG_ERROR("create msg element is failed!!");
        return NULL;
    }

    memcpy(msg_element, elm, sizeof(tmsg_element));

    return msg_element;
}

void free_tmsg_element(tmsg_element *msg_element)
{
    if (msg_element != NULL) {
        if (msg_element->dt != NULL) {
            free(msg_element->dt);
            msg_element->dt = NULL;
        }
        if (msg_element->dt1 != NULL) {
            free(msg_element->dt1);
            msg_element->dt1 = NULL;
        }
        free(msg_element);
        msg_element = NULL;
    }
}