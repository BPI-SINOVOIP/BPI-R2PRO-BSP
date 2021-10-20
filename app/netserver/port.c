#include <stdio.h>
#include <string.h>
#include "json-c/json.h"
#include "dbserver.h"
#include "log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "port.c"

static int port_num_get(int port_id)
{
    char *json_str = dbserver_port_get();
    if (!json_str)
        return -1;
    json_object *port_info = json_tokener_parse(json_str);
    json_object *port_data = json_object_object_get(port_info, "jData");
    json_object *port = json_object_array_get_idx(port_data, port_id);
    json_object *port_num_js = json_object_object_get(port, "iPortNo");
    int port_num = (int)json_object_get_int(port_num_js);

    free(json_str);
    json_object_put(port_info);
    return port_num;
}

void port_init(char *nginx_conf_path)
{
    LOG_INFO("port init, nginx_conf_path is %s\n", nginx_conf_path);
    int http_port_num = port_num_get(0);
    int rtmp_port_num = port_num_get(4);
    if ((http_port_num == -1) || (http_port_num == -1)) {
        LOG_INFO("dbserver port get fail\n");
        return;
    }

    char http_listen[255];
    char rtmp_listen[255];
    sprintf(http_listen, "        listen       %d;", http_port_num);
    sprintf(rtmp_listen, "        listen %d;", rtmp_port_num);
    LOG_INFO("http_listen is %s\n", http_listen);
    LOG_INFO("rtmp_listen is %s\n", rtmp_listen);

    FILE *fp = fopen(nginx_conf_path, "r+");
    if (!fp) {
        LOG_ERROR("can't open %s, port number use default value\n", nginx_conf_path);
        return;
    }

    char buf[255];
    while(fgets(buf, sizeof(buf), fp)) {
        if (strstr(buf, "http {")) {
            while(fgets(buf, sizeof(buf), fp)) {
                if (strstr(buf, "listen")) {
                    int pos_begin = ftell(fp) - strlen(buf);
                    // cover the line with new listen
                    fseek(fp, pos_begin, SEEK_SET);
                    fputs(http_listen, fp);
                    // avoid the original string is too long, part not covered
                    int len_change = strlen(buf) - 1 - strlen(http_listen);
                    if (len_change > 0) {
                        for (int i = 0; i < len_change; i++) {
                            fputs(" ", fp);
                        }
                    }
                    fputs("\n", fp);
                    break;
                }
            }
        }
        if (strstr(buf, "rtmp {")) {
            while(fgets(buf, sizeof(buf), fp)) {
                if (strstr(buf, "listen")) {
                    int pos_begin = ftell(fp) - strlen(buf);
                    // cover the line with new listen
                    fseek(fp, pos_begin, SEEK_SET);
                    fputs(rtmp_listen, fp);
                    // avoid the original string is too long, part not covered
                    int len_change = strlen(buf) - 1 - strlen(rtmp_listen);
                    if (len_change > 0) {
                        for (int i = 0; i < len_change; i++) {
                            fputs(" ", fp);
                        }
                    }
                    fputs("\n", fp);
                    break;
                }
            }
        }
    }
    fclose(fp);

    system("nginx -s reload");
    LOG_INFO("port init over\n");
}
