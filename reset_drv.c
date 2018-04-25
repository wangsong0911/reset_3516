#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
//# include "memdev.h"  

MODULE_LICENSE("GPL");
//EXPORT_SYMBOL_GPL(device_create);

#define CDEV_NAME "reset_key"

struct cdev cdev; 
static dev_t devno;
static unsigned char key_val;  
static volatile int ev_press = 0;
struct class *cdev_class;
struct device *device;
unsigned long counter_int = 0;

static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

//static volatile unsigned long *GPIO9_4_con = NULL;
static volatile unsigned long *GPIO9_4_dir  = NULL;
static volatile unsigned long *GPIO9_4_dat = NULL;
static volatile unsigned long *GPIO9_4_IS = NULL;
static volatile unsigned long *GPIO9_4_IEV = NULL;
static volatile unsigned long *GPIO9_4_IC = NULL;
static volatile unsigned long *GPIO9_4_IE = NULL;

static irqreturn_t key_int(int irq, void *dev_id)
{
	*GPIO9_4_IC = 0xFF;//GPIO9_4_IC |= 0x10;
    key_val = 0xff;
	printk("count_int=%d!\n", counter_int);
    counter_int++;
    /* 唤醒休眠的进程 */
    wake_up_interruptible(&button_waitq);
    return 0;	
}

static ssize_t reset_cdev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    wait_event_interruptible(button_waitq, counter_int);
    
    copy_to_user(buf, &counter_int, 4);
    //ev_press = 0;
	counter_int = 0;
	
    return 1;
}

static int reset_cdev_close(struct inode *inode, struct file *file)
{
    //free_irq(87, 0);
    return 0;
}

int reset_cdev_open( struct inode * inode, struct file * filp)
{
    printk("open file success\n");
    //*GPIO9_4_con = 0x00;
    *GPIO9_4_IS |= 0x10;
    *GPIO9_4_IEV = 0x00;//GPIO9_4_IEV &= 0xef;
    *GPIO9_4_IC = 0xFF;//GPIO9_4_IC |= 0x10;
    *GPIO9_4_IE = 0x10;//GPIO9_4_IE |= 0x10;
    request_irq(87, key_int, IRQF_TRIGGER_HIGH, "hi3516_key", 0);
    
    return 0;
}

static const struct file_operations cdev_fops =  
{  
    .owner = THIS_MODULE,  
    .read = reset_cdev_read,  
    //.write = cdev_write,  
    .open  = reset_cdev_open,
    .release = reset_cdev_close,
};

static int reset_cdev_init( void )  
{  
    int err;  
    printk("Initializing reset cdev!\n");

    /*动态申请设备号*/
    err = alloc_chrdev_region(&devno, 0, 1, CDEV_NAME);
    if(err<0)  
    {
        printk("alloc_fail!\n");
        return err;
    }
    printk("major=%d, minor=%d\n", MAJOR(devno), MINOR(devno)); 
    
    /*初始化cdev*/
    cdev_init(&cdev, &cdev_fops);
    err = cdev_add(&cdev, devno, 1);//register the char_dev into the system  
    if(err)
    {
        printk("add_fail!\n");
        return err;
    }
    /* 注册字符设备 */  
    cdev_class = class_create(THIS_MODULE, CDEV_NAME);
    if(IS_ERR(cdev_class))
    {
        printk("cdev create failed!\n");
        err = PTR_ERR(cdev_class);
    }
    device = device_create(cdev_class, NULL, devno, 0, CDEV_NAME);
    if(IS_ERR(device))
    {
        printk("device create failed!\n");
        err = PTR_ERR(device);
    // goto fail_device_create;
    }
    
    printk("Succedded to initialize reset cdev.\n");
    
    //GPIO9_4_con = (volatile unsigned long *)ioremap(0x200f0080, 32);
    GPIO9_4_dir = (volatile unsigned long *)ioremap(0x201d0400, 8);
    GPIO9_4_dat = (volatile unsigned long *)ioremap(0x201d0040, 8);
    GPIO9_4_IS = (volatile unsigned long *)ioremap(0x201d0404, 8);
    GPIO9_4_IEV = (volatile unsigned long *)ioremap(0x201d040C, 8);
    GPIO9_4_IC = (volatile unsigned long *)ioremap(0x201d041C, 8);
    GPIO9_4_IE = (volatile unsigned long *)ioremap(0x201d0410, 8);
    
    return 0;
}  
   
/*模块卸载函数*/  
static void reset_cdev_exit( void )  
{  
    //ndev = MKDEV(freg_major, freg_minor);  
    printk("Destroy reset cdev.\n");

	iounmap(GPIO9_4_dir);
    iounmap(GPIO9_4_dat);
    iounmap(GPIO9_4_IS);
    iounmap(GPIO9_4_IEV);
    iounmap(GPIO9_4_IC);
    iounmap(GPIO9_4_IE);
    
    free_irq(87, (void *)0);
	
    cdev_del(&cdev); //unregister the char_dev from the system  
    device_destroy(cdev_class, devno);
    class_destroy(cdev_class);
    unregister_chrdev_region(devno, 1);//free the device node number 
}  
   
module_init(reset_cdev_init);  
module_exit(reset_cdev_exit); 

