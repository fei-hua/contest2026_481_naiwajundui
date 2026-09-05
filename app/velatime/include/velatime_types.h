#ifndef VELATIME_TYPES_H
#define VELATIME_TYPES_H

#define VELATIME_MAX_TITLE    64
#define VELATIME_MAX_COURSE   32
#define VELATIME_MAX_DEADLINE 32
#define VELATIME_MAX_PRIORITY 16
#define VELATIME_MAX_ID       16
#define VELATIME_MAX_REASON   128

typedef enum
{
  VELATIME_STATUS_WAITING = 0,
  VELATIME_STATUS_DOING,
  VELATIME_STATUS_DONE,
  VELATIME_STATUS_POSTPONED
} velatime_status_t;

typedef struct
{
  char id[VELATIME_MAX_ID];
  char title[VELATIME_MAX_TITLE];
  char course[VELATIME_MAX_COURSE];
  char deadline[VELATIME_MAX_DEADLINE];   /* "2026-09-06 18:00" */
  int  estimated_minutes;
  char priority[VELATIME_MAX_PRIORITY];   /* "high" / "medium" / "low" */
  velatime_status_t status;
} velatime_task_t;

typedef struct
{
  char name[VELATIME_MAX_COURSE];
  int  weekday;               /* 1=周一 ... 7=周日 */
  char start[8];              /* "08:00" */
  char end[8];                /* "09:40" */
} velatime_course_t;

typedef struct
{
  char task_id[VELATIME_MAX_ID];
  char task_title[VELATIME_MAX_TITLE];
  int  available_minutes;
  char suggested_start[8];    /* "15:20" */
  char reason[VELATIME_MAX_REASON];
} velatime_recommend_t;

#endif /* VELATIME_TYPES_H */
