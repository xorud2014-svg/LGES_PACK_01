#include <linux/module.h>
#include <rtl.h>
#include <time.h>
#include <pthread.h>
#include <asm/io.h>
#include "../../INC/datastore.h"
#include "mbuff.h"
#include "rtTask.h"

pthread_t thread;

void *rt_task(void *arg) 
{
	long long ht0, ht1;
	struct sched_param p;

	p.sched_priority = 1;
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &p);
	pthread_make_periodic_np(pthread_self(), gethrtime(), 1000000); //1mS
	pthread_setfp_np(pthread_self(), 1);

    while(1) {
		ht0 = gethrtime();
		pthread_wait_np();
		ht1 = gethrtime();
	//	outb(0x03, 0x0543);
    }
	return 0;
}

int init_module(void)
{
	outw(0x0002, 0x543);
	return pthread_create(&thread, NULL, rt_task, 0);
}

void cleanup_module(void)
{
	pthread_delete_np(thread);
}
