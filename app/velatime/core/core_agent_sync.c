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
#define AGENT_HEARTBEAT_FILE AGENT_DATA_DIR "/HEARTBEAT.md"
#define AGENT_REMINDER_FILE AGENT_DATA_DIR "/REMINDER.txt"
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
  "7. Keep the final reply short.\n"
  "\n"
  "## Proactive recommendation\n"
  "VelaTime keeps writing /data/ai_agent/HEARTBEAT.md. When a heartbeat\n"
  "prompt asks you to check on the student, do exactly this:\n"
  "1. Read /data/ai_agent/TASKS.md.\n"
  "2. Pick the single pending task that should be started now:\n"
  "   earliest deadline first; if several share a day, prefer high\n"
  "   priority, then the shorter estimated time.\n"
  "3. Call get_current_time to resolve how much time is left.\n"
  "4. Write exactly one line to /data/ai_agent/REMINDER.txt in this form:\n"
  "   <task title> | <time left> | <what to do now>\n"
  "   Example: 交高数作业 | 明天截止 | 先花 30 分钟做完前两题\n"
  "   Use plain UTF-8 text, no markdown fences, no extra lines.\n"
  "5. Answer with the same sentence you wrote.\n"
  "If TASKS.md has no pending task, write 暂时没有待办任务 to\n"
  "/data/ai_agent/REMINDER.txt instead.\n";

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

/* ------------------------------------------------------------------ */
/* 主动提醒（proactive reminder）                                      */
/*                                                                    */
/* 链路：VelaTime 写 HEARTBEAT.md（含待办，触发 Agent 主动思考）         */
/*      → Agent heartbeat 线程按定时器读 TASKS.md 并选任务              */
/*      → 写 REMINDER.txt                                             */
/*      → VelaTime 轮询并把提醒显示到界面                               */
/* 说明：这不是"用户提问后回答"，触发源是设备侧定时器。                  */
/* ------------------------------------------------------------------ */

#define REMINDER_TEXT_MAX 192

/* 判断提醒文本是否为一个真实的待办标题（而不是模板占位符） */
static int looks_like_real_title(const char *s)
{
  static const char *placeholders[] =
  {
    "task title", "<task", "标题", "title>", "task_title", "示例"
  };
  size_t i;

  for (i = 0; i < sizeof(placeholders) / sizeof(placeholders[0]); i++)
    {
      if (strstr(s, placeholders[i]) != NULL)
        {
          return 0;
        }
    }

  return 1;
}

static int write_heartbeat_file(int pending)
{
  FILE *fp;

  fp = fopen(AGENT_HEARTBEAT_FILE, "w");
  if (fp == NULL)
    {
      printf("VelaTime: cannot write %s, errno=%d\n",
             AGENT_HEARTBEAT_FILE, errno);
      return -1;
    }

  if (pending <= 0)
    {
      fputs("- No pending task right now. Stay idle silently.\n", fp);
    }
  else
    {
      fputs("- The student has pending tasks. Do the proactive "
            "recommendation from the Student Task Planner skill now "
            "and write the result to /data/ai_agent/REMINDER.txt.\n",
            fp);
    }

  if (fclose(fp) != 0)
    {
      return -1;
    }

  return 0;
}

static int read_reminder_file(char *out, size_t out_size)
{
  FILE *fp;
  size_t length;

  if (out == NULL || out_size == 0)
    {
      return -1;
    }

  fp = fopen(AGENT_REMINDER_FILE, "r");
  if (fp == NULL)
    {
      return -1;
    }

  if (fgets(out, (int)out_size, fp) == NULL)
    {
      fclose(fp);
      out[0] = '\0';
      return 0;
    }

  fclose(fp);
  trim_title(out);

  /* 行首的 markdown 符号与引号一律去掉，只留可展示文本 */
  length = strlen(out);
  if (length > 0 && (out[0] == '-' || out[0] == '*' || out[0] == '"'))
    {
      memmove(out, out + 1, length);
      trim_title(out);
    }

  return 0;
}

int core_agent_reminder_publish(int pending)
{
  if (create_directory(AGENT_DATA_DIR) < 0)
    {
      return -1;
    }

  if (write_heartbeat_file(pending) < 0)
    {
      return -1;
    }

  printf("VelaTime: heartbeat armed for %d pending task(s)\n", pending);
  return 0;
}

int core_agent_reminder_check(int pending, char *out, size_t out_size)
{
  static uint32_t s_last_hash;
  static int s_last_valid;
  uint32_t hash;

  if (out == NULL || out_size == 0)
    {
      return -1;
    }

  out[0] = '\0';

  if (read_reminder_file(out, out_size) < 0)
    {
      return 0;                    /* 还没生成提醒：正常情况 */
    }

  if (out[0] == '\0')
    {
      return 0;
    }

  if (pending <= 0)
    {
      return 0;
    }

  if (strncmp(out, "HEARTBEAT_OK", 12) == 0)
    {
      return 0;
    }

  if (!looks_like_real_title(out))
    {
      return 0;                    /* 模型写了模板占位符，丢弃 */
    }

  hash = hash_bytes(2166136261u, out, strlen(out));
  if (s_last_valid && hash == s_last_hash)
    {
      return 0;                    /* 内容没变：不重复提醒 */
    }

  s_last_hash = hash;
  s_last_valid = 1;
  return 1;
}

int core_agent_reminder_local(const char *title, const char *reason,
                              const char *suggested_start,
                              char *out, size_t out_size)
{
  if (title == NULL || out == NULL || out_size == 0)
    {
      return -1;
    }

  /* 端侧自己也能给出主动提醒：不依赖网络与模型，任何时候都能响。
     格式与 Agent 写回的 REMINDER.txt 保持一致，便于两种来源互换。 */
  snprintf(out, out_size, "%s | %s | 建议 %s 开始",
           title,
           (reason != NULL) ? reason : "建议现在开始",
           (suggested_start != NULL) ? suggested_start : "--");

  printf("VelaTime: reminder published from device: %s\n", out);
  return 0;
}

/* ------------------------------------------------------------------ */
/* 任务落盘：把内存里的任务写回 TASKS.md                                */
/* 目的：应用侧的状态变更（开始 / 延后）重启后不丢，且与 Agent 共用同一份 */
/* 文件格式，两边谁改都能被对方读到。                                   */
/* ------------------------------------------------------------------ */

int core_agent_sync_save(void)
{
  FILE *fp;
  int total = core_task_count();
  int i;

  if (create_directory(AGENT_DATA_DIR) < 0)
    {
      return -1;
    }

  fp = fopen(AGENT_TASKS_FILE, "w");
  if (fp == NULL)
    {
      printf("VelaTime: cannot write %s, errno=%d\n", AGENT_TASKS_FILE, errno);
      return -1;
    }

  for (i = 0; i < total; i++)
    {
      const velatime_task_t *t = core_task_get(i);
      const char *mark;

      if (t == NULL)
        {
          continue;
        }

      mark = (t->status == VELATIME_STATUS_DONE) ? "x" : " ";

      if (t->deadline[0] != '\0')
        {
          fprintf(fp, "- [%s] [%s] %s\n", mark, t->deadline, t->title);
        }
      else
        {
          fprintf(fp, "- [%s] %s\n", mark, t->title);
        }
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

  printf("VelaTime: saved %d task(s) to %s\n", total, AGENT_TASKS_FILE);
  return total;
}
