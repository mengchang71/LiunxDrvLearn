/*
* a simple char device without driver: globalmem without mutex
*
* Licensed under GPLv2 or later
*/

#include <linux/module.h>
#include <linux/stat.h>
#include <linux/cdev.h>

#define GLOBALMEM_SIZE 0x1000
#define MEM_CLEAR 0x1
#define GLOBALMEM_MAJOR 230

struct globalmem_dev {
	struct cdev cdev;
	unsigned char mem[GLOBALMEM_SIZE];
};