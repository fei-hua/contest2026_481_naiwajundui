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

/*
 * 居中内容列：宽度固定 VELATIME_UI_COL_W，水平居中、垂直留 VELATIME_UI_PAD 边距。
 * 页面里的标题、列表、按钮全部作为它的子对象，从而保证四个页面：
 *   - 内容列在屏幕上的位置完全一致（居中）
 *   - 列内元素统一左对齐成一条竖线
 * 行距用 pad_row 控制，子对象之间不再手工调 y 偏移。
 */
lv_obj_t *velatime_ui_page_column(lv_obj_t *scr)
{
  lv_obj_t *col;

  if (scr == NULL)
    {
      return NULL;
    }

  col = lv_obj_create(scr);
  lv_obj_set_size(col, VELATIME_UI_COL_W,
                  VELATIME_UI_SCREEN_H - VELATIME_UI_PAD * 2);
  lv_obj_align(col, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_opa(col, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(col, 0, 0);
  lv_obj_set_style_pad_all(col, 0, 0);
  lv_obj_set_style_pad_row(col, 12, 0);
  lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(col, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_remove_flag(col, LV_OBJ_FLAG_SCROLLABLE);

  return col;
}
