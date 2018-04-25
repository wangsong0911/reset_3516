#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>

unsigned long heart = 0;

int main(void)
{
	int buttons_fd;
    char key_val;
    char key_num;

    buttons_fd = open("/dev/reset_key", O_RDWR);
    if (buttons_fd < 0) {
		printf("Can not open reset device\n");
		return -1;
    }
    printf(" Please press buttons on Hi3516 board\n");
	while(heart<6000)
	{
		read(buttons_fd, &heart, 4);
		printf("heart=%d\n", heart);
		if(heart>6000){
			printf("long\n");
			system("cp /home/pack/3516config /home/");
			system("cp /home/pack/ipconfig /home/");
			system("cp /home/pack/serverconfig /home/");
			sleep(1);
			system("reboot");
		}
		else{
			printf("short\n");
		}
	}
	close(buttons_fd);
    return 0;
}
