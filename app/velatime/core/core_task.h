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

/* 按 id 查找任务，找到返回指针，找不到返回 NULL */
velatime_task_t *core_task_find(const char *id);

/* 更新任务状态 */
int core_task_set_status(const char *id, velatime_status_t status);

/* 获取任务总数 */
int core_task_count(void);

/* 按下标获取任务（用于遍历），越界返回 NULL */
const velatime_task_t *core_task_get(int index);

#endif /* VELATIME_CORE_TASK_H */
