#define DEV_NAME "alarmy2_dev"

int alarmy2_open(void);
int alarmy2_close(int fd);
int alarmy2_activate(int fd, int dht11_data[5]);
int dust_sensor(void);
int server(int fd, int *dust, int *dht11_data, char *email);
void send_email(char *email, int dust, int dht11_data[5]);
