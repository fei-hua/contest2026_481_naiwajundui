# 验证记录（verification.md）

本文件记录 VelaTime 的可复现验证，命令与结果成对出现。

## 1. 编译验证

```bash
cd /home/yy/openvela-contest
./build.sh vendor/openvela/boards/vela/configs/goldfish-arm64-v8a-ap --cmake -j2
```

期望输出（实测通过）：

```
[3512/3512] cd /home/yy/openvela-contest/cmake_out/vela_goldfish-arm64-v8a-ap \
    && cp nuttx vela_ap.elf && cp vela_ap.bin
#### build completed successfully (08:53 (mm:ss)) ####
```

## 2. 固件内容检查

```bash
strings cmake_out/vela_goldfish-arm64-v8a-ap/nuttx | grep -m3 'HEARTBEAT.md'
strings cmake_out/vela_goldfish-arm64-v8a-ap/nuttx | grep -m5 'proactive reminder'
```

实测输出（节选）：

```
Read /data/ai_agent/HEARTBEAT.md and follow any instructions or tasks listed there.
VelaTime: proactive reminder: %s
VelaTime: heartbeat armed for %d pending task(s)
/data/ai_agent/REMINDER.txt
```

## 3. 应用启动与任务导入（模拟器）

```bash
./emulator.sh cmake_out/vela_goldfish-arm64-v8a-ap/ \
    -data /home/yy/openvela-persistent/vela_data.bin
# 串口 NSH 中：
velatime
```

实测串口日志：

```
VelaTime: installed agent skill at /data/ai_agent/skills/task-manager.md
VelaTime: parsed 1 task(s) from /data/ai_agent/TASKS.md
VelaTime: imported 1 agent task(s)
VelaTime: heartbeat armed for 1 pending task(s)
[  57.187375] [13] [  CRIT] [ap] [LVGL] check_stack_size: Stack size : 40720   # 要求 ≥32768
[  57.421019] [13] [  CRIT] [ap] [LVGL] lv_nuttx_fbdev_set_file: Resolution is set to 1280x800
[  57.440875] [13] [  CRIT] [ap] [LVGL] touchscreen /dev/input0 open success
```

## 4. 界面验收（人工截图确认）

- ✅ 中文任务标题正常显示（含此前缺字的"业"）
- ✅ 推荐卡片文字自动换行，**无溢出、无横向滚动条**
- ✅ Start / Delay 按钮并排可点，点击后底部出现 `Started` / `Postponed`
- ✅ 卡片显示元信息：`120 min free | start 12:00`

## 5. 运行时自动同步（宿主机单元测试）

```bash
bash vm_run_sync_unit.sh      # 编译工程真实 core_agent_sync.c + core_task.c
```

实测结果（15 项全部 PASS）：

```
[PASS] 首次导入条数                    got=1 want=1
[PASS] 导入后任务数                    got=1 want=1
[PASS] 已完成任务被忽略                got=1 want=1
[PASS] 未变化 第1次 if_changed          got=0 want=0
[PASS] 未变化 第2次 if_changed          got=0 want=0
[PASS] 变化 第1次（等稳定, 返回0）      got=0 want=0
[PASS] 变化 第2次（应用, 返回1）        got=1 want=1
[PASS] 同步后任务数                    got=2 want=2
[PASS] 同步后第2条标题                  got=1 want=1
[PASS] 同步后再调用（回0）              got=0 want=0
[PASS] 标记完成后触发同步              got=1 want=1
[PASS] 完成后剩余待办数                got=1 want=1
[PASS] 文件消失返回 -1                 got=-1 want=-1
[PASS] 文件消失后内存任务保留          got=1 want=1
[PASS] 文件恢复后同步                  got=1 want=1
== 结论: 全部通过 ==
```

## 6. 截止日期紧迫度解析（宿主机单元测试）

```
now = Mon Sep 14 22:06:55 2026
2026-09-14         urgency= 90  reason=今天截止 · 120 分钟空档
2026-09-15         urgency= 70  reason=明天截止 · 120 分钟空档
2026-09-16         urgency= 50  reason=2 天后截止 · 120 分钟空档
2026-09-18         urgency= 30  reason=09-18 截止 · 120 分钟空档
2026-10-01         urgency= 10  reason=10-01 截止 · 120 分钟空档
2026-09-13         urgency=100  reason=已逾期 · 30 分钟可完成
2026-09-01         urgency= 70  reason=已逾期 · 30 分钟可完成
2026-09-14 18:00   urgency=100  reason=已逾期 · 30 分钟可完成
2026-09-15 08:30   urgency= 70  reason=明天 08:30 截止 · 120 分钟空档
(empty)            urgency= 20  reason=30 分钟可完成，适合现在开始
garbage            urgency= 20  reason=30 分钟可完成，适合现在开始
```

要点：`2026-09-15 08:30` 正确表述为"明天"（按日历天判断，不按小时差）；
逾期超过一天的旧任务降级，不会长期霸占推荐位。

## 7. 主动提醒（宿主机单元测试）

```
[PASS] publish(待办=2) 返回 0
[PASS] HEARTBEAT.md 提到 pending
[PASS] publish(待办=0) 返回 0
[PASS] HEARTBEAT.md 改为 idle
[PASS] check 返回 0（文件不存在）
[PASS] 第一次 check 返回 1
       reminder = 交高数作业 | 明天截止 | 先花 30 分钟做完前两题
[PASS] 第二次 check 返回 0（同样内容不重复打扰）
[PASS] 第三次 check 返回 0
[PASS] 变化后 check 返回 1
[PASS] HEARTBEAT_OK 被过滤
[PASS] 模板占位符被过滤
[PASS] 空内容被过滤
[PASS] pending=0 时返回 0
== 结论: 全部通过 ==
```

## 8. 已知限制（如实记录）

- 模拟器 `adb shell` 恒为 `error: closed`（guest 内 adbd shell 服务初始化失败），
  调试请使用串口 NSH；`adb forward` 也无法建立可用通道
- 应用占用串口控制台后，外部注入的 NSH 命令不会被受理，
  因此"应用运行中改文件"这类操作需要人在模拟器窗口操作
- 宿主机为 Wayland 会话时 `xwd`/GNOME 截图接口均不可用，界面验收需人工截图
- Agent heartbeat 默认 30 分钟触发一次；演示时可等待，或让端侧即时提醒先展示
