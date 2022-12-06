#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<linux/platform_device.h>
#include<linux/slab.h>
#include<linux/cdev.h>
#include<linux/mod_devicetable.h>
#include<linux/of.h>

#include<linux/of_device.h>

#include"platform.h"



#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt,__func__

/*User creation of configuration items for different versions of devices*/
struct device_config
{
        int config_item1;
        int config_item2;
};

enum pcdev_names
{
        PCDEVA1X,
        PCDEVB1X,
        PCDEVC1X,
        PCDEVD1X
};

struct device_config pcdev_config[] = {
        [PCDEVA1X] = {
                .config_item1 = 60,
                .config_item2 = 21
        },
        [PCDEVB1X] = {
                .config_item1 = 50,
                .config_item2 = 20
        },
        [PCDEVC1X] = {
                .config_item1 = 60,
                .config_item2 = 19
        },
        [PCDEVD1X] = {
                .config_item1 = 70,
                .config_item2 = 18
        }
};



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
#if 1
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
#endif

	dev_info(&pdev->dev,"A device is removed\n");
	return 0;
} 

struct pcdev_platform_data* pcdev_get_platdata_from_dt(struct device *dev)
{
	struct device_node *dev_node = dev->of_node;
	struct pcdev_platform_data *pdata;
	if(!dev_node)
		/*this probe didn't happen because of device tree node*/
		return NULL;
	pdata = devm_kzalloc(dev,sizeof(*pdata),GFP_KERNEL);
	if(!pdata){
		dev_info(dev,"cannot allocate memory\n");
		return ERR_PTR(-ENOMEM);//because this function will return pointer only
	}

	/*in of.h, we are having the functions for reading string, integer(u32)*/
	if(of_property_read_string(dev_node,"org,device_serial_num",&pdata->serial_number) )
	{
		dev_info(dev,"Missing serial number\n");
		return ERR_PTR(-EINVAL);
	}
	if(of_property_read_u32(dev_node,"org,size",&pdata->size))
	{
		dev_info(dev,"Missing size property\n");
		return ERR_PTR(-EINVAL);
	}

        if(of_property_read_u32(dev_node,"org,perm",&pdata->perm))
        {
                dev_info(dev,"Missing size property\n");
                return ERR_PTR(-EINVAL);
        }
	return pdata;
}


/*gets called when matched plaform device is found.
 * *pdev is the pointer to matched data */
int pcd_platform_driver_probe(struct platform_device *pdev)  
{

	/*in this function we will use driver private data sturcture, for that we have declared the variable globally. In 1st step we will get the
	 * information of device details which had been done in 'pcd_device_setup.c' file. There we have registered the device details, so it will
	 *  fetched into the 'platform_device' structure in the field of 'dev' and in that we will have '*platform_data field', which will have the 
	 *  information of devive. And then by exracting the that dev field we will get the information of devices*/

	int ret,driver_data;
	struct pcdev_private_data *dev_data;
	
	struct pcdev_platform_data *pdata;

	struct device *dev = &pdev->dev;

	dev_info(dev,"A device is detected\n");

	/*Extracting the platform data and storing in pdata device tree method*/
	pdata = pcdev_get_platdata_from_dt(dev);
	if(IS_ERR(pdata))
	{
		return -EINVAL;
	}
	if(!pdata)//device_setup method 
	{

	       	 /*1. Get the platform data (extracting) and storing in pdata device setup method
		 * pdata = pdev->dev.plaform_data;
		 * another method of extracting below 
		 * And, it returns void pointer and need to typecast  */
		pdata = (struct pcdev_platform_data*)dev_get_platdata(dev);
		if(!pdata){
			dev_info(dev,"No platform data available\n");
			return -EINVAL;		
		}
		driver_data = pdev->id_entry->driver_data;
	}
	else//device_tree method
	{
		/*match = of_match_device(pdev->dev.driver->of_match_table,&pdev->dev);
		driver_data = (int)match->data;
		* instead of doing above method we will use below function, which is defined in of_device.h*/
		driver_data = (int) of_device_get_match_data(dev);
	}

        /*2. Dynamically allocate memory for the device private data*/

	dev_data = devm_kzalloc(&pdev->dev, sizeof(*dev_data),GFP_KERNEL);

	/*	//Added devm_kzalloc, no longer required kfree in remove function
	 *	//Doubt, already the pointer having 8 bytes then after assigning the same memory space
	 *	// *pointer give will total size of structure*/

       	if(!dev_data){
 		dev_info(dev,"cannot allocate memory\n");
		return -ENOMEM;  
	}

	/*Save the device private data pointer in platform device structure for remove function*/

	dev_set_drvdata(&pdev->dev,dev_data);

	dev_data->pdata.size = pdata->size;
	dev_data->pdata.perm = pdata->perm;
	dev_data->pdata.serial_number = pdata->serial_number;

	pr_info("Device serial number = %s\n",dev_data->pdata.serial_number);
	pr_info("Device size = %d\n",dev_data->pdata.size);
	pr_info("Device permission = %d\n",dev_data->pdata.perm);

	/*printing the configuretion items*/

	pr_info("Config item 1 = %d\n",pcdev_config[driver_data].config_item1);
	pr_info("Config item 2 = %d\n",pcdev_config[driver_data].config_item2);

        /*3. Dynamically allocate memory for the device buffer using size
 information form the platform data*/
	dev_data->buffer = devm_kzalloc(&pdev->dev,dev_data->pdata.size,GFP_KERNEL);//added devm_kzalloc()
	if(!dev_data->buffer){
		dev_info(dev,"Cannot allocate memory\n");
		return -ENOMEM;
	}


        /*4. Get the device number */
	dev_data->dev_num = pcdrv_data.device_num_base + pcdrv_data.total_devices;

        /*5. Do  cdev init and cdev add*/
	cdev_init(&dev_data->cdev,&pcd_fops);

	dev_data->cdev.owner = THIS_MODULE;
	ret = cdev_add(&dev_data->cdev,dev_data->dev_num,1);
	if(ret < 0){
		dev_err(dev,"cdev add failed\n");
		return ret;
	}

        /*6. Create device file for the detected platform device*/
	pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd,dev,dev_data->dev_num,NULL,"pcdev-%d",pcdrv_data.total_devices);
	if(IS_ERR(pcdrv_data.device_pcd)){
		dev_err(dev,"Device create failed\n");
		ret = PTR_ERR(pcdrv_data.device_pcd);
		cdev_del(&dev_data->cdev);
		return ret;
	}


	pcdrv_data.total_devices++;
	dev_info(dev,"The probe was successful\n");
	
	return 0;

/*7. Error handling.
 * If probe function got failed means, the function itself take care of clearing the allocated memories. So, no need to handle this,
cdev_del:
	cdev_del(&dev_data->cdev);
buffer_free:
	devm_kfree(&pdev->dev,dev_data->buffer);
dev_data_free:
	devm_kfree(&pdev->dev,dev_data);
out:
	pr_info("Device probe failed\n");
	return ret;*/


}
/*
struct device_config
{
	int config_item1;
	int config_item2;
};

enum pcdev_names
{
	PCDEVA1X,
	PCDEVB1X,
	PCDEVC1X,
	PCDEVD1X
};

struct device_config pcdev_config[] = {
	[PCDEVA1X] = {
		.config_item1 = 60,
		.config_item2 = 21
	},
	[PCDEVB1X] = {
		.config_item1 = 50,
		.config_item2 = 20
	},
	[PCDEVC1X] = {
		.config_item1 = 60,
		.config_item2 = 19
	},
	[PCDEVD1X] = {
		.config_item1 = 70,
		.config_item2 = 18
	}
};*/

/* in platform_driver structure we are having the 'id_table' field type of 'struct platform_device_id' and in that type structure we are having 
 * 'name and driver_data field' by using the 'name' we are doing id matching and with that 'driver_data' we are implementing the configuration
 * details of different versions of devices*/
struct platform_device_id pcdevs_id[] = {
	[0] = {
		.name = "pcdev-A1x",
		.driver_data = PCDEVA1X
	},
	[1] = {
		.name = "pcdev-B1x",
		.driver_data = PCDEVB1X
	},
	[2] = {
		.name = "pcdev-C1x",
		.driver_data = PCDEVC1X
	},
	[3] = {
		.name = "pcdev-D1x",
		.driver_data = PCDEVD1X
	},
	{}
};


/*for device tree matching need to create separate structure type of 'of_device_id' which is the element of 'device_driver' structure. In this
 *'of_device_id' field we will have information like compatible property, name, data*/
struct of_device_id org_pcdev_dt_match[] = {
	/*No need to mention index values*/
	{.compatible = "pcdev-A1x",.data = (void*)PCDEVA1X},// data is the type of void. and PCDEVA1X is number because of enum. need to typecast
	{.compatible = "pcdev-B1x",.data = (void*)PCDEVB1X},
	{.compatible = "pcdev-C1x",.data = (void*)PCDEVC1X},
	{.compatible = "pcdev-D1x",.data = (void*)PCDEVD1X},
	{}//null entry for termination.
};

struct platform_driver pcd_platform_driver = {
	.probe = pcd_platform_driver_probe,
	.remove = pcd_platform_driver_remove,
	.id_table = pcdevs_id,//initiating the id table field
	.driver = {
		/*when we used id means the name matching will no longer used*/
		.name = "pseudo-char-device",
		/*initializing of_match_table for DT*/
		.of_match_table = org_pcdev_dt_match
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
