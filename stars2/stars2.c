#include "apilib.h"

int rand(void);

void HariMain(void)
{
	int i;
	int x;
	int y;
	int win;
	char* buf;
	
	api_initmalloc();
	buf = api_malloc(150 * 100);
	win = api_openwin(buf, 150, 100, -1, "stars2");
	api_boxfilwin(win, 6, 26, 143, 93, 0);
	for (i = 0; i < 50; i ++)
	{
		x = (rand() % 137) + 6;
		y = (rand() % 67) + 26;
		api_point(win, x, y, 2);
	}
	api_refreshwin(win, 6, 26, 144, 94);
	for (; ;)
	{
		if (api_getkey(1) == 0x0a)
		{
			break;
		}
	}
	api_end();
}
