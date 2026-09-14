# VelaTime - 主动式 AI 学生时间管家

> 2026 首届 openvela AI 硬件开发者大赛 · AI 硬件产品创新赛道
> 参赛编号 **481** · 队伍 **naiwajundui**

## 一、项目定位

VelaTime 不是待办清单，也不是问答机器人，而是一个**会主动找你**的时间管家：

- 它知道你什么时候有课、什么时候空着；
- 它知道你有哪些作业、各自什么时候截止；
- 在合适的时间点，它**主动**站出来说"现在这 120 分钟，先写高数作业"。

一句话：**Agent 负责理解与规划，openvela 设备负责触达与执行。**

面向高校学生，解决三个真实痛点：任务容易忘、安排容易乱、碎片时间利用率低。

## 二、硬件与系统

| 项 | 内容 |
|---|---|
| 目标开发板 | 恒玄科技 **BES 2800BP**（已适配 openvela，面向智能手表/手环） |
| 系统 | openvela（NuttX 内核） + LVGL 9.1 + `ai_agent` 框架 |
| 大模型 | 小米 MiMo（Token Plan 专用端点） |
| 开发/验证环境 | VMware Ubuntu 22.04 + goldfish-arm64-v8a-ap 模拟器 |

## 三、系统架构

```
                    ┌──────────────────────────────┐
   自然语言输入 ───▶ │   ai_agent（Agent 运行时）    │
                    │  · Skill: Student Task Planner│
                    │  · 工具: get_current_time /   │
                    │    read_file / write_file /   │
                    │    cron_* / heartbeat         │
                    └──────────────┬───────────────┘
                                   │ 写入
                          /data/ai_agent/TASKS.md
                                   │ 读取（每秒轮询 + 哈希稳定检测）
                    ┌──────────────▼───────────────┐
                    │      VelaTime 应用（LVGL）    │
                    │  · core_task   任务存储        │
                    │  · core_schedule 课程/空闲窗口 │
                    │  · core_recommend 评分推荐     │
                    │  · core_agent_sync 文件桥/提醒 │
                    └──────────────┬───────────────┘
                                   │ 显示
                            手表屏幕（推荐卡片）

   主动方向（VelaTime → Agent）：
   VelaTime 写 /data/ai_agent/HEARTBEAT.md
        → agent heartbeat 定时器读到有待办
        → Agent 主动读 TASKS.md、选出最该做的
        → 写 /data/ai_agent/REMINDER.txt
        → VelaTime 轮询到新内容，直接显示提醒
```

**设计要点**：Agent 与应用之间只通过 `/data/ai_agent/` 下的文件通信，
不跨进程调用对方内部 API，因此两侧可以独立重启、独立演进。

## 四、功能清单

| 功能 | 实现位置 | 说明 |
|---|---|---|
| 自然语言建任务 | Agent + Skill | "明天下午五点交高数作业，大概半小时" → 写入 `TASKS.md` |
| 任务标记完成 | Agent + Skill | `- [ ]` → `- [x]`，应用自动忽略已完成项 |
| 课程表与空闲窗口 | `core_schedule` | 按星期计算当天可用时间窗口 |
| 评分推荐 | `core_recommend` | 紧迫度 + 优先级 + 时长匹配，输出最高分任务 |
| 可解释推荐理由 | `core_recommend` | 如 `明天 08:30 截止 · 120 分钟空档` |
| 任务状态机 | `ui_home` + `core_task` | Start → 进行中；Delay → 延后，并立即换下一条 |
| 运行时自动同步 | `core_agent_sync` | Agent 改完 `TASKS.md`，界面 1~2 秒内自动刷新 |
| **主动提醒** | `core_agent_sync` + Agent heartbeat | 设备侧定时器触发，非用户提问 |
| 中文显示 | `ui/velatime_font_cn.c` | 自带 GB2312 一级汉字字库，3886 字形 |

## 五、目录结构（比赛仓）

```
app/velatime/
├── CMakeLists.txt              # nuttx_add_application，SRCS 需手工登记每个 .c
├── Kconfig                     # EXAMPLES_VELATIME / PRIORITY / STACKSIZE=40960
├── Make.defs / Makefile
├── velatime_main.c             # 入口：初始化 + 任务导入 + 同步定时器 + UI
├── include/velatime_types.h    # 任务/课程/推荐的数据结构（两端契约）
├── core/
│   ├── core_task.[ch]          # 任务内存存储（含 replace_all 安全批量替换）
│   ├── core_schedule.[ch]      # 课程表 + 空闲窗口
│   ├── core_recommend.[ch]     # 评分推荐 + 可解释理由
│   └── core_agent_sync.[ch]    # skill 安装 / TASKS.md 同步 / 主动提醒
├── skills/student-task-planner.md   # 自定义 Skill 源文件
└── ui/
    ├── velatime_ui.h           # UI 接口 + VELATIME_FONT_CN 宏
    ├── ui_home.c               # 首页：推荐卡片（flex 布局）+ 按钮 + 提醒显示
    ├── ui_schedule.c / ui_tasks.c / ui_popup.c
    ├── ui_mock.[ch]            # 无 Agent 任务时的演示数据
    └── velatime_font_cn.c      # 中文字库（lv_font_conv 生成）
```

> ⚠️ 注意事项：NuttX CMake 的 `INCDIR` 在本工程不生效，**所有头文件必须用相对路径 include**；
> 新增 `.c` 文件必须手工加入 `CMakeLists.txt` 的 `SRCS`。

## 六、构建与运行

```bash
# 1) 编译（在 openvela 工作区根目录）
cd /home/yy/openvela-contest
./build.sh vendor/openvela/boards/vela/configs/goldfish-arm64-v8a-ap --cmake -j2

# 2) 启动模拟器（务必带 -data，否则每次编译都会清空任务数据）
./emulator.sh cmake_out/vela_goldfish-arm64-v8a-ap/ \
    -data /home/yy/openvela-persistent/vela_data.bin

# 3) 在模拟器串口 NSH 中
velatime            # 启动应用（命令名区分大小写，全小写）
ai_agent            # 启动 Agent（另一个终端/会话）
```

Agent 内的一次性配置：

```
set_proxy 10.0.2.2 7897                     # 虚拟机内 Clash 的 mixed 端口
set_llm https://token-plan-cn.xiaomimimo.com/v1 mimo-v2.5 tp-你的Key
```

## 七、自定义 Skill 与主动提醒

`skills/student-task-planner.md` 与内嵌在 `core_agent_sync.c` 的规则会在
VelaTime 启动时安装到 `/data/ai_agent/skills/task-manager.md`，约束 Agent：

1. 任务文件是 `/data/ai_agent/TASKS.md`，每行格式 `- [ ] [YYYY-MM-DD] 任务标题`；
2. 日期是**截止日期**（不是创建日期）；完成用 `- [x]`；
3. 建任务时必须同一轮并行调用 `get_current_time` 与 `read_file`（兼容未打补丁的 Agent）；
4. **主动推荐流程**：读 `TASKS.md` → 选最紧急的一条 → 写一行到
   `/data/ai_agent/REMINDER.txt`，格式 `标题 | 剩余时间 | 现在怎么做`。

VelaTime 启动时把待办情况写进 `/data/ai_agent/HEARTBEAT.md`，
Agent 的 heartbeat 线程（默认 30 分钟，演示构建为 1 分钟）检测到有待办就会
**主动发起一次思考**，无需用户输入。这就是本项目的"主动 + 执行"场景。

为方便评审复现，模拟器构建把 heartbeat 间隔改成了 60 秒：

```
apps/packages/ai_agent/CMakeLists.txt:
  add_compile_definitions(AGENT_HEARTBEAT_INTERVAL_MS=60000)
```

量产/默认值为 `30 * 60 * 1000`（在 `agent_config.h`，已加 `#ifndef` 保护）。

## 八、避坑记录（开发中真实踩过）

1. **数据丢失**：构建脚本的 `gen_images` 每次都会 `mkfs.fat` 重建 `cmake_out/.../vela_data.bin`，
   所以必须用 `-data` 指向独立数据盘，否则每次编译任务/密钥全没。
2. **`.config` 手改不生效**：改完要重新跑 `cmake -S nuttx -B cmake_out/...` 才会重写 `nuttx/config.h`。
3. **Agent 单轮工具捷径**：`agent_loop.c` 原逻辑下，单轮只调一次 `read_file` 就结束，
   导致"先读后写"流程走不完；已修正为 `read_file`/`edit_file` 必须回到模型。
   （该文件属公共源码，提交比赛仓时不含此补丁，故 Skill 里要求同轮并行调用以自保。）
4. **推荐分数曾经全部相同**：Agent 写的是 `[YYYY-MM-DD]`，而早期解析要求带时分，
   解析失败后所有任务都拿兜底分 → 推荐顺序失真。现已支持两种格式。
5. **中文方块**：内置 SimSun 字库不含"业"等常用字，且 `font_multilang_small` 只有百余汉字，
   最终自建 GB2312 一级汉字字库解决。
6. **include 路径**：见第五节注意事项，跨目录一律相对路径。
7. **模拟器 `adb shell` 不可用**（`error: closed`），调试请用串口 NSH。

## 九、赛道要求对照

| 官方要求 | 本项目的满足方式 |
|---|---|
| 编译 openvela + ai_agent 并在设备上运行 | ✅ 模拟器完整跑通（真机 BES 2800BP 适配中） |
| 至少 1 个自定义 Skill | ✅ `task-manager.md`（Student Task Planner，含主动推荐流程） |
| 至少 1 个"主动 + 执行"场景 | ✅ heartbeat 定时触发 → Agent 选任务 → 写入提醒 → 应用显示 |
| 完整应用场景说明 | 见本文第一、三、四节 |
| AI Coding 日志 | `logs/`（见该目录 README） |
| 通过 PR 提交到专属仓 | fork → PR → 自行 review 合入 |
