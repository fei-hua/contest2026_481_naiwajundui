#ifndef VELATIME_CORE_TASK_H
#define VELATIME_CORE_TASK_H

#include "../include/velatime_types.h"

#define VELATIME_MAX_TASKS 64

/* 初始化任务存储（加载文件） */
int core_task_init(void);

/* 保存全部任务到文件 */
int core_task_save(void);

/* 新增任务，成功返回任务 id（字符串），失败返回 NULL */
const char *core_task_add(const velatime_task_t *task);

/* 使用给定任务列表替换当前全部任务 */
int core_task_replace_all(const velatime_task_t *tasks, int count);

/* 按 id 查找任务，找到返回指针，找不到返回 NULL */
velatime_task_t *core_task_find(const char *id);

/* 更新任务状态 */
int core_task_set_status(const char *id, velatime_status_t status);

/* 删除任务（后续任务前移以保持顺序）。成功返回 0，找不到返回 -1。 */
int core_task_delete(const char *id);

/* 更新任务的标题 / 截止时间 / 状态。
   title 为 NULL 或空串表示不改标题；deadline 为 NULL 表示不改截止时间。
   成功返回 0，找不到返回 -1。 */
int core_task_update(const char *id, const char *title, const char *deadline,
                     velatime_status_t status);

/* 获取任务总数 */
int core_task_count(void);

/* 按下标获取任务（用于遍历），越界返回 NULL */
const velatime_task_t *core_task_get(int index);

#endif /* VELATIME_CORE_TASK_H */
