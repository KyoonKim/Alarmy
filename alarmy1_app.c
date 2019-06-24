#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "alarmy1.h"

#define BUFF_SIZE 1024

void main(void) {
	int fd, ret;
	int dust;
	char chr_dust[BUFF_SIZE+5];
	char email[50];
	int hour, minute;
	time_t t;
	struct tm *now;

	fd = alarmy1_oopen();
	if(fd == -1) {
		printf("open() fail\n");
		return;
	}

	
	//알람 설정부분
	t = time(NULL);
	now = localtime(&t);
	printf("## For your PERFECT day! ##\n");
	printf("##      A L A R M Y      ##\n");
	printf("##        [%02d:%02d]        ##\n", now->tm_hour, now->tm_min);
	printf("Enter your email address: ");
	scanf("%s", email);
	printf("Set your alarm\n");
	printf("Enter Hour: ");
	hour = inputhour();
	printf("Enter Minutes: ");
	minute = inputmin();
	printf("\n!Alarm setting Complete!\n");
	printf("...Zzz..z...\n");

	while(1) {
		t = time(NULL);
		now = localtime(&t);

		if((hour == now->tm_hour) && (minute == now->tm_min)) {
			printf("\n!!!  ALARM  !!!\n");
			printf("!!! WAKE UP !!!\n");
			printf("ALARMY is activated\n");
			break;
		}
	}
	//알라미 동작
	ret = alarmy1_activate(fd);
	if(ret == -1) {
		printf("alarmy1_activate() fail\n");
		alarmy1_close(fd);
		return;
	}

	ret = 0;
	while(ret != 1) {
		ret = alarmy1_flag(fd);
		usleep(500000);
	}

	//TCP클라이언트
	//라즈베리파이2으로부터 미세먼지농도와 온습도 데이터 수신 후 출력
	ret = client(chr_dust, email);
	
	//미세먼지 농도에 따라 led 키기
	dust = atoi(chr_dust);
	if(dust > 30) alarmy1_led(fd, 2);
	else alarmy1_led(fd,1);

	ret = alarmy1_close(fd);
	if(ret == -1) {
		printf("close() fail\n");
		return;
	}	
	
}
