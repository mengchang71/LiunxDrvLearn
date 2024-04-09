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
