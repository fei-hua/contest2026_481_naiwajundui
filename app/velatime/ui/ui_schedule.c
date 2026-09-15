#include "velatime_ui.h"
#include "../core/core_task.h"
#include "../core/core_schedule.h"
#include "../core/core_recommend.h"

#include <stdio.h>

/* 课程表：显示当天课程与由课程间隙算出的空闲窗口 */

static void on_back_click(lv_event_t *e)
{
  (void)e;
  velatime_ui_home_show();
}

static void build_course_row(lv_obj_t *parent, const velatime_course_t *course)
{
  lv_obj_t *row = lv_obj_create(parent);
  lv_obj_set_width(row, LV_PCT(100));
  lv_obj_set_height(row, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_color(row, lv_color_hex(0x1C2130), 0);
  lv_obj_set_style_radius(row, 8, 0);
  lv_obj_set_style_border_width(row, 0, 0);
  lv_obj_set_style_pad_all(row, 8, 0);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *name = lv_label_create(row);
  lv_label_set_text(name, course->name);
  lv_obj_set_style_text_color(name, lv_color_hex(0xFFFFFF), 0);

  lv_obj_t *right = lv_obj_create(row);
  lv_obj_set_height(right, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_opa(right, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(right, 0, 0);
  lv_obj_set_style_pad_all(right, 0, 0);
  lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(right, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);
  lv_obj_remove_flag(right, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *time = lv_label_create(right);
  lv_label_set_text_fmt(time, "%s-%s", course->start, course->end);
  lv_obj_set_style_text_color(time, lv_color_hex(0x8890A0), 0);

  if (course->room[0] != '\0')
    {
      lv_obj_t *room = lv_label_create(right);
      lv_label_set_text(room, course->room);
      lv_obj_set_style_text_color(room, lv_color_hex(0x6B7280), 0);
    }
}

static void build_slot_row(lv_obj_t *parent, const velatime_free_slot_t *slot)
{
  lv_obj_t *row = lv_obj_create(parent);
  lv_obj_set_width(row, LV_PCT(100));
  lv_obj_set_height(row, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_color(row, lv_color_hex(0x16301F), 0);
  lv_obj_set_style_radius(row, 8, 0);
  lv_obj_set_style_border_width(row, 0, 0);
  lv_obj_set_style_pad_all(row, 8, 0);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *tag = lv_label_create(row);
  lv_label_set_text_fmt(tag, "空闲 %s-%s", slot->start, slot->end);
  lv_obj_set_style_text_color(tag, lv_color_hex(0x00D26A), 0);

  lv_obj_t *mins = lv_label_create(row);
  lv_label_set_text_fmt(mins, "%d 分钟", slot->minutes);
  lv_obj_set_style_text_color(mins, lv_color_hex(0x8890A0), 0);
}

void velatime_ui_schedule_show(void)
{
  velatime_free_slot_t slots[VELATIME_FREE_SLOT_MAX];
  int weekday = core_recommend_today_weekday();
  int slot_count;
  int course_count = 0;
  int i;
  char summary[64];

  lv_obj_t *scr = lv_obj_create(NULL);
  velatime_ui_style_screen(scr);

  lv_obj_t *title = lv_label_create(scr);
  lv_label_set_text(title, "今日课程");
  lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(title, LV_ALIGN_TOP_LEFT, 16, 12);

  for (i = 0; i < core_schedule_count(); i++)
    {
      const velatime_course_t *c = core_schedule_get(i);
      if (c != NULL && c->weekday == weekday)
        {
          course_count++;
        }
    }

  slot_count = core_schedule_free_slots(weekday, slots, VELATIME_FREE_SLOT_MAX);
  snprintf(summary, sizeof(summary), "周%d · %d 门课 · %d 段空闲",
           weekday, course_count, slot_count);

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

  for (i = 0; i < core_schedule_count(); i++)
    {
      const velatime_course_t *c = core_schedule_get(i);
      if (c != NULL && c->weekday == weekday)
        {
          build_course_row(list, c);
        }
    }

  for (i = 0; i < slot_count; i++)
    {
      build_slot_row(list, &slots[i]);
    }

  if (course_count == 0 && slot_count == 0)
    {
      lv_obj_t *empty = lv_label_create(list);
      lv_label_set_text(empty, "今天没有课，整天空闲");
      lv_obj_set_style_text_color(empty, lv_color_hex(0x8890A0), 0);
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
