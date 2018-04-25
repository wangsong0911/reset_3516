ifneq ($(KERNELRELEASE),)

obj-m := 3516_reset_key.o

else
	
KDIR := /home/wangsong/Downloads/Hi3516A_SDK_V1.0.5.0/osdrv/opensource/kernel/linux-3.4.y
# CFLAGS += -I/usr/local/openssl/include
# CFLAGS += -I/opt/hisi-linux/x86-arm/arm-hisiv400-linux/target/usr/include
# LIBS += -L/opt/hisi-linux/x86-arm/arm-hisiv400-linux/target/usr/include
all:
	make -C $(KDIR) M=$(PWD) modules ARCH=arm CROSS_COMPILE=/opt/hisi-linux/x86-arm/arm-hisiv400-linux/target/bin/arm-hisiv400-linux-
clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers

endif
