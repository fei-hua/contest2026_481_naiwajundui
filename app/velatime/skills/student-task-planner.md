# Student Task Planner

帮助学生创建、查看、完成和安排课程任务。

## When to use

用户提到以下意图时使用：
- 创建任务、记录作业、添加待办、提醒我
- 查看任务、还有什么作业
- 现在做什么、推荐一个任务
- 完成任务、延后任务

## Storage

任务文件固定为：

`/data/ai_agent/TASKS.md`

每个待办任务必须单独占一行，使用以下格式：

`- [ ] [YYYY-MM-DD] 任务标题`

其中方括号内日期表示截止日期，不是创建日期。

已完成任务使用：

`- [x] [YYYY-MM-DD] 任务标题`

VelaTime 依赖这个格式，不要添加其他字段到任务行。

## Create task workflow

1. 调用 `get_current_time` 获取当前日期和时间。
2. 根据当前日期解析“今天、明天、后天”等相对日期。
3. 如果缺少任务标题或截止日期，先询问用户。
4. 如果 `/data/ai_agent/TASKS.md` 不存在，直接调用 `write_file` 创建。
5. 如果文件已存在，先调用 `read_file`，保留已有内容，再调用 `write_file` 写回已有内容和新任务。
6. 新任务格式必须为：`- [ ] [YYYY-MM-DD] 任务标题`
7. 写入成功后简短回复任务标题和截止日期。

## View task workflow

调用 `read_file` 读取 `/data/ai_agent/TASKS.md`，把 `[ ]` 解释为待办，把 `[x]` 解释为已完成。

## Complete task workflow

1. 读取 `/data/ai_agent/TASKS.md`。
2. 找到目标任务。
3. 调用 `edit_file` 把对应行的 `- [ ]` 改为 `- [x]`。
4. 未找到时不要修改文件。

## Rules

- 默认状态为待办。
- 不猜测缺失的截止日期。
- 不把截止日期称为创建日期。
- 不删除已有任务。
- 回复简短，适合小屏幕。
