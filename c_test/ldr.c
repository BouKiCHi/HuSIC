/*  HuC's standard header */
#include "huc.h"

main()
{
	int i;
	int count, x;

	count = 0;

	disp_off();
	cls();

	/* font foreground color */

	set_color_rgb(1, 7, 7, 7);
	set_font_color(1, 0);
	set_font_pal(0);

	load_default_font();

	disp_on();
  put_string("LOADER V1.0",0,0);

  /* exec next */
  cd_execoverlay(2);
  while(1);
}
