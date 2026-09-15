#include "core_schedule.h"

#include <string.h>
#include <stdio.h>

/* 一天的可安排区间：08:00 到 22:00 */
#define DAY_BEGIN_MIN (8 * 60)
#define DAY_END_MIN   (22 * 60)

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

/* 把 "HH:MM" 转成当天分钟数；非法输入返回 -1 */
static int time_to_minutes(const char *hhmm)
{
  int h, m;

  if (hhmm == NULL || sscanf(hhmm, "%d:%d", &h, &m) != 2)
    {
      return -1;
    }

  if (h < 0 || h > 23 || m < 0 || m > 59)
    {
      return -1;
    }

  return h * 60 + m;
}

/* 分钟数写回 "HH:MM"（每天最多 24 小时，缓冲区至少 6 字节） */
static void minutes_to_time(int minutes, char *out)
{
  if (minutes < 0)
    {
      minutes = 0;
    }
  if (minutes > 23 * 60 + 59)
    {
      minutes = 23 * 60 + 59;
    }

  snprintf(out, 6, "%02d:%02d", minutes / 60, minutes % 60);
}

/* 收集当天课程区间，按起始时间排序（简单选择排序，课程数很少） */
static int collect_busy(int weekday, int *begin, int *end, int max)
{
  int i;
  int n = 0;
  int a, b;

  for (i = 0; i < s_count && n < max; i++)
    {
      if (s_courses[i].weekday != weekday)
        {
          continue;
        }

      a = time_to_minutes(s_courses[i].start);
      b = time_to_minutes(s_courses[i].end);
      if (a < 0 || b < 0 || b <= a)
        {
          continue;
        }

      begin[n] = a;
      end[n] = b;
      n++;
    }

  for (a = 0; a < n; a++)
    {
      for (b = a + 1; b < n; b++)
        {
          if (begin[b] < begin[a])
            {
              int t;

              t = begin[a]; begin[a] = begin[b]; begin[b] = t;
              t = end[a];   end[a]   = end[b];   end[b]   = t;
            }
        }
    }

  return n;
}

int core_schedule_free_slots(int weekday, velatime_free_slot_t *out, int out_max)
{
  int busy_begin[VELATIME_MAX_COURSES];
  int busy_end[VELATIME_MAX_COURSES];
  int busy_count;
  int added = 0;
  int cursor;
  int i;

  if (out == NULL || out_max <= 0)
    {
      return 0;
    }

  busy_count = collect_busy(weekday, busy_begin, busy_end, VELATIME_MAX_COURSES);

  /* 从一天的起点开始，逐段跳过课程，剩下的就是空闲窗口 */
  cursor = DAY_BEGIN_MIN;

  for (i = 0; i < busy_count && added < out_max; i++)
    {
      int gap = busy_begin[i] - cursor;

      /* 小于 30 分钟的缝隙不算"可安排的空闲" */
      if (gap >= 30)
        {
          minutes_to_time(cursor, out[added].start);
          minutes_to_time(busy_begin[i], out[added].end);
          out[added].minutes = gap;
          added++;
        }

      if (busy_end[i] > cursor)
        {
          cursor = busy_end[i];
        }
    }

  /* 最后一门课之后到一天的终点 */
  if (added < out_max && (DAY_END_MIN - cursor) >= 30)
    {
      minutes_to_time(cursor, out[added].start);
      minutes_to_time(DAY_END_MIN, out[added].end);
      out[added].minutes = DAY_END_MIN - cursor;
      added++;
    }

  return added;
}
