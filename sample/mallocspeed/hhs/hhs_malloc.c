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

static inline int get_non_zero_index(uint64_t data) {
	uint64_t rightbit = ( uint64_t ) ( data & ((-1)*data) );
	uint64_t index = (rightbit * MAGIC_HASH ) >> 58;
	return magic_table[index];
}
/* @} */

/*! @struct hhs_malloc_list_t*
 *  definition of malloc manage data, to search data fast, add buffer_unused list and unfull list for check buffer_unused (because buffer_unused will over 
/* @{ */
typedef struct maloc_data_t {
	struct maloc_data_t * next;
	struct maloc_data_t * prev;
	int used;
	void * mem;
} malloc_data_t;

typedef struct hhs_malloc_list_t {
	malloc_data_t * head;
	malloc_data_t * tail;
	size_t max_size;
	size_t max_cnt;
	size_t slide_bit;/*!< keep slide bit size related to max_size, to search buffer fast*/
	uint8_t * buf;/*!< list of buffer for user + list of malloc_data_t*/
	uint8_t * user_buf;/*!< list of buffer for user */
	pthread_mutex_t lock;/*!< mutex */
} hhs_malloc_list_t;

static hhs_malloc_list_t malloc_list_g={
	.lock=PTHREAD_MUTEX_INITIALIZER
};

static inline void hhs_list_push(hhs_malloc_list_t * this, malloc_data_t * data);
static inline void hhs_list_pull(hhs_malloc_list_t * this, malloc_data_t * data);
static inline void hhs_list_head(hhs_malloc_list_t * this, malloc_data_t * data);
static inline void * hhs_get_memory();
static inline void hhs_unuse_memory(void *ptr);
static inline void hhs_unset_memory(malloc_data_t * memory, int is_used);
static inline uint64_t hhs_get_buffer_place(uint8_t * buffer_list, void * ptr);
static inline int hhs_is_not_ptr_in_buf(void * ptr);

/*define for pthread_cleanup_push*/
static inline void pthread_mutex_unlock_(void *arg) {
	pthread_mutex_unlock((pthread_mutex_t *)arg);
}
#define HHS_LOCK \
	pthread_mutex_lock(&malloc_list_g.lock);\
	pthread_cleanup_push(pthread_mutex_unlock_, &malloc_list_g.lock);
#define HHS_UNLOCK pthread_cleanup_pop(1);

/*! @name private API for hhs_malloc_list_t*/
/* @{ */

static inline void hhs_list_push(hhs_malloc_list_t * this, malloc_data_t * data) {
	/* add to tail */
	data->prev = this->tail;
	//slide tail
	if(this->tail) this->tail->next = data;

	this->tail = data;
	/* if head is null, set to head */
	if(!this->head) this->head = data;
}

static inline void hhs_list_pull(hhs_malloc_list_t * this, malloc_data_t * data) {
	if(!data) return;

	/* update content */
	if(this->head == data) this->head = data->next;
	/* else case, account is not head. So there is a prev. */
	else data->prev->next = data->next;

	if(this->tail == data) this->tail = data->prev;
	/* else case, account is not tail. So there is a next. */
	else data->next->prev = data->prev;
}

static inline void hhs_list_head(hhs_malloc_list_t * this, malloc_data_t * data) {

	//there is a prev.
	data->next = this->head;
	data->prev = NULL;
	this->head->prev = data;
	this->head = data;
}

static inline void * hhs_get_memory() {
	malloc_data_t * memory=NULL;
	if(!malloc_list_g.head->used) {
		memory = malloc_list_g.head;
		hhs_list_pull(&malloc_list_g, memory);
		hhs_unset_memory(memory, 1);
		hhs_list_push(&malloc_list_g, memory);
		return memory->mem;
	} else {
		return NULL;
	}
}

static inline void hhs_unuse_memory(void *ptr) {
	//get plage of memory, and get list from this place
	uint64_t place = hhs_get_buffer_place(malloc_list_g.user_buf, ptr);
	malloc_data_t * memory = (malloc_data_t *)(malloc_list_g.buf + (sizeof(malloc_data_t) * place));

	hhs_list_pull(&malloc_list_g, memory);
	hhs_unset_memory(memory, 0);
	memset(memory->mem, 0, malloc_list_g.max_size);
	hhs_list_head(&malloc_list_g,  memory);
}

static inline void hhs_unset_memory(malloc_data_t * memory, int is_used) {
	memory->used=is_used;
	memory->prev=NULL;
	memory->next=NULL;
}

static inline uint64_t hhs_get_buffer_place(uint8_t * buffer_list, void * ptr) {
	//this place is XXX00000 bit place, slide by using malloc_list_g.slide_bit
	return  ((uint64_t)ptr - (uint64_t)buffer_list) >> malloc_list_g.slide_bit;
}

static inline int hhs_is_not_ptr_in_buf(void * ptr) {
	
	return (uint64_t)ptr < (uint64_t)malloc_list_g.user_buf || (uint64_t)(malloc_list_g.user_buf) + malloc_list_g.max_cnt * malloc_list_g.max_size < (uint64_t)ptr;
}
/* @} */

/*! @name public API*/
/* @{ */
int hhs_malloc_init(size_t max_size, size_t max_cnt) {
	//create magic hash table first
	int ret=-1;
HHS_LOCK
	create_magic_table();

	malloc_list_g.max_size = max_size;	
	malloc_list_g.max_cnt = max_cnt;	
	//keep pointer line, head is list of malloc_data_t 
	malloc_list_g.buf = malloc(max_cnt * (sizeof(malloc_data_t) + max_size));
	if(!malloc_list_g.buf) goto err;

	//set user pointer list
	malloc_list_g.user_buf = malloc_list_g.buf + (max_cnt * sizeof(malloc_data_t));

	//set user pointer list
	malloc_data_t * memory;
	for(int i=0;i<max_cnt;i++) {
		memory = (malloc_data_t *)(malloc_list_g.buf + (sizeof(malloc_data_t) * i));
		memory->mem = malloc_list_g.user_buf + (max_size * i);
		hhs_list_push(&malloc_list_g, memory);
	}

	//get slide bit to use hash
	malloc_list_g.slide_bit = get_non_zero_index(max_size);

	ret = 0;
err:
HHS_UNLOCK
	return ret;
}

void hhs_malloc_exit() {
HHS_LOCK
	free(malloc_list_g.buf);
	memset(&malloc_list_g, 0, sizeof(malloc_list_g)-sizeof(pthread_mutex_t));
HHS_UNLOCK
}

void * hhs_calloc(size_t nmemb, size_t size) {
	if(malloc_list_g.max_size < nmemb * size) return calloc(nmemb, size);

	void * mem=NULL;
HHS_LOCK
	mem = hhs_get_memory();
	if(!mem) mem = calloc(nmemb, size);

HHS_UNLOCK
	return mem;
}

void hhs_free(void * ptr) {
	if(!ptr) return;

	if(hhs_is_not_ptr_in_buf(ptr)) {
		//no info
		free(ptr);
		return;
	}
HHS_LOCK
	hhs_unuse_memory(ptr);
HHS_UNLOCK
}
/* @} */
