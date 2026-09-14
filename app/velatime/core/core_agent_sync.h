#ifndef VELATIME_CORE_AGENT_SYNC_H
#define VELATIME_CORE_AGENT_SYNC_H

/* 将 VelaTime 的学生任务规划 skill 安装到 AI Agent 数据目录。
   成功或文件已存在返回 0，失败返回 -1。 */
int core_agent_skill_install(void);

/* 从 Agent 的任务文件读取任务并替换 core_task 中的全部任务。
   成功返回导入的任务数量，失败返回 -1。 */
int core_agent_sync_from_file(void);

/* 检测 Agent 任务文件是否稳定变化。
   应用新内容返回 1；未变化或仍在等待稳定返回 0；失败返回 -1。 */
int core_agent_sync_if_changed(void);

#endif /* VELATIME_CORE_AGENT_SYNC_H */
