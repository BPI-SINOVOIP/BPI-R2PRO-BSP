#ifndef __MANAGE_H__
#define __MANAGE_H__

void msg_init(void);
void manage_init(void);
void dev_add(char *dev, char *path);
void dev_remove(char *dev);
void add_db_storage_config(char *data);
void add_db_media_folder(char *data);
void add_db_disk_path(char *data);
void db_data_changed(char *json_str);
void dev_changed(char *dev);

#endif