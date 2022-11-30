#include<linux/module.h>
#include<linux/init.h>
#include<linux/platform_device.h>
#include"platform.h"

#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt,__func__


void pcdev_release(struct device *dev)
{
	pr_info("Device released\n");

}
//1. Create 2 platform data

struct pcdev_platform_data pcdev_pdata[4] = {
	[0] = {
		.size = 512,
		.perm = RDWR,
		.serial_number = "PCDEVABC1111"
	},
	[1] = {
		.size  = 1024,
		.perm  = RDWR,
		.serial_number = "PCDEVXYZ2222"
	},
	[2] = {
		.size = 128,
		.perm = RDONLY,
		.serial_number = "PCDEVMNO3333"
	},
	[3] = {
		.size = 32,
		.perm = WRONLY,
		.serial_number = "PCDEVPQR4444"
	}
};

//2. Create 2 platform devices


struct platform_device platform_pcdev_1 = {
	.name = "pcdev-A1x",
	.id = 0,
	.dev = {
		.platform_data = &pcdev_pdata[0],
		.release = pcdev_release
	}
};

struct platform_device platform_pcdev_2 = {
	.name = "pcdev-B1x",
	.id = 1,
	.dev = {
		.platform_data = &pcdev_pdata[1],
		.release = pcdev_release
	}
};

struct platform_device platform_pcdev_3 = {
	.name = "pcdev-C1x",
	.id = 2,
	.dev = {
		.platform_data = &pcdev_pdata[2],
		.release = pcdev_release
	}
};

struct platform_device platform_pcdev_4 = {
	.name = "pcdev-D1x",
	.id = 3,
	.dev = {
		.platform_data = &pcdev_pdata[3],
		.release = pcdev_release
	}
};

struct platform_device *platform_pcdevs[] = {
	&platform_pcdev_1,
	&platform_pcdev_2,
	&platform_pcdev_3,
	&platform_pcdev_4
};


static int __init pcdev_platform_init(void){

	/*register platform device

	 *if we used below method to register means we need to call this function as per the device count
	 * platform_device_register(&platform_pcdev_1);
	 * platform_device_register(&platform_pcdev_2);
	 * so we can use below function to register all the platform devices, for the need array of platform devices*/

	platform_add_devices(platform_pcdevs,ARRAY_SIZE(platform_pcdevs));


	pr_info("device setup module loaded\n");

	return 0;
}


static void __exit pcdev_platform_exit(void){
	 
	platform_device_unregister(&platform_pcdev_1);
	platform_device_unregister(&platform_pcdev_2);
	platform_device_unregister(&platform_pcdev_3);
	platform_device_unregister(&platform_pcdev_4);
	pr_info("Device setup module unloaded\n");

}


module_init(pcdev_platform_init);
module_exit(pcdev_platform_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module which registers platform devices");
