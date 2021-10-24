#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/prctl.h>
#include <minigui/common.h>

#include "rknn_msg.h"

int g_rknn_msgid = -1;
#define RKNN_IPCMSGKEY 1027

typedef struct _IPC_MSG {
	long msg_type;
	char rknn_msg[sizeof(RKNN_MSG)];
} IPC_MSG;


int ipcmsg_getnum(int msgid)
{
	int ret;
	struct msqid_ds msg_info;

	if (msgid < 0) {
		printf(" msgid = %d, exit\n", msgid);
		return -1;
	}
	ret = msgctl(msgid, IPC_STAT, &msg_info);
	if (ret < 0) {
		printf("failed to get info | errno=%d [%s]/n", errno, strerror(errno));
		return -1;
	}
	return msg_info.msg_qnum;
}

int ipcmsg_setnum(int msgid, int number)
{
	int ret;
	struct msqid_ds msg_info;

	if (msgid < 0)
		return -1;

	ret = msgctl(msgid, IPC_STAT, &msg_info);
	if (ret < 0) {
		printf("failed to get info | errno=%d [%s]/n", errno, strerror(errno));
		return -1;
	}
	msg_info.msg_qnum = msg_info.msg_qnum * number;
	msg_info.msg_qbytes = msg_info.msg_qbytes * number;
	ret = msgctl(msgid, IPC_SET, &msg_info);
	if (ret < 0) {
		printf("failed to get info | errno=%d [%s]/n", errno, strerror(errno));
		return -1;
	}
	ret = msgctl(msgid, IPC_STAT, &msg_info);
	if (ret < 0) {
		printf("failed to get info | errno=%d [%s]/n", errno, strerror(errno));
		return -1;
	}

	return msg_info.msg_qnum;

}

int ipcmsg_init(int msg_key, int *m_msgid)
{
	int msgid;
	int ret;
	int retry_num = 0;

RETRY:
	msgid = msgget(msg_key, IPC_EXCL);  /*check msg*/
	if (msgid < 0) {
		msgid = msgget(msg_key, IPC_CREAT | 0666); /*create msg*/
		if (msgid < 0) {
			printf("failed to create msq | errno=%d [%s]/n", errno, strerror(errno));
			return -1;
		}
	}

	ret = ipcmsg_getnum(msgid);
	if (ret > 0) {
		ret = msgctl(msgid, IPC_RMID, NULL);
		if (ret < 0)
			assert(0);
		retry_num++;
		if (retry_num > 3) /*retry, but the msg can't delete*/
			assert(0);
		goto RETRY;
	}
	*m_msgid = msgid;
	return 0;
}

void ipcmsg_deinit(int msg_key, int *m_msgid)
{
	int msgid;

	/* if ipcmsg init fail, the m_msgid will set to -1, so don't deinit again */
	if (*m_msgid < 0)
		return;

	msgid = msgget(msg_key, IPC_EXCL);  /*check ipc msg*/
	if (msgid < 0) {
		printf("failed to get id | errno=%d [%s]/n", errno, strerror(errno));
		return;
	}

	msgctl(msgid, IPC_RMID, 0); /*remove ipc msg*/

	*m_msgid = -1;
	return;
}

int ipcmsg_rec(RKNN_MSG *rknn_msg)
{
	IPC_MSG ipc_msg_recv;

	if (rknn_msg == NULL)
		return -1;
	if (g_rknn_msgid < 0)
		return -1;

	int ret = msgrcv(g_rknn_msgid, &ipc_msg_recv, sizeof(RKNN_MSG), 0, 0);
	if (ret < 0) {
		printf("msgsnd() read msg failed,errno=%d[%s]\n",
		         errno, strerror(errno));
		return -1;
	}
	memcpy((char *)rknn_msg, ipc_msg_recv.rknn_msg, sizeof(RKNN_MSG));

	return 0;
}

int ipcmsg_send(RKNN_MSG *rknn_msg)
{
	IPC_MSG ipc_msg_send;

	if (rknn_msg == NULL)
		return -1;
	if (g_rknn_msgid < 0)
		return -1;

	ipc_msg_send.msg_type = 1;
	memcpy(ipc_msg_send.rknn_msg, (char *)rknn_msg, sizeof(RKNN_MSG));
	int ret = msgsnd(g_rknn_msgid, (void *)&ipc_msg_send, sizeof(RKNN_MSG), IPC_NOWAIT);
	if (ret < 0) {
		printf("msgsnd() write msg failed,ret = %d errno=%d[%s]\n", ret, errno,
		         strerror(errno));
		printf("msgsnd() write msg failed,msg number = %d\n",
		         ipcmsg_getnum(g_rknn_msgid));
		return -1;
	}

	return 0;
}

int rknn_msg_init()
{
	return ipcmsg_init(RKNN_IPCMSGKEY, &g_rknn_msgid);
}

void rknn_msg_deinit()
{
	return ipcmsg_deinit(RKNN_IPCMSGKEY, &g_rknn_msgid);
}

int rknn_msg_send(void * predictions, void *output_classes,
	              int width, int heigh, void *group)
{
	RKNN_MSG m_msg;
	m_msg.out_data0 = predictions;
	m_msg.out_data1 = output_classes;
	m_msg.w = width;
	m_msg.h = heigh;
	m_msg.group = group;
	return ipcmsg_send(&m_msg);
}

int rknn_msg_recv(void ** predictions, void **output_classes,
	              int *width, int *heigh, void **group)
{
	RKNN_MSG m_msg;
	if (ipcmsg_rec(&m_msg) != 0)
		return -1;
	*predictions = m_msg.out_data0;
	*output_classes = m_msg.out_data1;
	*width = m_msg.w;
	*heigh = m_msg.h;
	*group = m_msg.group;
	return 0;
}

