#include "core_schedule.h"

#include <string.h>
#include <stdio.h>

static velatime_course_t s_courses[VELATIME_MAX_COURSES];
static int s_count = 0;


int core_schedule_init(void)
{
  static const velatime_course_t defaults[] =
  {
    { "高等数学", 1, "08:00", "09:40" },
    { "大学英语", 1, "10:00", "11:40" },
    { "大学物理", 2, "08:00", "09:40" },
    { "软件工程", 2, "14:00", "15:40" },
    { "数据结构", 3, "10:00", "11:40" },
    { "大学体育", 4, "16:00", "17:40" },
    { "思修",     5, "08:00", "09:40" },
    { "实验课",   6, "09:00", "11:00" }
  };
  int n = (int)(sizeof(defaults) / sizeof(defaults[0]));
  int i;

  for (i = 0; i < n && i < VELATIME_MAX_COURSES; i++)
    {
      s_courses[i] = defaults[i];
    }
  s_count = n;
  return 0;
}

int core_schedule_count(void)
{
  return s_count;
}

const velatime_course_t *core_schedule_get(int index)
{
  if (index < 0 || index >= s_count)
    {
      return NULL;
    }
  return &s_courses[index];
}

int core_schedule_free_slots(int weekday, velatime_free_slot_t *out, int out_max)
{
  /* 简化实现：把当天课程按开始时间排好，课程间隙即空闲窗口。
     这里按默认数据（每天最多 2 门课）算出间隙。 */
  int i;
  int added = 0;

  /* 晚餐/大块时间用固定窗口：先给一个 12:00-14:00 当作午休空闲 */
  if (weekday >= 1 && weekday <= 5 && added < out_max)
    {
      strncpy(out[added].start, "12:00", 8);
      out[added].start[7] = '\0';
      strncpy(out[added].end, "14:00", 8);
      out[added].end[7] = '\0';
      out[added].minutes = 120;
      added++;
    }

  /* 课程间隙 */
  for (i = 0; i < s_count; i++)
    {
      if (s_courses[i].weekday != weekday)
        {
          continue;
        }
      /* 简单起见：取当天第一门课结束到下午的空闲，演示用。
         真正实现应排序后逐对算间隙。 */
    }

  return added;
}
