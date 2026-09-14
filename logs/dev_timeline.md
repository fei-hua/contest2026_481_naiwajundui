# VelaTime 开发时间线（AI Coding 日志）

> 记录方式：每完成一个阶段就补一条，包含"做了什么 / 遇到什么 / 怎么验证"。
> 时间跨度：2026-08-31 ~ 2026-09-14

## 阶段 0：选题与报名（8/31 ~ 9/1）

- 目标：从零选一个适合初学者、又符合"AI 硬件产品创新"赛道的项目
- 过程：对比医疗健康等方向（数据来源与专业性风险高），最终定为
  **VelaTime —— 主动式 AI 学生时间管家**（面向高校学生，Agent 规划 + 设备触达）
- 结论：报名表四项（定位 / 开发板 / 功能与硬件能力 / 技术方案）定稿；
  开发板改为 **BES 2800BP**（原选 Gemini-S1 已发放完毕导致首次被拒）

## 阶段 1：环境搭建（9/4 ~ 9/5）

- 做了什么：VMware Ubuntu 22.04 → openvela 全量 `repo sync` → 交叉编译 → 模拟器启动
- 遇到的问题与解决：
  1. `repo sync` 反复中断、只同步了部分仓库 → 补装 `git-lfs` 后完整同步
  2. 链接报 `libgui_wrapper.a: file format not recognized` → 该文件是 Git LFS 指针，
     `git lfs pull` 取回真实 189MB 静态库
  3. 模拟器黑屏 → 排查确认内核/服务正常，根因是**当前 defconfig 没有默认 Launcher**；
     `lvgldemo` 可显示即证明图形栈正常
- 验证：`#### build completed successfully ####` + 模拟器出现小米窗口并能跑 `lvgldemo`

## 阶段 2：AI Agent 打通（9/5）

- 做了什么：配置 MiMo Token Plan、通过代理访问、验证 `ai_agent` 对话
- 遇到的问题与解决：
  1. `401 Invalid API Key` → 三个原因叠加：`tp-` 开头的是 **Token Plan 专属 Key**，
     必须用 `token-plan-cn.xiaomimimo.com/v1`；模拟器请求**没走代理**；
     以及一次命令过长导致 Key 截断
  2. 最终配置：`set_proxy 10.0.2.2 7897`（虚拟机内 Clash）+ `set_llm <token-plan 端点>`
- 验证：串口日志出现 `TLS handshake OK ... via proxy` 与 `[Agent]: OK`

## 阶段 3：VelaTime 应用与核心逻辑（9/5 ~ 9/6）

- 做了什么：
  - LVGL 应用骨架（`nuttx_add_application` + manifest linkfile）
  - `core_task`（任务存储）、`core_schedule`（课程与空闲窗口）、`core_recommend`（评分推荐）
  - 首页推荐卡片、Start/Delay 状态机
- 遇到的问题与解决（编译期全部被 `-Werror` 拦下，逐条修掉）：
  1. `INCDIR` 在本工程不生效 → 所有 include 改**相对路径**
  2. 缺 `<stdio.h>` → `snprintf` 隐式声明
  3. 静态函数先调用后定义 → 调整顺序
  4. `format-truncation` → `VELATIME_MAX_ID` 16 → 24
  5. 新增 `.c` 未加入 `CMakeLists.txt` → SRCS 手工登记
- 验证：模拟器显示推荐卡片，点按钮出现 `Started` / `Postponed`

## 阶段 4：与 Agent 打通（9/6 ~ 9/7）

- 做了什么：
  - 自定义 Skill `student-task-planner`，安装为 `/data/ai_agent/skills/task-manager.md`
  - Agent 自然语言建任务 → 写入 `/data/ai_agent/TASKS.md`
  - VelaTime 读取该文件并导入任务
- 遇到的问题与解决：
  1. `agent_loop.c` 的"单轮单工具捷径"让"先读后写"流程中断 → 修改为
     `read_file`/`edit_file` 必须回到模型（该文件属公共源码，比赛仓不含此补丁，
     故 Skill 要求"同轮并行调用 `get_current_time` + `read_file`"以自保）
  2. 两个相似 Skill 竞争（模型总选内置的）→ 改为**覆盖写入内置 `task-manager.md`**
  3. `/data` 数据"丢失" → 真因是构建脚本每次 `mkfs.fat` 重建
     `cmake_out/.../vela_data.bin`；改用独立数据盘
     `-data /home/yy/openvela-persistent/vela_data.bin` 后彻底解决
- 验证：Agent 回复 `OK: wrote ... TASKS.md`；VelaTime 日志
  `parsed N task(s)` / `imported N task(s)`，界面显示任务标题

## 阶段 5：运行时自动同步与紧迫度（9/10、9/14）

- 做了什么：
  - `core_task_replace_all` 安全批量替换 + 内容哈希"连续两次稳定才应用"
  - LVGL 1 秒定时器驱动同步，界面原地刷新
  - 修复截止日期解析：支持 `YYYY-MM-DD` 与 `YYYY-MM-DD HH:MM`；
    紧迫度按日历天分层（逾期/今天/明天/3天内/一周内/更远），
    逾期超一天自动降级；推荐理由改为可解释文案
- 遇到的问题与解决：
  - **推荐分数曾经全部相同**：Agent 写 `[YYYY-MM-DD]`，旧解析要求带时分 →
    解析失败走兜底分，推荐顺序失真
  - 按"小时差"判断导致 22:00 时把明早 08:30 说成"今天" → 改为按日历天判断
- 验证：宿主机 gcc 编译真实源码跑单元测试，15 项全部通过（见 `verification.md`）

## 阶段 6：中文字库（9/14）

- 问题：中文显示为方块。内置 `lv_font_simsun_16_cjk` 是精简字库（连"业"字都没有），
  `font_multilang_small` 仅约 103 个汉字
- 方案：用 `lv_font_conv` 生成 **GB2312 一级汉字 3755 字 + ASCII + 标点**的
  16px/4bpp 字库（3886 字形，3.10MB），并在根对象设置字体让子控件继承
- 遇到的问题与解决：
  - lv_font_conv 要求 Node ≥ 14，而 Ubuntu 22.04 只有 Node 12 →
    改为在 Windows 侧生成字库，虚拟机只需拷 `.c` 文件
  - NuttX 的 `.config` 手改不生效 → 需要重新配置以重写 `nuttx/config.h`
- 验证：界面中文正常（含"业"字），推荐卡片无溢出

## 阶段 7：主动提醒（9/14）

- 需求：赛道要求"至少 1 个主动 + 执行场景"
- 实现：
  - VelaTime 启动时写 `/data/ai_agent/HEARTBEAT.md`（含待办）并加载到 agent 目录
  - Agent 的 heartbeat 线程（`agent_config.h` 默认 30 分钟）读到有待办，
    **无需用户输入**即发起一次思考：读 `TASKS.md`、选最紧急任务、
    写一行到 `/data/ai_agent/REMINDER.txt`
  - VelaTime 每秒轮询该文件，出现新内容即显示到界面卡片
  - 额外：端侧也能**立即**生成一次提醒（不依赖网络/模型），保证离线可演示
- 验证：
  - 单元测试 13 项全部通过（publish/check/去重/模板过滤/无待办不打扰）
  - 固件符号检查：`heartbeat armed for %d pending task(s)`、
    `VelaTime: proactive reminder: %s`、`REMINDER.txt` 均在固件中

## 阶段 8：交付（9/14）

- README（项目说明、架构、构建运行、避坑、赛道要求对照）
- 本 `logs/` 目录（时间线、会话摘要、提示词、验证记录）
- 代码按功能分组提交并推送 fork，后续合并回专属仓
