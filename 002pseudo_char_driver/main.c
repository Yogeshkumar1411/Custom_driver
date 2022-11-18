#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>

#define DEV_MEM_SIZE 512  // limit of this driver

/*pseudo device's memory*/
char device_buffer[DEV_MEM_SIZE];

/*This holds the device number*/
dev_t device_number;

/* Cdev


static int __init pcd_driver_init(void){

	/*1. Dynamically allocate a device number*/
	alloc_chrdev_region(&device_number,0,7,"pcd");

	return 0;
}

static void __exit pcd_driver_cleanup(void){

}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yogesh");
MODULE_DESCRIPTION("pcd");

