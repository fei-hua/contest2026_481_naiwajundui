#ifndef VELATIME_CORE_SCHEDULE_H
#define VELATIME_CORE_SCHEDULE_H

#include "../include/velatime_types.h"

#define VELATIME_MAX_COURSES 64

/* 当天空闲窗口 */
typedef struct
{
  char start[8];  /* "15:20" */
  char end[8];    /* "16:30" */
  int minutes;    /* 空闲分钟数 */
} velatime_free_slot_t;

/* 初始化课程表 */
int core_schedule_init(void);

/* 获取课程总数 */
int core_schedule_count(void);

/* 按下标获取课程 */
const velatime_course_t *core_schedule_get(int index);

/* 给定星期(1-7)，计算当天空闲窗口，返回窗口数量 */
int core_schedule_free_slots(int weekday, velatime_free_slot_t *out, int out_max);

#endif /* VELATIME_CORE_SCHEDULE_H */
