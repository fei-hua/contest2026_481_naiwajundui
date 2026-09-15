#ifndef VELATIME_CORE_RECOMMEND_H
#define VELATIME_CORE_RECOMMEND_H

#include "../include/velatime_types.h"

#define VELATIME_FREE_SLOT_MAX 8

typedef struct
{
  char task_id[VELATIME_MAX_ID];
  char task_title[VELATIME_MAX_TITLE];
  char deadline[VELATIME_MAX_DEADLINE];
  int available_minutes;
  char suggested_start[8];
  char reason[VELATIME_MAX_REASON];
} velatime_recomm_book_t;

/* 根据当前空闲窗口选出一个推荐任务。返回 1 有推荐，0 没有。 */
int core_recommend_pick(int weekday, velatime_recomm_book_t *out);

/* 当前星期，1=周一 … 7=周日（调用方不要再硬编码 weekday）。 */
int core_recommend_today_weekday(void);

#endif /* VELATIME_CORE_RECOMMEND_H */
