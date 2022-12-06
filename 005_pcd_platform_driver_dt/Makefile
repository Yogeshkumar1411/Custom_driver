obj-m := pcd_platform_driver_dt.o

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

copy-dtb:
	scp /home/yogesh/Documents/yogesh/workspace/ldd/source/linux/arch/arm/boot/dts/am335x-boneblack.dtb debian@192.168.0.239:/home/debian/yogesh

copy-drv:
	scp *.ko debian@192.168.0.239:/home/debian/yogesh
