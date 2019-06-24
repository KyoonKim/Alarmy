#define DEV_NAME "alarmy1_dev"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2
#define IOCTL_NUM3 IOCTL_START_NUM+3

#define ALARMY1_NUM 'z'
#define ALARMY1_ACTIVATE _IOWR(ALARMY1_NUM, IOCTL_NUM1, unsigned long *)
#define ALARMY1_LED _IOWR(ALARMY1_NUM, IOCTL_NUM2, unsigned long *)
#define ALARMY1_FLAG _IOWR(ALARMY1_NUM, IOCTL_NUM3, unsigned long *)

int alarmy1_oopen(void);
int alarmy1_close(int fd);
int alarmy1_activate(int fd);
int alarmy1_led(int fd, int num);
int alarmy1_flag(int fd);
int client(char *dust, char *email);
void print_data(int dust, int temp, int hum);
int isnum(char num[]);
int inputhour(void);
int inputmin(void);
