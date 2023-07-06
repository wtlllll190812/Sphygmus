#ifndef _IO_BIT_H_
#define _IO_BIT_H_
 
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr&0xFFFFF)<<5)+(bitnum<<2))
#define MEM_ADDR(addr) *((volatile unsigned long *)(addr))
#define BIT_ADDR(addr, bitnum) MEM_ADDR(BITBAND(addr, bitnum))
//F103XXIO �ڵ�ַӳ��
#define GPIOA_ODR_Addr (GPIOA_BASE+0x0C) //0x40020014
#define GPIOB_ODR_Addr (GPIOB_BASE+0x0C) //0x40020414 
#define GPIOC_ODR_Addr (GPIOC_BASE+0x0C) //0x40020814
#define GPIOD_ODR_Addr (GPIOD_BASE+0x0C) //0x40020C14
#define GPIOE_ODR_Addr (GPIOE_BASE+0x0C) //0x40021014
#define GPIOA_IDR_Addr (GPIOA_BASE+0x08) //0x40020010
#define GPIOB_IDR_Addr (GPIOB_BASE+0x08) //0x40020410
#define GPIOC_IDR_Addr (GPIOC_BASE+0x08) //0x40020810
#define GPIOD_IDR_Addr (GPIOD_BASE+0x08) //0x40020C10
#define GPIOE_IDR_Addr (GPIOE_BASE+0x08) //0x40021010

//F103XXIO �ڲ���,ֻ�Ե�һ��IO ��,n ��ֵ��Χ[0-15]
#define PAout(n) 		BIT_ADDR(GPIOA_ODR_Addr,n) //GPIOA ĳһλ���
#define PAin(n) 		BIT_ADDR(GPIOA_IDR_Addr,n) //GPIOA ĳһλ����
#define PBout(n) 		BIT_ADDR(GPIOB_ODR_Addr,n) //GPIOB ĳһλ���
#define	PBin(n) 		BIT_ADDR(GPIOB_IDR_Addr,n) //GPIOB ĳһλ����
#define PCout(n)		BIT_ADDR(GPIOC_ODR_Addr,n) //GPIOC ĳһλ���
#define PCin(n) 		BIT_ADDR(GPIOC_IDR_Addr,n) //GPIOC ĳһλ����
#define PDout(n) 		BIT_ADDR(GPIOD_ODR_Addr,n) //GPIOD ĳһλ���
#define PDin(n) 		BIT_ADDR(GPIOD_IDR_Addr,n) //GPIOD ĳһλ����
#define PEout(n) 		BIT_ADDR(GPIOE_ODR_Addr,n) //GPIOE ĳһλ���
#define PEin(n) 		BIT_ADDR(GPIOE_IDR_Addr,n) //GPIOE ĳһλ����


#endif

