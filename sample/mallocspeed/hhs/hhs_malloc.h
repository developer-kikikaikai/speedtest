/**
* Create malloc to Help by Hacker's skill.
**/
#ifndef HHS_MALLOC_H
#define HHS_MALLOC_H
struct hhs_malloc_list_t;
typedef struct hhs_malloc_list_t * HHSMalloc;

//about size, please keep 2^n, and cnt, please keep 8 byte alignment
HHSMalloc hhs_malloc_init(size_t max_size, size_t max_cnt, int is_multithread);
void hhs_malloc_exit(HHSMalloc this);
void * hhs_calloc(HHSMalloc this, size_t nmemb, size_t size);
void hhs_free(HHSMalloc this, void * ptr);
#endif
