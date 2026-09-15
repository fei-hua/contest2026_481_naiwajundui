#include "velatime_ui.h"

/* 各页面共用的根对象样式：深色底、中文字库、关闭滚动 */
void velatime_ui_style_screen(lv_obj_t *scr)
{
  if (scr == NULL)
    {
      return;
    }

  lv_obj_set_style_bg_color(scr, lv_color_hex(0x10131A), 0);
  lv_obj_set_style_text_font(scr, VELATIME_FONT_CN, 0);
  lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
}
