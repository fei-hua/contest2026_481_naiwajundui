#include "velatime_ui.h"
#include "../core/core_recommend.h"
#include "../core/core_task.h"
#include "../core/core_agent_sync.h"
#include "../include/velatime_types.h"

/* VelaTime 自带中文字库：GB2312 一级汉字 + ASCII，见 ui/velatime_font_cn.c */
LV_FONT_DECLARE(velatime_font_cn)

#include <stdio.h>
#include <string.h>

static lv_obj_t *g_card_title = NULL;
static lv_obj_t *g_card_meta = NULL;
static lv_obj_t *g_card_reason = NULL;
static lv_obj_t *g_btn_start = NULL;
static lv_obj_t *g_btn_delay = NULL;
static lv_obj_t *g_status_label = NULL;
static char g_reminder[192] = "";
static int g_reminder_shown = 0;

static void apply_reminder_text(void)
{
  /* 提醒不挤占卡片（卡片固定显示推荐理由），改为弹出提醒页。
     用 g_reminder_shown 防止用户关掉弹窗后又被立刻弹回。 */
  if (g_reminder[0] != '\0' && !g_reminder_shown)
    {
      g_reminder_shown = 1;
      velatime_ui_popup_show();
    }
}

void velatime_ui_set_reminder(const char *text)
{
  if (text == NULL)
    {
      return;
    }

  strncpy(g_reminder, text, sizeof(g_reminder) - 1);
  g_reminder[sizeof(g_reminder) - 1] = '\0';
  g_reminder_shown = 0;          /* 新提醒：允许弹一次 */
  apply_reminder_text();
}

void velatime_ui_home_refresh(void)
{
  velatime_recomm_book_t rec;
  char meta[96];
  int has_rec;

  if (g_card_title == NULL || g_card_meta == NULL ||
      g_card_reason == NULL)
    {
      return;
    }

  has_rec = core_recommend_pick(core_recommend_today_weekday(), &rec);
  lv_label_set_text(g_card_title,
                    has_rec ? rec.task_title : "No recommendation");

  snprintf(meta, sizeof(meta), "%d min free | start %s",
           has_rec ? rec.available_minutes : 0,
           has_rec ? rec.suggested_start : "--");
  lv_label_set_text(g_card_meta, meta);
  lv_label_set_text(g_card_reason,
                    has_rec ? rec.reason : "Add tasks to get started");
  apply_reminder_text();

  if (g_btn_start != NULL && g_btn_delay != NULL)
    {
      if (has_rec)
        {
          lv_obj_clear_state(g_btn_start, LV_STATE_DISABLED);
          lv_obj_clear_state(g_btn_delay, LV_STATE_DISABLED);
        }
      else
        {
          lv_obj_add_state(g_btn_start, LV_STATE_DISABLED);
          lv_obj_add_state(g_btn_delay, LV_STATE_DISABLED);
        }
    }
}

static void on_start_click(lv_event_t *e)
{
  velatime_recomm_book_t rec;
  (void)e;

  if (core_recommend_pick(core_recommend_today_weekday(), &rec))
    {
      core_task_set_status(rec.task_id, VELATIME_STATUS_DOING);
      if (g_status_label != NULL)
        {
          lv_label_set_text(g_status_label, "Started");
        }

      velatime_ui_home_refresh();
      core_agent_sync_save();      /* 状态写回 TASKS.md，重启不丢 */
    }
}

static void on_delay_click(lv_event_t *e)
{
  velatime_recomm_book_t rec;
  (void)e;

  if (core_recommend_pick(core_recommend_today_weekday(), &rec))
    {
      core_task_set_status(rec.task_id, VELATIME_STATUS_POSTPONED);
      if (g_status_label != NULL)
        {
          lv_label_set_text(g_status_label, "Postponed");
        }

      velatime_ui_home_refresh();
      core_agent_sync_save();      /* 状态写回 TASKS.md，重启不丢 */
    }
}

static void on_schedule_click(lv_event_t *e)
{
  (void)e;
  velatime_ui_schedule_show();
}

static void on_tasks_click(lv_event_t *e)
{
  (void)e;
  velatime_ui_tasks_show();
}

void velatime_ui_init(void)
{
}

void velatime_ui_home_show(void)
{
  lv_obj_t *scr = lv_obj_create(NULL);
  velatime_ui_style_screen(scr);

  lv_obj_t *title = lv_label_create(scr);
  lv_label_set_text(title, "VelaTime");
  lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(title, LV_ALIGN_TOP_LEFT, 24, 16);

  lv_obj_t *subtitle = lv_label_create(scr);
  lv_label_set_text(subtitle, "现在推荐");
  lv_obj_set_style_text_color(subtitle, lv_color_hex(0x8890A0), 0);
  lv_obj_align_to(subtitle, title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

  /* 卡片：按屏幕比例布局（模拟器 1280x800），flex 纵向排列，长文本自动换行 */
  lv_obj_t *card = lv_obj_create(scr);
  lv_obj_set_size(card, LV_PCT(56), LV_PCT(60));
  lv_obj_align_to(card, subtitle, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
  lv_obj_set_style_bg_color(card, lv_color_hex(0x1C2130), 0);
  lv_obj_set_style_radius(card, 16, 0);
  lv_obj_set_style_border_width(card, 0, 0);
  lv_obj_set_style_pad_all(card, 24, 0);
  lv_obj_set_style_pad_row(card, 14, 0);
  lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_remove_flag(card, LV_OBJ_FLAG_SCROLLABLE);

  g_card_title = lv_label_create(card);
  lv_obj_set_style_text_color(g_card_title, lv_color_hex(0xFFFFFF), 0);
  /* 大标题字体：待子集字库就位后启用 VELATIME_FONT_CN24 */
  lv_obj_set_width(g_card_title, LV_PCT(100));
  lv_label_set_long_mode(g_card_title, LV_LABEL_LONG_WRAP);

  g_card_reason = lv_label_create(card);
  lv_obj_set_style_text_color(g_card_reason, lv_color_hex(0xFF8A3D), 0);
  lv_obj_set_width(g_card_reason, LV_PCT(100));
  lv_label_set_long_mode(g_card_reason, LV_LABEL_LONG_WRAP);

  g_card_meta = lv_label_create(card);
  lv_obj_set_style_text_color(g_card_meta, lv_color_hex(0x8890A0), 0);
  lv_obj_set_width(g_card_meta, LV_PCT(100));
  lv_label_set_long_mode(g_card_meta, LV_LABEL_LONG_WRAP);

  lv_obj_t *btn_row = lv_obj_create(card);
  lv_obj_set_width(btn_row, LV_PCT(100));
  lv_obj_set_height(btn_row, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_opa(btn_row, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(btn_row, 0, 0);
  lv_obj_set_style_pad_all(btn_row, 0, 0);
  lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_remove_flag(btn_row, LV_OBJ_FLAG_SCROLLABLE);

  g_btn_start = lv_button_create(btn_row);
  lv_obj_set_size(g_btn_start, 150, 56);
  lv_obj_t *start_label = lv_label_create(g_btn_start);
  lv_label_set_text(start_label, "Start");
  lv_obj_center(start_label);
  lv_obj_add_event_cb(g_btn_start, on_start_click, LV_EVENT_CLICKED, NULL);

  g_btn_delay = lv_button_create(btn_row);
  lv_obj_set_size(g_btn_delay, 150, 56);
  lv_obj_t *delay_label = lv_label_create(g_btn_delay);
  lv_label_set_text(delay_label, "Delay");
  lv_obj_center(delay_label);
  lv_obj_add_event_cb(g_btn_delay, on_delay_click, LV_EVENT_CLICKED, NULL);

  g_status_label = lv_label_create(scr);
  lv_label_set_text(g_status_label, "");
  lv_obj_set_style_text_color(g_status_label, lv_color_hex(0x00D26A), 0);
  lv_obj_align(g_status_label, LV_ALIGN_BOTTOM_MID, 0, -92);

  /* 底部导航：课程表 / 任务列表 */
  lv_obj_t *nav = lv_obj_create(scr);
  lv_obj_set_size(nav, LV_PCT(60), 64);
  lv_obj_align(nav, LV_ALIGN_BOTTOM_MID, 0, -16);
  lv_obj_set_style_bg_opa(nav, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(nav, 0, 0);
  lv_obj_set_style_pad_all(nav, 0, 0);
  lv_obj_set_flex_flow(nav, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(nav, LV_FLEX_ALIGN_SPACE_EVENLY,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_remove_flag(nav, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *btn_sched = lv_button_create(nav);
  lv_obj_set_size(btn_sched, 200, 52);
  lv_obj_t *sched_label = lv_label_create(btn_sched);
  lv_label_set_text(sched_label, "课程表");
  lv_obj_center(sched_label);
  lv_obj_add_event_cb(btn_sched, on_schedule_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t *btn_tasks = lv_button_create(nav);
  lv_obj_set_size(btn_tasks, 200, 52);
  lv_obj_t *tasks_label = lv_label_create(btn_tasks);
  lv_label_set_text(tasks_label, "任务列表");
  lv_obj_center(tasks_label);
  lv_obj_add_event_cb(btn_tasks, on_tasks_click, LV_EVENT_CLICKED, NULL);

  velatime_ui_home_refresh();
  lv_scr_load(scr);
}
