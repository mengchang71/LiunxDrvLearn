# 编写一个自定义字符设备

## 1、加载卸载
### 1.1、加载初始化模块
``` c {.line-numbers}
static void globalmem_setup_cdev(struct globalmem_dev *dev, int index)
{
	int devno = MKDEV(globalmem_major, index);
	int err = cdev_add(&dev->cdev, devno, 1);
	if (err)
		printk(KERN_NOTICE "Error %d adding globalmem%d", err, index);
}

static int __init globalmem_init(void)
{
	printk(KERN_NOTICE "globalmem_init");
	dev_t devno = MKDEV(globalmem_major, 0);

	int ret = 0;
	if (globalmem_major) {
		ret = register_chrdev_region(devno, 1, "globalmem");
	} else {
		ret = alloc_chrdev_region(&devno, 0, 1, "globalmem");
		globalmem_major = MAJOR(devno);
	}
	if (ret < 0)
		return ret;
	
	globalmem_devp = kzalloc(sizeof(struct globalmem_dev), GFP_KERNEL);
	if (!globalmem_devp) {
		ret = -ENOMEM;
		goto fail_malloc;
	}
	globalmem_setup_cdev(globalmem_devp, 0);
	return 0;

fail_malloc:
	unregister_chrdev_region(devno, 1);
	return ret;
}

module_init(globalmem_init);
```

### 1.2、卸载模块
``` c {.line-numbers}
static void __exit globalmem_exit(void)
{
	printk(KERN_NOTICE "globalmem_exit");
	cdev_del(&globalmem_devp->cdev);
	kfree(globalmem_devp);
	unregister_chrdev_region(MKDEV(globalmem_major, 0), 1);
}

module_exit(globalmem_exit);
```

### 1.3、执行结果
![执行结果_内核打印](https://github.com/mengchang71/LiunxDrvLearn/blob/main/images/img_Day3/1.3_%E6%89%A7%E8%A1%8C%E7%BB%93%E6%9E%9C.png)
---

## 2、打开 释放 读写函数
### 2.1 打开
``` c {.line-numbers}
static int globalmem_open(struct inode *inode, struct file *filp)
{
	filp->private_data = globalmem_devp;
	return 0;
}
```

### 2.2、释放
``` c {.line-numbers}
static int globalmem_release(struct inode *inode, struct file *file)
{
	return 0;
}
```

### 2.3、读写
``` c {.line-numbers}
static ssize_t globalmem_read(struct file *filp, char __user *buf, size_t size,
	loff_t *ppos)
{
	unsigned long p = *ppos;
	unsigned int count = size;

	struct globalmem_dev *dev = filp->private_data;
	int ret = 0;
	if (p >= GLOBALMEM_SIZE)
		return 0;
	if (count > GLOBALMEM_SIZE - p)
		count = GLOBALMEM_SIZE - p;

	if (copy_to_user(buf, dev->mem + p, count)) {
		ret = -EFAULT;
	} else {
		*ppos += count;
		ret = count;

		printk(KERN_INFO "read %u byte(s) from %lu\n", count, p);
	}

	return ret;
}

static ssize_t globalmem_write(struct file *filp, const char __user *buf,
	size_t size, loff_t *ppos)
{
	unsigned long p = *ppos;
	unsigned int count = size;
	struct globalmem_dev *dev = filp->private_data;

	int ret = 0;
	if (p > GLOBALMEM_SIZE)
		return 0;
	if (count > GLOBALMEM_SIZE - p)
		count = GLOBALMEM_SIZE - p;

	if (copy_from_user(dev->mem + p, buf, count))
		ret = -EFAULT;
	else {
		*ppos += count;
		ret = count;

		printk(KERN_INFO "written %u byte(s) from %lu\n", count, p);
	}

	return ret;
}
```

2.4、设备控制
``` c {.line-numbers}
static long globalmem_ioctl(struct file *filp, unsigned int cmd,
	unsigned long arg)
{
	struct globalmem_dev *dev = filp->private_data;

	switch (cmd) {
	case MEM_CLEAR:
		memset(dev->mem, 0, GLOBALMEM_SIZE);
		printk(KERN_INFO "globalmem is set to zero\n");
		break;

	default:
		return -EINVAL;
	}

	return 0;
}
```
2.5、bash以及执行结果
``` bash
insmod globalmem.ko
cat /proc/devices
```
输出
![2.5.1查看设备结果](https://github.com/mengchang71/LiunxDrvLearn/blob/main/images/img_Day3/2.5.1.png)

``` bash
mknod /dev/globalmem c 230 0
echo "hello world" > /dev/globalmem
cat /dev/globalmem
```
输出
![2.5.2设备输出](https://github.com/mengchang71/LiunxDrvLearn/blob/main/images/img_Day3/2.5.2.png)