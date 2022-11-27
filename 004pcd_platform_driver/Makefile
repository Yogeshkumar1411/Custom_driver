obj-m := pcd_device_setup.o

KDIR = /home/yogesh/Documents/yogesh/workspace/ldd/source/linux

KDIR_HOST = /lib/modules/$(shell uname -r)/build/

ARCH = arm

CROSS_COMPILE = arm-linux-gnueabihf-


all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules

host:
	make -C $(KDIR_HOST) M=$(PWD) modules

clean:
	make -C $(KDIR_HOST) M=$(PWD) clean
