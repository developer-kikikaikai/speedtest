#include <elf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "hhs_malloc.h"

/*! @name magic table hash API */
/* @{ */
#define HASH_LENGTH (64)
#define MAX_CHECK_LENGTH (32)
#define MAX_CHECK_LENGTH_SLIDE (5)
static int magic_table[HASH_LENGTH];

#define MAGIC_HASH 0x03F566ED27179461UL
static void create_magic_table(void) {
	int i=0;
	uint64_t hash = MAGIC_HASH;
	for( i = 0; i < HASH_LENGTH; i ++ ) {
		magic_table[ hash >> 58 ] = i;
		hash <<= 1;
	}
}

static int get_non_zero_index(uint64_t data) {
	uint64_t rightbit = ( uint64_t ) ( data & ((-1)*data) );
	uint64_t index = (rightbit * MAGIC_HASH ) >> 58;
	return magic_table[index];
}
/* @} */

//max size: 64*64

/*! @struct hhs_maloc_data_t*
 *  definition of malloc manage data, to search data fast, add buffer_unused list and unfull list for check buffer_unused (because buffer_unused will over 
/* @{ */
typedef struct hhs_maloc_data_t {
	size_t max_size;
	size_t max_cnt;
	size_t slide_bit;/*!< keep slide bit size related to max_size, to search buffer fast*/
	uint8_t * buffer_list;/*!< list of buffer for user*/
	uint64_t * buffer_unused;/*!< unused list of buffer */
	uint64_t buffer_unused_unfull;/*!< to fast search, add full flag of all buffer_unused */
	pthread_mutex_t lock;/*!< mutex */
} hhs_maloc_data_t;

static hhs_maloc_data_t maloc_data_g={.buffer_unused_unfull=0,.lock=PTHREAD_MUTEX_INITIALIZER};

static inline void * hhs_get_unused_buffer(uint64_t place);
static inline uint64_t hhs_get_buffer_place(uint8_t * buffer_list, void * ptr);
static inline void hhs_set_used_flag(uint64_t place, uint64_t * pt);
static inline void hhs_set_unused_flag(uint64_t place, uint64_t * pt);

#define BUF_UNFULL(i) maloc_data_g.buffer_unused_unfull[(i)]
#define BUF_UNUSED(i) maloc_data_g.buffer_unused[(i)]

/*define for pthread_cleanup_push*/
static inline void pthread_mutex_unlock_(void *arg) {
	pthread_mutex_unlock((pthread_mutex_t *)arg);
}
#define HHS_LOCK \
        pthread_mutex_lock(&maloc_data_g.lock);\
        pthread_cleanup_push(pthread_mutex_unlock_, &maloc_data_g.lock);
#define HHS_UNLOCK pthread_cleanup_pop(1);

/*! @name private API for hhs_maloc_data_t*/
/* @{ */
static inline void * hhs_get_unused_buffer(uint64_t place) {
	uint64_t offset = place << maloc_data_g.slide_bit;
	return (void *)maloc_data_g.buffer_list + offset;
}

static inline uint64_t hhs_get_buffer_place(uint8_t * buffer_list, void * ptr) {
	uint64_t place = (uint64_t)ptr - (uint64_t)buffer_list;
	//this place is XXX00000 bit place, slide by using maloc_data_g.slide_bit
	place = place >> maloc_data_g.slide_bit;
	return place;
}

static inline void hhs_set_used_flag(uint64_t place, uint64_t * pt) {
	*pt &= ~(0x1<< place);
}

static inline void hhs_set_unused_flag(uint64_t place, uint64_t * pt) {
	*pt |= 0x01<< place;
}
/* @} */

/*! @name public API*/
/* @{ */
int hhs_malloc_init(size_t max_size, size_t max_cnt) {
	//create magic hash table first
	int ret=-1;
	if(MAX_CHECK_LENGTH * MAX_CHECK_LENGTH + 1 < max_cnt) return -1;
HHS_LOCK
	create_magic_table();

	maloc_data_g.max_size = max_size;	
	maloc_data_g.max_cnt = max_cnt;	
	//keep pointer line, head is maloc_data_g.buffer_list, tail is maloc_data_g.buffer_list + (max_cnt * max_size)
	maloc_data_g.buffer_list = malloc(max_cnt * max_size);
	if(!maloc_data_g.buffer_list) goto err;

	uint64_t size = max_cnt / MAX_CHECK_LENGTH;
	uint64_t remainder = max_cnt % MAX_CHECK_LENGTH;
	uint64_t used_size = size + ((remainder==0 )?0:1);

	maloc_data_g.buffer_unused = calloc(1, sizeof(uint64_t)*(used_size));
	if(!maloc_data_g.buffer_unused) {
		free(maloc_data_g.buffer_list);
		goto err;
	}

	//set 1 for used memory
	memset(maloc_data_g.buffer_unused, 0xFF, (max_cnt>>2));

	//set 1 for unused_full_check
	int i=0;
	for(i = 0; i < used_size; i ++ ) maloc_data_g.buffer_unused_unfull |= (0x01) << i;

	//gett slide bit to use hash
	maloc_data_g.slide_bit = get_non_zero_index(max_size);
	ret = 0;
err:
HHS_UNLOCK
	return ret;
}

void hhs_malloc_exit() {
HHS_LOCK
	free(maloc_data_g.buffer_unused);
	free(maloc_data_g.buffer_list);
HHS_UNLOCK
}

void * hhs_calloc(size_t nmemb, size_t size) {
	if(maloc_data_g.max_size < nmemb * size) {
		return calloc(nmemb, size);
	}
	void * ptr=NULL;
HHS_LOCK
	//search none 0
	int i=0;
	// check full
	if(maloc_data_g.buffer_unused_unfull != 0) {

		//get buffer_unused index
		int unused_index = get_non_zero_index(maloc_data_g.buffer_unused_unfull);
		int unused_place = get_non_zero_index(maloc_data_g.buffer_unused[unused_index]);
		ptr = hhs_get_unused_buffer(unused_place + MAX_CHECK_LENGTH * unused_index);

		hhs_set_used_flag(unused_place, &maloc_data_g.buffer_unused[unused_index]);
		if(maloc_data_g.buffer_unused[unused_index] == 0) {
			hhs_set_used_flag(unused_index, &maloc_data_g.buffer_unused_unfull);
		}
		memset(ptr, 0, nmemb*size);
	} else {
		ptr = calloc(nmemb, size);
	}
HHS_UNLOCK
	return ptr;
}

void hhs_free(void * ptr) {
	if(!ptr) return;

	if((uint64_t)ptr < (uint64_t)maloc_data_g.buffer_list || (uint64_t)(maloc_data_g.buffer_list) + maloc_data_g.max_cnt * maloc_data_g.max_size < (uint64_t)ptr ) {
		//no info
		free(ptr);
		return;
	}
HHS_LOCK
	uint64_t place = hhs_get_buffer_place(maloc_data_g.buffer_list, ptr);
	uint64_t unused_place = place >> MAX_CHECK_LENGTH_SLIDE;
	uint64_t unused = maloc_data_g.buffer_unused[unused_place];
	hhs_set_unused_flag(place, &maloc_data_g.buffer_unused[unused_place]);//change to unused
	if(unused == 0) {
		hhs_set_unused_flag(unused_place, &maloc_data_g.buffer_unused_unfull);
	}
HHS_UNLOCK
}
/* @} */
