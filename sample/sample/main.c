#include<stdio.h>
#include<stdlib.h>
#ifdef THREAD_SAFE
#include<pthread.h>
#endif

#include "timetestlog.h"
#define DELIMITER "/*this is test word to separate message*/"
#define MAXLOGLEN (100)
#define MAXSTORESIZE (100)
#define STORESIZE (90)

void * showlog_loop(void *handle) {
	int  i=0;
#ifdef THREAD_SAFE
	unsigned int id = (unsigned int)pthread_self();
#else
	unsigned int id = (unsigned int)getpid();
#endif
	for (i=0;i<STORESIZE;i++) {
		timetestlog_store_printf(handle, "store(from id %u): testlo[%d]g!!!\n", id, i);
		usleep(1);
	}
	printf("time;%d\n", (int)time(NULL));
}

int main() {
	void *handle = timetestlog_init("__@@@__", MAXLOGLEN, MAXSTORESIZE);
#ifdef THREAD_SAFE
	pthread_t  tid1, tid2;

	//create thread
	pthread_create(&tid1, NULL, showlog_loop, handle);
	pthread_create(&tid2, NULL, showlog_loop, handle);

	//create join
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
#else
	showlog_loop(handle);
#endif

	sleep(5);
	printf("now time;%d, exit handle to show log\n", (int)time(NULL));
	timetestlog_exit(handle);
	return 0;
}
