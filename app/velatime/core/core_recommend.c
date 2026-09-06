#include "core_recommend.h"
#include "core_task.h"
#include "core_schedule.h"

#include <string.h>
#include <stdio.h>

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
      int urgency = 20;
      int match = 0;

      if (!t || t->status != VELATIME_STATUS_WAITING)
        {
          continue;
        }

      /* 紧急分：这里用简化固定值，后续接入真实时钟再细化 */
      urgency = 100;

      /* 时长匹配：任务时长 <= 第一个空闲窗口则加分 */
      if (slot_count > 0 && t->estimated_minutes <= slots[0].minutes)
        {
          match = 50;
        }

      score = urgency + priority_score(t->priority) + match;

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
