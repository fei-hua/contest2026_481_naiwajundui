#ifndef VELATIME_CORE_AGENT_SYNC_H
#define VELATIME_CORE_AGENT_SYNC_H

#include <stddef.h>

/* 将 VelaTime 的学生任务规划 skill 安装到 AI Agent 数据目录。
   成功或文件已存在返回 0，失败返回 -1。 */
int core_agent_skill_install(void);

/* 从 Agent 的任务文件读取任务并替换 core_task 中的全部任务。
   成功返回导入的任务数量，失败返回 -1。 */
int core_agent_sync_from_file(void);

/* 检测 Agent 任务文件是否稳定变化。
   应用新内容返回 1；未变化或仍在等待稳定返回 0；失败返回 -1。 */
int core_agent_sync_if_changed(void);

/* 主动提醒：写入 HEARTBEAT.md，让 Agent 的 heartbeat 定时器在空闲时
   主动检查任务并给出建议。pending 为当前待办数量。成功返回 0。 */
int core_agent_reminder_publish(int pending);

/* 主动提醒：读取 Agent 写回的 REMINDER.txt。
   出现新的、可展示的提醒返回 1，out 里是该提醒文本；
   没有新提醒返回 0；参数错误返回 -1。 */
int core_agent_reminder_check(int pending, char *out, size_t out_size);

/* 主动提醒：端侧本地生成提醒文本（不依赖网络/模型）。
   入参为推荐结果的三个关键字段，避免与 core_recommend 产生头文件依赖。
   输出格式与 Agent 写回的一致：任务 | 剩余时间 | 建议开始时间。 */
int core_agent_reminder_local(const char *title, const char *reason,
                              const char *suggested_start,
                              char *out, size_t out_size);

/* 把当前内存里的任务写回 TASKS.md（保持与 Agent 相同的行格式），
   使应用侧的状态变更（开始/延后）在重启后仍然存在。
   写入任务数返回 >=0，失败返回 -1。 */
int core_agent_sync_save(void);

#endif /* VELATIME_CORE_AGENT_SYNC_H */
