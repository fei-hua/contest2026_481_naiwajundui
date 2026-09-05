#include "velatime_ui.h"

void velatime_ui_schedule_show(void)
{
  lv_obj_t *scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x10131A), 0);

  lv_obj_t *label = lv_label_create(scr);
  lv_label_set_text(label, "Schedule (TODO)");
  lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_center(label);

  lv_scr_load(scr);
}
