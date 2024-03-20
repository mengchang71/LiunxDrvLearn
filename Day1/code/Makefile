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

msg:
	dmesg