#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/platform_device.h>
#include<linux/string.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/init.h>
#include<linux/device.h>
#include<linux/io.h>
#include<linux/of.h>
#include<linux/of_device.h>

#undef pr_fmt
#define pr_fmt(fmt) "%s : "fmt,__func__

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yogesh");
MODULE_DESCRIPTION("A gpio sysfs driver");

/*device private data structure*/
struct gpiodev_private_data
{
	char label[20];
};


/*driver private data structure*/
struct gpiodrv_private_data
{
	int toatl_devices;
	struct class *class_gpio;
};

struct gpiodrv_private_data gpio_drv_data;


int gpio_sysfs_remove(struct platform_device *pdev)
{
	return 0;
}


int gpio_sysfs_probe(struct platform_device *pdev)
{
	return 0;
}

const struct of_device_id gpio_device_match[] =
{
	{.compatible = "org,bone-gpio-sysfs"},
	{}
};

struct platform_driver gpiosysfs_platform_driver = 
{
	.probe = gpio_sysfs_probe,
	.remove = gpio_sysfs_remove,
	.driver = {
		.name = "bone-gpio-sysfs",
		.of_match_table = of_match_ptr(gpio_device_match)
	}


};

static int __init gpio_sysfs_init(void)
{
	gpio_drv_data.class_gpio = class_create(THIS_MODULE,"bone_gpios");
	if(IS_ERR(gpio_drv_data.class_gpio))
	{
		pr_err("Error in creating class\n");
		return PTR_ERR(gpio_drv_data.class_gpio);
	}
	
	platform_driver_register(&gpiosysfs_platform_driver);
	pr_info("Module load success\n");
	return 0;
}


static void __exit gpio_sysfs_exit(void)
{
	platform_driver_unregister(&gpiosysfs_platform_driver);
	class_destroy(gpio_drv_data.class_gpio);
}
