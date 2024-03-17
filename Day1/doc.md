# Day 1

## 一 安装Linux环境

1. ubuntu-20.04.6-desktop-amd64.iso
- https://mirror.nju.edu.cn/ubuntu-releases/

2. 安装ssh
```shell
sudo apt update
sudo apt install openssh-server
sudo systemctl status ssh
sudo ufw allow ssh
```

3. 安装Xshell
- https://www.xshell.com/zh/free-for-home-school/

## 二 编写第一个内核程序

需要切换到**root**用户
