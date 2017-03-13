/*
* hello.c -- the example of printf "hello world!" in the screen of driver program
*/
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");/* declare the license of the module ,it is necessary */

static int hello_init(void)
{
 printk(KERN_ALERT "Hello World enter!\n");
 printk("Hello World enter!\n");
 return 0;
}

static int hello_exit(void)
{
 printk(KERN_ALERT "Hello world exit!\n");
 printk("Hello World exit!\n");
 return 0;
}

module_init(hello_init); /* load the module */
module_exit(hello_exit); /* unload the module */