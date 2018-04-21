#include<stdio.h>
#include<stdlib.h>
#include "timetestlog.h"
#define DELIMITER "/*this is test word to separate message*/"
#define MAXLOGLEN (100)
#define MAXSTORESIZE (10)
//#define STORESIZE (5)
#define STORESIZE (12)
int main() {
	void *handle = timetestlog_init("__@@@__", MAXLOGLEN, MAXSTORESIZE);
	int  i=0;
	for (i=0;i<STORESIZE;i++) {
		printf("store: testlo[%d]g!!!\n", i);
		timetestlog_store_printf(handle, "testlo[%d]g!!!\n", i);
	}
	printf("time;%d\n", time(NULL));
	sleep(5);
	printf("now time;%d, exit handle to show log\n", time(NULL));
	timetestlog_exit(handle);
	return 0;
}
