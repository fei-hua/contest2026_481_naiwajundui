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
  /* 数据来源：本机课表 （已隐去授课教师姓名），共 27 个每周时段。
     生成时间 2026-09-15，生成脚本 parse_ics_courses.js + gen_schedule_c.js。
     如需更新课表：重新导出 .ics → 解析成 JSON → 重新生成此表。 */
  static const velatime_course_t defaults[] =
  {
    { "C语言程序设计", 1, "08:00", "09:50", "学4307室" },
    { "应用写作", 1, "10:10", "12:00", "学3301室" },
    { "线性代数", 1, "13:30", "15:20", "中4301室" },
    { "中国近现代史纲要", 1, "15:40", "17:30", "中4204室" },
    { "C语言程序设计", 2, "08:00", "09:50", "学4405室" },
    { "C语言程序设计实验", 2, "08:00", "09:50", "中1302室（信工学院基础实验室（6））" },
    { "大学英语A（2）", 2, "10:10", "12:00", "学2403室" },
    { "C语言程序设计", 2, "13:30", "15:20", "学4404室" },
    { "大学生心理健康教育", 2, "15:40", "17:30", "学2201室（大学生心理健康教育专用教室）" },
    { "大学生心理健康教育", 2, "15:40", "17:30", "" },
    { "高等数学（2）", 3, "08:00", "09:50", "学3103室" },
    { "大学体育（2）", 3, "10:10", "12:00", "田径场（6）" },
    { "形势与政策（2）", 3, "10:10", "12:00", "学3304室" },
    { "中国近现代史纲要", 3, "10:10", "12:00", "中4204室" },
    { "动漫影视艺术鉴赏", 3, "18:30", "20:20", "学3203室" },
    { "大学英语A（2）", 4, "08:00", "09:50", "中3402室" },
    { "高等数学（2）", 4, "10:10", "12:00", "中4101室" },
    { "线性代数", 4, "10:10", "12:00", "" },
    { "线性代数", 4, "10:10", "12:00", "中3303室" },
    { "高等数学（2）", 4, "13:30", "15:20", "学4306室" },
    { "应用写作", 4, "15:40", "17:30", "学3107室" },
    { "应用写作", 4, "15:40", "17:30", "" },
    { "C语言程序设计实验", 5, "10:10", "12:00", "中1202-1室（信工学院基础实验室（3）-1）" },
    { "大学体育（2）", 5, "13:30", "15:20", "羽毛球场" },
    { "数据通信", 7, "08:00", "09:50", "学2204室" },
    { "数据通信", 7, "08:00", "09:50", "中1201室（信工学院基础实验室（2））" },
    { "数据通信", 7, "10:10", "12:00", "学2204室" }
  };
  int n = (int)(sizeof(defaults) / sizeof(defaults[0]));
  int i;

  for (i = 0; i < n && i < VELATIME_MAX_COURSES; i++)
    {
      s_courses[i] = defaults[i];
    }
  s_count = (n < VELATIME_MAX_COURSES) ? n : VELATIME_MAX_COURSES;
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

/* 分钟数写回 "HH:MM"（缓冲区至少 6 字节） */
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
