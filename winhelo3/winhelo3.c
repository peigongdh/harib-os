#include "apilib.h"

void HariMain(void)
{
	int win;
	char* buf;
	
	api_initmalloc();
	buf = api_malloc(150 * 50);
	win = api_openwin(buf, 150, 50, -1, "hello");
	api_boxfilwin(win, 8, 36, 141, 43, 6/* »ÆÉ« */);
	api_putstrwin(win, 28, 28, 0/* ºÚÉ« */, 12, "hello, world");
	for (; ;)
	{
		if (api_getkey(1) == 0x0a)
		{
			break;
		}
	}
	api_end();
}
