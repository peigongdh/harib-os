/* GDT,IDT初始化 */ 

#include "bootpack.h"

/* 初始化GDT,IDT */ 
void init_gdtidt(void)
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
	struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *)ADR_IDT;
	int i;
	
	/* GDT(global segment descriptor table)-全局段号记录表初始化 */
	for (i = 0; i <= LIMIT_GDT / 8; i ++)
	{
		set_segmdesc(gdt + i, 0, 0, 0);
	} 
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);
	set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER);
	/* 将指定段上限和地址赋值给GDTR寄存器 */ 
	load_gdtr(LIMIT_GDT, ADR_GDT);
	
	/* IDT(interrupt descriptor table)-中断记录表初始化 */
	for (i = 0; i <= LIMIT_IDT / 8; i ++)
	{
		set_gatedesc(idt + i, 0, 0, 0);
	}
	/* 将指定段上限和地址赋值给IDTR寄存器 */  
	load_idtr(LIMIT_IDT, ADR_IDT);
	
	/* IDT的设定 */ 
	/* 键盘、鼠标等外部设备的控制，以及异常处理的中断，禁止应用程序调用 */
	
	/* 异常中断设定 */
	set_gatedesc(idt + 0x0c, (int)asm_inthandler0c, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x0d, (int)asm_inthandler0d, 2 * 8, AR_INTGATE32);
	/* 键盘、鼠标等外部设备的控制中断 */ 
	set_gatedesc(idt + 0x20, (int)asm_inthandler20, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x21, (int)asm_inthandler21, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x27, (int)asm_inthandler27, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x2c, (int)asm_inthandler2c, 2 * 8, AR_INTGATE32);
	/* 仅对INT0x40提供可供应用程序作为API来调用的中断 */
	set_gatedesc(idt + 0x40, (int)asm_hrb_api, 2 * 8, AR_INTGATE32 + 0x60);
	
	return;
}


void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	if (limit > 0xfffff)
	{
		ar |= 0x8000;	/* G_bit = 1 */
		limit /= 0x1000;
	}
	sd->limit_low = limit & 0xffff;
	sd->base_low = base & 0xffff;
	sd->base_mid = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high = (base >> 24) & 0xff;
	return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low = offset & 0xffff;
	gd->selector = selector;
	gd->dw_count = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high = (offset >> 16) & 0xffff;
	return;
}

