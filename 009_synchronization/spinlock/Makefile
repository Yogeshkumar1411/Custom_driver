obj-m:=spinlock.o

KDIR_HOST = /lib/modules/$(shell uname -r)/build/
KDIR = /home/yogesh/Documents/rasperry_pi/linux/
ARM = arm
CROSS_COMPILE = arm-linux-gnueabihf-

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules

host:
	make -C $(KDIR_HOST) M=$(PWD) modules

clean:
	make -C $(KDIR_HOST) M=$(PWD) clean
