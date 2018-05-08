/**
 *    @brief      Implement of libtimetestlog library API, defined in timetestlog.h
**/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <config.h>
#include "timetestlog.h"
#ifdef THREAD_SAFE
#include <pthread.h>
#endif

/*************
 * define debug
*************/
#ifdef DBGFLAG
#include <errno.h>
#define DEBUG_ERRPRINT(...)  DEBUG_ERRPRINT_(__VA_ARGS__, "")
#define DEBUG_ERRPRINT_(fmt, ...)  \
        fprintf(stderr, "%s(%d): "fmt"%s", __FUNCTION__,__LINE__, __VA_ARGS__)
#else
#define DEBUG_ERRPRINT(...) 
#endif

/*************
 * public define
*************/
/*! @name Error definition */
/* @{ */
#define TTLOG_FAILED (-1) /*! error */
#define TTLOG_SUCCESS (0) /*! success */
/* @} */

/*! @brief default delimiter definition */
#define TTLOG_DELIMITER_DEFAULT (char *)" "

/*! @struct timetestlog_buffer_s
 * @brief Buffer structure
*/
struct timetestlog_buffer_s {
	char *buf; /*! buffer */
	size_t size; /*! buffer size */
};

/*! @name Create and free buffer API definition */
/* @{ */
/*! Create. buf is allocated data, this API allocate buffer in timetestlog_buffer_s */
static int timetestlog_buffer_create(size_t len, struct timetestlog_buffer_s *buf);
/*! Free. only free member of timetestlog_buffer_s */
static void timetestlog_buffer_free(struct timetestlog_buffer_s *buf);
/* @} */

/*
 * @brief log class interface definition, like interface class
 *  all input parameter * is related structure.
 *  store : store buffer log into related structure
 *  show : show stored data
 *  free : free member (not free ownself)
*/
#define TTLOG_INTERFACE_CLASS\
	int (*store)(struct timetestlog_buffer_s *buff, void *);\
	void (*show)(void *);\
	void (*free)(void *);

/*  @brief macro to set log class interface method.
 *  Please define method as XXX_ifname
 * 
 */
#define TTLOG_INTERFACE_CLASS_SET_KEY(data, classname, key) \
	(data)->key = classname ## _ ## key;

#define TTLOG_INTERFACE_CLASS_SET(data, classname) \
	TTLOG_INTERFACE_CLASS_SET_KEY(data, classname, store)\
	TTLOG_INTERFACE_CLASS_SET_KEY(data, classname, show)\
	TTLOG_INTERFACE_CLASS_SET_KEY(data, classname, free)

/*! @struct timetestlog_data_s
 * @brief log data with timestamp, defined like class
*/
struct timetestlog_data_s{
	//private member
	struct timespec time;/*! timestamp */
	char *delimiter_p; /*! Delimiter string when show log. If delimiter is "___", log will show "Timestamp___log" */
	struct timetestlog_buffer_s buf;/*! log buffer */
	/*! log interface methods */
	TTLOG_INTERFACE_CLASS
};

/*! @name Public API for timetestlog_data_s, only create API can't into member */
/* @{ */
/*! Create. buf is allocated data, this API allocate buffer in timetestlog_buffer_s */
static int timetestlog_data_create(char *delimiter, size_t len, struct timetestlog_data_s *data);
/* @} */

/*! @name log class interface implement API for timetestlog_data_s */
/* @{ */
static void timetestlog_data_show(void *handle);
static inline int timetestlog_data_store(struct timetestlog_buffer_s *buffer, void* handle);
static void timetestlog_data_free(void *handle);
/* @} */

/*! @struct timetestlog_mng_s
 * @brief storaged log data management structure
*/
struct timetestlog_mng_s{
	//private member
	char *delimiter; /*! delimiter */
	struct timetestlog_buffer_s tmpbuf; /*! tmp buffer for using store log */
	unsigned long current_num; /*! numnber of current stored log */
	unsigned long list_num; /*! max size of log_list */
	struct timetestlog_data_s * log_list; /*! log list */
#ifdef THREAD_SAFE
	pthread_mutex_t lock; 
#endif

	/*! log interface methods */
	TTLOG_INTERFACE_CLASS
};

/*! @name Public API definition for timetestlog_mng_s */
/* @{ */
/*! Create, return allocate data */
static struct timetestlog_mng_s * timetestlog_mng_create(char *delimiter, size_t maxloglen, unsigned long maxstoresize);
/*! Get tmp buffer to remove extra allocate */
static inline struct timetestlog_buffer_s * timetestlog_get_buffer(struct timetestlog_mng_s * mng);
#ifdef THREAD_SAFE
/*! lock handle */
static inline void timetestlog_lock(void *handle);
/*! unlock handle */
static inline void timetestlog_unlock(void *handle);
#define TTLOG_LOCK(handle) \
	timetestlog_lock(handle);\
	pthread_cleanup_push(timetestlog_unlock, handle);
#define TTLOG_UNLOCK pthread_cleanup_pop(1);
#else
#define TTLOG_LOCK(handle)
#define TTLOG_UNLOCK 
#endif
/* @} */

/*! @name log class interface implement API for timetestlog_mng_s */
/* @{ */
static void timetestlog_mng_show(void * handle);
static inline int timetestlog_mng_store(struct timetestlog_buffer_s *buffer, void *handle);
static void timetestlog_mng_free(void * handle);
/* @} */

/*! @name Private API definition for timetestlog_mng_s */
/* @{ */
/*! delete log_list into mng */
static void timetestlog_mng_delete_log_list(struct timetestlog_mng_s * mng);
/* @} */

/*************
 * implement
*************/
static int timetestlog_buffer_create(size_t len, struct timetestlog_buffer_s *buf) {
	buf->buf = (char *)calloc(1, len + 1);//len + \0
	if(!buf->buf) {
		return TTLOG_FAILED;
	}
	buf->size = len;
	return TTLOG_SUCCESS;
}

static void timetestlog_buffer_free(struct timetestlog_buffer_s *buf) {
	free(buf->buf);
	memset(buf, 0, sizeof(struct timetestlog_buffer_s));
}

/**for timetestlog_data_s**/
//for timetestlog_data_s, I don't write fail safe because this is private
static int timetestlog_data_create(char *delimiter, size_t len, struct timetestlog_data_s *data) {
	if(timetestlog_buffer_create(len, &data->buf) == TTLOG_FAILED) {
		return TTLOG_FAILED;
	}

	//keep delimiter
	data->delimiter_p = delimiter;

	//set interface
	TTLOG_INTERFACE_CLASS_SET(data, timetestlog_data);
	return TTLOG_SUCCESS;
}

static inline int timetestlog_data_store(struct timetestlog_buffer_s *buffer, void* handle) {
	struct timetestlog_data_s * data = (struct timetestlog_data_s *)handle;
	//set time
	clock_gettime(CLOCK_REALTIME, &data->time);
	memcpy(data->buf.buf, buffer->buf, data->buf.size);
	return TTLOG_SUCCESS;
}

static void timetestlog_data_show(void *handle) {
	struct timetestlog_data_s * data = (struct timetestlog_data_s *)handle;
	printf("%u.%09lu%s%s", (unsigned int)data->time.tv_sec, data->time.tv_nsec, data->delimiter_p, data->buf.buf);
}

static void timetestlog_data_free(void *handle) {
	struct timetestlog_data_s * data = (struct timetestlog_data_s *)handle;
	timetestlog_buffer_free(&data->buf);
	//don't free ownself
	memset(data, 0, sizeof(struct timetestlog_data_s));
}

/**for timetestlog_mng_s**/
/*publit*/
static struct timetestlog_mng_s * timetestlog_mng_create(char *delimiter, size_t maxloglen, unsigned long maxstoresize) {
	struct timetestlog_mng_s * mng=NULL;
	mng=(struct timetestlog_mng_s *)calloc(1, sizeof(struct timetestlog_mng_s));
	if(!mng) {
		DEBUG_ERRPRINT("calloc mng error:%s\n", strerror(errno));
		return NULL;
	}

	//set interface class
	TTLOG_INTERFACE_CLASS_SET(mng, timetestlog_mng);

	//keep tmp buffer
	if(timetestlog_buffer_create(maxloglen, &mng->tmpbuf) == TTLOG_FAILED) {
		DEBUG_ERRPRINT("calloc tmpbuffer error:%s\n", strerror(errno));
		goto err;
	}

	//copy delimiter data
	char * delimiter_p=delimiter;
	if(!delimiter_p) {
		delimiter_p=TTLOG_DELIMITER_DEFAULT;
	}
	int len = strlen(delimiter_p);
	mng->delimiter=(char *)calloc(1, len + 1);
	if(!mng->delimiter) {
		DEBUG_ERRPRINT("calloc delimiter error:%s\n", strerror(errno));
		goto err;
	}
	memcpy(mng->delimiter, delimiter_p, len);

	//alloc list data
	mng->log_list=(struct timetestlog_data_s *)calloc(maxstoresize, sizeof(struct timetestlog_data_s));
	if(!mng->log_list){
		DEBUG_ERRPRINT("calloc mng data list(%lu) error:%s\n", maxstoresize, strerror(errno));
		goto err;
	}

	//alloc all data
	int ret =0;
	for(mng->list_num = 0; mng->list_num < maxstoresize; mng->list_num++) {
		ret = timetestlog_data_create(mng->delimiter, maxloglen, &mng->log_list[mng->list_num]);
		if(ret < 0) {
			DEBUG_ERRPRINT("calloc mng data[%lu] error:%s\n", mng->list_num, strerror(errno));
			goto err;
		}
	}

#ifdef THREAD_SAFE
	//add mutex
	pthread_mutex_init(&mng->lock, NULL);
#endif
	return mng;

err:
	mng->free(mng);
	free(mng);
	return NULL;
}

static inline int timetestlog_mng_store(struct timetestlog_buffer_s *buffer, void *handle) {
	//can't store more
	struct timetestlog_mng_s * mng=(struct timetestlog_mng_s * )handle;
	if(mng->list_num <= mng->current_num) {
		DEBUG_ERRPRINT("log is max, can't store more log\n");
		return TTLOG_FAILED;
	}

	//store log and count up
	int ret = mng->log_list[mng->current_num].store(buffer, &mng->log_list[mng->current_num]);
	if(ret != TTLOG_FAILED) {
		mng->current_num++;
	}
	return ret;
}

static inline struct timetestlog_buffer_s * timetestlog_get_buffer(struct timetestlog_mng_s * mng) {
	return &mng->tmpbuf;
}

#ifdef THREAD_SAFE
static inline void timetestlog_lock(void *handle) {
	struct timetestlog_mng_s * mng=(struct timetestlog_mng_s *)handle;
	pthread_mutex_lock(&mng->lock);
}
static inline void timetestlog_unlock(void *handle) {
	struct timetestlog_mng_s * mng=(struct timetestlog_mng_s *)handle;
	pthread_mutex_unlock(&mng->lock);
}
#endif

/*interface*/
static void timetestlog_mng_show(void *handle) {
	struct timetestlog_mng_s * mng=(struct timetestlog_mng_s *)handle;
	unsigned long i=0;
	for(i = 0; i < mng->current_num; i ++) {
		//show 1 data
		mng->log_list[i].show(&mng->log_list[i]);
	}
}

static void timetestlog_mng_delete_log_list(struct timetestlog_mng_s * mng) {
	if(!mng->log_list) {
		return;
	}

	//free all data
	unsigned long i=0;
	for(i = 0; i < mng->list_num; i ++) {
		mng->log_list[i].free(&mng->log_list[i]);
	}

	//free list data
	free(mng->log_list);
}

static void timetestlog_mng_free(void *handle) {
	struct timetestlog_mng_s * mng=(struct timetestlog_mng_s *)handle;
	timetestlog_mng_delete_log_list(mng);
	free(mng->delimiter);
	timetestlog_buffer_free(&mng->tmpbuf);
}

/*************
 * public interface API implement
*************/
void * timetestlog_init(char *delimiter, size_t maxloglen, unsigned long maxstoresize) {
	//handle: struct timetestlog_mng_s *
	return (void *) timetestlog_mng_create(delimiter, maxloglen, maxstoresize);
}

int timetestlog_store_printf(void * handle, const char *format, ...) {
	int ret=0;
	//fail safe
	if(!handle || !format) {
		return TTLOG_FAILED;
	}

	struct timetestlog_mng_s * mng= (struct timetestlog_mng_s *)handle;
	if(mng->list_num <= mng->current_num) {
		return TTLOG_FAILED;
	}

TTLOG_LOCK(handle)

	struct timetestlog_buffer_s * buffer=timetestlog_get_buffer(mng);
	va_list arg;
	va_start(arg, format);
	vsnprintf(buffer->buf, buffer->size, format, arg);
	va_end(arg);
	ret = mng->store(buffer, mng);

TTLOG_UNLOCK

	return ret;
}

void timetestlog_exit(void * handle) {
	//fail safe
	if(!handle) {
		return ;
	}

	struct timetestlog_mng_s * mng= (struct timetestlog_mng_s *)handle;

TTLOG_LOCK(handle)
	//show log
	mng->show(mng);

	//free mng
	mng->free(mng);
TTLOG_UNLOCK
//destroy

#ifdef THREAD_SAFE
	pthread_mutex_destroy(&mng->lock);
#endif
	free(mng);
}

