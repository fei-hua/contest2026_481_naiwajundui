#include "core_recommend.h"
#include "core_task.h"
#include "core_schedule.h"

#include <string.h>
#include <stdio.h>
#include <time.h>

static int priority_score(const char *priority)
{
  if (strcmp(priority, "high") == 0)
    {
      return 40;
    }
  else if (strcmp(priority, "medium") == 0)
    {
      return 20;
    }
  return 0;
}

static time_t deadline_to_time(const char *s)
{
  struct tm tmv;
  int y, mo, d, h, mi;

  if (sscanf(s, "%d-%d-%d %d:%d", &y, &mo, &d, &h, &mi) != 5)
    {
      return 0;
    }

  memset(&tmv, 0, sizeof(tmv));
  tmv.tm_year = y - 1900;
  tmv.tm_mon  = mo - 1;
  tmv.tm_mday = d;
  tmv.tm_hour = h;
  tmv.tm_min  = mi;
  tmv.tm_isdst = -1;

  return mktime(&tmv);
}

static int urgency_score(const char *deadline)
{
  struct timespec ts;
  time_t now, dl;
  long diff_minutes;

  if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
    {
      return 20;
    }
  now = ts.tv_sec;

  dl = deadline_to_time(deadline);
  if (dl == 0)
    {
      return 20;
    }

  diff_minutes = (long)(dl - now) / 60;

  if (diff_minutes <= 0)
    {
      return 100;
    }
  else if (diff_minutes <= 24 * 60)
    {
      return 80;
    }
  else if (diff_minutes <= 72 * 60)
    {
      return 50;
    }
  return 20;
}

int core_recommend_pick(int weekday, velatime_recomm_book_t *out)
{
  velatime_free_slot_t slots[VELATIME_FREE_SLOT_MAX];
  int slot_count;
  int total;
  int i;
  int best_index = -1;
  int best_score = -1;

  if (!out)
    {
      return 0;
    }

  slot_count = core_schedule_free_slots(weekday, slots, VELATIME_FREE_SLOT_MAX);
  total = core_task_count();

  for (i = 0; i < total; i++)
    {
      const velatime_task_t *t = core_task_get(i);
      int score = 0;
      int match = 0;

      if (!t || t->status != VELATIME_STATUS_WAITING)
        {
          continue;
        }

      if (slot_count > 0 && t->estimated_minutes <= slots[0].minutes)
        {
          match = 50;
        }

      score = urgency_score(t->deadline) + priority_score(t->priority) + match;

      if (score > best_score)
        {
          best_score = score;
          best_index = i;
        }
    }

  if (best_index < 0)
    {
      return 0;
    }

  {
    const velatime_task_t *t = core_task_get(best_index);
    strncpy(out->task_id, t->id, VELATIME_MAX_ID - 1);
    out->task_id[VELATIME_MAX_ID - 1] = '\0';

    strncpy(out->task_title, t->title, VELATIME_MAX_TITLE - 1);
    out->task_title[VELATIME_MAX_TITLE - 1] = '\0';

    strncpy(out->deadline, t->deadline, VELATIME_MAX_DEADLINE - 1);
    out->deadline[VELATIME_MAX_DEADLINE - 1] = '\0';

    if (slot_count > 0)
      {
        strncpy(out->suggested_start, slots[0].start, 8);
        out->available_minutes = slots[0].minutes;
      }
    else
      {
        out->available_minutes = 30;
        strncpy(out->suggested_start, "15:20", 8);
      }
    out->suggested_start[7] = '\0';

    strncpy(out->reason, "综合紧急度、优先级和可用空闲时间推荐", VELATIME_MAX_REASON - 1);
    out->reason[VELATIME_MAX_REASON - 1] = '\0';
  }

  return 1;
}
