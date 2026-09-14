#ifndef VELATIME_UI_H
#define VELATIME_UI_H

#include <lvgl/lvgl.h>

/* VelaTime 自带中文字库（GB2312 一级汉字 + ASCII），见 ui/velatime_font_cn.c */
extern const lv_font_t velatime_font_cn;
#define VELATIME_FONT_CN (&velatime_font_cn)

#include <lvgl/lvgl.h>

void velatime_ui_init(void);
void velatime_ui_home_show(void);
void velatime_ui_home_refresh(void);
void velatime_ui_schedule_show(void);
void velatime_ui_tasks_show(void);
void velatime_ui_popup_show(void);

#endif /* VELATIME_UI_H */
