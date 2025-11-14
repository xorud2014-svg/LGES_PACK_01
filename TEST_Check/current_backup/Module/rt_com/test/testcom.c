/**
 * rt_com test
 * ===========
 *
 * RT-Linux kernel module for communication across serial lines.
 *
 * Copyright (C) 1999 Michael Barabanov <baraban@fsmlabs.com>
 */

#include <rtl.h>
#include <rtl_fifo.h>
#include <rtl_sched.h>

#include <time.h>
#include <pthread.h>
#include <asm/io.h>
#include "../rt_com.h"

#define COUNT_COM_PORTS 2

pthread_t thread;

void *thread_code(void *arg) {

	pthread_make_periodic_np(thread, gethrtime(), 1000000000); //1s
	//pthread_make_periodic_np(thread, gethrtime(), 100000000); //100ms
	//pthread_make_periodic_np(thread, gethrtime(), 10000000); //10ms
	//pthread_make_periodic_np(thread, gethrtime(), 2000000); //2ms

	do {
		int size, i, j;
		char buf[210];

		pthread_wait_np();

		for(i=0; i < COUNT_COM_PORTS; i++) {
			memset(buf, 0, sizeof(buf));
			size = rt_com_read(i, buf, sizeof(buf));
			if(size > 0) {
				buf[size] = 0;
				//rtl_printf("com%d:%s\n", i, buf);
				rtl_printf("rx com%d:", i);
				for(j=0; j < size; j++) {
					rtl_printf("%02x ", (unsigned char)buf[j]);
				}
				rtl_printf("\n");
			}
		}

		for(i=0; i < COUNT_COM_PORTS; i++) {
			memset(buf, 0, sizeof(buf));
			/*size = 5;
			buf[0] = 0xF0;
			buf[1] = 0x80;
			buf[2] = 0x00;
			buf[3] = 0x80;
			buf[4] = 0xE0;
			rt_com_write(i, buf, 5);*/

			size = 19;
			buf[0] = 0x40;
			buf[1] = 0x36;
			buf[2] = 0x30;
			buf[3] = 0x43;
			buf[4] = 0x4F;
			buf[5] = 0x31;
			buf[6] = 0x30;
			buf[7] = 0x36;
			buf[8] = 0x30;
			buf[9] = 0x30;
			buf[10] = 0x30;
			buf[11] = 0x30;
			buf[12] = 0x30;
			buf[13] = 0x30;
			buf[14] = 0x30;
			buf[15] = 0x37;
			buf[16] = 0x35;
			buf[17] = 0x31;
			buf[18] = 0x0D;
			if(i == 0) {
				rt_com_write(i, buf, size);

			rtl_printf("tx com%d:", i);
			for(j=0; j < size; j++) {
				rtl_printf("%02x ", (unsigned char)buf[j]);
			}
			rtl_printf("\n");
			}
		}
	} while (1);

	return 0;
}

int init_module(void)
{
	int thread_status, i;
	struct sched_param sched_parameters;
	pthread_attr_t attr;

	for(i=0; i < COUNT_COM_PORTS; i++) {
		//rt_com_setup(i, 115200, RT_COM_PARITY_NONE, 1, 8);
		rt_com_setup(i, 9600, RT_COM_PARITY_NONE, 1, 8);
	}

	pthread_attr_init(&attr);
	pthread_attr_setcpu_np(&attr, 0);
	sched_parameters.sched_priority = 1;
	pthread_attr_setschedparam(&attr, &sched_parameters);
	thread_status = pthread_create(&thread, &attr, thread_code, (void *)1);
	if(thread_status < 0) {
		printk("failed to create RT-thread\n");
		return -1;
	} else {
		printk("created RT-thread\n");
	}

	return 0;
}

void cleanup_module(void)
{
	int i;

	printk("Removing module on CPU %d\n", rtl_getcpuid());
	pthread_delete_np(thread);

	for(i=0; i < COUNT_COM_PORTS; i++) {
		rt_com_setup(i, -1, 0, 0, 0);
	}

}

/**
 * Local Variables:
 * mode: C
 * c-file-style: "Stroustrup"
 * End:
 */
