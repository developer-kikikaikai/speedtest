#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <elf.h>
#include <pthread.h>
#include "hhs_malloc.h"
#include "own_malloc.h"
#include "timetestlog.h"

#define MAXSIZE 1024
#define MAXNUM 1024

typedef struct hhs_result_t {
	HHSMalloc this;
	int is_multithread;
	int maxsize;
	int result;
} hhs_result_t;

void *handle;
void * test_hhs_malloc_thread(void *arg) {
	timetestlog_store_printf(handle, "starrt %d\n", (int)pthread_self());
	hhs_result_t * data = (hhs_result_t *)arg;

	void *mem_list[MAXNUM];
	int i=0;
	for(i=0;i<MAXNUM;i++) {
		mem_list[i] = hhs_calloc(data->this, 1, MAXSIZE);
	}

	uint64_t size=0;
	if(!data->is_multithread) {
	//memory check
	for(i=0;i<MAXNUM-1;i++) {
		size=(uint64_t)mem_list[i+1] - (uint64_t) mem_list[i];
		//printf("size[%d] %d\n", i,  size);
		if(size != MAXSIZE) {
			printf( "###(%d)failed\n", __LINE__);
			goto end;
		}
	}
	}

	void * local = hhs_calloc(data->this, 1, MAXSIZE);
	for(i=0;i<MAXNUM;i++) {
		//printf("0x%x\n", mem_list[i]);
		if(local == mem_list[i]) {
			printf( "###(%d)failed\n", __LINE__);
			goto end;
		}
	}
//	printf("0x%x\n", local);

	hhs_free(data->this, local);

	//no free tmp data
	for(i=0;i<MAXNUM;i++) {
		memset(mem_list[i], 0, MAXSIZE);
	}

	//free data
	hhs_free(data->this, mem_list[10]);
	hhs_free(data->this, mem_list[50]);

	local = hhs_calloc(data->this, 1, data->maxsize+1);
	for(i=0;i<MAXNUM;i++) {
		if(local == mem_list[i]) {
			printf( "###(%d)failed\n", __LINE__);
			goto end;
		}
	}

	void * local2 = hhs_calloc(data->this, 2, data->maxsize/2 + 1);
	for(i=0;i<MAXNUM;i++) {
		if(local == mem_list[i]) {
			printf( "###(%d)failed\n", __LINE__);
			goto end;
		}
	}

	hhs_free(data->this, local2);
	hhs_free(data->this, local);

	local = hhs_calloc(data->this, 1, MAXSIZE);
	local2 = hhs_calloc(data->this, 1, MAXSIZE);
	if(!data->is_multithread) {
	if(local != mem_list[10] && local != mem_list[50]) {
		printf( "###(%d)failed\n", __LINE__);
		goto end;
	}

	if((local2 != mem_list[10] && local2 != mem_list[50]) || local == local2) {
		printf( "###(%d)failed\n", __LINE__);
		goto end;
	}
	} else {
		mem_list[10]=local;
		mem_list[50]=local2;
	}

	for(i=0;i<MAXNUM;i++) {
		hhs_free(data->this, mem_list[i]);
	}

	//exit
	printf( "(%d)Finish to %s test!!\n", __LINE__, __FUNCTION__);
	timetestlog_store_printf(handle, "end %d\n", (int)pthread_self());
	data->result = 0;
end:
	if(data->is_multithread) {pthread_exit(NULL);}
	return NULL;
}

int test_hhs_malloc() {
	handle = timetestlog_init(",", 100, 100);
	hhs_result_t result={NULL, 0, MAXSIZE, -1};
	result.this = hhs_malloc_init(MAXSIZE, MAXNUM, result.is_multithread);

	test_hhs_malloc_thread(&result);
	hhs_malloc_exit(result.this);
	if(result.result == -1) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}

	hhs_result_t result2;
	result.maxsize = MAXSIZE*2;
	result.is_multithread = 1;
	result.this = hhs_malloc_init(MAXSIZE*2, MAXNUM, result.is_multithread);
	memcpy(&result2, &result, sizeof(result));

	pthread_t tid1, tid2;

	pthread_create(&tid1, NULL, test_hhs_malloc_thread, &result);
	pthread_create(&tid2, NULL, test_hhs_malloc_thread, &result2);
	pthread_join(tid1, NULL);
	pthread_join(tid2,NULL);

	hhs_malloc_exit(result.this);
	if(result.result == -1) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}

	if(result2.result == -1) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}
	printf( "(%d)Finish to %s test!!\n", __LINE__, __FUNCTION__);
	return 0;
}

int test_dp_malloc() {
	dp_malloc_init(MAXSIZE, MAXNUM);

	void *mem_list[MAXNUM];
	int i=0, j=0;
	for(i=0;i<MAXNUM;i++) {
		mem_list[i] = dp_calloc(1, MAXSIZE);
	}

	for(i=0;i<MAXNUM/2;i++) {
		for(j=i+1;j<MAXNUM;j++) {
			if(mem_list[i] == mem_list[j]) {
				printf( "###(%d)failed\n", __LINE__);
				return -1;
			}
		}
	}
	void * local = dp_calloc(1, MAXSIZE);
	for(i=0;i<MAXNUM;i++) {
		if(local == mem_list[i]) {
			printf( "###(%d)failed\n", __LINE__);
			return -1;
		}
	}
//	printf("0x%x\n", local);

	dp_free(local);

	//no free tmp data
	for(i=0;i<MAXNUM;i++) {
		memset(mem_list[i], 0, MAXSIZE);
	}

	//free data
	dp_free(mem_list[10]);
	dp_free(mem_list[50]);

	local = dp_calloc(1, MAXSIZE+1);
	for(i=0;i<MAXNUM;i++) {
		if(local == mem_list[i]) {
			printf( "###(%d)failed\n", __LINE__);
			return -1;
		}
	}

	void * local2 = dp_calloc(2, MAXSIZE/2 + 1);
	for(i=0;i<MAXNUM;i++) {
		if(local == mem_list[i]) {
			printf( "###(%d)failed\n", __LINE__);
			return -1;
		}
	}

//	printf("0x%x\n", local);
//	printf("0x%x\n", local2);
	dp_free(local2);
	dp_free(local);

	local = dp_calloc(1, MAXSIZE);
	if(local != mem_list[10] && local != mem_list[50]) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}

	local2 = dp_calloc(1, MAXSIZE);
	if((local2 != mem_list[10] && local2 != mem_list[50]) || local == local2) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}

	for(i=0;i<MAXNUM;i++) {
		dp_free(mem_list[i]);
	}

	//exit
	dp_malloc_exit();
	printf( "(%d)Finish to %s test!!\n", __LINE__, __FUNCTION__);
	return 0;
}

int main(int argc, char *argv[]) {
	if(test_hhs_malloc()) {
		printf( "###(%d)failed\n", __LINE__);
	}

	if(test_dp_malloc()) {
		printf( "###(%d)failed\n", __LINE__);
	}

	printf( "(%d)Finish to test!!\n", __LINE__);
	timetestlog_exit(handle);
	return 0;
}
