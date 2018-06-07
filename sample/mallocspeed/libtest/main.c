#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <elf.h>
#include "hhs_malloc.h"
#include "own_malloc.h"

#define MAXSIZE 1024
#define MAXNUM 1024

int test_hhs_malloc() {
	hhs_malloc_init(MAXSIZE, MAXNUM);

	void *mem_list[MAXNUM];
	int i=0;
	for(i=0;i<MAXNUM;i++) {
		mem_list[i] = hhs_calloc(1, MAXSIZE);
	}

	uint64_t size=0;
	//memory check
	for(i=0;i<MAXNUM-1;i++) {
		size=(uint64_t)mem_list[i+1] - (uint64_t) mem_list[i];
		//printf("size[%d] %d\n", i,  size);
		if(size != MAXSIZE) {
			printf( "###(%d)failed\n", __LINE__);
			return -1;
		}
	}

	void * local = hhs_calloc(1, MAXSIZE);
	for(i=0;i<MAXNUM;i++) {
		//printf("0x%x\n", mem_list[i]);
		if(local == mem_list[i]) {
			printf( "###(%d)failed\n", __LINE__);
			return -1;
		}
	}
//	printf("0x%x\n", local);

	hhs_free(local);

	//no free tmp data
	for(i=0;i<MAXNUM;i++) {
		memset(mem_list[i], 0, MAXSIZE);
	}

	//free data
	hhs_free(mem_list[10]);
	hhs_free(mem_list[50]);

	local = hhs_calloc(1, MAXSIZE+1);
	for(i=0;i<MAXNUM;i++) {
		if(local == mem_list[i]) {
			printf( "###(%d)failed\n", __LINE__);
			return -1;
		}
	}

	void * local2 = hhs_calloc(2, MAXSIZE/2 + 1);
	for(i=0;i<MAXNUM;i++) {
		if(local == mem_list[i]) {
			printf( "###(%d)failed\n", __LINE__);
			return -1;
		}
	}

	hhs_free(local2);
	hhs_free(local);

	local = hhs_calloc(1, MAXSIZE);
	if(local != mem_list[10] && local != mem_list[50]) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}

	local2 = hhs_calloc(1, MAXSIZE);
	if((local2 != mem_list[10] && local2 != mem_list[50]) || local == local2) {
		printf( "###(%d)failed\n", __LINE__);
		return -1;
	}

	for(i=0;i<MAXNUM;i++) {
		hhs_free(mem_list[i]);
	}

	//exit
	hhs_malloc_exit();
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
	return 0;
}
