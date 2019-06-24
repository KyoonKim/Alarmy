#include "alarmy2.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFF_SIZE 1024


int alarmy2_open(void) {
	int fd, ret;
	fd = open("/dev/alarmy2_dev", O_RDWR);
	if(fd < 0) return -1;

	return fd;
}

int alarmy2_close(int fd) {
	int ret;

	ret = close(fd);
	if(ret < 0) return -1;

	return 0;
}

int alarmy2_activate(int fd, int *dht11_data) { //온습도센서
	int ret;
	unsigned long value = 0;

	printf("Getting the temperature and humidity from outside...\n");
	
	ret = read(fd, dht11_data, 20);
	if(ret != 0) return -1;
	
	printf("Humidity: %d.%d %%, Temperature: %d.%d C\n", dht11_data[0], dht11_data[1], dht11_data[2], dht11_data[3]);

	return 0;
}

int dust_sensor(void) { //먼지센서(와이어링파이)
	int dust;

	printf("Getting the concentration of fine dust from outside...\n");

	printf("Dust: %d\n", dust);
	return 0;
}

int server(int fd, int *dust, int *dht11_data, char *email) {
	int ret;

	int serv_sock;
	int cli_sock;
	int cli_addr_size;

	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;

	char buff_snd[BUFF_SIZE+5];
	char buff_rcv[BUFF_SIZE+5];

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1) {
		perror("socket() error!\n");
		exit(0);
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(4000);

	if(bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
		perror("bind() error\n");
		exit(0);
	}

	if(listen(serv_sock, 5) == -1) {
		perror("listen() error\n");
		exit(0);
	}

	cli_addr_size = sizeof(cli_addr);
	cli_sock = accept(serv_sock, (struct sockaddr *)&cli_addr, &cli_addr_size);

	if(cli_sock == -1) {
		perror("accept() error\n");
		exit(0);
	}

	//미먼, 온습도 작동
	ret = alarmy2_activate(fd, dht11_data);
	if(ret == -1) {
		printf("alarmy2_activate() fail\n");
		close(cli_sock);
		close(serv_sock);
		return -1;
	}
	//*dust = dust_sensor();
	*dust = 10;

	//습도 전송
	read(cli_sock, buff_rcv, BUFF_SIZE);
	printf("sending %s\n", buff_rcv);
	if(strncmp("hum", buff_rcv, strlen(buff_rcv)) == 0) {
		sprintf(buff_snd, "%d.%d", dht11_data[0], dht11_data[1]);
		write(cli_sock, buff_snd, strlen(buff_snd)+1);
	}
	memset(buff_rcv, 0x00, strlen(buff_rcv));

	//온도 전송
	read(cli_sock, buff_rcv, BUFF_SIZE);
	printf("sending %s\n", buff_rcv);
	if(strncmp("temp", buff_rcv, strlen(buff_rcv)) == 0) {
		sprintf(buff_snd, "%d.%d", dht11_data[2], dht11_data[3]);
		write(cli_sock, buff_snd, strlen(buff_snd)+1);
	}
	memset(buff_rcv, 0x00, strlen(buff_rcv));

	//미세먼지 전송
	read(cli_sock, buff_rcv, BUFF_SIZE);
	printf("sending %s\n", buff_rcv);
	if(strncmp("dust", buff_rcv, strlen(buff_rcv)) == 0) {
		sprintf(buff_snd, "%d", *dust);
		write(cli_sock, buff_snd, strlen(buff_snd)+1);
	}

	//메일 주소 수신
	read(cli_sock, email, 50);
	printf("send email to %s\n", email);

	close(cli_sock);
	close(serv_sock);

	return 0;
}

void send_email(char *email, int dust, int *dht11_data) {
	char scmd[50];
	FILE *f;

	//SMTP를 이용하여 미세먼지, 온도, 습도를 메일로 전송
	printf("Start sending an email\n");

	f = popen("/usr/lib/sendmail -t -i", "w");

	fprintf(f, "From:%s\r\n", "kihoon0611@naver.com");
	fprintf(f, "To:%s\r\n", email);
	fprintf(f, "Subject:%s\r\n", "weather info");
	fprintf(f, "Content-type: text/html; charset=euc_kr\r\n\r\n");

	fprintf(f, "Humidity : %d.%d\r\n", dht11_data[0], dht11_data[1]);
	fprintf(f, "Temerature : %d.%d\r\n", dht11_data[2], dht11_data[3]);
	fprintf(f, "Dust : 10\n");

	pclose(f);
}
