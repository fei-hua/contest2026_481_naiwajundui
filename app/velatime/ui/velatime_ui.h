#ifndef VELATIME_UI_H
#define VELATIME_UI_H

#include <lvgl/lvgl.h>

/* VelaTime 自带中文字库（GB2312 一级汉字 + ASCII），见 ui/velatime_font_cn.c
 * 说明：16px 用于正文；如需更大字号，可生成 velatime_font_cn52 并在此声明 */
extern const lv_font_t velatime_font_cn;
#define VELATIME_FONT_CN (&velatime_font_cn)

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
