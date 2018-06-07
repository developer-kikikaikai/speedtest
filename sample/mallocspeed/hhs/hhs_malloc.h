/**
* Create malloc to Help by Hacker's skill.
**/
#ifndef HHS_MALLOC_H
#define HHS_MALLOC_H
//about size, please keep 2^n, and cnt, please keep 8 byte alignment
int hhs_malloc_init(size_t max_size, size_t max_cnt);
void hhs_malloc_exit();
void * hhs_calloc(size_t nmemb, size_t size);
void hhs_free(void * ptr);
#endif
