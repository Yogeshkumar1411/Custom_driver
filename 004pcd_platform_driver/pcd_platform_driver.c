#include<linux/module.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<linux/platform_device.h>

#include"platform.h"



#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt,__func__

int check_permission(int dev_perm, int acc_mode)
{
	if(dev_perm == RDWR)
		return 0;
	/*ensures readonly access*/
	if( (dev_perm == RDONLY) && ( (acc_mode & FMODE_READ) && !(acc_mode & FMODE_WRITE) ) )
		return 0;
	/*ensures writeonly access*/
	if( (dev_perm == WRONLY) && ( (acc_mode & FMODE_WRITE) && !(acc_mode & FMODE_READ) ) )
		return 0;

	return -EPERM;
}

int pcd_open(struct inode *pcd_inode,struct file *pcd_file)
{

	return 0;
}

int pcd_release(struct inode *pcd_inode,struct file *pcd_file)
{

	pr_info("Release was successful\n");
	return 0;
}

ssize_t pcd_read(struct file *pcd_file, char __user *pcd_buffer,size_t count, loff_t *f_pos)
{

	return 0;
}

ssize_t pcd_write(struct file *pcd_file, const char __user *pcd_buffer,size_t count, loff_t *f_pos)
{

	return -ENOMEM;
}

loff_t pcd_lseek(struct file *pcd_file, loff_t offset, int whence)
{

	return 0;
}


 
/* file operations of the driver */

struct file_operations pcd_fops = {
	.open = pcd_open,
	.release = pcd_release,
	.read = pcd_read,
	.write = pcd_write,
	.llseek = pcd_lseek,
	.owner = THIS_MODULE
};

/*gets called when the device is removed from the system*/
int pcd_platform_driver_remove(struct platform_device *pdev)
{
	pr_info("A device is removed\n");
	return 0;
}

/*gets called when matched plaform device is found*/
int pcd_platform_driver_probe(struct platform_device *pdev) 
{
	pr_info("A device is detected\n");
	
	return 0;
}

struct platform_driver pcd_platform_driver = {
	.probe = pcd_platform_driver_probe,
	.remove = pcd_platform_driver_remove,
	.driver = {
		.name = "pseudo-char-device"
	}

};

static int __init platform_driver_init(void){

	platform_driver_register(&pcd_platform_driver);
	pr_info("pcd platform driver loaded\n");

	return 0;

}


static void __exit platform_driver_exit(void){

	platform_driver_unregister(&pcd_platform_driver);
	pr_info("pcd platform driver unloaded\n");

}


module_init(platform_driver_init);
module_exit(platform_driver_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yogesh");
MODULE_DESCRIPTION("A pseudo character platform driver which handles n platform pcdevs");
