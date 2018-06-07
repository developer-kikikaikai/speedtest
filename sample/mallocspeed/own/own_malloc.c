#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <pthread.h>
#include "own_malloc.h"

//get
static pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
typedef struct maloc_data_t {
	struct maloc_data_t * next;
	struct maloc_data_t * prev;
	int used;
	void * mem;
} malloc_data_t;

typedef struct malloc_list_t {
	malloc_data_t * head;
	malloc_data_t * tail;
	size_t size;
	size_t cnt;
	pthread_mutex_t mutex;
} malloc_list_t;
static malloc_list_t malloc_list_g = {NULL, NULL, 0, 0, PTHREAD_MUTEX_INITIALIZER};
/* @} */

/*define for pthread_cleanup_push*/
static inline void pthread_mutex_unlock_(void *arg) {
	pthread_mutex_unlock((pthread_mutex_t *)arg);
}
#define OWN_LOCK \
        pthread_mutex_lock(&malloc_list_g.mutex);\
        pthread_cleanup_push(pthread_mutex_unlock_, &malloc_list_g.mutex);
#define OWN_UNLOCK pthread_cleanup_pop(1);

static void dputil_list_push(malloc_list_t * this, malloc_data_t * data) {
        /* add to tail */
        data->prev = this->tail;
        //slide tail
        if(this->tail) {
                this->tail->next = data;
        }
        this->tail = data;

        /* if head is null, set to head */
        if(!this->head) {
                this->head = data;
        }
}

static void dputil_list_pull(malloc_list_t * this, malloc_data_t * data) {
        if(!data) {
                return;
        }

        /* update content */
        if(this->head == data) {
                this->head = data->next;
        } else {
                /* else case, account is not head. So there is a prev. */
                data->prev->next = data->next;
        }

        if(this->tail == data) {
                this->tail = data->prev;
        } else {
                /* else case, account is not tail. So there is a next. */
                data->next->prev = data->prev;
        }
}

static malloc_data_t * dputil_list_pop(malloc_list_t * this) {
        malloc_data_t * data = this->head;
	dputil_list_pull(this, data);
	return data;
}

static void dputil_list_head(malloc_list_t * this, malloc_data_t * data) {

	//there is a prev.
	data->next = this->head;
	data->prev = NULL;
	this->head->prev = data;
	this->head = data;
}

void dp_malloc_init(size_t max_size, size_t max_cnt) {
	malloc_list_g.cnt = max_cnt;
	malloc_list_g.size = max_size;

	malloc_data_t * memory;
	for(int i=0;i<malloc_list_g.cnt;i++) {
		memory = calloc(1, sizeof(*memory));
		memory->mem = malloc(malloc_list_g.size);
		dputil_list_push(&malloc_list_g, memory);
	}
}

void dp_malloc_exit() {
	malloc_data_t * memory = dputil_list_pop(&malloc_list_g);
	while(memory) {
		free(memory->mem);
		free(memory);
		memory = dputil_list_pop(&malloc_list_g);
	}
}

static inline void * dp_get_memory() {
	malloc_data_t * memory=NULL;
	if(!malloc_list_g.head->used) {
		memory = malloc_list_g.head;
		dputil_list_pull(&malloc_list_g, memory);
		memory->prev=NULL;
		memory->next=NULL;
		memory->used=1;
		dputil_list_push(&malloc_list_g, memory);
		return memory->mem;
	} else {
		return NULL;
	}
}

static inline int dp_unuse_memory(void *ptr) {
	malloc_data_t * memory = malloc_list_g.tail;
	malloc_data_t * memory_tmp=NULL;
	while(memory && memory->used) {
		if(ptr == memory->mem) {
			dputil_list_pull(&malloc_list_g, memory);
			memory->used=0;
			memory->prev=NULL;
			memory->next=NULL;
			dputil_list_head(&malloc_list_g,  memory);
			return 0;
		}
		memory = memory->prev;
	}
	return -1;
}

void * dp_calloc(size_t nmemb, size_t size) {
	void * mem=NULL;
	if(malloc_list_g.size < size*nmemb ) {
		return calloc(nmemb, size);
	}

OWN_LOCK
	mem = dp_get_memory();
	if(!mem) {
		mem = calloc(nmemb, size);
	}
OWN_UNLOCK
	return mem;
}

void dp_free(void * ptr) {
OWN_LOCK
	if(dp_unuse_memory(ptr)) {
		free(ptr);
	}
OWN_UNLOCK
}
