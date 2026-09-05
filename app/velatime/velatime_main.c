#include <nuttx/config.h>
#include <unistd.h>
#include <sys/boardctl.h>

#include <lvgl/lvgl.h>

#include "ui/velatime_ui.h"

#undef NEED_BOARDINIT

#if defined(CONFIG_BOARDCTL) && !defined(CONFIG_NSH_ARCHINIT)
#  define NEED_BOARDINIT 1
#endif

int main(int argc, FAR char *argv[])
{
  lv_nuttx_dsc_t info;
  lv_nuttx_result_t result;

  if (lv_is_initialized())
    {
      return -1;
    }

#ifdef NEED_BOARDINIT
  boardctl(BOARDIOC_INIT, 0);
#endif

  lv_init();

  lv_nuttx_dsc_init(&info);

#ifdef CONFIG_LV_USE_NUTTX_LCD
  info.fb_path = "/dev/lcd0";
#endif

#ifdef CONFIG_INPUT_TOUCHSCREEN
  info.input_path = "/dev/input0";
#endif

  lv_nuttx_init(&info, &result);

  if (result.disp == NULL)
    {
      return 1;
    }

  velatime_ui_init();
  velatime_ui_home_show();

  while (1)
    {
      uint32_t idle;
      idle = lv_timer_handler();
      idle = idle ? idle : 1;
      usleep(idle * 1000);
    }

  lv_nuttx_deinit(&result);
  lv_deinit();
  return 0;
}
