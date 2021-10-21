Base on openwrt-21.02

# 注意：
1，不要用root进行编译

2，国内用户编译前最好做好科学上网

3，默认登录IP 192.168.3.1 密码123456

4，本仓库并非完整编译出openwrt，编译完成后只取用rootfs，望知悉

# 编译
```bash
$ cp configs/ROC-3568-PC_config .config
$ make defconfig
$./scripts/feeds update -a
$./scripts/feeds install -a
$ make -j1 V=s
```

# 保存默认配置

运行如下命令，生成简化过的defconfig文件

```bash
$ ./scripts/diffconfig.sh > defconfig
```

以后可以直接使用该文件进行配置：

```bash
$ cp defconfig .config

# 扩展为完整的.config
$ make defconfig 
```

