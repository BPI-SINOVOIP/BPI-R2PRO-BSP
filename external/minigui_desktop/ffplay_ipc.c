/*
 * Copyright (C) 2018 hertz wangh@rock-chips.com
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "common.h"
#include "ffplay_ipc.h"

#define MESSAGE_MAX_LEN 1024
#define MESSAGE_SEPATATOR "="
// state
#define STATE       "state"

#define READY       "ready"
#define PLAYING     "playing"
#define PAUSED      "paused"
#define QUIT        "quit"
#define ERROR       "error"
#define END        "end"

#define MESSAGE_STATE_READY STATE MESSAGE_SEPATATOR READY

// other message
#define DURATION    "duration"
#define CUR_TIME    "current time"

// cmd
#define QUERY           "query"
#define SEEK_RELATIVE   "seek relative"
#define SEEK_TO         "seek to"
#define SET_STATE       "set state"

#define FFPLAY_IPC_SOCK_PATH "/tmp/ffplay.socket"

typedef struct Message
{
    char *msg;
    int len;
    struct Message *next;
} Message;

typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    Message *first;
} IPCMessageQueue;

typedef struct
{
    pthread_t ipc_tid;
    int quit;
    int socket_fd;
    IPCMessageQueue send_queue;
    pthread_t init_tid;
    HWND hWnd;
    int ffplay_is_quit;
    int start_time;
} IPCState;

typedef struct
{
    IPCState *ipcs;
    int socket_fd;
    pthread_t send_tid;
    pthread_t recv_tid;
} IPCClientState;

int is_media_loop = 0;
static IPCState ipc_state;

static volatile int ready = 0;
static volatile int ffplay_exist = 0;

static Message *alloc_ipc_message(char *key, char *value)
{
    Message *ret;
    int len = strlen(key) + strlen(value) + sizeof(MESSAGE_SEPATATOR);
    char *str;

    if (len > MESSAGE_MAX_LEN)
    {
        fprintf(stderr, "Message is too long\n");
        return NULL;
    }
    str = malloc(len);
    if (!str)
    {
        fprintf(stderr, "No Memory\n");
        return NULL;
    }
    ret = malloc(sizeof(*ret));
    if (!ret)
    {
        free(str);
        fprintf(stderr, "No Memory\n");
        return NULL;
    }
    snprintf(str, len, "%s%s%s", key, MESSAGE_SEPATATOR, value);
    ret->msg = str;
    ret->len = len;
    ret->next = NULL;

    return ret;
}

static void free_ipc_message(Message *m)
{
    if (!m)
        return;
    if (m->msg)
        free(m->msg);
    memset(m, 0, sizeof(*m));
    free(m);
}

static int init_ipc_queue(IPCMessageQueue *queue)
{
    int ret;

    ret = pthread_mutex_init(&queue->mutex, NULL);
    if (ret)
    {
        fprintf(stderr, "pthread_mutex_init(): %s\n", strerror(errno));
        return -errno;
    }

    ret = pthread_cond_init(&queue->cond, NULL);
    if (ret)
    {
        pthread_mutex_destroy(&queue->mutex);
        fprintf(stderr, "pthread_cond_init(): %s\n", strerror(errno));
        return -errno;
    }

    return 0;
}

static void clean_ipc_queue(IPCMessageQueue *queue)
{
    Message *m;

    pthread_mutex_lock(&queue->mutex);
    m = queue->first;
    while (m)
    {
        free(&m->msg);
        m = m->next;
        memset(m, 0, sizeof(*m));
        free(m);
    }
    pthread_mutex_unlock(&queue->mutex);
}

static void deinit_ipc_queue(IPCMessageQueue *queue)
{
    clean_ipc_queue(queue);
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);
}

static void push_ipc_message(IPCState *ipcs, Message *m)
{
    Message *end;
    IPCMessageQueue *queue = &ipcs->send_queue;
    pthread_mutex_lock(&queue->mutex);
    end = queue->first;
    while (end)
    {
        if (!end->next)
        {
            end->next = m;
            m->next = NULL;
            break;
        }
        end = end->next;
    }
    if (!end)
        queue->first = m;
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

static Message *pop_ipc_message(IPCState *ipcs)
{
    Message *first;
    IPCMessageQueue *queue = &ipcs->send_queue;
    pthread_mutex_lock(&queue->mutex);
    first = queue->first;
    if (!first)
    {
        struct timeval now;
        struct timespec timeout;
        int64_t increase;
        gettimeofday(&now, NULL);
        timeout.tv_sec = now.tv_sec;
        timeout.tv_nsec = (now.tv_usec + 100 * 1000LL) * 1000LL;
        increase = timeout.tv_nsec / 1000000000LL;
        if (increase > 0)
        {
            timeout.tv_sec += increase;
            timeout.tv_nsec = timeout.tv_nsec - increase * 1000000000LL;
        }
        pthread_cond_timedwait(&queue->cond, &queue->mutex, &timeout);
    }
    first = queue->first;
    if (first)
    {
        queue->first = first->next;
        first->next = NULL;
        pthread_mutex_unlock(&queue->mutex);
        return first;
    }
    pthread_mutex_unlock(&queue->mutex);

    return NULL;
}

static void *ipc_main_thread(void *arg);
static int init_ipc(IPCState *ipcs)
{
    int ret;
    memset(ipcs, 0, sizeof(*ipcs));
    ipcs->socket_fd = -1;
    ret = init_ipc_queue(&ipcs->send_queue);
    if (ret)
        return ret;
    ret = pthread_create(&ipcs->ipc_tid, NULL, ipc_main_thread, ipcs);
    if (ret)
    {
        deinit_ipc_queue(&ipcs->send_queue);
        fprintf(stderr, "pthread_create(): %s\n", strerror(errno));
        ret = -1;
    }
    return ret;
}

static void deinit_ipc(IPCState *ipcs)
{
    ipcs->quit = 1;
    if (ipcs->ipc_tid)
        pthread_join(ipcs->ipc_tid, NULL);
    deinit_ipc_queue(&ipcs->send_queue);
}

static void handle_remote_ipc_message(char *key, char *value, int *alive)
{
    /* test code begin */
    {
        if (!strcmp(key, STATE))
        {
            printf("recv remote state message: %s%s%s\n", key, MESSAGE_SEPATATOR, value);
            if (!strcmp(value, READY))
            {
                __atomic_store_n(&ready, 1, __ATOMIC_SEQ_CST);
            }
            else if (!strcmp(value, QUIT) || !strcmp(value, ERROR))
            {
                clean_ipc_queue(&ipc_state.send_queue);
                *alive = 0;
                __atomic_store_n(&ready, 0, __ATOMIC_SEQ_CST);
                ipc_state.ffplay_is_quit = ipc_state.quit = 1;
            }
            else if (!strcmp(value, END))
            {
                if (ipc_state.hWnd)
                    PostMessage(ipc_state.hWnd, MSG_MEDIA_UPDATE, MEDIA_CMD_END, 0);
            }
        }
        else if (!strcmp(key, DURATION))
        {
            int titletime = atoi(value);
            if (ipc_state.hWnd)
                PostMessage(ipc_state.hWnd, MSG_MEDIA_UPDATE, MEDIA_CMD_TOTAL_TIME, titletime);
        }
        else if (!strcmp(key, CUR_TIME))
        {
            int curtime = atoi(value);
            if (ipc_state.hWnd)
                PostMessage(ipc_state.hWnd, MSG_MEDIA_UPDATE, MEDIA_CMD_CUR_TIME, curtime);
        }
        // TODO
    }
    /* test code end */
}

static void *ipc_recv_thread(void *arg)
{
    char buf[MESSAGE_MAX_LEN];
    IPCClientState *cs = (IPCClientState *)arg;
    IPCState *ipcs = cs->ipcs;
    int fd = cs->socket_fd;
    int client_alive = 1;

    while (!ipcs->quit && client_alive)
    {
        char *str;
        int recv_len;
        int str_offset;

        recv_len = recv(fd, buf, sizeof(buf), 0);
        if (recv_len <= 0)
        {
            if (errno != EAGAIN && client_alive)
            {
                fprintf(stderr, "Fail to recv message: %s\n", strerror(errno));
                handle_remote_ipc_message((char *)STATE, (char *)ERROR, &client_alive);
                break;
            }
            usleep(10 * 1000);
            continue;
        }

        if (buf[recv_len - 1] != 0)
        {
            int i = 0;
            buf[recv_len - 1] = 0;
            fprintf(stderr, "Warning : Remote Message may be truncated, ");
            while (i < recv_len)
                fprintf(stderr, "%c", buf[i++]);
            fprintf(stderr, "\n");
        }
        str_offset = 0;
        while (str_offset < recv_len)
        {
            char *str;
            char *start = buf + str_offset;
            int str_len = strlen(start) + 1;
            str_offset += str_len;
            str = strstr(start, MESSAGE_SEPATATOR);
            if (!str)
            {
                fprintf(stderr, "Remote Message is broken <%s>?\n", start);
                continue;
            }
            *str = 0;
            handle_remote_ipc_message(start, str + 1, &client_alive);
        }
    }

    printf("exit %s, fd = %d\n", __FUNCTION__, fd);
    return NULL;
}

static void *ipc_send_thread(void *arg)
{
    int ret;
    IPCClientState *cs = (IPCClientState *)arg;
    IPCState *ipcs = cs->ipcs;
    int fd = cs->socket_fd;

    // pthread_detach(pthread_self());
    while (!ipcs->quit)
    {
        char *send_str;
        int send_len;
        Message *m = pop_ipc_message(ipcs);
        if (!m)
        {
            continue;
        }
        send_str = m->msg;
        send_len = m->len;
        while (send_len > 0)
        {
            ret = send(fd, send_str, send_len, 0);
            if (ret <= 0)
            {
                fprintf(stderr, "send errno: %s\n", strerror(errno));
                if (ret < 0 && errno != EAGAIN)
                {
                    fprintf(stderr, "Fail to send msg<%s>: %s\n",
                            m->msg, strerror(errno));
                    break;
                }
            }
            // printf("send ipc msg<%d bytes>: %s\n", ret, m->msg);
            send_len -= ret;
            send_str += ret;
        }
        free_ipc_message(m);
    }

    printf("exit %s, fd = %d\n", __FUNCTION__, fd);
    pthread_join(cs->recv_tid, NULL);
    close(fd);
    free(cs);
    __atomic_store_n(&ffplay_exist, 0, __ATOMIC_SEQ_CST);
    pthread_exit(NULL);
}

static void *ipc_main_thread(void *arg)
{
    int ret = 0;
    IPCState *ipcs = (IPCState *)arg;
    int loop = 1;
    int fd = -1;
    struct sockaddr_un address;
    pthread_t send_tid = 0;
    pthread_t recv_tid = 0;

    fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
    if (fd < 0)
    {
        fprintf(stderr, "Fail to create socket: %s\n", strerror(errno));
        ret = -1;
        goto out;
    }

    unlink(FFPLAY_IPC_SOCK_PATH);
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, FFPLAY_IPC_SOCK_PATH);

    ret = bind(fd, (struct sockaddr *)&address, sizeof(address));
    if (ret < 0)
    {
        fprintf(stderr, "Fail to bind socket: %s\n", strerror(errno));
        goto out;
    }

    // only one
    ret = listen(fd, 1);

    if (ret < 0)
    {
        fprintf(stderr, "Fail to listen socket: %s\n", strerror(errno));
        goto out;
    }

    ipcs->socket_fd = fd;

    while (!ipcs->quit)
    {
        int client_fd;
        struct sockaddr_un client_addr;
        socklen_t len = sizeof(client_addr);
        IPCClientState *client;
        struct pollfd fds;
        int p_ret;

        fds.fd = fd;
        fds.events = POLLIN | POLLERR;
        fds.revents = 0;

        p_ret = poll(&fds, 1, 100);
        if (p_ret < 0 || !(fds.revents & POLLIN))
            continue;

        client = malloc(sizeof(IPCClientState));
        if (!client)
        {
            fprintf(stderr, "No memory\n");
            break;
        }
        memset(client, 0, sizeof(*client));
        client_fd = accept(ipcs->socket_fd, (struct sockaddr *)&client_addr, &len);
        if (client_fd < 0)
        {
            fprintf(stderr, "Fail to accept socket: %s\n", strerror(errno));
            free(client);
            continue;
        }
        printf("client fd: %d\n", client_fd);
        client->ipcs = ipcs;
        client->socket_fd = client_fd;
        ret = pthread_create(&recv_tid, NULL, ipc_recv_thread, client);
        if (ret)
        {
            close(client_fd);
            free(client);
            fprintf(stderr, "Fail to create ipc_recv_thread: %s\n", strerror(errno));
            goto out;
        }
        client->recv_tid = recv_tid;
        ret = pthread_create(&send_tid, NULL, ipc_send_thread, client);
        if (ret)
        {
            close(client_fd);
            free(client);
            fprintf(stderr, "Fail to create ipc_send_thread: %s\n", strerror(errno));
            goto out;
        }
        client->send_tid = send_tid;
        __atomic_store_n(&ffplay_exist, 1, __ATOMIC_SEQ_CST);
        pthread_join(send_tid, NULL);
        send_tid = 0;
        recv_tid = 0;
    }

out:
    if (fd > 0)
        close(fd);

    if (send_tid)
        pthread_join(send_tid, NULL);

    if (recv_tid)
        pthread_join(recv_tid, NULL);

    printf("exit %s, fd = %d\n", __FUNCTION__, fd);
    return (void *)ret;
}

#define ALSA_DEVICE_CFG_PATH_ENV    "ALSA_DEVICE_CFG"
#define ALSA_DEVICE_CFG_PATH_DEFAULT    "/tmp/alsa_device.cfg"

static void *run_ffplay_thread(void *arg)
{
    int ret;
    char cmd[1024];
    char value[32];
    Message *m;
    char *file_path = (char *)arg;
    int audio_cfg_fd = -1;
    char buf[256] = "\0";
    const char *audio_cfg_file;
    int wait_times = 6 * 100;
    int retry_wait = wait_times;

    audio_cfg_file = getenv(ALSA_DEVICE_CFG_PATH_ENV);
    if (!audio_cfg_file)
        audio_cfg_file = ALSA_DEVICE_CFG_PATH_DEFAULT;

    audio_cfg_fd = open(audio_cfg_file, O_RDONLY);
    if (audio_cfg_fd > 0)
    {
        read(audio_cfg_fd, buf, sizeof(buf));
        close(audio_cfg_fd);
        if (buf[strlen(buf) - 1] == '\n')
            buf[strlen(buf) - 1] = '\0';
    }
    else
    {
        sprintf(buf, "default");
    }
    setenv("AUDIODEV", buf, 1);
    printf("minigui_game export AUDIODEV=%s\n", buf);

    ipc_state.ffplay_is_quit = 0;
    //pthread_detach(pthread_self());
    // -nodisp
    // -autoexit  -vcodec h264
    if (ipc_state.start_time > 0) {
        snprintf(cmd, sizeof(cmd), "/usr/bin/rkffplay -hide_banner -nostats -forceoverlay -fs -ss %d \"%s\" &", ipc_state.start_time, file_path);
    } else {
        snprintf(cmd, sizeof(cmd), "/usr/bin/rkffplay -hide_banner -nostats -forceoverlay -fs \"%s\" &", file_path);
    }
    printf("%s\n", cmd);
    ret = system(cmd);
    if (ret)
    {
        fprintf(stderr, "Fail to system %s, exit game\n", cmd);
        exit(ret);
    }
    // wait ready
    while (!__atomic_load_n(&ready, __ATOMIC_SEQ_CST))
    {
        wait_times--;
        if (ipc_state.ffplay_is_quit == 1 || wait_times < 0)
        {
            printf("ffplay <%s> error %s\n", file_path, wait_times < 0 ? "timeout" : "");
            if (wait_times < 0 && retry_wait > 0 && __atomic_load_n(&ffplay_exist, __ATOMIC_SEQ_CST))
            {
                retry_wait /= 4;
                wait_times = retry_wait;
                continue;
            }
            free(file_path);
            goto out;
        }
        usleep(10 * 1000);
    }
    printf("ffplay<%s> ready\n", file_path);
    free(file_path);

    // operate after ffplay is ready
    m = alloc_ipc_message((char *)QUERY, (char *)DURATION);
    if (m)
    {
        push_ipc_message(&ipc_state, m);
    }

    m = alloc_ipc_message((char *)QUERY, (char *)CUR_TIME);
    if (m)
    {
        push_ipc_message(&ipc_state, m);
    }

out:
    ipc_state.init_tid = 0;
    pthread_detach(pthread_self());
    pthread_exit(NULL);
}

void media_pause(void)
{
    Message *m;

    if (ipc_state.ffplay_is_quit == 0)
    {
        m = alloc_ipc_message((char *)SET_STATE, (char *)PAUSED);
        if (m)
        {
            push_ipc_message(&ipc_state, m);
        }
    }
}

void media_restore(void)
{
    Message *m;

    if (ipc_state.ffplay_is_quit == 0)
    {
        m = alloc_ipc_message((char *)SET_STATE, (char *)PLAYING);
        if (m)
        {
            push_ipc_message(&ipc_state, m);
        }
    }
}

void media_play(const char *file_path, HWND hWnd, int start_time)
{
    int ret;
    char *path;

    ret = init_ipc(&ipc_state);
    if (ret)
        return;
    ipc_state.hWnd = hWnd;
    ipc_state.start_time = start_time;
    path = malloc(strlen(file_path) + 1);
    if (path)
    {
        strcpy(path, file_path);
        if (pthread_create(&ipc_state.init_tid, NULL, run_ffplay_thread, path))
        {
            printf("Failed to create run_ffplay_thread\n");
            free(path);
        }
    }
}

void media_seek_to(char *value)
{
    Message *m;

    if (ipc_state.ffplay_is_quit == 0)
    {
        m = alloc_ipc_message((char *)SEEK_TO, value);
        //printf("msg:%s len:%d\n",m->msg,m->len);
        if (m)
        {
            push_ipc_message(&ipc_state, m);
        }
    }
}

void media_wait(void)
{
    if (ipc_state.init_tid)
    {
        pthread_join(ipc_state.init_tid, NULL);
        ipc_state.init_tid = 0;
    }
}

void media_exit(void)
{
    Message *m;

    if (ipc_state.init_tid)
    {
        pthread_join(ipc_state.init_tid, NULL);
        ipc_state.init_tid = 0;
    }
    if (ipc_state.ffplay_is_quit == 0)
    {
        int wait_times = 6 * 100;
        int retry_wait = wait_times;
        m = alloc_ipc_message((char *)SET_STATE, (char *)QUIT);
        if (m)
        {
            push_ipc_message(&ipc_state, m);
        }
        else
        {
            printf("!!! must not occur %s_%d", __FUNCTION__, __LINE__);
        }

        usleep(800 * 1000);
        // wait quit
        while (__atomic_load_n(&ready, __ATOMIC_SEQ_CST))
        {
            if (wait_times-- < 0 && retry_wait > 0 && __atomic_load_n(&ffplay_exist, __ATOMIC_SEQ_CST))
            {
                retry_wait /= 4;
                wait_times = retry_wait;
                continue;
            }
            usleep(10 * 1000);
        }
        if (wait_times < 0)
            printf("wait ffplay exit timeout, ffplay die?\n");
    }
    printf("ffplay_is_quit is %d\n", ipc_state.ffplay_is_quit);
    deinit_ipc(&ipc_state);
    ipc_state.hWnd = 0;
    printf("exit %s\n", __FUNCTION__);
}
