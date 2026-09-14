#include "core_agent_sync.h"
#include "core_task.h"
#include "../include/velatime_types.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define AGENT_DATA_DIR "/data/ai_agent"
#define AGENT_SKILLS_DIR AGENT_DATA_DIR "/skills"
#define AGENT_TASKS_FILE AGENT_DATA_DIR "/TASKS.md"
#define STUDENT_TASK_SKILL_FILE \
  AGENT_SKILLS_DIR "/task-manager.md"

static const char g_student_task_skill[] =
  "# Student Task Planner\n"
  "\n"
  "Manage student assignments and tasks for VelaTime.\n"
  "\n"
  "## When to use\n"
  "Use when the user asks to create, view, complete, postpone, "
  "or recommend a task.\n"
  "\n"
  "## Storage\n"
  "Task file: /data/ai_agent/TASKS.md\n"
  "Each pending task must use exactly this format:\n"
  "- [ ] [YYYY-MM-DD] task title\n"
  "The date is the deadline, not the creation date.\n"
  "Completed tasks use - [x].\n"
  "\n"
  "## Create\n"
  "1. In the same tool round, call get_current_time and read_file "
  "for /data/ai_agent/TASKS.md.\n"
  "2. Use current time to resolve relative dates.\n"
  "3. Ask if the title or deadline is missing.\n"
  "4. If TASKS.md is missing, create it with write_file.\n"
  "5. Otherwise write back all old lines plus the new task line.\n"
  "6. Never delete existing tasks.\n"
  "7. Keep the final reply short.\n";

static int create_directory(const char *path)
{
  if (mkdir(path, 0755) == 0 || errno == EEXIST)
    {
      return 0;
    }

  printf("VelaTime: cannot create directory %s, errno=%d\n",
         path, errno);
  return -1;
}

int core_agent_skill_install(void)
{
  FILE *fp;

  if (create_directory(AGENT_DATA_DIR) < 0 ||
      create_directory(AGENT_SKILLS_DIR) < 0)
    {
      return -1;
    }

  fp = fopen(STUDENT_TASK_SKILL_FILE, "w");
  if (fp == NULL)
    {
      printf("VelaTime: cannot install agent skill, errno=%d\n",
             errno);
      return -1;
    }

  if (fputs(g_student_task_skill, fp) == EOF)
    {
      printf("VelaTime: cannot write agent skill, errno=%d\n",
             errno);
      fclose(fp);
      return -1;
    }

  if (fclose(fp) != 0)
    {
      printf("VelaTime: cannot close agent skill, errno=%d\n",
             errno);
      return -1;
    }

  printf("VelaTime: installed agent skill at %s\n",
         STUDENT_TASK_SKILL_FILE);
  return 0;
}

static uint32_t g_applied_hash;
static uint32_t g_candidate_hash;
static int g_applied_hash_valid;
static int g_candidate_hash_valid;

static void trim_title(char *title)
{
  size_t length = strlen(title);

  while (length > 0 &&
         (title[length - 1] == ' ' || title[length - 1] == '\t' ||
          title[length - 1] == '\r' || title[length - 1] == '\n'))
    {
      title[--length] = '\0';
    }
}

static uint32_t hash_bytes(uint32_t hash, const char *data, size_t length)
{
  size_t i;

  for (i = 0; i < length; i++)
    {
      hash ^= (unsigned char)data[i];
      hash *= 16777619u;
    }

  return hash;
}

static int read_agent_tasks(velatime_task_t *tasks, int *task_count,
                            uint32_t *content_hash)
{
  FILE *fp;
  char line[256];
  int count = 0;
  uint32_t hash = 2166136261u;

  fp = fopen(AGENT_TASKS_FILE, "r");
  if (fp == NULL)
    {
      return -1;
    }

  while (fgets(line, sizeof(line), fp) != NULL)
    {
      velatime_task_t task;
      char title[128];
      char date[32];

      hash = hash_bytes(hash, line, strlen(line));
      memset(&task, 0, sizeof(task));

      /* 只导入 "- [ ] [2026-09-07] 高数作业" 格式的待办。 */
      if (sscanf(line, "- [ ] [%15[^]]] %127[^（(]", date, title) != 2)
        {
          continue;
        }

      trim_title(title);
      if (title[0] == '\0')
        {
          continue;
        }

      if (count >= VELATIME_MAX_TASKS)
        {
          printf("VelaTime: too many tasks in %s\n", AGENT_TASKS_FILE);
          fclose(fp);
          return -1;
        }

      strncpy(task.deadline, date, VELATIME_MAX_DEADLINE - 1);
      task.deadline[VELATIME_MAX_DEADLINE - 1] = '\0';

      strncpy(task.title, title, VELATIME_MAX_TITLE - 1);
      task.title[VELATIME_MAX_TITLE - 1] = '\0';

      task.estimated_minutes = 30;
      strncpy(task.priority, "medium", VELATIME_MAX_PRIORITY - 1);
      task.priority[VELATIME_MAX_PRIORITY - 1] = '\0';
      task.status = VELATIME_STATUS_WAITING;
      tasks[count++] = task;
    }

  if (ferror(fp))
    {
      fclose(fp);
      return -1;
    }

  if (fclose(fp) != 0)
    {
      return -1;
    }

  *task_count = count;
  *content_hash = hash;
  return 0;
}

int core_agent_sync_from_file(void)
{
  velatime_task_t tasks[VELATIME_MAX_TASKS];
  uint32_t content_hash;
  int count;

  if (read_agent_tasks(tasks, &count, &content_hash) < 0)
    {
      printf("VelaTime: cannot read %s\n", AGENT_TASKS_FILE);
      return -1;
    }

  if (core_task_replace_all(tasks, count) < 0)
    {
      printf("VelaTime: cannot replace tasks from %s\n",
             AGENT_TASKS_FILE);
      return -1;
    }

  g_applied_hash = content_hash;
  g_applied_hash_valid = 1;
  g_candidate_hash_valid = 0;

  printf("VelaTime: parsed %d task(s) from %s\n",
         count, AGENT_TASKS_FILE);
  return count;
}

int core_agent_sync_if_changed(void)
{
  velatime_task_t tasks[VELATIME_MAX_TASKS];
  uint32_t content_hash;
  int count;

  if (read_agent_tasks(tasks, &count, &content_hash) < 0)
    {
      g_candidate_hash_valid = 0;
      return -1;
    }

  if (g_applied_hash_valid && content_hash == g_applied_hash)
    {
      g_candidate_hash_valid = 0;
      return 0;
    }

  if (!g_candidate_hash_valid || content_hash != g_candidate_hash)
    {
      g_candidate_hash = content_hash;
      g_candidate_hash_valid = 1;
      return 0;
    }

  if (core_task_replace_all(tasks, count) < 0)
    {
      g_candidate_hash_valid = 0;
      return -1;
    }

  g_applied_hash = content_hash;
  g_applied_hash_valid = 1;
  g_candidate_hash_valid = 0;

  printf("VelaTime: synchronized %d task(s) from %s\n",
         count, AGENT_TASKS_FILE);
  return 1;
}
