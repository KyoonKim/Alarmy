obj-m := alarmy2.o

KERNELDIR := ~/linux-rpi/
ARM := ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi-
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) $(ARM) modules
#	gcc -c alarmy1_lib.c
	arm-linux-gnueabi-gcc -c alarmy2_lib.c
	ar crv libalarmy2_lib.a alarmy2_lib.o
#	gcc -o alarmy1_app alarmy1_app.c -L. -l alarmy1_lib
	arm-linux-gnueabi-gcc -o alarmy2_app alarmy2_app.c -L. -l alarmy2_lib
copy:
#scp alarmy1.ko alarmy1_app alarmy1_mknod.sh pi@192.168.43.17:~
	scp alarmy2.ko alarmy2_app alarmy2_mknod.sh pi@192.168.43.237:~
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) $(ARM) clean
