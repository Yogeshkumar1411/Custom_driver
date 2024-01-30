/*
 * Name: Yogesh
 * Date: 23/1/24
 * Code: i2c driver for rtc
 */

#include<linux/module.h>
#include<linux/init.h>
#include<linux/slab.h>
#include<linux/i2c.h>
#include<linux/delay.h>
#include<linux/kernel.h>
#include<linux/rtc.h>



#define I2C_AVAILABLE 1
#define SLAVE_DEVICE_NAME "RTC"
#define RTC_SLAVE_ADDR 0xd0


static struct i2c_adapter *rtc_i2c_adapter = NULL;

static struct i2c_client *rtc_i2c_client = NULL;


static struct class *rtc_i2c_class;
static struct device *rtc_device;
static dev_t rtc_dev_num;


static int rtc_read(struct device *dev, struct rtc_time *tm)
{
    return 0;
}

static int rtc_set(struct device *dev, struct rtc_time *tm)
{
    return 0;
}

static const struct rtc_class_ops rtc_ops = {
    .read_time = rtc_read,
    .set_time = rtc_set,
};


static int rtc_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
	pr_info("Rtc probed\n");
	return 0;
}

static int rtc_remove(struct i2c_client *client)
{
	pr_info("rtc removed\n");
	return 0;
}


static const struct i2c_device_id rtc_id[] = {
	{SLAVE_DEVICE_NAME,0},
	{}
};

MODULE_DEVICE_TABLE(i2c,rtc_id);


static struct i2c_driver rtc_driver = {
	.driver = {
		.name = SLAVE_DEVICE_NAME,
		.owner = THIS_MODULE,
	},
	.probe = rtc_probe,
	.remove = rtc_remove,
	.id_table = rtc_id,
};



static struct i2c_board_info rtc_i2c_board_info = {
	I2C_BOARD_INFO(SLAVE_DEVICE_NAME,RTC_SLAVE_ADDR)
};

static int __init rtc_driver_init(void)
{
	int ret;
	rtc_i2c_adapter = i2c_get_adapter(0);

	if(rtc_i2c_adapter!=NULL)
	{
	pr_info("adapter pass\n");
		rtc_i2c_client = i2c_new_client_device(rtc_i2c_adapter,&rtc_i2c_board_info);

		if(rtc_i2c_client!=NULL)
		{
			i2c_add_driver(&rtc_driver);
		}
		i2c_put_adapter(rtc_i2c_adapter);
	}


	pr_info("RTC driver with I2C support registered\n");
	


	ret = alloc_chrdev_region(&rtc_dev_num, 0, 1, "rtc_device");
	if (ret < 0) {
		pr_err("Failed to allocate device number: %d\n", ret);
		goto err_unregister_driver;
	}




        rtc_i2c_class = class_create(THIS_MODULE, "rtc_class");
        if (IS_ERR(rtc_i2c_class)) {
                pr_err("Failed to create RTC class\n");
                ret = PTR_ERR(rtc_i2c_class);
                goto err_unregister_dev_num;
        }
	

	// Create a device
	rtc_device = device_create(rtc_i2c_class, NULL, rtc_dev_num, NULL, "rtc_device");
	if (IS_ERR(rtc_device)) {
		pr_err("Failed to create RTC device\n");
	        ret = PTR_ERR(rtc_device);
        	goto err_destroy_class;
	}

	pr_info("RTC device created\n");

	return 0;

err_destroy_class:
	class_destroy(rtc_i2c_class);


err_unregister_dev_num:
        unregister_chrdev_region(rtc_dev_num, 1);


err_unregister_driver:
	i2c_del_driver(&rtc_driver);

	return ret;
}



static void __exit rtc_driver_exit(void)
{
	device_destroy(rtc_i2c_class, rtc_dev_num);
	class_destroy(rtc_i2c_class);
	unregister_chrdev_region(rtc_dev_num, 1);
	i2c_del_driver(&rtc_driver);
	i2c_unregister_device(rtc_i2c_client);

	pr_info("Driver removed\n");

}

module_init(rtc_driver_init);
module_exit(rtc_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yogesh");
MODULE_DESCRIPTION("Simple RTC Driver");


