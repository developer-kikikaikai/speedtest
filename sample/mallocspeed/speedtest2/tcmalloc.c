#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "timetestlog.h"
#ifdef USE_TCMALLOC
#include <gperftools/tcmalloc.h>
#endif
#define MAXSIZE 4096
#define MAXNUM 2560
#define CON_REQUEST 256
#define SAME_TIMING_CLIENT 10

#define FREE_TIMIN (8)

struct connection_t {
	void *con_buf[CON_REQUEST];
};

int main(int argc, char *argv[]) {
#ifdef USE_TCMALLOC
	if(argc < 2) return 0;
	int connection=atoi(argv[1])/CON_REQUEST;

	int cnt=0;
	int close_con_table[]={3, 0, 5, 1,6, 2, 4, 9, 8, 7};//適当な切断順

	void *handle = timetestlog_init(",", 100, 100);
	if(!handle) return 0;

	struct connection_t * client = calloc(SAME_TIMING_CLIENT, MAXSIZE);
	timetestlog_store_printf(handle, "CALL:tc_calloc(calloc/free under initial size)\n");
	while(cnt < connection) {
		for(int i=0;i<SAME_TIMING_CLIENT;i++) {
			//connect client
			for(int j=0;j<CON_REQUEST;j++) {
				client[i].con_buf[j] = tc_calloc(1, MAXSIZE);
			}
		}

		for(int i=0;i<SAME_TIMING_CLIENT;i++) {
			//connect client
			for(int j=0;j<CON_REQUEST;j++) {
				tc_free(client[close_con_table[i]].con_buf[j]);
			}
		}
		cnt+=SAME_TIMING_CLIENT;
	}
	timetestlog_store_printf(handle, "end\n");
	free(client);
	//exit
	timetestlog_exit(handle);
#endif
	return 0;
}
