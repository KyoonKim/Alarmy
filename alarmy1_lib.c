#include "alarmy1.h"
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

int alarmy1_oopen() {
	int fd, ret;
	fd = open("/dev/alarmy1_dev", O_RDWR);
	if(fd < 0) return -1;

	return fd;
}

int alarmy1_close(int fd) {
	int ret;

	ret = close(fd);
	if(ret < 0) return -1;

	return 0;
}

int alarmy1_activate(int fd) {
	int ret;
	int value = 0;

	ret = ioctl(fd, ALARMY1_ACTIVATE, &value);
	if(ret < 0) return -1;

	return 0;
}

int alarmy1_led(int fd, int num) {
	int ret;
	int value = num;

	ret = ioctl(fd, ALARMY1_LED, &value);
	if(ret < 0) return -1;

	return 0;
}

int alarmy1_flag(int fd) {
	int value = 0;

	return ioctl(fd, ALARMY1_FLAG, &value);
}

int client(char *dust, char *email) {
	int cli_sock;

	struct sockaddr_in serv_addr;

	char hum[BUFF_SIZE+5];
	char temp[BUFF_SIZE+5];

	cli_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(cli_sock == -1) {
		perror("socket() error!\n");
		exit(0);
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("192.168.43.237");
	serv_addr.sin_port = htons(4000);

	if(connect(cli_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
		perror("connect() error\n");
		exit(0);
	}
	printf("\n");

	//습도 수신
	write(cli_sock, "hum", 4);
	//printf("write hum\n");
	read(cli_sock, hum, BUFF_SIZE);
	printf("Humiditiy: %s%%\n", hum);

	//온도 수신
	write(cli_sock, "temp", 5);
	//printf("write temp\n");
	read(cli_sock, temp, BUFF_SIZE);
	printf("Temperature: %sC\n", temp);
	
	//미세먼지 농도 수신
	write(cli_sock,"dust", 5);
	//printf("write dust\n");
	read(cli_sock, dust, BUFF_SIZE);
	printf("Dust: %s\n", dust);

	//메일주소 전송
	write(cli_sock, email, strlen(email)+1);

	close(cli_sock);

	return 0;
}

int isnum(char *num) {
	int i;

	if ((num[0] != '+') && (num[0] != '-') && !isdigit(num[0]))
		return 1;
	else {
		for(i=1; i<strlen(num); i++) {
			if(!isdigit(num[i]))
				return 1;
		}
	}

	return 0;
}

int inputhour() {
	char num[30];
	int ret;

	while(1) {
		scanf("%s", num);

		if(isnum(num)) { //입력이 숫자가 아닌 경우
			printf("!Enter integer. Try again!\n");
			printf("Enter Hour: ");
		}
		else { //숫자인 경우
			ret = atoi(num);
			if((ret < 0) || (ret > 23)) { //0~23이 아닌 경우
				printf("!Enter hour between 0 and 23!\n");
				printf("Enter Hour: ");
			}
			else
				break;
		}
	}

	return ret;
}

int inputmin() {
	char num[30];
	int ret;

	while(1) {
		scanf("%s", num);

		if(isnum(num)) { //입력이 숫자가 아닌 경우
			printf("!Enter integer. Try again!\n");
			printf("Enter Minutes: ");
		}
		else { //숫자인 경우
			ret = atoi(num);
			if((ret < 0) || (ret > 59)) { //0~59가 아닌경우
				printf("!Enter minute between 0 and 59!\n");
				printf("Enter Minutes: ");
			}
			else
				break;
		}
	}

	return ret;
}
