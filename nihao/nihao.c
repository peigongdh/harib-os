#include "apilib.h"
#include <stdio.h>
#include <string.h>

void HariMain(void)
{
	int i;
	static char s[5] = {0xc4, 0xe3, 0xba, 0xc3, 0x00};
//	static char s[5] = {0xe4, 0xbd, 0xa0, 0xe5, 0xa5};
	api_putstr0(s);
	static char s1[5] = "你好";
	char s2[5];
	for (i = 0; i < 5; i ++)
	{
		sprintf(s2, "%02x, ", s1[i] & 0xff);
		api_putstr0(s2);
	}
	api_end();
}
