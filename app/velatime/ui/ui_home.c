#include "velatime_ui.h"
#include "ui_mock.h"
#include <stdio.h>

static void style_screen(lv_obj_t *scr)
{
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x10131A), 0);
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

  lv_obj_t *card = lv_obj_create(scr);
  lv_obj_set_size(card, 220, 140);
  lv_obj_set_style_bg_color(card, lv_color_hex(0x1C2130), 0);
  lv_obj_set_style_radius(card, 12, 0);
  lv_obj_set_style_border_width(card, 0, 0);
  lv_obj_align(card, LV_ALIGN_TOP_LEFT, 16, 84);

  lv_obj_t *card_title = lv_label_create(card);
  lv_label_set_text(card_title, mock_recommend.task_title);
  lv_obj_set_style_text_color(card_title, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(card_title, LV_ALIGN_TOP_LEFT, 12, 10);

  char meta[96];
  snprintf(meta, sizeof(meta), "%d min free | suggested %s",
           mock_recommend.available_minutes,
           mock_recommend.suggested_start);
  lv_obj_t *card_meta = lv_label_create(card);
  lv_label_set_text(card_meta, meta);
  lv_obj_set_style_text_color(card_meta, lv_color_hex(0x8890A0), 0);
  lv_obj_align_to(card_meta, card_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

  lv_obj_t *card_reason = lv_label_create(card);
  lv_label_set_text(card_reason, mock_recommend.reason);
  lv_obj_set_style_text_color(card_reason, lv_color_hex(0xFF8A3D), 0);
  lv_obj_align_to(card_reason, card_meta, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

  lv_obj_t *btn_start = lv_button_create(card);
  lv_obj_set_size(btn_start, 80, 36);
  lv_obj_align(btn_start, LV_ALIGN_BOTTOM_LEFT, 12, -12);
  lv_obj_t *start_label = lv_label_create(btn_start);
  lv_label_set_text(start_label, "Start");
  lv_obj_center(start_label);

  lv_obj_t *btn_delay = lv_button_create(card);
  lv_obj_set_size(btn_delay, 80, 36);
  lv_obj_align(btn_delay, LV_ALIGN_BOTTOM_RIGHT, -12, -12);
  lv_obj_t *delay_label = lv_label_create(btn_delay);
  lv_label_set_text(delay_label, "Delay");
  lv_obj_center(delay_label);

  lv_scr_load(scr);
}
