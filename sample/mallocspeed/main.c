#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "timetestlog.h"
#include "own_malloc.h"
#include "hhs_malloc.h"
#ifdef USE_TCMALLOC
#include <gperftools/tcmalloc.h>
#endif
#define MAXSIZE 1024
#define MAXNUM 1024

int main(int argc, char *argv[]) {
	if(argc < 2) return 0;
	int fsize=atoi(argv[1]);

	dp_malloc_init(MAXSIZE, MAXNUM);
	hhs_malloc_init(MAXSIZE, MAXNUM);

	void *handle = timetestlog_init(",", 100, 100);
	if(!handle) return 0;

	void **mem_list=calloc(2*fsize, MAXSIZE*2);
	timetestlog_store_printf(handle, "CALL:normal calloc(calloc/free under initial size)\n");
	for(int i=0;i<fsize;i++) {
		mem_list[i] = calloc(1, MAXSIZE);
	}
	timetestlog_store_printf(handle, "free \n");
	for(int i=0;i<fsize;i++) {
		free(mem_list[i]);
	}
	timetestlog_store_printf(handle, "end\n");

	timetestlog_store_printf(handle, "CALL:wrap calloc code(calloc/free under initial size)\n");
	for(int i=0;i<fsize;i++) {
		mem_list[i] = dp_calloc(1, MAXSIZE);
	}
	timetestlog_store_printf(handle, "free \n");
	for(int i=0;i<fsize;i++) {
		dp_free(mem_list[i]);
	}
	timetestlog_store_printf(handle, "end\n");

	timetestlog_store_printf(handle, "CALL:hhs_calloc(calloc/free under initial size)\n");
	for(int i=0;i<fsize;i++) {
		mem_list[i] = hhs_calloc(1, MAXSIZE);
	}
	timetestlog_store_printf(handle, "free \n");
	for(int i=0;i<fsize;i++) {
		hhs_free(mem_list[i]);
	}
	timetestlog_store_printf(handle, "end\n");

#ifdef USE_TCMALLOC
	timetestlog_store_printf(handle, "CALL:tc_calloc(calloc/free under initial size)\n");
	for(int i=0;i<fsize;i++) {
		mem_list[i] = tc_calloc(1, MAXSIZE);
	}
	timetestlog_store_printf(handle, "free \n");
	for(int i=0;i<fsize;i++) {
		tc_free(mem_list[i]);
	}
	timetestlog_store_printf(handle, "end\n");
#endif

	timetestlog_store_printf(handle, "CALL:normal calloc(calloc/free initial size+over size)\n");
	for(int i=0;i<2*fsize;i++) {
		mem_list[2*i] = calloc(1, MAXSIZE);
		mem_list[2*i + 1] = calloc(1, MAXSIZE*2);
	}
	timetestlog_store_printf(handle, "free \n");
	for(int i=0;i<2*fsize;i++) {
		free(mem_list[2*i]);
		free(mem_list[2*i + 1]);
	}
	timetestlog_store_printf(handle, "end\n");

	timetestlog_store_printf(handle, "CALL:wrap calloc(calloc/free initial size+over size)\n");
	for(int i=0;i<2*fsize;i++) {
		mem_list[2*i] = dp_calloc(1, MAXSIZE);
		mem_list[2*i + 1] = dp_calloc(1, MAXSIZE*2);
	}
	timetestlog_store_printf(handle, "free \n");
	for(int i=0;i<2*fsize;i++) {
		dp_free(mem_list[2*i]);
		dp_free(mem_list[2*i + 1]);
	}
	timetestlog_store_printf(handle, "end\n");

	timetestlog_store_printf(handle, "CALL:hhs_calloc(calloc/free initial size+over size)\n");
	for(int i=0;i<2*fsize;i++) {
		mem_list[2*i] = hhs_calloc(1, MAXSIZE);
		mem_list[2*i + 1] = hhs_calloc(1, MAXSIZE*2);
	}
	timetestlog_store_printf(handle, "free \n");
	for(int i=0;i<2*fsize;i++) {
		hhs_free(mem_list[2*i]);
		hhs_free(mem_list[2*i + 1]);
	}
	timetestlog_store_printf(handle, "end\n");

#ifdef USE_TCMALLOC
	timetestlog_store_printf(handle, "CALL:tc_calloc(calloc/free initial size+over size)\n");
	for(int i=0;i<2*fsize;i++) {
		mem_list[2*i] = tc_calloc(1, MAXSIZE);
		mem_list[2*i + 1] = tc_calloc(1, MAXSIZE*2);
	}
	timetestlog_store_printf(handle, "free \n");
	for(int i=0;i<2*fsize;i++) {
		tc_free(mem_list[2*i]);
		tc_free(mem_list[2*i + 1]);
	}
	timetestlog_store_printf(handle, "end\n");
#endif

	//exit
	hhs_malloc_exit();
	dp_malloc_exit();
	timetestlog_exit(handle);
	return 0;
}
