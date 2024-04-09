obj-m := globalmem.o

KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm .*.*.cmd
	rm *.mod.*
	rm *.mod
	rm *.o
	rm *.symvers
	rm *.order