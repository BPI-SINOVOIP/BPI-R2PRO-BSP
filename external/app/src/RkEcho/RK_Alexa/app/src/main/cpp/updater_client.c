#include <jni.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <android/log.h>  
#include <time.h>
#include <sys/ioctl.h>

#define LOG_TAG "System.out"

//control debug print.
#define DEBUG_ON	1


#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

//updater server listen port.
#define TCP_PORT 8888
//UDP port
#define UDP_PORT 8888

//获取版本信息内存长度.
#define VERSION_BUFF_LEN	150
#define MSG_BUFF_LEN	VERSION_BUFF_LEN

//模式类型及掩码
#define MODE_UPDATER	0xC000
#define MODE_GENERATE	0x0000
#define MODE_MASK		0xC000

//状态码
#define RET_STATE_REBOOT	0x01	//升级初始化完成,设备正进入重启状态.
#define RET_STATE_FINISH	0x02	//升级完成.
#define RET_STATE_DOWNLOAD	0x03	//需要从升级服务器下载固件.
#define RET_STATE_VERSION_LATEST	0x04	//固件已经是最新.无需升级

//Messge type
typedef struct{
    unsigned short type;
#define MSG_MODE_GET		0x0001
#define MSG_MODE_OFFER		0x0002
#define MSG_MODE_SET		0x0004
#define MSG_FW_REQUST		0x0008
#define MSG_FW_INFO			0x0010
#define MSG_VERSION_GET		0x0020
#define MSG_VERSION_OFFER	0x0040
#define MSG_VERSION_SET		0x0080
#define MSG_UPDATE_START	0x0100
#define MSG_UPDATE_FINISH	0x0200

    unsigned short length;
    unsigned short csum;
    unsigned short pad;
}MessgeHead;

typedef struct{
    unsigned short id;
    unsigned short mode;
}MsgModePayload;

typedef struct{
    //bit0 -> bit5 : uboot.img -> data.img
    unsigned int imgbit;
}MsgUpdaterList;

/* client snd to server paramter.
 * @memery introduce:
 *		data_size:: 	client can recv data length.
 *		file_offset::	from where(img file) to start to send the img data.
 *		img_id:: 		indicate witch img to send to.
 */
typedef struct{
    unsigned short	data_size;
    unsigned int 	file_offset;
    unsigned short	img_id;
    //for img file id.
#define IMG_UBOOT		1
#define IMG_RESOURCE	2
#define IMG_KERNEL		3
#define IMG_ROOTFS		4
#define IMG_DATA		5
    //for img total num
#define IMG_CNT			6
}MsgFwRequest;

typedef struct{
    //xxx.img length.
    unsigned int	img_len;
    //md5 hash code.
    char	img_hash[16];
}MsgFwInfo;

typedef struct{
    char name[32];
    char path[200];
}pathNode;

pathNode fwPathArray[IMG_CNT] = {
        {
                NULL,NULL
        },
        {
                "uboot.img","imgs/uboot.img"
        },
        {
                "resource.img","imgs/resource.img"
        },
        {
                "kernel.img","imgs/kernel.img"
        },
        {
                "rootfs.img","imgs/rootfs.img"
        },
        {
                "data.img","imgs/data.img"
        }
};

static unsigned short g_dev_mode = 0;
static unsigned int g_dev_id = 0;

static char* device_ip_address = "10.201.126.1";

JNIEXPORT jstring JNICALL
Java_com_rockchip_alexa_utils_SocketUtil_getCString(JNIEnv *env, jclass type) {
    char* hello = "Hello World.";
    return (*env)->NewStringUTF(env, hello);
}

char* jstringToCharArray(JNIEnv* env, jstring jstr)
{
    char* rtn = NULL;
    jclass clsstring =(*env)->FindClass(env,"java/lang/String");
    jstring strencode = (*env)->NewStringUTF(env,"utf-8");
    jmethodID mid = (*env)->GetMethodID(env,clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr= (jbyteArray)(*env)->CallObjectMethod(env,jstr,mid,strencode);
    jsize alen = (*env)->GetArrayLength(env,barr);
    jbyte* ba = (*env)->GetByteArrayElements(env,barr, JNI_FALSE);
    if (alen > 0)
    {
        rtn = (char*)malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    (*env)->ReleaseByteArrayElements(env,barr, ba, 0);
    return rtn;
}

int jstringToCharArray1(JNIEnv* env, jstring jstr,char* rtn)
{
    jclass clsstring =(*env)->FindClass(env,"java/lang/String");
    jstring strencode = (*env)->NewStringUTF(env,"utf-8");
    jmethodID mid = (*env)->GetMethodID(env,clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr= (jbyteArray)(*env)->CallObjectMethod(env,jstr,mid,strencode);
    jsize alen = (*env)->GetArrayLength(env,barr);
    jbyte* ba = (*env)->GetByteArrayElements(env,barr, JNI_FALSE);
    LOGD("length: %d",alen);
    if (alen > 0)
    {
        rtn = (char*)malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    (*env)->ReleaseByteArrayElements(env,barr, ba, 0);
    return 0;
}

/**
 * 数据和校验
 * [msghdr_csum description]
 * @param  hdr [description]
 * @return     [description]
 */
unsigned short msghdr_csum(void* hdr)
{
    unsigned short* data = (unsigned short*)hdr;
    unsigned int csum = 0;
    csum += *data;
    csum += *(data+1);
    csum += *(data+2);
    csum += *(data+3);

    //high 16bit add low 16bit
    csum = (csum & 0x0000FFFF) + ((csum & 0xFFFF0000) >> 16);

    return ~((unsigned short)csum);
}

int version_to_str(unsigned int ver, char* buff)
{
    //修改
}

int str_to_version(char* str, int len)
{
    int tmp[10];
    int dot = -1;
    int i = 0;
    int id = 0;
    int result = 0;

    for(i=0,id=0; i<len; i++)
    {
        if(str[i] == '.'){
            dot = i+1;
            continue;
        }else{
            tmp[id++] = (unsigned char)(*(str + i)) - '0';
        }
    }
    tmp[id] = dot;

    for(i=0; i<=id; i++)
    {
        result = result*10 + tmp[i];
    }

    return result;
}

/* +++++ 待扩展 ++++++++++++ */
/* 生成升级ID,该ID随机产生,最好确保唯一性.
 * newid: true->生成新的ID. faulse->从文件中读取老的ID. */
unsigned int updater_getid(int newid,JNIEnv *env)
{
    unsigned short id = 0;
    if(newid)
    {
        // 设置并获取updateId
        jclass dpclazz = (*env)->FindClass(env,"com/rockchip/alexa/jacky/utils/updater_client");
        jmethodID method1 = (*env)->GetStaticMethodID(env,dpclazz,"getNewUpdateId","()J");
        jlong result = (*env)->CallStaticLongMethod(env,dpclazz,method1);
//        /* +++++++ 产生新的随机ID, 并写入文件 +++++++++++ */
        id = (unsigned short)result;
        LOGD("new UpdateId: %x",id);
    }
    else
    {
        // 获取updateId
        jclass dpclazz = (*env)->FindClass(env,"com/rockchip/alexa/jacky/utils/updater_client");
        jmethodID method1 = (*env)->GetStaticMethodID(env,dpclazz,"getOldUpdateId","()J");
        jlong result = (*env)->CallStaticLongMethod(env,dpclazz,method1);
//        /* +++++++ 产生新的随机ID, 并写入文件 +++++++++++ */
        id = (unsigned short)result;
        LOGD("get OldUpdateId: %x",id);
    }
    return id;
}


/* 获取模式信息.模式信息存放在info_buf中. */
int updater_getmode(int sockfd, char* info_buf)
{
    int ret = 0;
    MessgeHead* hdr = (MessgeHead*)info_buf;

    LOGD("#Get mode info...\n");

    hdr->type = htons(MSG_MODE_GET);
    hdr->length = htons(sizeof(MessgeHead));
    hdr->pad = 0;
    hdr->csum = 0;
    hdr->csum = msghdr_csum(hdr);

    ret = send(sockfd, hdr, sizeof(MessgeHead), 0);
    if(ret < 0)
    {
        LOGD("%s<%d>:Send mode get Messge failed!\n", __func__, __LINE__);
        return -1;
    }

    ret = recv(sockfd, info_buf, MSG_BUFF_LEN, 0);
    if(ret < 0)
    {
        LOGD("%s<%d>:recv mode Messge failed!\n", __func__, __LINE__);
        return -1;
    }

    if(msghdr_csum(hdr))
    {
        LOGD("%s<%d>:recv mode Messge csum error!\n",__func__,__LINE__);
        return -1;
    }
    MsgModePayload* payload = (MsgModePayload*)(info_buf + sizeof(MessgeHead));
//    g_dev_mode = payload->mode & MODE_MASK;
//    g_dev_id = payload->id;

    LOGD("\tCurrent Dev mode:%x\n",payload->mode & MODE_MASK);
    LOGD("\tCurrent Dev ID:%x\n",payload->id);

    return 0;
}

/**获取模式信息.
 * 参数解析:
 * 1.sockfd: 链接到设备端的TCP套接口.
 * 2.info_buf: 构建"MSG_VERSION_GET"消息的存储空间.
 * 3.ver_buf: 存放版本信息字符串.
 * 4.buf_len: ver_buf 长度
 */
int updater_getversion(int sockfd, char* info_buf, char* ver_buf, int buf_len)
{
    LOGD("version get begin");
    int ret = 0;
    MessgeHead* hdr = (MessgeHead*)info_buf;

    LOGD("#Get version info...\n");

    if(buf_len < VERSION_BUFF_LEN)
    {
        LOGD("%s<%d>:sizeof(ver_buf) is too small!\n",__func__,__LINE__);
        return -1;
    }

    hdr->type = htons(MSG_VERSION_GET);
    hdr->length = htons(sizeof(MessgeHead));
    hdr->pad = 0;
    hdr->csum = 0;
    hdr->csum = msghdr_csum(hdr);

    ret = send(sockfd, hdr, sizeof(MessgeHead), 0);
    if(ret < 0)
    {
        LOGD("%s<%d>:Send version get Messge failed!\n",__func__,__LINE__);
        return -1;
    }

    ret = recv(sockfd, ver_buf, MSG_BUFF_LEN, 0);
    if(ret < 0)
    {
        LOGD("%s<%d>:recv version Messge failed!\n",__func__,__LINE__);
        return -1;
    }
    *(ver_buf + ret) = '\0';
    LOGD("ret : %d \n",ret - sizeof(MessgeHead));

    if(msghdr_csum(ver_buf))
    {
        LOGD("%s<%d>:recv version Messge csum error!\n",__func__,__LINE__);
        return -1;
    }
    hdr = (MessgeHead*)ver_buf;
    LOGD("hdr->length = %d <---> %d\n",ntohs(hdr->length) - sizeof(MessgeHead), strlen(ver_buf + sizeof(MessgeHead)));
    LOGD("Current Version is:%s\n", ver_buf + sizeof(MessgeHead));

    return 0;
}

/**设置模式信息和升级ID
 * 参数解析:
 * 1.sockfd: 链接到设备端的TCP套接口.
 * 2.info_buf: 构建发送命令的存储空间.
 * 3.new_mode: 新的模式.
 * 4.new_id: 新的ID.
 */
int updater_setmode(int sockfd, char* info_buf, unsigned short new_mode, unsigned short new_id)
{
    int ret = 0;
    MessgeHead* hdr = (MessgeHead*)info_buf;
    MsgModePayload* payload = (MsgModePayload*)(info_buf + sizeof(MessgeHead));

    LOGD("#Set mode info...\n");

    /* 模式合法性检查. bit1
     * 5-bt14, bit4-bit0为有效位,其他强制为0. */
    if(new_mode & 0x3FE0)
    {
        LOGD("%s<%d>:New mode(0x%x) is not valid!\n",__func__,__LINE__, new_mode);
        return -1;
    }

    LOGD("\tnew_mode=0x%x, new_id=0x%x\n", new_mode, new_id);

    hdr->type = htons(MSG_MODE_SET);
    hdr->length = htons(sizeof(MessgeHead) + sizeof(MsgModePayload));
    hdr->pad = 0;
    hdr->csum = 0;
    hdr->csum = msghdr_csum(hdr);

    payload->mode = g_dev_mode = new_mode;
    payload->id = g_dev_id = new_id;

    ret = send(sockfd, hdr, sizeof(MessgeHead) + sizeof(MsgModePayload), 0);
    if(ret < 0)
    {
        LOGD("%s<%d>:Send mode set Messge failed!\n",__func__,__LINE__);
        return -1;
    }

    return 0;
}

/**设置版本信息.
 * 参数解析:
 * 1.sockfd: 链接到设备端的TCP套接口.
 * 2.info_buf: 构建发送命令的存储空间.
 * 3.vstr: 版本信息字符串.
 * 4.str_len: 版本字符串长度.
 */
int updater_setversion(int sockfd, char* info_buf, char* vstr, int str_len)
{
    int ret = 0;
    MessgeHead* hdr = (MessgeHead*)info_buf;

    LOGD("#Set version info...\n");
    LOGD("\tAt cpp file New version:%s\n", vstr);

    hdr->type = htons(MSG_VERSION_SET);
    hdr->length = htons(sizeof(MessgeHead) + str_len);
    hdr->pad = 0;
    hdr->csum = 0;
    hdr->csum = msghdr_csum(hdr);

    memcpy((info_buf + sizeof(MessgeHead)), vstr, str_len);

    ret = send(sockfd, hdr, sizeof(MessgeHead)+str_len, 0);
    if(ret < 0)
    {
        LOGD("%s<%d>:Send mode get Messge failed!\n",__func__,__LINE__);
        return -1;
    }

    return 0;
}

/**发送升级开始命令,设备获得该命令则准备从网络接收固件数据.
 * 参数解析:
 * 1.sockfd: 链接到设备端的TCP套接口.
 * 2.info_buf: 构建发送命令的存储空间.
 */
int updater_sndstart(int sockfd, char* info_buf)
{
    int ret = 0;
    MessgeHead* hdr = (MessgeHead*)info_buf;

    LOGD("#Snd start commend...\n");

    hdr->type = htons(MSG_UPDATE_START);
    hdr->length = htons(sizeof(MessgeHead));
    hdr->pad = 0;
    hdr->csum = 0;
    hdr->csum = msghdr_csum(hdr);

    ret = send(sockfd, hdr, sizeof(MessgeHead), 0);
    if(ret < 0)
    {
        LOGD("%s<%d>:Send updater start Messge failed!\n",__func__,__LINE__);
        return -1;
    }

    return 0;
}

/**创建新的TCP套接口,并与设备建立链接.
 * IP:设备IP地址.
 */
int tcp_socket(char* IP)
{
    int sockfd = 0;
    struct sockaddr_in servaddr;
    MsgFwRequest* fwRequset = NULL;
    int ret = 0;
    struct timeval timeout={20,0};

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        LOGD("create socket failed!\n");
        return 0;
    }
    //set connect timeout.
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO,(const char*)&timeout, sizeof(timeout));
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(const char*)&timeout, sizeof(timeout));

    //connect
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr= inet_addr(IP);
    servaddr.sin_port=htons(TCP_PORT);
    ret = connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    if(ret < 0)
    {
        LOGD("Socket Connect failed!\n");
        close(sockfd);
        return 0;
    }

    //reset socket timeout
//    timeout.tv_sec = 0;
//    timeout.tv_usec = 0;
//    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO,(const char*)&timeout, sizeof(timeout));

    return sockfd;
}

/**发送当前固件长度和MD5值.
 * 参数解析:
 * 1.sockfd: 链接到设备端的TCP套接口.
 * 2.info_buf: 构建发送命令的存储空间.
 * 3.len：文件长度。
 * 4.hashcode：MD5哈希值。
 */
int updater_sndfwinfo(int sockfd, char* info_buf, unsigned int len, char* hashcode)
{
    int ret = 0;
    MessgeHead* hdr = (MessgeHead*)info_buf;
    MsgFwInfo* payload = (MsgFwInfo*)(info_buf + sizeof(MessgeHead));

    printf("#Snd fwinfo message...\n");

    hdr->type = htons(MSG_FW_INFO);
    hdr->length = htons(sizeof(MessgeHead) + sizeof(MsgFwInfo));
    hdr->pad = 0;
    hdr->csum = 0;
    hdr->csum = msghdr_csum(hdr);

    payload->img_len = htonl(len);
    memcpy(payload->img_hash, hashcode,16);
    LOGD("img-hash: %s",payload->img_hash);

    ret = send(sockfd, hdr, sizeof(MessgeHead)+sizeof(MsgFwInfo), 0);
    if(ret < 0)
    {
        printf("%s<%d>:Send fwinfo Messge failed!\n",__func__,__LINE__);
        return -1;
    }
    return 0;
}


int md5_hash_code(char* filePath,char* md5code,JNIEnv *env)
{
    memset(md5code,0, sizeof(md5code));
    jclass dpclazz = (*env)->FindClass(env,"com/rockchip/alexa/jacky/utils/Md5Util");
    jmethodID method1 = (*env)->GetStaticMethodID(env,dpclazz,"getFileMD5String","(Ljava/lang/String;)[B");
    jbyteArray result = (jbyteArray)((*env)->CallStaticObjectMethod(env,dpclazz,method1,(*env)->NewStringUTF(env,filePath)));
    jbyte* data = (*env)->GetByteArrayElements(env,result,0);
    jsize size = (*env)->GetArrayLength(env,result);
    memcpy(md5code,(char*)data,size);
    return 0;
}

//read data from img file. and send to client.
int sending_fw_datas(int sockfd, MsgFwRequest* param,JNIEnv *env)
{
    //for read data from img file.and send to client.
    unsigned char* databuff = NULL;
    unsigned int offset = 0;
    unsigned short data_size = 0;
    unsigned int file_len = 0;
    int file = 0;
    char length_info[20];
    int ret = 0;
    unsigned char md5code[16];
    char info_buf[MSG_BUFF_LEN];

    //for debug.
    int snded_len = 0;

    //网络字节序转换.
    param->img_id = ntohs(param->img_id);
    param->data_size = ntohs(param->data_size);
    param->file_offset = ntohl(param->file_offset);

    //检查请求固件信息合法性.
    if( (param->img_id < IMG_UBOOT) ||
        (param->img_id > IMG_DATA) )
    {
        LOGD("%s<%d> Device request img id error!\n", __func__, __LINE__);
        return -1;
    }

#if DEBUG_ON
    LOGD("\n=== Start updating %s ===\n", fwPathArray[param->img_id].path);
    LOGD("\t-->data_size:%d\n", param->data_size);
    LOGD("\t-->file_offset:%d\n", param->file_offset);
    LOGD("\t-->img_id:%d\n", param->img_id);
#endif

    file = open(fwPathArray[param->img_id].path, O_RDONLY);
    if(file < 0)
    {
        LOGD("%s<%d> Open %s file failed!\n", __func__, __LINE__, fwPathArray[param->img_id].path);
        return -1;
    }

    offset = param->file_offset;

    //get file total length.
    file_len = lseek(file, 0, SEEK_END);
    if(offset >= file_len)
    {
        close(file);
        LOGD("%s<%d> %s length(%d) less then Offset(%d)!\n", __func__, __LINE__, fwPathArray[param->img_id].path, file_len, offset);
        return -1;
    }

#if DEBUG_ON
    LOGD("\t%s size = %d\n", fwPathArray[param->img_id].path, file_len);
#endif
    md5_hash_code(fwPathArray[param->img_id].path, md5code,env);
    updater_sndfwinfo(sockfd,info_buf,file_len,md5code);

//    /* 发送当前固件文件大小, 该大小固定使用20字节消息传送.
//     * 20字节代表文件长度(十进制)不超过20位.该值足够大.
//     */
//    memset(length_info, '\0', sizeof(length_info));
//    sprintf(length_info, "%d", file_len);
//    if(send(sockfd, length_info, sizeof(length_info), 0) < 0)
//    {
//        close(file);
//        LOGD("%s<%d> Send %s length failed!\n", __func__, __LINE__, fwPathArray[param->img_id].path);
//        return -1;
//    }

    //根据文件大小分配内存.
    data_size = ((file_len - offset) > param->data_size) ? param->data_size:(file_len - offset);
    if(data_size)
    {
        databuff = (unsigned char*)malloc(data_size);
        if(databuff == NULL)
        {
            LOGD("%s<%d> Memory alloc failed!\n", __func__, __LINE__);
            close(file);
            return -1;
        }
    }

    //修真文件长度变量file_len, 此时该变量表示需要网络传输字节数;
    file_len -= offset;
    //snd file from offset position.
    lseek(file, offset, SEEK_SET);

#if DEBUG_ON
    LOGD("\tBegin to snd file(size = %d)!\n", file_len);
#endif

    while(file_len)
    {
        ret = read(file, databuff, data_size);
        if(ret < 0){
            LOGD("%s<%d> Read %s failed!\n", __func__, __LINE__, fwPathArray[param->img_id].path);
            return -1;
        }else if(ret == 0){
            LOGD("\n\tRead file END!\n");
            break;
        }

        //send data to client by TCP Socket.
        if(send(sockfd, databuff, ret, 0) < 0)
        {
            LOGD("%s<%d> Send fw data to Device failed!\n", __func__, __LINE__);
            return -1;
        }

        if(file_len > ret)
            file_len -= ret;
        else
            file_len = 0;
    }

#if DEBUG_ON
    LOGD("\tEnd to send file(size = %d)!\n", file_len);
#endif

    if(databuff)
        free(databuff);
    close(file);

    return 0;
}

/* 升级模式下处理函数,发送固件数据给设备. */
int updater_start(int sockfd, char* info_buf,JNIEnv *env)
{
    int ret = 0;
    MessgeHead* hdr = NULL;
    MsgModePayload* payload = NULL;
    MsgFwRequest* fwRequset = NULL;
    short imgBit = 0;

    hdr = (MessgeHead*)info_buf;
    payload = (MsgModePayload*)(info_buf + sizeof(MessgeHead));
    fwRequset = (MsgFwRequest *)(payload);

    //如果ID和更新列表不同,则需要重新解析版本,并设置更新列表.
    LOGD("old UpdateId<--->payloadId: %x,%x",g_dev_id,payload->id);
    if( (g_dev_id != payload->id))
    {
        //获取版本信息
        ret = updater_getversion(sockfd, info_buf, info_buf, MSG_BUFF_LEN);
        if(ret < 0)
            return ret;

        jstring versionResult = (*env)->NewStringUTF(env, info_buf + sizeof(MessgeHead));

        // 在c代码里面调用java代码里面的方法
        // 获取最新的版本信息
        jclass dpclazz = (*env)->FindClass(env,"com/rockchip/alexa/jacky/utils/updater_client");
        jmethodID method2 = (*env)->GetStaticMethodID(env,dpclazz,"getUpdateImageBit","(Ljava/lang/String;)S");
        short result = (*env)->CallStaticShortMethod(env,dpclazz,method2,versionResult);

        imgBit = result;
        //根据上面对比的结果,设定imgBit.
        LOGD("imgBit = %x", result);

        //更新模式信息,更新列表, 和设置升级ID.
        ret = updater_setmode(sockfd, info_buf, (MODE_UPDATER | imgBit), updater_getid(1,env));
        if(ret < 0)
            return ret;

        sleep(1);
    }

    //发送升级开始消息.
    ret = updater_sndstart(sockfd, info_buf);
    if(ret < 0)
        return ret;

    while(1)
    {
        LOGD("recv begin1111..");
        //recv paramter info from dev.
        ret = recv(sockfd, info_buf, MSG_BUFF_LEN, 0);
        LOGD("recv end11111..");
        if(ret < 0){
            LOGD("%s<%d> recv Error!, (%d)%s\n", __func__, __LINE__, ret,strerror(ret));
            return -1;
        }else if(ret == 0){
            LOGD("%s<%d> Dev close the connection. The reomte Device has something error!\n", __func__, __LINE__);
            return -1;
        }

        if(msghdr_csum(hdr))
        {
            LOGD("%s<%d>: Message recved from Device csum error!\n",__func__,__LINE__);
            return -1;
        }

        if(hdr->type == htons(MSG_FW_REQUST))
        {
            LOGD("MSG_FW_REQUST 1");
            ret = sending_fw_datas(sockfd, fwRequset,env);
            if(ret < 0)
                return ret;
        }
        else if(hdr->type == htons(MSG_UPDATE_FINISH))
        {
            int str_len = 0;
            char* vstr = NULL;

            /* +++++++++ 待扩展: 获取当前固件版本信息,组织成如下格式,注意尾巴的','不要少 ++++++++++++++ */
            // 在c代码里面调用java代码里面的方法
            // 获取可更新image的信息
            jclass dpclazz = (*env)->FindClass(env,"com/rockchip/alexa/jacky/utils/updater_client");
            jmethodID method1 = (*env)->GetStaticMethodID(env,dpclazz,"getUpdateVersionStr","()Ljava/lang/String;");
            jstring result = (*env)->CallStaticObjectMethod(env,dpclazz,method1);
            vstr = jstringToCharArray(env,result);

            str_len = strlen(vstr);

            LOGD("@@@@ Recved Finish message!\n");
            LOGD("@@@@ Snd set version message!\n");
            //设置新的版本信息.
            ret = updater_setversion(sockfd, info_buf, vstr, str_len);
            if(ret < 0)
                return ret;

            LOGD("@@@@ Snd set mode message!\n");

            //设置mode为普通模式,触发设备重启.
            ret = updater_setmode(sockfd, info_buf, MODE_GENERATE, 0);
            if(ret < 0)
                return ret;

            sleep(1);
            break; //while(1) 出口.
        }
        else
        {
            LOGD("%s<%d>:recv message Type error!\n",__func__,__LINE__);
            return -1;
        }
    }//end while(1)

    return RET_STATE_FINISH;
}

/**
 * 普通模式下处理函数.对升级进行一些初始化操作.
 * 设置模式为升级模式,并设置更新列表(imgBit)
 * 设置升级ID,用于升级校对.
 */
int updater_init(int sockfd, char* info_buf,JNIEnv *env)
{
    int ret = 0;
    /* 记录待更新img的比特位,每位对应一个img.
     * bit0->uboot.img, bit1->resource.img, bit2->kernel.img,
     * bit3->rootfs.img, bit4->data.img.
     */
    short imgBit = 0;
    MessgeHead* msghdr = NULL;
    MsgModePayload* payload = NULL;

    msghdr = (MessgeHead*)info_buf;
    payload = (MsgModePayload*)(info_buf + sizeof(MessgeHead));

    //获取版本信息
    ret = updater_getversion(sockfd, info_buf, info_buf, MSG_BUFF_LEN);
    if(ret < 0)
        return ret;

    jstring versionResult = (*env)->NewStringUTF(env, info_buf + sizeof(MessgeHead));
    LOGD("%s",info_buf + sizeof(MessgeHead));

    // 获取最新的版本信息
    jclass dpclazz = (*env)->FindClass(env,"com/rockchip/alexa/jacky/utils/updater_client");
    jmethodID method2 = (*env)->GetStaticMethodID(env,dpclazz,"getUpdateImageBit","(Ljava/lang/String;)S");
    short result = (*env)->CallStaticShortMethod(env,dpclazz,method2,versionResult);

    imgBit = result;
    //根据上面对比的结果,设定imgBit.
    LOGD("imgBit = %x", result);
    if(imgBit == 0){
        return RET_STATE_VERSION_LATEST;
    }
    //更新模式信息,更新列表, 和设置升级ID.
    ret = updater_setmode(sockfd, info_buf, (MODE_UPDATER | imgBit), updater_getid(1,env));
    if(ret < 0)
        return ret;

    return RET_STATE_REBOOT;
}

int do_updater(char* IP,JNIEnv *env)
{
    int ret = 0;
    int sockfd = 0;
    char info_buf[MSG_BUFF_LEN];
    MessgeHead* msghdr = NULL;
    MsgModePayload* payload = NULL;

    msghdr = (MessgeHead*)info_buf;
    payload = (MsgModePayload*)(info_buf + sizeof(MessgeHead));

    //读取老的ID.
    g_dev_id = updater_getid(0,env);

    //创建TCP套接口.
    sockfd = tcp_socket(IP);
    if(sockfd < 0)
        return -1;

    //获取mode信息.
    ret = updater_getmode(sockfd, info_buf);
    if(ret < 0)
        return -1;
    //信息验证.
    if(msghdr_csum(msghdr)){
        LOGD("%s<%d>:recv mode Messge csum error!\n",__func__,__LINE__);
        return -1;
    }else if(msghdr->type != htons(MSG_MODE_OFFER)){
        LOGD("%s<%d>:recv mode Messge Type error(not MSG_MODE_OFFER)!\n",__func__,__LINE__);
        return -1;
    }

    switch(payload->mode & MODE_MASK)
    {
        case MODE_GENERATE:
        {
            LOGD("MODE_GENERATE");
            ret = updater_init(sockfd, info_buf,env);
            break;
        }
        case MODE_UPDATER:
        {
            LOGD("MODE_UPDATER");
            ret = updater_start(sockfd, info_buf,env);
            break;
        }
        default:
            LOGD("Sorry, Device is in Error Mode, Please connect manufacturer!\n");
            ret = -1;
    }
    close(sockfd);
    return ret;
}

JNIEXPORT jstring JNICALL
Java_com_rockchip_alexa_jacky_utils_updater_1client_getDeviceVersion(JNIEnv *env, jclass type) {
    int ret = 0;
    int sockfd = 0;
    char info_buf[MSG_BUFF_LEN];

    //创建TCP套接口.
    sockfd = tcp_socket(device_ip_address);
    if(sockfd < 0)
        goto Exit_Failed;

    ret = updater_getversion(sockfd, info_buf, info_buf, MSG_BUFF_LEN);
    if(ret < 0)
        goto Exit_Failed;

    close(sockfd);
    return (*env)->NewStringUTF(env, info_buf + sizeof(MessgeHead));

    Exit_Failed:
    close(sockfd);
    return NULL;
}

JNIEXPORT jint JNICALL
Java_com_rockchip_alexa_jacky_utils_updater_1client_doUpdater(JNIEnv *env, jclass type)
{
    int ret = 0;
    while(1)
    {
        LOGD("do_updater");
        ret = do_updater(device_ip_address,env);\
        if(ret == RET_STATE_REBOOT)
        {
            jclass dpclazz = (*env)->FindClass(env,"com/rockchip/alexa/jacky/utils/updater_client");
            jmethodID method1 = (*env)->GetStaticMethodID(env,dpclazz,"waitDeviceReboot","()I");
            while(1){
                jint result = (*env)->CallStaticIntMethod(env,dpclazz,method1);
                LOGD("result: %d",result);
                if(result == 0 || result >= 100){
                    break;
                }
                sleep(1);
            }
            continue;
        }
        else if(ret == RET_STATE_DOWNLOAD)
        {
            LOGD("Please download update firmware files from server!\n");
            break;
        }
        else if(ret == RET_STATE_VERSION_LATEST)
        {   LOGD("device has latest version of image,return.");
            jclass dpclazz = (*env)->FindClass(env,"com/rockchip/alexa/jacky/utils/updater_client");
            jmethodID method1 = (*env)->GetStaticMethodID(env,dpclazz,"onVersionLatest","()V");
            (*env)->CallStaticVoidMethod(env,dpclazz,method1);
            break;
        }
        else if(ret == RET_STATE_FINISH)
        {
            LOGD("Congratulations! updater success!\n");
            jclass dpclazz = (*env)->FindClass(env,"com/rockchip/alexa/jacky/utils/updater_client");
            jmethodID method1 = (*env)->GetStaticMethodID(env,dpclazz,"onUpdateSuccess","()V");
            (*env)->CallStaticVoidMethod(env,dpclazz,method1);
            break;
        }
        else
        {
            LOGD("\n Sorry, updater failed! please make sure the follow steps is OK:\n\t1.device is power on. \n\t2.the net between your mobile phone and device is OK.\n if still update failed, please connetc the manufacturer!\n");
            jclass dpclazz = (*env)->FindClass(env,"com/rockchip/alexa/jacky/utils/updater_client");
            jmethodID method1 = (*env)->GetStaticMethodID(env,dpclazz,"onUpdateFailed","()V");
            (*env)->CallStaticVoidMethod(env,dpclazz,method1);
            break;
        }
    }
}

JNIEXPORT void JNICALL
Java_com_rockchip_alexa_jacky_utils_updater_1client_setImageDirectory(JNIEnv *env, jclass type,
                                                                      jstring imageDirectory_) {
    const char *imageDirectory = (*env)->GetStringUTFChars(env, imageDirectory_, 0);

    memset(fwPathArray[1].path,0, sizeof(fwPathArray[1].path));
    memset(fwPathArray[2].path,0, sizeof(fwPathArray[1].path));
    memset(fwPathArray[3].path,0, sizeof(fwPathArray[1].path));
    memset(fwPathArray[4].path,0, sizeof(fwPathArray[1].path));
    memset(fwPathArray[5].path,0, sizeof(fwPathArray[1].path));
    strcat(strcat(fwPathArray[1].path,imageDirectory),fwPathArray[1].name);
    strcat(strcat(fwPathArray[2].path,imageDirectory),fwPathArray[2].name);
    strcat(strcat(fwPathArray[3].path,imageDirectory),fwPathArray[3].name);
    strcat(strcat(fwPathArray[4].path,imageDirectory),fwPathArray[4].name);
    strcat(strcat(fwPathArray[5].path,imageDirectory),fwPathArray[5].name);
}
