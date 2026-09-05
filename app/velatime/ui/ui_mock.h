#ifndef VELATIME_UI_MOCK_H
#define VELATIME_UI_MOCK_H

#include "../include/velatime_types.h"

#define VELATIME_MOCK_TASK_COUNT   5
#define VELATIME_MOCK_COURSE_COUNT 8

extern velatime_task_t      mock_tasks[VELATIME_MOCK_TASK_COUNT];
extern velatime_course_t    mock_courses[VELATIME_MOCK_COURSE_COUNT];
extern velatime_recommend_t mock_recommend;

#endif /* VELATIME_UI_MOCK_H */
