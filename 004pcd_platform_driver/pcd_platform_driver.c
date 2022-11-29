#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<linux/platform_device.h>
#include<linux/slab.h>
#include<linux/cdev.h>


#include"platform.h"



#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt,__func__


/*Device private data strucure*/
struct pcdev_private_data
{
	struct pcdev_platform_data pdata;
	char *buffer;
	dev_t dev_num;
	struct cdev cdev;
};

/*Driver private data structure */
struct pcdrv_private_data
{
	int total_devices;
	dev_t device_num_base;
	struct class *class_pcd;
	struct device *device_pcd;

};

/*for the driver we are going to create one variable globally, but for the device we will create dynamically, whenever our probve function gets called*/

struct pcdrv_private_data pcdrv_data;


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
	/*Removing all the allocated memories in probe function we can get the by extracting pdev->dev->driver_data, which is we saved in probe function*/
	
	struct pcdev_private_data *dev_data = dev_get_drvdata(&pdev->dev);	 

	/*1. Remove a device that was created with device_create()*/
	device_destroy(pcdrv_data.class_pcd,dev_data->dev_num);

	/*2. Remove a cdev entry from the system*/
	cdev_del(&dev_data->cdev);

	/*3. Free the memory held by the device*/
	
	/*because of the usage of devm_kzalloc in probe() function no need of kfree(), when kernel detects the release of the device, it will auto
	 * -matically release the memory*/
	
	pcdrv_data.total_devices--;

	pr_info("A device is removed\n");
	return 0;
} 

/*gets called when matched plaform device is found.
 * *pdev is the pointer to matched data */
int pcd_platform_driver_probe(struct platform_device *pdev)  
{

	/*in this function we will use driver private data sturcture, for that we have declared the variable globally. In 1st step we will get the
	 * information of device details which had been done in 'pcd_device_setup.c' file. There we have registered the device details, so it will
	 *  fetched into the 'platform_device' structure in the field of 'dev' and in that we will have '*platform_data field', which will have the 
	 *  information of devive. And then by exracting the that dev field we will get the information of devices*/

	int ret;
	struct pcdev_private_data *dev_data;
	
	struct pcdev_platform_data *pdata;

	pr_info("A device is detected\n");

        /*1. Get the platform data (extracting) and storing in pdata
	 * pdata = pdev->dev.plaform_data;
	 * another method of extracting below 
	 * And, it returns void pointer and need to typecast  */
	pdata = (struct pcdev_platform_data*)dev_get_platdata(&pdev->dev);
	if(!pdata){
		pr_info("No platform data available\n");
		ret = -EINVAL;
		goto out;
	}

        /*2. Dynamically allocate memory for the device private data*/
	dev_data = devm_kzalloc(&pdev->dev, sizeof(*dev_data),GFP_KERNEL);//Added devm_kzalloc, no longer required kfree in remove function//Doubt, already the pointer having 8 bytes then after assigning the same memory space// *pointer give will total size of structure
       	if(!dev_data){
 		pr_info("No platform data available\n");
		ret = -EINVAL;
		goto out;
	}

	/*Save the device private data pointer in platform device structure for remove function*/

	dev_set_drvdata(&pdev->dev,dev_data);

	dev_data->pdata.size = pdata->size;
	dev_data->pdata.perm = pdata->perm;
	dev_data->pdata.serial_number = pdata->serial_number;

	pr_info("Device serial number = %s\n",dev_data->pdata.serial_number);
	pr_info("Device size = %d\n",dev_data->pdata.size);
	pr_info("Device permission = %d\n",dev_data->pdata.perm);

        /*3. Dynamically allocate memory for the device buffer using size
 information form the platform data*/
	dev_data->buffer = devm_kzalloc(&pdev->dev,dev_data->pdata.size,GFP_KERNEL);//added devm_kzalloc()
	if(!dev_data->buffer){
		pr_info("Cannot allocate memory\n");
		ret = -ENOMEM;
		goto dev_data_free; 
	}


        /*4. Get the device number */
	dev_data->dev_num = pcdrv_data.device_num_base + pdev->id;

        /*5. Do  cdev init and cdev add*/
	cdev_init(&dev_data->cdev,&pcd_fops);

	dev_data->cdev.owner = THIS_MODULE;
	ret = cdev_add(&dev_data->cdev,dev_data->dev_num,1);
	if(ret < 0){
		pr_err("cdev add failed\n");
		goto buffer_free;
	}

        /*6. Create device file for the detected platform device*/
	pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd,NULL,dev_data->dev_num,NULL,"pcdev-%d",pdev->id);
	if(IS_ERR(pcdrv_data.device_pcd)){
		pr_err("Device create failed\n");
		ret = PTR_ERR(pcdrv_data.device_pcd);
		goto cdev_del;
	}


	pcdrv_data.total_devices++;
	pr_info("The probe was successful\n");
	
	return 0;

/*7. Error handling */
cdev_del:
	cdev_del(&dev_data->cdev);
buffer_free:
	devm_kfree(&pdev->dev,dev_data->buffer);
dev_data_free:
	devm_kfree(&pdev->dev,dev_data);
out:
	pr_info("Device probe failed\n");
	return ret;
}

struct platform_driver pcd_platform_driver = {
	.probe = pcd_platform_driver_probe,
	.remove = pcd_platform_driver_remove,
	.driver = {
		.name = "pseudo-char-device"
	}

};

#define MAX_DEVICES 10

static int __init platform_driver_init(void){
	
	int ret;

	/*1. Dynamically allocate a device number for MAX_DEVICES*/
	ret = alloc_chrdev_region(&pcdrv_data.device_num_base,0,MAX_DEVICES,"pcdevs");
	if(ret < 0)
	{
		pr_err("Alloc chrdev failed\n");
		return ret;
	}


	/*2. Create device class under /sys/class*/
	pcdrv_data.class_pcd = class_create(THIS_MODULE,"pcd_class");
	 if(IS_ERR(pcdrv_data.class_pcd)){
		 pr_err("Class creation failed\n");
		 ret = PTR_ERR(pcdrv_data.class_pcd);
		 unregister_chrdev_region(pcdrv_data.device_num_base,MAX_DEVICES);
		 return ret;
	 }


	/*3. Register a platform driver*/
	platform_driver_register(&pcd_platform_driver);
	 
	pr_info("pcd platform driver loaded\n");

	return 0;

}


static void __exit platform_driver_exit(void){

	/*1. Unregister the platform module*/
	platform_driver_unregister(&pcd_platform_driver);

	/*2. Class destroy*/
	class_destroy(pcdrv_data.class_pcd);

	/*3. Unregister device numbers for MAX_DRVICES*/
	unregister_chrdev_region(pcdrv_data.device_num_base,MAX_DEVICES);

	pr_info("pcd platform driver unloaded\n");

}


module_init(platform_driver_init);
module_exit(platform_driver_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yogesh");
MODULE_DESCRIPTION("A pseudo character platform driver which handles n platform pcdevs");
