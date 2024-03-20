# Day 1

## 一 环境搭建

1. ubuntu-22.04.4-desktop-amd64.iso
- https://mirror.nju.edu.cn/ubuntu-releases/22.04/

2. 安装ssh
```bash
sudo apt update
sudo apt install openssh-server
sudo systemctl status ssh
sudo ufw allow ssh
```

3. 安装Xshell
- https://www.xshell.com/zh/free-for-home-school/

4. 配置和构造内核树
``` bash
sudo apt update
sudo apt install build-essential linux-headers-$(uname -r)
```

## 二 编写第一个内核程序
- hello.c
``` c
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

static int hello_init(void)
{
    printk(KERN_ALERT "Hello World linux_driver_module\n");
    return 0;
}

static void hello_exit(void)
{
    printk(KERN_ALERT "Goodbey linux_driver_module\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("mengc");
```

- MakeFile
``` makefile
#sample driver module
obj-m := hello.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	rm .*.*.cmd
	rm *.mod.*
	rm *.mod
	rm *.o
	rm *.symvers
	rm *.order

load:
	insmod hello.ko

unload:
	rmmod hello.ko

o:
	dmesg
```

- 测试
``` bash
make all
make load
make unload
make msg
make clean
```

- 输出
``` bash
[ 6516.173473] Hello World linux_driver_module
[ 6521.668400] Goodbey linux_driver_module
```

**Day1 Complete！！！**

![内核功能划分](https://github.com/mengchang71/LiunxDrvLearn/blob/main/images/img_Day1/%E5%86%85%E6%A0%B8%E5%8A%9F%E8%83%BD%E5%88%92%E5%88%86.png)