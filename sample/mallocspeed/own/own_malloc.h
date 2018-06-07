#ifndef OWN_MALLOC_H
#define OWN_MALLOC_H
void dp_malloc_init(size_t max_size, size_t max_cnt);
void dp_malloc_exit();
void * dp_calloc(size_t nmemb, size_t size);
void dp_free(void * ptr);
#endif
