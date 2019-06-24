//#include "alarmy2.h"
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/moduleparam.h>

MODULE_LICENSE("GPL");

#define MAX_TIMING 85
#define DHT11 21

#define DEV_NAME "alarmy2_dev"

static int dht11_data[5] = {0, };

static dev_t dev_num;
static struct cdev *cd_cdev;

static int dht11_read(void) {
	int last_state = 1;
	int counter = 0;
	int i = 0, j = 0;

	printk("start reading\n");
	dht11_data[0] = dht11_data[1] = dht11_data[2] = dht11_data[3] = dht11_data[4] = 0;
	
	gpio_direction_output(DHT11, 0);
	mdelay(18);
	gpio_set_value(DHT11, 1);
	udelay(40);
	gpio_direction_input(DHT11);

	for(i=0; i<MAX_TIMING; i++) {
		counter = 0;

		while(gpio_get_value(DHT11) == last_state) {
			counter++;
			udelay(1);
			if(counter == 255) break;
		}
		last_state = gpio_get_value(DHT11);

		if(counter == 255) break;

		if((i>=4) && (i%2==0)) {
			dht11_data[j/8] <<= 1;
			if(counter > 16)
				dht11_data[j/8] |= 1;
			j++;
		}
	}

	if( (j>=40) &&
			(dht11_data[4] == ( (dht11_data[0] + dht11_data[1] + dht11_data[2] + dht11_data[3]) & 0xff) ) ) 
	{
		printk("Humidity: %d.%d Temperature = %d.%d C\n", dht11_data[0], dht11_data[1], dht11_data[2], dht11_data[3]);
		return 1;
	}
	else {
		printk("Data not good, skip\n");
		return 0;
	}
}

static int alarmy2_open(struct inode *inode, struct file *file) {
	printk("alarmy2 open\n");
	return 0;
}

static int alarmy2_release(struct inode *inode, struct file *file) {
	printk("alarmy2 close\n");
	return 0;
}

static ssize_t alarmy2_read(struct file *file, char *buf, size_t len, loff_t *lof) {
	int i=0, ret;

	printk("alarmy2 read\n");

	do{
		i=dht11_read();
		mdelay(100);
	}while(i==0);

	ret = copy_to_user(buf, dht11_data, 20);
	printk("alarmy2_read ok!\n");
	return ret;
}

struct file_operations alarmy2_fops = {
	.read = alarmy2_read,
	.open = alarmy2_open,
	.release = alarmy2_release,
};

static int __init alarmy2_init(void) {
	printk("Init Module\n");

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &alarmy2_fops);
	cdev_add(cd_cdev, dev_num, 1);

	gpio_request(DHT11, "DHT11");

	return 0;
}

static void __exit alarmy2_exit(void) {
	printk("Exit Module\n");

	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
	gpio_set_value(DHT11, 0);
	gpio_free(DHT11);
}

module_init(alarmy2_init);
module_exit(alarmy2_exit);
