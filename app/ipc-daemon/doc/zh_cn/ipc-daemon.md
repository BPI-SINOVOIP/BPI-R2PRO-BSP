***
ipc-daemon
=============

简介
----

ipc-daemon是ipc后台进程，负责服务启动管理，提供系统命令服务。
1. database file check and restore
2. start ipc services and monitor status of them
   * dbserver
   * storage_manager
   * netserver
   * mediaserver
3. create dbus service, process system command
   * reboot

   * factory reset：simple reset / full reset

   * export  device parameters

   * export debug log

   * import device parameters

   * upgrade

   * notify:  system status  (TODO)


