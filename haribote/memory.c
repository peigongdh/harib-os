#include "bootpack.h"

#define	EFLAGS_AC_BIT		0x00040000
#define	CRO_CACHE_DISABLE	0x60000000

/* ��486��EFLAGS�Ĵ����ĵ�18λ��AC��־λ������386�в����� */
/* ����ʶ��1д����һλ���ٶ���EFLAGS��ֵ���̶����AC��־λ�Ƿ���Ϊ1 */
/* ����ȷ��CPU��386����486���ϵ� */
unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;
	eflg = io_load_eflags();
	/* ac-bit = 1 */
	eflg |= EFLAGS_AC_BIT;
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	/* �����386����ʹ�趨AC=1��AC��ֵ�����Զ��ص�0 */
	if ((eflg & EFLAGS_AC_BIT) != 0)
	{
		flg486 = 1;
	}
	/* ��ԭ��־λ */ 
	eflg &= ~EFLAGS_AC_BIT;
	io_store_eflags(eflg);
	
	if (flg486 != 0)
	{
		cr0 = load_cr0();
		/* ��ֹ���� */
		cr0 |= CRO_CACHE_DISABLE;
		store_cr0(cr0);
	}
	
	i = memtest_sub(start, end);
	
	if (flg486 != 0)
	{
		cr0 = load_cr0();
		/* ������ */
		cr0 &= ~CRO_CACHE_DISABLE;
		store_cr0(cr0);
	}
	
	return i;
}

/* �ڴ洦���ʼ�� */
void memman_init(struct MEMMAN *man)
{
	man->frees = 0;		/* ������Ϣ��Ŀ */
	man->maxfrees = 0;	/* ���ڹ۲����״����frees�����ֵ */
	man->lostsize = 0;	/* �ͷ�ʧ�ܵ��ڴ�Ĵ�С�ܺ� */ 
	man->losts = 0;		/* �ͷ�ʧ�ܴ��� */
	return;
}

/* ��������ڴ��С�ĺϼ� */
unsigned int memman_total(struct MEMMAN *man)
{
	unsigned int i, t = 0;
	for (i = 0; i < man->frees; i ++)
	{
		t += man->free[i].size;
	}
	return t;
}

/* �����ڴ� */
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
	unsigned int i, a;
	for (i = 0; i < man->frees; i ++)
	{
		/* ���ҵ����㹻����ڴ� */
		if (man->free[i].size >= size)
		{
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			/* ���free[i]�����0���ͼ���һ��������Ϣ */
			if (man->free[i].size == 0)
			{
				man->frees --;
				for (; i < man->frees; i ++)
				{
					man->free[i] = man->free[i + 1];
				}
			}
			return a;
		}
	}
	return 0;
}

/* �ͷ��ڴ� */
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i, j;
	/* Ϊ�˱��ڹ����ڴ棬��free[]����addr��˳������ */
	/* ���ԣ��Ⱦ���Ӧ�÷������� */
	for (i = 0; i < man->frees; i ++)
	{
		if (man->free[i].addr > addr)
		{
			break;
		}
	}
	if (i > 0)
	{
		/* ǰ���п����ڴ� */
		if (man->free[i - 1].addr + man->free[i - 1].size == addr)
		{
			/* ������ǰ��Ŀ����ڴ���ɵ�һ�� */
			man->free[i - 1].size += size;
			if (i < man->frees)
			{
				/* ����Ҳ�� */
				if (addr + size == man->free[i].addr)
				{
					man->free[i - 1].size += man->free[i].size;
					/* man->free[i]ɾ�� */
					/* free[i]���0����ɵ�ǰ��ȥ */
					man->frees --;
					for (; i < man->frees; i ++)
					{
/* man->free[i]�Ƿ����Խ�磿�����ܣ���Ϊman->frees -- */
						man->free[i] = man->free[i + 1];
					}
				}
			}
			/* �ɹ���� */
			return 0;
		}
	}
	/* ������ǰ��Ŀ��ÿռ���ɵ�һ�� */
	if (i < man->frees)
	{
		/* ���滹�� */
		if (addr + size == man->free[i].addr)
		{
			/* �������������ݹ��ɵ�һ�� */
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;
		}
	}
	/* �Ȳ�����ǰ����ɵ�һ��Ҳ�����������ɵ�һ�� */
	if (man->frees < MEMMAN_FREES)
	{
		/* free[i]֮��ģ�����ƶ����ڳ�һ����ÿռ� */
		for (j = man->frees; j > i; j --)
		{
			man->free[j] = man->free[j - 1];
		}
		man->frees ++;
		if (man->maxfrees < man->frees)
		{
			/* �������ֵ */
			man->maxfrees = man->frees;
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;
	}
	/* ���������ƶ� */
	man->losts ++;
	man->lostsize += size;
	/* ʧ�� */
	return -1;
}

/* ��4KB�ֽ�Ϊ��λ�����ڴ� */
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
	unsigned int a;
	size = (size + 0xfff) & 0xfffff000;
	a = memman_alloc(man, size);
	return a;
}

/* ��4KB�ֽ�Ϊ��λ�ͷ��ڴ� */
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memman_free(man, addr, size);
	return i;
}

