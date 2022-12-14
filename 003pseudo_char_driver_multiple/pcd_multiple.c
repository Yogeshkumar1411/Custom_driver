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

#define RDONLY 0X01
#define WRONLY 0X10
#define RDWR 0X11

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
			.perm = RDONLY //RDONLY
		},
                [1] = {
                        /*device elements*/
                        .buffer = device_buffer_pcdev2,
                        .size = MEM_SIZE_MAX_PCDEV2,
                        .serial_number = "pcdev2xyz123",
                        .perm = WRONLY //WRONLY
                },
                [2] = {
                        /*device elements*/
                        .buffer = device_buffer_pcdev3,
                        .size = MEM_SIZE_MAX_PCDEV3,
                        .serial_number = "pcdev3xyz123",
                        .perm = RDWR //RDWR
                },
                [3] = {
                        /*device elements*/
                        .buffer = device_buffer_pcdev4,
                        .size = MEM_SIZE_MAX_PCDEV4,
                        .serial_number = "pcdev4xyz123",
                        .perm = RDWR //RDWR
                }
	}
};


/*for the time being check_permission return 0*/

int check_permission(int dev_perm,int access_mode)
{
	if(dev_perm == RDWR)
		return 0;
	if( (dev_perm == RDONLY) && ( (access_mode & FMODE_READ) && !(access_mode & FMODE_WRITE) ) )
		return 0;
	if( (dev_perm == WRONLY) && ( (access_mode & FMODE_WRITE) && !(access_mode & FMODE_READ) ) )
		return 0;
	return -EPERM;
}


int pcd_open(struct inode *pcd_inode,struct file *pcd_filp)
{
	int ret;

	int minor_n;

	struct pcdev_private_data *pcdev_data;

	/*Find out which device filem open was attempted by the userspace*/
	minor_n = MINOR(pcd_inode->i_rdev);
	pr_info("Minor access = %d\n",minor_n);


	/*get the device's private data structure*/
	pcdev_data = container_of(pcd_inode->i_cdev,struct pcdev_private_data,cdev);


	/*to supply device private data to other methods of the driver*/
	pcd_filp->private_data = pcdev_data;

	/*check permission*/
	ret=check_permission(pcdev_data->perm,pcd_filp->f_mode);


	(!ret)?pr_info("Open was successful\n"):pr_info("Open was unsuccessful\n");


	return ret;
}

int pcd_release(struct inode *pcd_inode,struct file *pcd_filp)
{
	pr_info("Release was successful\n");

	return 0;
}

ssize_t pcd_read(struct file *pcd_filp,char __user *pcd_buffer,size_t count,loff_t *f_pos)
{

	struct pcdev_private_data *pcdev_data = (struct pcdev_private_data*)pcd_filp->private_data;

	int max_size = pcdev_data->size;

	pr_info("read requested for %zu bytes \n",count);
	pr_info("Current file position = %lld\n",*f_pos);
	


	/*
	 * Adjust the count*/
	if((*f_pos+count) > max_size)
		count = max_size - *f_pos;
	
	/*copy to user*/
	if(copy_to_user(pcd_buffer,pcdev_data->buffer+(*f_pos),count))
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


}
ssize_t pcd_write(struct file *pcd_filp,const char __user *pcd_buffer,size_t count,loff_t *f_pos)
{

	struct pcdev_private_data *pcdev_data = (struct pcdev_private_data*)(pcd_filp->private_data);

	int max_size = pcdev_data->size;

	pr_info("Write requested for %zu bytes\n",count);
	pr_info("Current file position = %lld\n",*f_pos);

	/*
	 * Adjust the count*/
	if((*f_pos+count)>max_size)
		count = max_size - *f_pos;
	if(!count)
	{
		pr_err("No space left on the device\n");
		return -ENOMEM;
	}

	/*copy from user*/
	if(copy_from_user(pcdev_data->buffer+(*f_pos),pcd_buffer,count))
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
}
loff_t pcd_lseek(struct file *pcd_filp,loff_t offset,int whence)
{
	struct pcdev_private_data *pcdev_data = (struct pcdev_private_data*)(pcd_filp->private_data);
	int max_size = pcdev_data->size;

	loff_t temp;
	pr_info("lseek requested\n");
	pr_info("Current value of the file position = %lld\n",pcd_filp->f_pos);
	switch(whence)
	{
		case SEEK_SET:
			if((offset > max_size) || (offset < 0))
				return -EINVAL;
			pcd_filp->f_pos = offset;
			break;

		case SEEK_CUR:
			temp = pcd_filp->f_pos + offset;
			if((temp > max_size) || (temp < 0))
				return -EINVAL;
			pcd_filp->f_pos = temp;
			break;

		case SEEK_END:
			temp = max_size + offset;
			if((temp > max_size) || (temp < 0))
				return -EINVAL;
			pcd_filp->f_pos = temp;
			break;

		default:
			return -EINVAL;
	}

	pr_info("New value of the file position = %lld\n",pcd_filp->f_pos);
	return pcd_filp->f_pos;
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

	int ret,i;
	/*
	 * 1. Dynamically allocating device numbers*/
	ret = alloc_chrdev_region(&pcdrv_data.device_number,0,NO_OF_DEVICES,"pcd_devices");
	if(ret < 0)
	{
		pr_err("Alloc chrdev failed\n");
		goto out;
	}

	        
        /* Class create
	 * creating device class under /sys/class
	 * This shouldn't be in loop
         */

        pcdrv_data.class_pcd = class_create(THIS_MODULE,"pcd_multiple_class");

        /*class_create returns pointer. So while failing-
         * it will returns pointer to negative value. So the macro-
         * IS_ERR is used
         */
        if(IS_ERR(pcdrv_data.class_pcd))
        {
                pr_err("Class creation failed\n");
                /*
                 * PTR_ERR() converts pointer to error code(int)
                 * ERR_PTR() converts error code(int) to pointer
                 */
                ret = PTR_ERR(pcdrv_data.class_pcd);
                goto unreg_chrdev;
        }

	for(i=0;i<NO_OF_DEVICES;i++)
	{
		pr_info("Device number <major>:<minor> = %d:%d\n",MAJOR(pcdrv_data.device_number+i),MINOR(pcdrv_data.device_number+i));
	

		/* cdev initializaion*/
		cdev_init(&pcdrv_data.pcdev_data[i].cdev,&pcd_fops);

		/*Registering the cdev in VFS*/
		pcdrv_data.pcdev_data[i].cdev.owner=THIS_MODULE;
		ret = cdev_add(&pcdrv_data.pcdev_data[i].cdev,pcdrv_data.device_number+i,1);
		if(ret < 0)
		{
			pr_err("cdev_add failed\n");
			goto cdev_del;
		}

		/*
		 * Populate the sysfs with device information
		 */
		pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd,NULL,pcdrv_data.device_number+i,NULL,"pcdev-%d",i);//formatted string,																           every device has 																	different name, so 																mentioning the deveice																	 name with i
		if(IS_ERR(pcdrv_data.device_pcd))
		{
			pr_err("Device create failed\n");
			ret = PTR_ERR(pcdrv_data.device_pcd);
			goto class_del;
		}
	}

	pr_info("Module init loaded\n");


	return 0;

cdev_del:
class_del:
	for(;i>=0;i--)
	{
		device_destroy(pcdrv_data.class_pcd,pcdrv_data.device_number+i);
		cdev_del(&pcdrv_data.pcdev_data[i].cdev);
	}
	class_destroy(pcdrv_data.class_pcd);



unreg_chrdev:
	unregister_chrdev_region(pcdrv_data.device_number,NO_OF_DEVICES);

out:
	pr_info("Module insertion failed\n");
	return ret;

}


static void __exit pcd_cleanup(void){
	int i;
       for(i=0;i<NO_OF_DEVICES;i++)
        {
                device_destroy(pcdrv_data.class_pcd,pcdrv_data.device_number+i);
                cdev_del(&pcdrv_data.pcdev_data[i].cdev);
        }
        class_destroy(pcdrv_data.class_pcd);

	unregister_chrdev_region(pcdrv_data.device_number,NO_OF_DEVICES);

	pr_info("Module unloaded\n");

}


module_init(pcd_init);
module_exit(pcd_cleanup);


MODULE_LICENSE("GPL");	
MODULE_AUTHOR("Yogesh");
MODULE_DESCRIPTION("A simple pcd with multiple devices");
