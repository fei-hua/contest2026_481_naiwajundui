#ifndef VELATIME_UI_H
#define VELATIME_UI_H

#include <lvgl/lvgl.h>

/* VelaTime 自带中文字库（GB2312 一级汉字 + ASCII），见 ui/velatime_font_cn.c
 * 说明：16px 用于全部正文；如需更大字号需改用"常用字子集"方案 */
extern const lv_font_t velatime_font_cn;
#define VELATIME_FONT_CN (&velatime_font_cn)

/* ------------------------------------------------------------------
 * 统一布局度量（模拟器屏幕固定 1280x800）
 * 所有页面共用这一套，保证边距、对齐、控件尺寸一致：
 *   - 页面左右边距 32，内容整体左对齐并等宽
 *   - 卡片内边距 24
 *   - 主按钮 200x64
 * ------------------------------------------------------------------ */
#define VELATIME_UI_SCREEN_W    1280
#define VELATIME_UI_SCREEN_H    800
#define VELATIME_UI_PAD         32     /* 页面边距 */
#define VELATIME_UI_PAD_CARD    24     /* 卡片内边距 */
#define VELATIME_UI_CONTENT_W   (VELATIME_UI_SCREEN_W - VELATIME_UI_PAD * 2)  /* 1216 */
#define VELATIME_UI_COL_W       960    /* 居中内容列宽度（所有页面统一） */
#define VELATIME_UI_BTN_W       200
#define VELATIME_UI_BTN_H       64
#define VELATIME_UI_ROW_H       76     /* 列表行高 */

/* 建一个居中内容列并返回它；页面所有控件都放进去，
   这样四个页面的"内容停靠方式"完全一致（列居中、列内左对齐）。 */
lv_obj_t *velatime_ui_page_column(lv_obj_t *scr);

/* 各页面共用的根对象样式（底色 / 字体 / 滚动），见 ui/ui_theme.c */
void velatime_ui_style_screen(lv_obj_t *scr);

void velatime_ui_init(void);
void velatime_ui_home_show(void);
void velatime_ui_home_refresh(void);
void velatime_ui_set_reminder(const char *text);
void velatime_ui_schedule_show(void);
void velatime_ui_tasks_show(void);
void velatime_ui_task_actions_show(const char *task_id);
void velatime_ui_popup_show(void);

#endif /* VELATIME_UI_H */
