# rkauto_tool授权工具使用说明

# 申请用户名和密码

请联系对应业务申请用户名和密码（每个账号有次数限制，请妥善保管）

# 执行授权

## 直接设备上授权（设备可以联网）

- 将对应平台的rkauth_tool程序，通过adb或拷贝方式部署到设备中
  
- 执行授权命令

```
./rkauth_tool --user="xxxxxx" --passwd="xxxxxx" --output="key.lic"
```

执行成功后授权文件保存到"key.lic"文件，如果设备重新烧写固件可能会丢失，可以备份一份到PC

## PC授权（设备无法联网）

- 将对应平台的rkdevice_info执行程序，通过adb或拷贝方式部署到设备中

- 执行`rkdevice_info`命令，执行成功将生成`device.inf`文件，将该文件拷贝回pc

- 在PC执行授权命令

确保PC能够联网

```
./rkauth_tool --user="xxxxxx" --passwd="xxxxxx" --output="key.lic" --device_info=/path/to/device.inf
```

执行成功后授权文件保存到"key.lic"文件，将其拷贝到设备中使用。

## Windows版本授权工具（设备无法联网）

确保PC可以联网

- PC 上执行 `rkauth_too.exe`，输入申请的用户名、密码，选择授权主机 IP。Rockchip 授权主机外网使用 IP：`58.22.7.114`。
- 运行成功后，会在结果框显示运行结果。若过程中出错，会显示红色高亮当前行。可查看工具目录下Log目录中的log文件。
- 工具具体执行流程如下：
  - 根据主界面配置，将工具目录中指定Target arch目录中 `rkdevice_info` 通过 adb 命令 push 到指定设备端的 `/oem` 或 `/userdata` 目录下。
  -  通过 `adb` 命令 修改板端 `rkdevice_info` 的执行权限后，执行该程序，然后判断是否成功生成 `device.inf` 文件，`device.inf` 生成在 `rkdevice_info` 同一目录中。
  - 通过`adb` 命令拉取 `device.inf` 文件到本工具的 bin 目录。
  - 工具本地调用执行 bin 目录下的 `rkauth_tool.exe` 程序，根据工具界面提供的user、password、hostip、输入参数，最终生成 `key.lic` 文件。
  - 若成功生成`key.lic`文件，通过 `adb` 命令将`key.lic`文件推送到设备端指定目录。