#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <asm/io.h>

int main(void)
{
	int i, max;

    if(iopl(3)) exit(1);

	outb(0x02,0x50A);
	outb(0x01,0x511);

	max = 100;
	for(i=0; i < max; i++) {
		printf("outb 30 : %d\n", i);
		outb(0x30,0x512);
		printf("sleep %d\n", i);
		sleep(4);
		printf("outb 00 : %d\n", i);
		outb(0x00,0x512);
		sleep(4);
	}
	outb(0x00,0x50A);
	outb(0x00,0x511);
    exit(0);
}




