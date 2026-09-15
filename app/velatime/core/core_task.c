#include "core_task.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TASK_FILE "/data/velatime/tasks.txt"

static velatime_task_t s_tasks[VELATIME_MAX_TASKS];
static int s_count = 0;

int core_task_init(void)
{
  /* 简单起见，目前仅重置。后续接入文件读写 */
  s_count = 0;
  return 0;
}

int core_task_save(void)
{
  /* 占位：后续写文件 */
  return 0;
}

const char *core_task_add(const velatime_task_t *task)
{
  if (!task || s_count >= VELATIME_MAX_TASKS)
    {
      return NULL;
    }

  /* 如果外部没给 id，自动生成 */
  if (task->id[0] == '\0')
    {
      snprintf(s_tasks[s_count].id, VELATIME_MAX_ID, "task_%03d", s_count + 1);
    }
  else
    {
      strncpy(s_tasks[s_count].id, task->id, VELATIME_MAX_ID - 1);
      s_tasks[s_count].id[VELATIME_MAX_ID - 1] = '\0';
    }

  strncpy(s_tasks[s_count].title, task->title, VELATIME_MAX_TITLE - 1);
  s_tasks[s_count].title[VELATIME_MAX_TITLE - 1] = '\0';

  strncpy(s_tasks[s_count].course, task->course, VELATIME_MAX_COURSE - 1);
  s_tasks[s_count].course[VELATIME_MAX_COURSE - 1] = '\0';

  strncpy(s_tasks[s_count].deadline, task->deadline, VELATIME_MAX_DEADLINE - 1);
  s_tasks[s_count].deadline[VELATIME_MAX_DEADLINE - 1] = '\0';

  s_tasks[s_count].estimated_minutes = task->estimated_minutes;

  strncpy(s_tasks[s_count].priority, task->priority, VELATIME_MAX_PRIORITY - 1);
  s_tasks[s_count].priority[VELATIME_MAX_PRIORITY - 1] = '\0';

  s_tasks[s_count].status = task->status;
  s_count++;

  return s_tasks[s_count - 1].id;
}

int core_task_replace_all(const velatime_task_t *tasks, int count)
{
  int i;

  if (count < 0 || count > VELATIME_MAX_TASKS ||
      (count > 0 && tasks == NULL))
    {
      return -1;
    }

  s_count = 0;
  for (i = 0; i < count; i++)
    {
      if (core_task_add(&tasks[i]) == NULL)
        {
          s_count = 0;
          return -1;
        }
    }

  return s_count;
}

velatime_task_t *core_task_find(const char *id)
{
  int i;
  if (!id)
    {
      return NULL;
    }
  for (i = 0; i < s_count; i++)
    {
      if (strcmp(s_tasks[i].id, id) == 0)
        {
          return &s_tasks[i];
        }
    }
  return NULL;
}

int core_task_set_status(const char *id, velatime_status_t status)
{
  velatime_task_t *t = core_task_find(id);
  if (!t)
    {
      return -1;
    }
  t->status = status;
  return 0;
}

int core_task_delete(const char *id)
{
  int i;

  if (!id || id[0] == '\0')
    {
      return -1;
    }

  for (i = 0; i < s_count; i++)
    {
      if (strcmp(s_tasks[i].id, id) == 0)
        {
          int j;

          /* 后面的任务整体前移，保持顺序不变 */
          for (j = i; j + 1 < s_count; j++)
            {
              s_tasks[j] = s_tasks[j + 1];
            }
          s_count--;
          memset(&s_tasks[s_count], 0, sizeof(s_tasks[s_count]));
          return 0;
        }
    }

  return -1;
}

int core_task_update(const char *id, const char *title, const char *deadline,
                     velatime_status_t status)
{
  velatime_task_t *t = core_task_find(id);

  if (!t)
    {
      return -1;
    }

  if (title != NULL && title[0] != '\0')
    {
      strncpy(t->title, title, VELATIME_MAX_TITLE - 1);
      t->title[VELATIME_MAX_TITLE - 1] = '\0';
    }

  if (deadline != NULL)
    {
      strncpy(t->deadline, deadline, VELATIME_MAX_DEADLINE - 1);
      t->deadline[VELATIME_MAX_DEADLINE - 1] = '\0';
    }

  t->status = status;
  return 0;
}

int core_task_count(void)
{
  return s_count;
}

const velatime_task_t *core_task_get(int index)
{
  if (index < 0 || index >= s_count)
    {
      return NULL;
    }
  return &s_tasks[index];
}
