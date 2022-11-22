#include<linux/module.h>
#include<linux/fs.h>
#include<linux/init.h>
#include<linux/kdev_t.h>
#include<linux/cdev.h>
#include<linux/uaccess.h>
#include<linux/device.h>

#define NO_OF_DEVICES 4

#define MEM_SIZE_MAX_PCDEV1 1024
#define MEM_SIZE_MAX_PCDEV2 512
#define MEM_SIZE_MAX_PCDEV3 1024
#define MEM_SIZE_MAX_PCDEV4 512

#undef pr_fmt
#define pr_fmt(fmt) "%s:" fmt,__func__
/*pseudo device's memory*/
char device_buffer_pcdev1[MEM_SIZE_MAX_PCDEV1];
char device_buffer_pcdev2[MEM_SIZE_MAX_PCDEV2];
char device_buffer_pcdev3[MEM_SIZE_MAX_PCDEV3];
char device_buffer_pcdev4[MEM_SIZE_MAX_PCDEV4];

/*Device private data structure*/
struct pcdev_private_data
{
	char *buffer;
	unsigned size;
	const char *serial_number;
	int perm;
	struct cdev cdev;
};

/* Driver private data structure */
struct pcdrv_private_data
{
	int total_devices;
	/*This holds the device number and it is driver's property*/
	dev_t device_number;

	/*device creation and it belongs to driver*/
	struct class *class_pcd;

	struct device *device_pcd;

	struct pcdev_private_data pcdev_data[NO_OF_DEVICES];
};


/* initializing the pcdrv_private_data structure*/
struct pcdrv_private_data pcdrv_data = 
{
	.total_devices = NO_OF_DEVICES,
	.pcdev_data = {
		[0] = {
			/*device elements*/
			.buffer = device_buffer_pcdev1,
			.size = MEM_SIZE_MAX_PCDEV1,
			.serial_number = "pcdev1xyz123",
			.perm = 0x1 //RDONLY,
		},
                [1] = {
                        /*device elements*/
                        .buffer = device_buffer_pcdev2,
                        .size = MEM_SIZE_MAX_PCDEV2,
                        .serial_number = "pcdev2xyz123",
                        .perm = 0x10 //WRONLY,
                },
                [2] = {
                        /*device elements*/
                        .buffer = device_buffer_pcdev1,
                        .size = MEM_SIZE_MAX_PCDEV1,
                        .serial_number = "pcdev1xyz123",
                        .perm = 0x11 //RDWR,
                },
                [3] = {
                        /*device elements*/
                        .buffer = device_buffer_pcdev1,
                        .size = MEM_SIZE_MAX_PCDEV1,
                        .serial_number = "pcdev1xyz123",
                        .perm = 0x11 //RDWR,
                }
	}
};



int pcd_open(struct inode *pcd_inode,struct file *pcd_filp)
{
	pr_info("Open was successful\n");

	return 0;
}

int pcd_release(struct inode *pcd_inode,struct file *pcd_filp)
{
	pr_info("Release was successful\n");

	return 0;
}

ssize_t pcd_read(struct file *pcd_filp,char __user *pcd_buffer,size_t count,loff_t *f_pos)
{
#if 0
	pr_info("read requested for %zu bytes \n",count);
	pr_info("Current file position = %lld\n",*f_pos);
	
	/*
	 * Adjust the count*/
	if((*f_pos+count)>DEV_MEM_SIZE)
		count = DEV_MEM_SIZE - *f_pos;
	
	/*copy to user*/
	if(copy_to_user(&pcd_buffer,&device_buffer[*f_pos],count))
	{
		return -EFAULT;
	}

	/*update the current file position*/

	*f_pos += count;

	pr_info("Number of bytes successfully read = %zu\n",count);
	pr_info("Updaed file position = %lld\n",*f_pos);

	/*
	 * return number of bytes successfully read
	 * */
	return count;
#endif
	return 0;
}
ssize_t pcd_write(struct file *pcd_filp,const char __user *pcd_buffer,size_t count,loff_t *f_pos)
{
#if 0
	pr_info("Write requested for %zu bytes\n",count);
	pr_info("Current file position = %lld\n",*f_pos);

	/*
	 * Adjust the count*/
	if((*f_pos+count)>DEV_MEM_SIZE)
		count = DEV_MEM_SIZE - *f_pos;
	if(!count)
	{
		pr_err("No space left on the device\n");
		return -ENOMEM;
	}

	/*copy from user*/
	if(copy_from_user(&device_buffer[*f_pos],&pcd_buffer,count))
	{
		return -EFAULT;
	}

	/*update the current file position*/
	*f_pos += count;

	pr_info("Number of bytes successfully write = %zu\n",count);
	pr_info("Updated file position = %lld\n",*f_pos);

	/*
	 * Return number of bytes which have been successfully written
	 * */
	return count;
#endif 
	return 0;
}
loff_t pcd_lseek(struct file *pcd_filp,loff_t offset,int whence)
{
#if 0
	loff_t temp;
	pr_info("lseek requested\n");
	pr_info("Current value of the file position = %lld\n",pcd_filp->f_pos);
	switch(whence)
	{
		case SEEK_SET:
			if((offset > DEV_MEM_SIZE) || (offset < 0))
				return -EINVAL;
			pcd_filp->f_pos = offset;
			break;

		case SEEK_CUR:
			temp = pcd_filp->f_pos + offset;
			if((temp > DEV_MEM_SIZE) || (temp < 0))
				return -EINVAL;
			pcd_filp->f_pos = temp;
			break;

		case SEEK_END:
			temp = DEV_MEM_SIZE + offset;
			if((temp > DEV_MEM_SIZE) || (temp < 0))
				return -EINVAL;
			pcd_filp->f_pos = temp;
			break;

		default:
			return -EINVAL;
	}

	pr_info("New value of the file position = %lld\n",pcd_filp->f_pos);
	return pcd_filp->f_pos;
#endif
	return 0;
}


/*for file operations*/
struct file_operations pcd_fops =
{
	.open = pcd_open,
	.release = pcd_release,
	.read = pcd_read,
	.write = pcd_write,
	.llseek = pcd_lseek,
	.owner = THIS_MODULE
};

/*device creation and it belongs to driver*/
struct class *class_pcd;

struct device *device_pcd;

static int __init pcd_init(void){
#if 0

	int ret;
	/*
	 * 1. Allocating device number*/
	ret = alloc_chrdev_region(&device_number,0,4,"pcd_multiple");
	pr_info("Device number <major>:<minor> = %d:%d\n",MAJOR(device_number),MINOR(device_number));
	if(ret < 0)
	{
		pr_err("Alloc chrdev failed\n");
		goto out;
	}
	
	/*2. cdev initializaion*/
	cdev_init(&pcd_cdev,&pcd_fops);

	/*3. Registering the cdev in VFS*/
	pcd_cdev.owner=THIS_MODULE;
	ret = cdev_add(&pcd_cdev,device_number,4);
	if(ret < 0)
	{
		pr_err("cdev_add failed\n");
		goto unreg_chrdev;
	}

	/* 4. Creating device files
	 * */
	/* creating device class under /sys/class
	 */
	class_pcd = class_create(THIS_MODULE,"pcd_multiple_class");

	/*class_create returns pointer. So while failing-
	 * it will returns pointer to negative value. So the macro-
	 * IS_ERR is used
	 */
	if(IS_ERR(class_pcd))
	{
		pr_err("Class creation failed\n");
		/*
		 * PTR_ERR() converts pointer to error code(int)
		 * ERR_PTR() converts error code(int) to pointer
		 */
		ret = PTR_ERR(class_pcd);
		goto cdev_del;
	}
	/*
	 * Populate the sysfs with device information
	 */
	device_pcd = device_create(class_pcd,NULL,device_number,NULL,"pcd_multiple");
	if(IS_ERR(device_pcd))
	{
		pr_err("Device create failed\n");
		ret = PTR_ERR(device_pcd);
		goto class_del;
	}

	pr_info("Module init loaded\n");


	return 0;

class_del:
	class_destroy(class_pcd);

cdev_del:
	cdev_del(&pcd_cdev);

unreg_chrdev:
	unregister_chrdev_region(device_number,4);

out:
	pr_info("Module insertion failed\n");
	return ret;
#endif
	return 0;

}


static void __exit pcd_cleanup(void){
#if 0

	device_destroy(class_pcd,device_number);
	class_destroy(class_pcd);
	cdev_del(&pcd_cdev);
	unregister_chrdev_region(device_number,4);

	pr_info("Module unloaded\n");
#endif
}


module_init(pcd_init);
module_exit(pcd_cleanup);


MODULE_LICENSE("GPL");	
MODULE_AUTHOR("Yogesh");
MODULE_DESCRIPTION("A simple pcd with multiple devices");
