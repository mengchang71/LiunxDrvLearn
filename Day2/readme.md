# Day2 编写一个完整的字符设备驱动程序

## 一 主设备号和次设备号

**字符设备**：通常位于 _/dev_ 目录，通过`ls -l`输出的第一列中的"c"来识别，块设备也出现在 _/dev_ 下,由字符"b"标识
``` bash
root@mengc-virtual-machine:/dev# ls -l
total 0
crw-r--r--  1 root  root     10, 235  3月 17 15:14 autofs
drwxr-xr-x  2 root  root         320  3月 17 15:14 block
drwxr-xr-x  2 root  root          80  3月 17 15:13 bsg
crw-------  1 root  root     10, 234  3月 17 15:14 btrfs-control
drwxr-xr-x  3 root  root          60  3月 17 15:13 bus
brw-rw----  1 root  disk      7,   0  3月 17 15:14 loop0
```
>上面7、10是主设备号，0、60、80、234、235、320是次设备号；

**主设备号**表示对应驱动程序， _/dev/autofs_ 由驱动10管理;
**次设备号**由内核使用，用于正确确定设备文件所指的设备，通过次设备号，可以获得一个指向内核设备的直接指针;
## 二 设备号的内部表达
在内核中，dev_t类型(在<linux/types.h>中定义)用来保存设备编号，其中包含了主设备号和次设备号。dev_t是一个32位的数，12位用来表示主设备号，其余20位用来表示次设备号。
**获取主设备号：MAJOR(dev_t dev)**
**获取次设备号：MINOR(dev_t dev)**
**主设备和次设备号转换成dev_t类型：MKDEV(int major, int minor);**

## 三 分配和释放设备编号
建议使用动态分配设备编号：
`int alloc_chrdev_region(dev_t* dev, unsigned int firstminor, unsigned int count, char* name);`
`firstminor`:要使用的被请求的第一个次设备号，通常是0
`count`:请求的设备连续编号个数
`name`:该范围编号关联的设备名称
释放设备编号：
`void unregister_chrdev_region(dev_t dev, unsigned int count);`
**分配的设备编号可以在 _/proc/devices_ 获取到**
``` bash
root@mengc-virtual-machine:/dev# cat /proc/devices 
Character devices:
  1 mem
  4 /dev/vc/0
  4 tty
  4 ttyS
  5 /dev/tty
  5 /dev/console
  5 /dev/ptmx
  5 ttyprintk
  6 lp
  7 vcs
 10 misc
 13 input
 14 sound/midi
 14 sound/dmmidi
```
## 四 一些重要的数据结构
设备编号的注册是驱动程序代码必须完成许多工作的第一步
大部分基本驱动涉及三个重要的内核数据结构：**file_operations, file, inode**
### 文件操作
**file_operations**：将驱动程序连接到分配的编号
定义在 _<linux/fs.h>_ , 包含一组函数指针，每个打开的文件和一组函数关联
**file结构**：表示一个打开的文件
定义在 _<linux/fs.h>_ , 由内核在 _open_ 时创建, 并传递给在该文件上进行操作的所有函数, 直到最后的 _close_ 函数，内核会释放这个数据结构
**inode结构**：包含大量有关文件信息
对于单个文件，可能会有许多个表示打开的文件描述符的file结构，但它们都指向单个inode结构

# 五 Linux字符设备驱动结构
## 5.1 cdev结构体
> 代码清单5.1 cdv结构体 -- dev结构体描述一个字符设备
``` c {.line-numbers}
struct cdev {
  struct kobkect kobj;          // 内嵌的kobject对象
  struct module* owner;         // 所属模块
  struct file_operations* ops;  // 文件操作结构体
  struct list_head list;
  dev_t dev;                    // 设备号
  unsigned int count;
};
```

> 代码5.2 cdev_init()函数 -- 初始化cdev的成员，并建立cdev和file_operations之间的连接
``` c {.line-numbers}
void cdev_init(struct cdev *cdev, struct file_operations *fops)
{
  memset(cdev, 0, sizeof *cdev);
  INIT_LIST_HEAD(&cdev->list);
  kobject_init(&cdev->kobj, &ktype_cdev_default);
  cdev->ops = fops; /* 将传入的文件操作结构体指针赋值给cdev的ops*/
}
```

> 代码清单5.3 cdv_alloc()函数 -- 动态申请一个cdev内存
``` c {.line-numbers}
struct cdev *cdev_alloc(void)
{
  struct cdev *p = kzalloc(sizeof(struct cdev), GFP_KERNEL);
  if (p) {
    INIT_LIST_HEAD(&p->list);
    kobject_init(&p->kobj, &ktype_cdev_dynamic);
  }
  return p;
}
```
cdev_add（）函数和cdev_del（）函数分别向系统添加和删除一个cdev，完成字符设备注册和注销

## 5.2 file_operations结构体
进行Linux的open(), write(), read(), close()
> 代码清单 5.4 file_operations结构体
```c {.line-numbers}
struct file_operations {
  struct module *owner;
  loff_t (*llseek) (struct file *, loff_t, int);
  ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
  ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
  ssize_t (*aio_read) (struct kiocb *, const struct iovec *,
    unsigned long, loff_t);
  ssize_t (*aio_write) (struct kiocb *, const struct iovec *,
    unsigned long, loff_t);
  int (*iterate) (struct file *, struct dir_context *);
  unsigned int (*poll) (struct file *, struct poll_table_struct *);
  long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
  long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
  int (*mmap) (struct file *, struct vm_area_struct *);
  int (*open) (struct inode *, struct file *);
  int (*flush) (struct file *, fl_owner_t id);
  int (*release) (struct inode *, struct file *);
  int (*fsync) (struct file *, loff_t, loff_t, int datasync);
  int (*aio_fsync) (struct kiocb *, int datasync);
  int (*fasync) (int, struct file *, int);
  int (*lock) (struct file *, int, struct file_lock *);
  ssize_t (*sendpage) (struct file *, struct page *, int, size_t,
    loff_t *, int);
  unsigned long (*get_unmapped_area)(struct file *,
    unsigned long, unsigned long, unsigned long, unsigned long);
  int (*check_flags)(int);
  int (*flock) (struct file *, int, struct file_lock *);
  ssize_t (*splice_write)(struct pipe_inode_info *, struct file *,
    loff_t *,  size_t, unsigned int);
  ssize_t (*splice_read)(struct file *, loff_t *,
    struct pipe_inode_info *,  size_t, unsigned int);
  int (*setlease)(struct file *, long, struct file_lock **);
  long (*fallocate)(struct file *file, int mode, loff_t offset,
    loff_t len);
  int (*show_fdinfo)(struct seq_file *m, struct file *f);
};
```

**主要成员:**
1. llseek（）函数用来修改一个文件的当前读写位置，并将新位置返回，在出错时，这个函数返回一个负值
2. read（）函数用来从设备中读取数据，成功时函数返回读取的字节数，出错时返回一个负值。它与用户空间应用程序中的ssize_t read(int fd，void*buf，size_t count)和size_t fread(void* ptr，size_t size，size_t nmemb，FILE*stream)对应。
3. write（）函数向设备发送数据，成功时该函数返回写入的字节数。如果此函数未被实现，当用户进行write（）系统调用时，将得到-EINVAL返回值。它与用户空间应用程序中的ssize_t write(int fd，const void*buf，size_t count)和size_t fwrite（const void*ptr，size_t size，size_t nmemb，FILE*stream）对应。
**read（）和write（）如果返回0，则暗示end-of-file（EOF）。**
4. unlocked_ioctl（）提供设备相关控制命令的实现（既不是读操作，也不是写操作），当调用成功时，返回给调用程序一个非负值。它与用户空间应用程序调用的int fcntl(int fd，int cmd，.../*arg*/)和int ioctl（int d，int request，...）对应。
5. mmap（）函数将设备内存映射到进程的虚拟地址空间中，如果设备驱动未实现此函数，用户进行mmap（）系统调用时将获得-ENODEV返回值。这个函数对于帧缓冲等设备特别有意义，帧缓冲被映射到用户空间后，应用程序可以直接访问它而无须在内核和应用间进行内存复制。它与用户空间应用程序中的void* mmap(void*addr，size_t length，int prot，int flags，int fd，off_t offset)函数对应。
6. 当用户空间调用Linux API函数open（）打开设备文件时，设备驱动的open（）函数最终被调用。驱动程序可以不实现这个函数，在这种情况下，设备的打开操作永远成功。与open（）函数对应的是release（）函数。
7. poll（）函数一般用于询问设备是否可被非阻塞地立即读写。当询问的条件未触发时，用户空间进行select（）和poll（）系统调用将引起进程的阻塞。
8. aio_read（）和aio_write（）函数分别对与文件描述符对应的设备进行异步读、写操作。设备实现这两个函数后，用户空间可以对该设备文件描述符执行SYS_io_setup、SYS_io_submit、SYS_io_getevents、SYS_io_destroy等系统调用进行读写。

## 5.3 字符设备驱动模块加载与卸载
> 代码清单 5.5 字符设备驱动模块加载与卸载函数模板
``` c

```

> 代码清单 5.6 字符设备驱动读、写、I/O控制函数模板
``` c

```

> 代码清单 5.7 字符设备驱动文件操作结构体模板
``` c

```

![字符设备驱动的结构]