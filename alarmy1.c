#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/unistd.h>
#include <linux/timer.h>
#include <linux/time.h>

//왼쪽 모터
#define PIN1 6
#define PIN2 13
#define PIN3 19
#define PIN4 26

//오른쪽 모터
#define PIN11 12
#define PIN22 16
#define PIN33 20
#define PIN44 21

//스피커
#define SPEAKER 18

//GLED, RLED
#define GLED 17
#define RLED 27

//초음파
#define TRIGP 23
#define ECHOP 24

//버튼
#define BUTTON 22

//스텝 수
#define STEPS 8

#include "alarmy1.h"
MODULE_LICENSE("GPL");

int steps[STEPS][4] = {
	{1, 0, 0, 0},
	{1, 1, 0, 0},
	{0, 1, 0, 0},
	{0, 1, 1, 0},
	{0, 0, 1, 0},
	{0, 0, 1, 1},
	{0, 0, 0, 1},
	{1, 0, 0, 1},
};

int notes[] = {1911, 1702, 1516, 1431, 1275, 1136, 1012}; //도 레 미 파 솔 라 시

static int irq_num_us; //초음파센서 irq_num
static int irq_num_b; //버튼 irq_num
long duration;
long distance;
int state;

struct timeval startTime, endTime;
unsigned long st, et;

struct task_struct *test_task1 = NULL; //스레드1
struct task_struct *test_task2 = NULL; //스레드2
struct task_struct *test_task3 = NULL; //스레드3

int isstop1 = 0;
int isstop2 = 0;
int isstop3 = 0;

static struct workqueue_struct *my_wq = NULL; //워크큐
struct work_struct my_work; //워크

int bflag = 0;

unsigned long flags;

void setStep(int num, int p1, int p2, int p3, int p4) {
	if(num == 1) { //왼쪽모터
		gpio_set_value(PIN1, p1);
		gpio_set_value(PIN2, p2);
		gpio_set_value(PIN3, p3);
		gpio_set_value(PIN4, p4);
	}
	else if(num == 2) { //오른쪽모터
		gpio_set_value(PIN11, p1);
		gpio_set_value(PIN22, p2);
		gpio_set_value(PIN33, p3);
		gpio_set_value(PIN44, p4);
	}
}

void go_straight(void) { //왼쪽바퀴는 반시계방향, 오른쪽바퀴는 시계방향
	int i, j;
	for(i=0; i<STEPS; i++) {
		j = 7 - i;
		setStep(1, steps[j][0], steps[j][1], steps[j][2], steps[j][3]);
		setStep(2, steps[i][0], steps[i][1], steps[i][2], steps[i][3]);
		usleep_range(960, 961);
	}
}

void turn_left(void) { //두바퀴 다 시계방향으로 270도 회전
	int value, i, j;
	value = (300*8*64) / 360;

	printk("turn_left\n");

	for(i=0; i<value; i++) {
		for(j=0; j<STEPS; j++) {
			setStep(1, steps[j][0], steps[j][1], steps[j][2], steps[j][3]);
			setStep(2, steps[j][0], steps[j][1], steps[j][2], steps[j][3]);
			usleep_range(960, 961);
		}
	}
}

void turn_right(void) {//두바퀴 다 반시계방향으로 270도 회전
	int value, i, j;
	value = (300*8*64) / 360;

	printk("turn_right\n");

	for(i=0; i<value; i++) {
		for(j=STEPS-1; j>=0; j--) {
			setStep(1, steps[j][0], steps[j][1], steps[j][2], steps[j][3]);
			setStep(2, steps[j][0], steps[j][1], steps[j][2], steps[j][3]);
			usleep_range(960, 961);
		}
	}
}

void play(int note, int a) {
	int i = 0;
	for(i=0; i<20*a; i++) { //i는 음의 지속 길이와 연관있음
		gpio_set_value(SPEAKER, 1);
		usleep_range(note, note+1);
		gpio_set_value(SPEAKER, 0);
		usleep_range(note, note+1);
	}
}

int thread_func1(void *data) { //모터 쓰레드
	printk("Motor kthread is activated\n");

	while(!kthread_should_stop()) {
		go_straight();
	}

	return 0;
}

int thread_func2(void *data) { //알람 쓰레드
	int i = 5;
	int j;

	printk("Speaker kthread is activated\n");

	while(!kthread_should_stop()) { //반짝반짝 작은 별
		j = (5-i) * 20;
		
		if(kthread_should_stop()) break;
		play(notes[0], i); //도
		msleep(200 - j);
		
		if(kthread_should_stop()) break;
		play(notes[0], i); //도
		msleep(200 - j);
		
		if(kthread_should_stop()) break;
		play(notes[4], i); //솔
		msleep(200 - j);
		
		if(kthread_should_stop()) break;
		play(notes[4], i); //솔
		msleep(200 - j);
		
		if(kthread_should_stop()) break;
		play(notes[5], i); //라
		msleep(200 - j);
		
		if(kthread_should_stop()) break;
		play(notes[5], i); //라
		msleep(200 - j);
		
		if(kthread_should_stop()) break;
		play(notes[4], i*2); //솔~
		msleep(200 - j);
		
		if(kthread_should_stop()) break;
		play(notes[3], i); //파
		msleep(200 - j);
		
		if(kthread_should_stop()) break;
		play(notes[3], i); //파
		msleep(200 - j);
		
		if(kthread_should_stop()) break;
		play(notes[2], i); //미
		msleep(200 - j);
		
		if(kthread_should_stop()) break;
		play(notes[2], i); //미
		msleep(200 - j);
		
		if(kthread_should_stop()) break;
		play(notes[1], i); //레
		msleep(200 - j);
		
		if(kthread_should_stop()) break;
		play(notes[1], i); //레
		msleep(200 - j);
		
		if(kthread_should_stop()) break;
		play(notes[0], i*2); //도~
		msleep(200 - j);
		
		if(i != 1)
			i -= 2; //점점 빨라짐
	}

	return 0;
}

int thread_func3(void *data) { //초음파 스레드
	int tmp = 0;
	int value, i, j;
	value = (300*8*64) / 360;

	printk("Ultrasonic kthread is activated\n");
	enable_irq(irq_num_us);

	while(!kthread_should_stop()) {		
		do {
			tmp = 0;
			duration = 0;
	
			if(kthread_should_stop()) return 0;
			gpio_set_value(TRIGP, 1);
			usleep_range(10, 11);
			gpio_set_value(TRIGP, 0);
			
			if(kthread_should_stop()) return 0;
			msleep(10);

			st = (unsigned long)startTime.tv_sec*1000000 + (unsigned long)startTime.tv_usec;
			et = (unsigned long)endTime.tv_sec*1000000 + (unsigned long)endTime.tv_usec;
		} while(st==0 || et==0);

		duration = et - st;
		//printk("duration: %ld ms\n", duration);
		distance = (34000 * duration) / 2000000;
		printk("distance: %ld cm\n", distance);

		if(kthread_should_stop()) break;
		if(distance <= 15) { //거리가 8센치 이내이면
			if(test_task1) { //모터 스레드 멈추고
				kthread_stop(test_task1);
				printk("test kernel thread1 STOP\n");
				isstop1 = 1;
			}
			//turn_right(); //오른쪽으로 회전

			printk("turn_right\n");
			for(i=0; i<value; i++) {
				for(j=STEPS-1; j>=0; j--) {
					if(kthread_should_stop()) return 0;
					setStep(1, steps[j][0], steps[j][1], steps[j][2], steps[j][3]);
					setStep(2, steps[j][0], steps[j][1], steps[j][2], steps[j][3]);
					usleep_range(960, 961);
				}
			}
			
			if(kthread_should_stop()) break;
			test_task1 = kthread_create(thread_func1, NULL, "my_thread1");
			wake_up_process(test_task1); //회전 후 다시 직진
			isstop1 = 0;
		}

		if(kthread_should_stop()) break;
		msleep(1000);
	}

	//disable_irq(irq_num_us);
	return 0;
}

static void my_wq_func(struct work_struct *work) { //버튼 클릭 후 할 일들
	printk("my_wq_func\n");
	disable_irq(irq_num_b);
	disable_irq(irq_num_us);

	//모터, 스피커, 초음파 스레드 다 멈추기
	if(test_task1 && (isstop1 == 0)) {
		kthread_stop(test_task1);
		printk("test kernel thread1 STOP\n");
	}
	if(test_task2 && (isstop2 == 0)) {
		kthread_stop(test_task2);
		printk("test kernel thread2 STOP\n");
	}
	if(test_task3 && (isstop3 == 0)) {
		kthread_stop(test_task3);
		printk("test kernel thread3 STOP\n");
	}

	bflag = 1;
}


static int alarmy1_open(struct inode *inode, struct file *file) {
	printk("alarmy1 open\n");
	return 0;
}

static int alarmy1_release(struct inode *inode, struct file *file) {
	printk("alarmy1 close\n");
	return 0;
}

static long alarmy1_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	int ret = 0;
	int *value = (int *)arg;

	printk("alarmy1 ioctl\n");

	switch(cmd) {
	case ALARMY1_ACTIVATE:
		wake_up_process(test_task1);
		wake_up_process(test_task2);
		wake_up_process(test_task3);
		break;
	case ALARMY1_LED:
		if((*value) == 1) {
			gpio_set_value(GLED, 1);
			gpio_set_value(RLED, 0);
		}
		else if((*value) == 2) {
			gpio_set_value(GLED, 0);
			gpio_set_value(RLED, 1);
		}
		break;
	case ALARMY1_FLAG:
		if(bflag == 0) ret = 0;
		else if(bflag == 1) ret = 1;
		break;
	default:
		return -1;
	}

	return ret;
}

static irqreturn_t us_isr(int irq, void* dev_id) { //초음파 센서 인터룹트 핸들러
	if(gpio_get_value(ECHOP) == 1) { //rising
		do_gettimeofday(&startTime);
	}
	else { //falling
		do_gettimeofday(&endTime);
	}

	return IRQ_HANDLED;
}

static irqreturn_t b_isr(int irq, void* dev_id) { //버튼 인터룹트 핸들러
	int ret;

	printk("button interrupt\n");
	
	if(my_wq) { //버튼누르면 할 일들은 워크큐로 처리
		INIT_WORK(&my_work, my_wq_func);
		ret = queue_work(my_wq, &my_work);
	}

	return IRQ_HANDLED;
}

struct file_operations alarmy1_fops = {
	.unlocked_ioctl = alarmy1_ioctl,
	.open = alarmy1_open,
	.release = alarmy1_release,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init alarmy_init(void) {
	int ret;

	printk("Init Module\n");

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &alarmy1_fops);
	cdev_add(cd_cdev, dev_num, 1);

	//왼쪽 모터
	gpio_request_one(PIN1, GPIOF_OUT_INIT_LOW, "p1");
	gpio_request_one(PIN2, GPIOF_OUT_INIT_LOW, "p2");
	gpio_request_one(PIN3, GPIOF_OUT_INIT_LOW, "p3");
	gpio_request_one(PIN4, GPIOF_OUT_INIT_LOW, "p4");

	//오른쪽 모터
	gpio_request_one(PIN11, GPIOF_OUT_INIT_LOW, "p11");
	gpio_request_one(PIN22, GPIOF_OUT_INIT_LOW, "p22");
	gpio_request_one(PIN33, GPIOF_OUT_INIT_LOW, "p33");
	gpio_request_one(PIN44, GPIOF_OUT_INIT_LOW, "p44");

	//스피커
	gpio_request_one(SPEAKER, GPIOF_OUT_INIT_LOW, "speaker");

	//GLED, RLED
	gpio_request_one(GLED, GPIOF_OUT_INIT_LOW, "gled");
	gpio_request_one(RLED, GPIOF_OUT_INIT_LOW, "rled");

	//초음파
	gpio_request_one(TRIGP, GPIOF_OUT_INIT_LOW, "trig_pin");
	gpio_request_one(ECHOP, GPIOF_IN, "echo_pin");

	//버튼
	gpio_request_one(BUTTON, GPIOF_IN, "button");

	//초음파 irq
	irq_num_us = gpio_to_irq(ECHOP);
	ret = request_irq(irq_num_us, us_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "us_irq", NULL);
	if(ret) {
		printk(KERN_ERR "unable: %d\n", ret);
		free_irq(irq_num_us, NULL);
	}
	else {
		disable_irq(irq_num_us);
	}

	//버튼 irq
	irq_num_b = gpio_to_irq(BUTTON);
	ret = request_irq(irq_num_b, b_isr, IRQF_TRIGGER_FALLING, "b_irq", NULL);
	if(ret) {
		printk(KERN_ERR "unable: %d\n", ret);
		free_irq(irq_num_b, NULL);
	}

	//스레드 할당
	test_task1 = kthread_create(thread_func1, NULL, "my_thread1");
	test_task2 = kthread_create(thread_func2, NULL, "my_thread2");
	test_task3 = kthread_create(thread_func3, NULL, "my_thread3");
	if(IS_ERR(test_task1)) {
		test_task1 = NULL;
		printk("test kernel thread1 ERROR\n");
	}
	if(IS_ERR(test_task2)) {
		test_task2 = NULL;
		printk("test kernel thread2 ERROR\n");
	}
	if(IS_ERR(test_task3)) {
		test_task3 = NULL;
		printk("test kernel thread3 ERROR\n");
	}

	//워크큐 생성
	my_wq = create_workqueue("my_workqueue");

	return 0;
}

static void __exit alarmy_exit(void) {
	printk("Exit Module\n");

	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);

	gpio_free(PIN1);
	gpio_free(PIN2);
	gpio_free(PIN3);
	gpio_free(PIN4);

	gpio_free(PIN11);
	gpio_free(PIN22);
	gpio_free(PIN33);
	gpio_free(PIN44);

	gpio_free(GLED);
	gpio_free(RLED);

	gpio_free(SPEAKER);

	gpio_free(TRIGP);
	gpio_free(ECHOP);
	free_irq(irq_num_us, NULL);

	gpio_free(BUTTON);
	free_irq(irq_num_b, NULL);
}

module_init(alarmy_init);
module_exit(alarmy_exit);
