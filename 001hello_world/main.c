#include<linux/module.h>


static int __init helloworld_init(void){
	
	printk(KERN_INFO "Hello world\n");
	return 0;
}


static void __exit helloworld_cleanup(void){

	printk(KERN_INFO "Good bye, world\n");

}

module_init(helloworld_init);
module_exit(helloworld_cleanup);


MODULE_LICENSE("GPL");
