#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <thread>
#include <iostream>
#include <fstream>
#include <arpa/inet.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
using namespace std;

#define CAMERA_FACTORY_TEST_SERVER_PORT 8095

void read_cb(struct bufferevent* bev, void* arg);
void write_cb(struct bufferevent* bev, void* arg);
void event_cb(struct bufferevent* bev, short events, void* arg);
void cb_listener(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* addr, int len, void* ptr);
void ProcessCommand(bufferevent* bev, char* commandBuffer);

void read_cb(struct bufferevent* bev, void* arg)
{
    char data[1024] = {0};
    char* ip = (char*)arg;

    bufferevent_read(bev, data, sizeof(data));
    cout << "client " << ip << " received:" << data << endl;

    ProcessCommand(bev, data);
}

void write_cb(struct bufferevent* bev, void* arg)
{
    cout << __PRETTY_FUNCTION__ << " exit" << endl;
}

void event_cb(struct bufferevent* bev, short events, void* arg)
{
    char* ip = (char*)arg;
    if(events & BEV_EVENT_EOF)
    {
        cout << "connection closed:" << ip << endl;
    }
    else if(events & BEV_EVENT_ERROR)
    {
        cout << "BEV_EVENT_ERROR" << endl;
    }
    bufferevent_free(bev);
    cout << __PRETTY_FUNCTION__ << " exit" << endl;
}

void cb_listener(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* addr, int len, void* ptr)
{
    struct sockaddr_in* client = (sockaddr_in*)addr ;
    cout << "connect new client: " << inet_ntoa(client->sin_addr) << "::" << ntohs(client->sin_port) << endl;
    struct event_base* base = (struct event_base*)ptr;
    struct bufferevent* bev;
    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, read_cb, write_cb, event_cb, inet_ntoa(client->sin_addr));
    bufferevent_enable(bev, EV_READ);
}

int ExecTestNpuCommand(const char* command, char* result, string refStr)
{
    int refStrCounter = 0;

    FILE* fpRead;
    fpRead = popen(command, "r");
    char buf[4096] = {0};
    memset(buf, '\0', sizeof(buf));
    while(fgets(buf, 4096 - 1, fpRead) != NULL)
    {
        if(buf[strlen(buf) - 1] == '\n')
        {
            buf[strlen(buf) - 1] = '\0';
        }
        strcpy(result, buf);

        string tmpStr = buf;
        cout << tmpStr.c_str() << endl;
        if(tmpStr.find(refStr) != string::npos)
        {
            refStrCounter += 1;
        }
    }
    if(fpRead != NULL)
    {
        pclose(fpRead);
    }

    return refStrCounter;
}

void ProcessCommand(bufferevent* bev, char* commandBuffer)
{
    string commandReceived = commandBuffer;
    cout << "command received: " << commandReceived.c_str() << endl;
    if(commandReceived == "##GET-SWVERSION##")
    {
        string filePath = "/etc/SW_VERSION";
        if(access(filePath.c_str(), 0) != -1)
        {
            ifstream swVersionFilePath(filePath);
            string swVersion;
            getline(swVersionFilePath, swVersion);
            cout << "SWVERSION:" << swVersion.c_str() << endl;
            if(strlen(swVersion.c_str()) > 0)
            {
                string sendStr = string("##SWVERSION##") + swVersion;
                bufferevent_write(bev, sendStr.c_str(), strlen(sendStr.c_str()));
                bufferevent_flush(bev, EV_WRITE, BEV_FINISHED);
                usleep(1000 * 20);
            }
        }
        else
        {
            cout << "SWVERSION file not exist" << endl;
            bufferevent_write(bev, "##CMD-FAILED##", 14);
            bufferevent_flush(bev, EV_WRITE, BEV_FINISHED);
            usleep(1000 * 20);
        }
    }
    else if(commandReceived == "##RUN-NPUTEST##")
    {
        if(access("/rockchip_test/npu/models_data_rv1109_rv1126_v1.3.4_6.4.0.227915_20200815", 0) != -1)
        {
            string cmdStr = "cd /rockchip_test/npu/rknn_test_rv1109_rv1126_linux_armhf && ./test.sh /rockchip_test/npu/models_data_rv1109_rv1126_v1.3.4_6.4.0.227915_20200815 && cd -";
            char cmdResult[4096] = {};
            string refStr = "Test pass";

            cout << "##RUN-NPUTEST## " << cmdResult << endl;
            if(ExecTestNpuCommand(cmdStr.c_str(), cmdResult, refStr) == 2) // "Test pass" twice
            {
                string sendStr = string("##NPUTEST-SUCCESS##");
                cout << sendStr.c_str() << endl;
                bufferevent_write(bev, sendStr.c_str(), strlen(sendStr.c_str()));
                bufferevent_flush(bev, EV_WRITE, BEV_FINISHED);
                usleep(1000 * 20);
            }
            else
            {
                cout << "##NPUTEST-FAILED## send result to pc" << endl;
                bufferevent_write(bev, "##NPUTEST-FAILED##", 18);
                bufferevent_flush(bev, EV_WRITE, BEV_FINISHED);
                usleep(1000 * 20);
            }
        }
        else
        {
            cout << "##NPUTEST-FAILED## send result to pc" << endl;
            bufferevent_write(bev, "##NPUTEST-FAILED##", 18);
            bufferevent_flush(bev, EV_WRITE, BEV_FINISHED);
            usleep(1000 * 20);
        }
    }
    cout << __PRETTY_FUNCTION__ << " exit" << endl;
}

int main()
{
    cout << "CameraFactoryTestServer v1.0.6 START" << endl;

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(CAMERA_FACTORY_TEST_SERVER_PORT);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);

    struct event_base* base;
    base = event_base_new();
    struct evconnlistener* listener;
    listener = evconnlistener_new_bind(base, cb_listener, base, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 36, (struct  sockaddr*)&serv, sizeof(serv));

    event_base_dispatch(base);

    if(listener != NULL)
    {
        evconnlistener_free(listener);
    }
    else
    {
        cout << "event listener bind failed." << endl;
    }
    if(base != NULL)
    {
        event_base_free(base);
    }

    return 0;
}
