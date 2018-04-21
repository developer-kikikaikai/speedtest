#ifndef TIMETESTLOG_
#define TIMETESTLOG_
/**
 * @brief This is API to store log with timestamp, to measure speed spec.
**/

/**
 * @brief Init store log
 *
 * @param[in] delimiter delimiter string if you want, default:" "
 * @param[in] maxloglen  Max length of logs
 * @param[in] maxstoresize  Storaged log size
 * @retval !NULL handle pointer to use other method
 * @retval NULL error
 */
void * timetestlog_init(char *delimiter, size_t maxloglen, unsigned long maxstoresize);

/**
 * @brief Store log
 *
 * @param[in] handle handle returned from timetestlog_init
 * @param[in] format log format as printf
 * @retval 0<=val success
 * @retval other failed (same as prinf)
 */
int timetestlog_store_printf(void * handle, const char *format, ...); 

/**
 * @brief Exit stored log, and show stored log
 *
 * @param[in] handle handle returned from timetestlog_init
 * @return none
 */
void timetestlog_exit(void * handle);
#endif
