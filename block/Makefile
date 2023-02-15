obj-m := block_Sample.o

KDIR=/lib/modules/$(shell uname -r)/build/

KDIR_TARGET=/home/yogesh/Documents/yogesh/workspace/ldd/source/linux


all:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C $(KDIR_TARGET) M=$(PWD) modules

host:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

