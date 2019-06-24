#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "alarmy2.h"

void main(void) {
	int fd, ret;
	char email[50];
	int dust = 0;
	int dht11_data[5] = {0, };

	printf("## ALARMY2 ##\n");
	printf("\nWait for a contact from RPi1...\n");
	fd = alarmy2_open();
	if(fd == -1) {
		printf("open() fail\n");
		return;
	}
	//TCP 서버역할
	//라즈베리파이1이 소켓에 접속하면
	//온습도 센서 작동
	//미세먼지 센서 작동
	//온습도값과 미세먼지값 라즈베리파이1으로 전송
	ret = server(fd, &dust, dht11_data, email);
	if(ret == -1) {
		printf("server() fail\n");
		alarmy2_close(fd);
		return;
	}	

	//이메일 전송
	send_email(email, dust, dht11_data);
	if(ret == -1) {
		printf("send_email() fail\n");
		alarmy2_close(fd);
		return;
	}

	ret = alarmy2_close(fd);
	if(ret == -1) {
		printf("close() fail\n");
		return;
	}
	
}
