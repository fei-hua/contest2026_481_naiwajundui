#include "velatime_ui.h"
#include "../core/core_recommend.h"
#include "../core/core_task.h"
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

static void style_screen(lv_obj_t *scr)
{
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x10131A), 0);
  /* 根对象设字体，子控件继承：中文用自带字库，ASCII 也在同一字库内 */
  lv_obj_set_style_text_font(scr, &velatime_font_cn, 0);
  lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
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

  has_rec = core_recommend_pick(1, &rec);
  lv_label_set_text(g_card_title,
                    has_rec ? rec.task_title : "No recommendation");

  snprintf(meta, sizeof(meta), "%d min free | start %s",
           has_rec ? rec.available_minutes : 0,
           has_rec ? rec.suggested_start : "--");
  lv_label_set_text(g_card_meta, meta);
  lv_label_set_text(g_card_reason,
                    has_rec ? rec.reason : "Add tasks to get started");

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

  if (core_recommend_pick(1, &rec))
    {
      core_task_set_status(rec.task_id, VELATIME_STATUS_DOING);
      if (g_status_label != NULL)
        {
          lv_label_set_text(g_status_label, "Started");
        }

      velatime_ui_home_refresh();
    }
}

static void on_delay_click(lv_event_t *e)
{
  velatime_recomm_book_t rec;
  (void)e;

  if (core_recommend_pick(1, &rec))
    {
      core_task_set_status(rec.task_id, VELATIME_STATUS_POSTPONED);
      if (g_status_label != NULL)
        {
          lv_label_set_text(g_status_label, "Postponed");
        }

      velatime_ui_home_refresh();
    }
}

void velatime_ui_init(void)
{
}

void velatime_ui_home_show(void)
{
  lv_obj_t *scr = lv_obj_create(NULL);
  style_screen(scr);

  lv_obj_t *title = lv_label_create(scr);
  lv_label_set_text(title, "VelaTime");
  lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(title, LV_ALIGN_TOP_LEFT, 16, 12);

  lv_obj_t *subtitle = lv_label_create(scr);
  lv_label_set_text(subtitle, "Now recommended:");
  lv_obj_set_style_text_color(subtitle, lv_color_hex(0x8890A0), 0);
  lv_obj_align_to(subtitle, title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

  /* 卡片：flex 纵向布局，内容超宽自动换行，不再溢出/出现横向滚动条 */
  lv_obj_t *card = lv_obj_create(scr);
  lv_obj_set_size(card, 212, 150);
  lv_obj_set_pos(card, 14, 66);
  lv_obj_set_style_bg_color(card, lv_color_hex(0x1C2130), 0);
  lv_obj_set_style_radius(card, 12, 0);
  lv_obj_set_style_border_width(card, 0, 0);
  lv_obj_set_style_pad_all(card, 10, 0);
  lv_obj_set_style_pad_row(card, 4, 0);
  lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_remove_flag(card, LV_OBJ_FLAG_SCROLLABLE);

  g_card_title = lv_label_create(card);
  lv_obj_set_style_text_color(g_card_title, lv_color_hex(0xFFFFFF), 0);
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
  lv_obj_set_size(g_btn_start, 80, 34);
  lv_obj_t *start_label = lv_label_create(g_btn_start);
  lv_label_set_text(start_label, "Start");
  lv_obj_center(start_label);
  lv_obj_add_event_cb(g_btn_start, on_start_click, LV_EVENT_CLICKED, NULL);

  g_btn_delay = lv_button_create(btn_row);
  lv_obj_set_size(g_btn_delay, 80, 34);
  lv_obj_t *delay_label = lv_label_create(g_btn_delay);
  lv_label_set_text(delay_label, "Delay");
  lv_obj_center(delay_label);
  lv_obj_add_event_cb(g_btn_delay, on_delay_click, LV_EVENT_CLICKED, NULL);

  g_status_label = lv_label_create(scr);
  lv_label_set_text(g_status_label, "");
  lv_obj_set_style_text_color(g_status_label, lv_color_hex(0x00D26A), 0);
  lv_obj_align(g_status_label, LV_ALIGN_BOTTOM_MID, 0, -12);

  velatime_ui_home_refresh();
  lv_scr_load(scr);
}
