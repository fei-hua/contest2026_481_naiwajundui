#include "velatime_ui.h"
#include "../core/core_task.h"
#include "../core/core_recommend.h"
#include "../core/core_schedule.h"

#include <stdio.h>
#include <string.h>

/* 任务列表：显示 Agent 写入的全部任务（待办在前，已完成置灰） */

static const char *status_icon(const velatime_task_t *task)
{
  switch (task->status)
    {
      case VELATIME_STATUS_DONE:      return "[x]";
      case VELATIME_STATUS_DOING:     return "[>]";
      case VELATIME_STATUS_POSTPONED: return "[~]";
      default:                        return "[ ]";
    }
}

static const char *status_text(const velatime_task_t *task)
{
  switch (task->status)
    {
      case VELATIME_STATUS_DONE:      return "已完成";
      case VELATIME_STATUS_DOING:     return "进行中";
      case VELATIME_STATUS_POSTPONED: return "已延后";
      default:                        return "待办";
    }
}

static uint32_t status_color(const velatime_task_t *task)
{
  switch (task->status)
    {
      case VELATIME_STATUS_DONE:      return 0x6B7280;   /* 灰 */
      case VELATIME_STATUS_DOING:     return 0x00D26A;   /* 绿 */
      case VELATIME_STATUS_POSTPONED: return 0xF0B429;   /* 黄 */
      default:                        return 0xFFFFFF;   /* 白 */
    }
}

static void on_back_click(lv_event_t *e)
{
  (void)e;
  velatime_ui_home_show();
}

static void build_row(lv_obj_t *parent, const velatime_task_t *task)
{
  lv_obj_t *row = lv_obj_create(parent);
  lv_obj_set_width(row, LV_PCT(100));
  lv_obj_set_height(row, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_color(row, lv_color_hex(0x1C2130), 0);
  lv_obj_set_style_radius(row, 8, 0);
  lv_obj_set_style_border_width(row, 0, 0);
  lv_obj_set_style_pad_all(row, 8, 0);
  lv_obj_set_style_pad_row(row, 2, 0);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_COLUMN);
  lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *title = lv_label_create(row);
  lv_label_set_text_fmt(title, "%s %s", status_icon(task), task->title);
  lv_obj_set_style_text_color(title, lv_color_hex(status_color(task)), 0);
  lv_obj_set_width(title, LV_PCT(100));
  lv_label_set_long_mode(title, LV_LABEL_LONG_WRAP);

  lv_obj_t *meta = lv_label_create(row);
  if (task->deadline[0] != '\0')
    {
      lv_label_set_text_fmt(meta, "%s · 截止 %s · 预计 %d 分钟",
                            status_text(task), task->deadline,
                            task->estimated_minutes);
    }
  else
    {
      lv_label_set_text_fmt(meta, "%s · 预计 %d 分钟",
                            status_text(task), task->estimated_minutes);
    }
  lv_obj_set_style_text_color(meta, lv_color_hex(0x8890A0), 0);
  lv_obj_set_width(meta, LV_PCT(100));
  lv_label_set_long_mode(meta, LV_LABEL_LONG_WRAP);
}

void velatime_ui_tasks_show(void)
{
  int total = core_task_count();
  int i;
  int waiting = 0;
  char summary[64];

  lv_obj_t *scr = lv_obj_create(NULL);
  velatime_ui_style_screen(scr);

  lv_obj_t *title = lv_label_create(scr);
  lv_label_set_text(title, "任务列表");
  lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(title, LV_ALIGN_TOP_LEFT, 16, 12);

  for (i = 0; i < total; i++)
    {
      const velatime_task_t *t = core_task_get(i);
      if (t != NULL && t->status == VELATIME_STATUS_WAITING)
        {
          waiting++;
        }
    }

  snprintf(summary, sizeof(summary), "共 %d 条 · 待办 %d 条", total, waiting);
  lv_obj_t *subtitle = lv_label_create(scr);
  lv_label_set_text(subtitle, summary);
  lv_obj_set_style_text_color(subtitle, lv_color_hex(0x8890A0), 0);
  lv_obj_align_to(subtitle, title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);

  lv_obj_t *list = lv_obj_create(scr);
  lv_obj_set_size(list, 224, 200);
  lv_obj_set_pos(list, 8, 76);
  lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(list, 0, 0);
  lv_obj_set_style_pad_all(list, 0, 0);
  lv_obj_set_style_pad_row(list, 6, 0);
  lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_scroll_dir(list, LV_DIR_VER);

  if (total <= 0)
    {
      lv_obj_t *empty = lv_label_create(list);
      lv_label_set_text(empty, "暂无任务\n对 Agent 说：帮我创建一个任务");
      lv_obj_set_style_text_color(empty, lv_color_hex(0x8890A0), 0);
      lv_obj_set_width(empty, LV_PCT(100));
      lv_label_set_long_mode(empty, LV_LABEL_LONG_WRAP);
    }
  else
    {
      for (i = 0; i < total; i++)
        {
          const velatime_task_t *t = core_task_get(i);
          if (t != NULL)
            {
              build_row(list, t);
            }
        }
    }

  lv_obj_t *btn_back = lv_button_create(scr);
  lv_obj_set_size(btn_back, 100, 32);
  lv_obj_align(btn_back, LV_ALIGN_BOTTOM_LEFT, 10, -10);
  lv_obj_t *back_label = lv_label_create(btn_back);
  lv_label_set_text(back_label, "返回");
  lv_obj_center(back_label);
  lv_obj_add_event_cb(btn_back, on_back_click, LV_EVENT_CLICKED, NULL);

  lv_scr_load(scr);
}
