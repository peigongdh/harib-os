/* ͼ����Ӵ��� */

#include "bootpack.h"

/* ��ʼ��ͼ����� */ 
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
	struct SHTCTL *ctl;
	int i;
	ctl = (struct SHTCTL *) memman_alloc_4k(memman, sizeof(struct SHTCTL));
	if (ctl == 0)
	{
		goto err;
	}
	ctl->map = (unsigned char *) memman_alloc_4k(memman, xsize * ysize);
	if (ctl->map == 0)
	{
		memman_free_4k(memman, (int) ctl, sizeof (struct SHTCTL));
		goto err;
	}
	
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1;	/* ��ʼ��һ��ͼ�㶼û�� */
	for (i = 0; i < MAX_SHEETS; i ++)
	{
		/* ���Ϊδʹ�� */
		ctl->sheets0[i].flags = 0;
		ctl->sheets0[i].ctl = ctl;
	}
err:
	return ctl;
}

#define SHEET_USE	1

/* ȡ�������ɵ�δʹ��ͼ�� */
struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
	struct SHEET *sht;
	int i;
	for (i = 0; i < MAX_SHEETS; i ++)
	{
		if (ctl->sheets0[i].flags == 0)
		{
			sht = &ctl->sheets0[i];
			sht->flags = SHEET_USE;	/* ���Ϊ����ʹ�� */ 
			sht->height = -1;		/* ���� */
			sht->task = 0;			/* ��ʹ���Զ��رչ��� */ 
			return sht;
		}
	} 
	return 0;
}

/* ͼ���������� */
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;
	return;
} 


/* ͼ�������ƶ� */ 
void sheet_updown(struct SHEET *sht, int height)
{
	struct SHTCTL *ctl = sht->ctl;
	int h;
	int old = sht->height;	/* �洢����ǰ�ĸ߶���Ϣ */
	/* ���ָ���ĸ߶ȹ��߻���ͣ���������� */
	if (height > ctl->top + 1)
	{
		height = ctl->top + 1;
	}
	if (height < -1)
	{
		height = -1;
	}
	sht->height = height;
	
	/* ������Ҫ�ǽ���sheets[]���������� */
	if (old > height)
	{
		if (height >= 0)
		{
			for (h = old; h > height; h --)
			{
				ctl->sheets[h] = ctl->sheets[h - 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
			sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1);
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 
				height + 1, old);
		}
		/* height == 0 ����ͼ�� */
		else
		{
			if (ctl->top > old)
			{
				for (h = old; h < ctl->top; h ++)
				{
					ctl->sheets[h] = ctl->sheets[h + 1];
					ctl->sheets[h]->height = h;
				}
			}
			ctl->top --;
		}
		sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0);
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 
			0, old - 1);
	}
	else if (old < height)
	{
		if (old >= 0)
		{
			for (h = old; h < height; h ++)
			{
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		}
		/* ����״̬->��ʾ״̬ */
		else
		{
			for (h = ctl->top; h >= height; h --)
			{
				ctl->sheets[h + 1] = ctl->sheets[h];
				ctl->sheets[h + 1]->height = h + 1;
			}
			ctl->sheets[height] = sht;
			ctl->top ++;
		}
		sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height);
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 
			height, height);
	}
	return;
}

/* �ɷ��� */
/* ˢ��ͼ�㣬�������ϰ����ص����������������� */
/*
void sheet_refresh(struct SHTCTL *ctl) 
{
	int h, bx, by, vx, vy;
	unsigned char *buf, c, *vram = ctl->vram;
	struct SHEET *sht;
	for (h = 0; h <= ctl->top; h ++)
	{
		sht = ctl->sheets[h];
		buf = sht->buf;
		for (by = 0; by < sht->bysize; by ++)
		{
			vy = sht->vy0 + by;
			for (bx = 0; bx < sht->bxsize; bx ++)
			{
				vx = sht->vx0 + bx;
				c = buf[by * sht->bxsize + bx];
				if (c != sht->col_inv)
				{
					vram[vy * ctl->xsize + vx] = c;
				}
			}
		}
	}
	return;
}
*/

/* ���þֲ�ˢ�µķ�����дsheet_refresh */
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
	if (sht->height >= 0)
	{
		sheet_refreshsub(sht->ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1, 
			sht->height, sht->height);
	}
	return;
}

/* �Ե�ͼ�ķ�ʽ���ֲ�ˢ�� */
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	int* p;
	int sid4;
	unsigned char *buf, sid, *map = ctl->map;
	struct SHEET *sht;
	/* ���refresh�ķ�Χ�����˻��������� */
	if (vx0 < 0)
	{
		vx0 = 0;
	}
	if (vy0 < 0)
	{
		vy0 = 0;
	}
	if (vx1 > ctl->xsize)
	{
		vx1 = ctl->xsize;
	}
	if (vy1 > ctl->ysize)
	{
		vy1 = ctl->ysize;
	}
	for (h = h0; h <= ctl->top; h ++)
	{
		sht = ctl->sheets[h];
		/* �����˼�������ĵ�ַ��Ϊͼ�����ʹ�� */
		sid = sht - ctl->sheets0;
		buf = sht->buf;
		/* ʹ��vx0~vy1����bx0~by1���е��� */
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if (bx0 < 0)
		{
			bx0 = 0;
		}
		if (by0 < 0)
		{
			by0 = 0;
		}
		if (bx1 > sht->bxsize)
		{
			bx1 = sht->bxsize;
		}
		if (by1 > sht->bysize)
		{
			by1 = sht->bysize;
		}
		if (sht->col_inv == -1)
		{
			/* ��͸��ɫͼ��ר�õĸ��ٰ�(4�ֽ���) */
			if ((sht->vx0 & 3) == 0 && (bx0 & 3) == 0 && (bx1 & 3) == 0) {
				bx1 = (bx1 - bx0) / 4;
				sid4 = sid | sid << 8 | sid << 16 | sid << 24;
				for (by = by0; by < by1; by ++)
				{
					vx = sht->vx0 + bx0;
					vy = sht->vy0 + by;
					p = (int* )&map[vy * ctl->xsize + vx];
					for (bx = 0; bx < bx1; bx ++)
					{
						p[bx] = sid4;
					}
				}
			}
			/* ��͸��ɫͼ��ר�õĸ��ٰ�(1�ֽ���) */
			else
			{
				for (by = by0; by < by1; by ++)
				{
					vy = sht->vy0 + by;
					for (bx = bx0; bx < bx1; bx ++)
					{
						vx = sht->vx0 + bx;
						map[vy * ctl->xsize + vx] = sid;
					}
				}
			}
		}
		else
		{
			/* ��͸��ɫͼ��ר�õ���ͨ�� */
			for (by = by0; by < by1; by ++)
			{
				vy = sht->vy0 + by;
				for (bx = bx0; bx < bx1; bx ++)
				{
					vx = sht->vx0 + bx;
					if (buf[by * sht->bxsize + bx] != sht->col_inv)
					{
						map[vy * ctl->xsize + vx] = sid;
					}
				}
			}
		}
	}
	return;
}

/* �ɷ��� */
/* �ֲ�ˢ�� */
/*
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1)
{
	int h, bx, by, vx, vy;
	unsigned char *buf, c, *vram = ctl->vram;
	struct SHEET *sht;
	for (h = 0; h <= ctl->top; h ++)
	{
		sht = ctl->sheets[h];
		buf = sht->buf;
		for (by = 0; by < sht->bysize; by ++)
		{
			vy = sht->vy0 + by;
			for (bx = 0; bx < sht->bxsize; bx ++)
			{
				vx = sht->vx0 + bx;
				if (vx0 <= vx && vx < vx1 && vy0 <= vy && vy < vy1)
				{
					c = buf[by * sht->bxsize + bx];
					if (c != sht->col_inv)
					{
						vram[vy * ctl->xsize + vx] = c;
					}
				}
			}
		}
	}
	return;
}
*/

/* ��h0����h1���µ�ͼ�㿪ʼ�ֲ�ˢ�� */
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, *vram = ctl->vram, *map = ctl->map, sid;
	struct SHEET *sht;
	int* p;
	int* q;
	int* r;
	int sid4;
	int bx2;
	int i;
	int i1;
	/* ���refresh�ķ�Χ�����˻��������� */
	if (vx0 < 0)
	{
		vx0 = 0;
	}
	if (vy0 < 0)
	{
		vy0 = 0;
	}
	if (vx1 > ctl->xsize)
	{
		vx1 = ctl->xsize;
	}
	if (vy1 > ctl->ysize)
	{
		vy1 = ctl->ysize;
	}
	for (h = h0; h <= h1; h ++)
	{
		sht = ctl->sheets[h];
		buf = sht->buf;
		sid = sht - ctl->sheets0;
		/* ʹ��vx0~vy1����bx0~by1���е��� */
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if (bx0 < 0)
		{
			bx0 = 0;
		}
		if (by0 < 0)
		{
			by0 = 0;
		}
		if (bx1 > sht->bxsize)
		{
			bx1 = sht->bxsize;
		}
		if (by1 > sht->bysize)
		{
			by1 = sht->bysize;
		}
		if ((sht->vx0 & 3) == 0)
		{
			i = (bx0 + 3) / 4;
			i1 = bx1 / 4;
			i1 = i1 - i;
			sid4 = sid | sid << 8 | sid << 16 | sid << 24;
			for (by = by0; by < by1; by ++)
			{
				vy = sht->vy0 + by;
				for (bx = bx0; bx < bx1 && (bx & 3) != 0; bx ++)
				{
					vx = sht->vx0 + bx;
					if (map[vy * ctl->xsize + vx] == sid)
					{
						vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
					}
				}
				vx = sht->vx0 + bx;
				p = (int* )&map[vy * ctl->xsize + vx];
				q = (int* )&vram[vy * ctl->xsize + vx];
				r = (int* )&buf[by * sht->bxsize + bx];
				for (i = 0; i < i1; i ++)
				{
					if (p[i] == sid4)
					{
						q[i] = r[i];
					}
					else
					{
						bx2 = bx + i * 4;
						vx = sht->vx0 + bx2;
						if (map[vy * ctl->xsize + vx + 0] == sid)
						{
							vram[vy * ctl->xsize + vx + 0] = buf[by * sht->bxsize + bx2 + 0];
						}
						if (map[vy * ctl->xsize + vx + 1] == sid)
						{
							vram[vy * ctl->xsize + vx + 1] = buf[by * sht->bxsize + bx2 + 1];
						}
						if (map[vy * ctl->xsize + vx + 2] == sid)
						{
							vram[vy * ctl->xsize + vx + 2] = buf[by * sht->bxsize + bx2 + 2];
						}
						if (map[vy * ctl->xsize + vx + 3] == sid)
						{
							vram[vy * ctl->xsize + vx + 3] = buf[by * sht->bxsize + bx2 + 3];
						}
					}
				}
				for (bx += i1 * 4; bx < bx1; bx ++)
				{
					vx = sht->vx0 + bx;
					if (map[vy * ctl->xsize + vx] == sid)
					{
						vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
					}
				}
			}
		}
		else
		{
			for (by = by0; by < by1; by ++)
			{
				vy = sht->vy0 + by;
				for (bx = bx0; bx < bx1; bx ++)
				{
					vx = sht->vx0 + bx;
					if (map[vy * ctl->xsize + vx] == sid)
					{
						vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
					}
				}
			}
		}
	}
	return;
}

/* ͼ��߶Ȳ��䣬ˮƽƽ�� */
void sheet_slide(struct SHEET *sht, int vx0, int vy0) 
{
	int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if (sht->height >= 0)
	{
		/*�ػ�ƽ��ǰ�ľֲ���ͼ */
		sheet_refreshmap(sht->ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);
		/*�ػ�ƽ�ƺ�ľֲ���ͼ */
		sheet_refreshmap(sht->ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);
		/* ˢ��ƽ��ǰ�ľֲ����� */
		sheet_refreshsub(sht->ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 
			0, sht->height - 1);
		/* ˢ��ƽ�ƺ�ľֲ����� */
		sheet_refreshsub(sht->ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, 
			sht->height, sht->height);
	}
	return;
}

/* �ͷ���ʹ�õ�ͼ�� */
void sheet_free(struct SHEET *sht)
{
	if (sht->height >= 0)
	{
		sheet_updown(sht, -1);
	}
	sht->flags = 0;
	return;
}