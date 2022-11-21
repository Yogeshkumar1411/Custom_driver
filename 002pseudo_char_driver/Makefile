obj-m := pcd.o

ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
KDIR=/home/yogesh/Documents/yogesh/workspace/ldd/source/linux
KDIR_HOST=/lib/modules/$(shell uname -r)/build/

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules
	
host:
	make -C $(KDIR_HOST) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
help:
	make -C $(KDIR) M=$(PWD) help
