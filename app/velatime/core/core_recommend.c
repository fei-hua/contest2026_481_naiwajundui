#include "core_recommend.h"
#include "core_task.h"
#include "core_schedule.h"

#include <string.h>
#include <stdio.h>

int core_recommend_pick(int weekday, velatime_recomm_book_t *out)
{
  /* 简化逻辑：遍历任务，找第一个状态为 waiting 且优先级高的任务。
     实际可扩展：结合空闲窗口 + 截止时间排序。 */
  int i;
  int total = core_task_count();

  if (!out)
    {
      return 0;
    }

  for (i = 0; i < total; i++)
    {
      const velatime_task_t *t = core_task_get(i);
      if (!t)
        {
          continue;
        }
      if (t->status == VELATIME_STATUS_WAITING)
        {
          strncpy(out->task_id, t->id, VELATIME_MAX_ID - 1);
          out->task_id[VELATIME_MAX_ID - 1] = '\0';

          strncpy(out->task_title, t->title, VELATIME_MAX_TITLE - 1);
          out->task_title[VELATIME_MAX_TITLE - 1] = '\0';

          strncpy(out->deadline, t->deadline, VELATIME_MAX_DEADLINE - 1);
          out->deadline[VELATIME_MAX_DEADLINE - 1] = '\0';

          out->available_minutes = 40;
          strncpy(out->suggested_start, "15:20", 8);
          out->suggested_start[7] = '\0';
          strncpy(out->reason, "当前有可执行任务，建议现在开始", VELATIME_MAX_REASON - 1);
          out->reason[VELATIME_MAX_REASON - 1] = '\0';
          return 1;
        }
    }

  return 0;
}
