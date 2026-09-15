#include "velatime_ui.h"
#include "../core/core_recommend.h"
#include "../core/core_task.h"
#include "../core/core_agent_sync.h"

#include <stdio.h>

/* 主动提醒弹窗：展示 Agent 或端侧给出的提醒，并可直接操作 */

static void on_close_click(lv_event_t *e)
{
  (void)e;
  velatime_ui_home_show();
}

static void on_start_click(lv_event_t *e)
{
  velatime_recomm_book_t rec;
  (void)e;

  if (core_recommend_pick(core_recommend_today_weekday(), &rec))
    {
      /* 提醒弹窗里的"现在开始"直接把该任务置为进行中 */
      core_task_set_status(rec.task_id, VELATIME_STATUS_DOING);
    }

  velatime_ui_home_show();
}

void velatime_ui_popup_show(void)
{
  velatime_recomm_book_t rec;
  char reminder[192];
  int has_rec = core_recommend_pick(core_recommend_today_weekday(), &rec);

  lv_obj_t *scr = lv_obj_create(NULL);
  velatime_ui_style_screen(scr);

  lv_obj_t *card = lv_obj_create(scr);
  lv_obj_set_size(card, 216, 190);
  lv_obj_center(card);
  lv_obj_set_style_bg_color(card, lv_color_hex(0x1C2130), 0);
  lv_obj_set_style_radius(card, 12, 0);
  lv_obj_set_style_border_width(card, 0, 0);
  lv_obj_set_style_pad_all(card, 12, 0);
  lv_obj_set_style_pad_row(card, 6, 0);
  lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
  lv_obj_remove_flag(card, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *title = lv_label_create(card);
  lv_label_set_text(title, "主动提醒");
  lv_obj_set_style_text_color(title, lv_color_hex(0xFF8A3D), 0);

  lv_obj_t *task = lv_label_create(card);
  lv_label_set_text(task, has_rec ? rec.task_title : "暂无待办任务");
  lv_obj_set_style_text_color(task, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_width(task, LV_PCT(100));
  lv_label_set_long_mode(task, LV_LABEL_LONG_WRAP);

  lv_obj_t *detail = lv_label_create(card);
  if (has_rec &&
      core_agent_reminder_local(rec.task_title, rec.reason,
                                rec.suggested_start,
                                reminder, sizeof(reminder)) == 0)
    {
      lv_label_set_text(detail, reminder);
    }
  else
    {
      lv_label_set_text(detail, "添加任务后我会主动提醒你");
    }
  lv_obj_set_style_text_color(detail, lv_color_hex(0x8890A0), 0);
  lv_obj_set_width(detail, LV_PCT(100));
  lv_label_set_long_mode(detail, LV_LABEL_LONG_WRAP);

  lv_obj_t *row = lv_obj_create(card);
  lv_obj_set_width(row, LV_PCT(100));
  lv_obj_set_height(row, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(row, 0, 0);
  lv_obj_set_style_pad_all(row, 0, 0);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *btn_start = lv_button_create(row);
  lv_obj_set_size(btn_start, 84, 32);
  lv_obj_t *start_label = lv_label_create(btn_start);
  lv_label_set_text(start_label, "现在开始");
  lv_obj_center(start_label);
  lv_obj_add_event_cb(btn_start, on_start_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t *btn_close = lv_button_create(row);
  lv_obj_set_size(btn_close, 84, 32);
  lv_obj_t *close_label = lv_label_create(btn_close);
  lv_label_set_text(close_label, "稍后再说");
  lv_obj_center(close_label);
  lv_obj_add_event_cb(btn_close, on_close_click, LV_EVENT_CLICKED, NULL);

  lv_scr_load(scr);
}
