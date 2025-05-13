#ifndef	_JLX12864G_H
#define	_JLX12864G_H

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif


#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h" 
#include "inc/hw_types.h" 
#include "driverlib/pin_map.h"  
#include "driverlib/sysctl.h"	



//用于连接JLX12864液晶屏和TM4C1294的管脚定义，使用GPIO连接LCD驱动IC接口、字库IC接口
#define LCD_CS_H GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_PIN_4)		//LCD_CS <--> PB4
#define LCD_CS_L GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, 0)
#define LCD_SCK_H GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, GPIO_PIN_5)		//LCD_SCK <--> PB5
#define LCD_SCK_L GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0)
#define LCD_SDA_H GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_PIN_4)		//LCD_SDA <--> PE4
#define LCD_SDA_L GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, 0)
#define LCD_RST_H GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_0, GPIO_PIN_0)		//LCD_RST <--> PK0
#define LCD_RST_L GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_0, 0 )
#define LCD_RS_H GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_1, GPIO_PIN_1)		//LCD_RS <--> PK1
#define LCD_RS_L GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_1, 0)

#define LCD_ROM_IN_H GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_2, GPIO_PIN_2)		//ROM_IN <--> PK2
#define LCD_ROM_IN_L GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_2, 0)
#define LCD_ROM_OUT_H GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, GPIO_PIN_6)		//ROM_OUT <--> PC6
#define LCD_ROM_OUT_L GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0)
#define LCD_ROM_SCK_H GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, GPIO_PIN_5)		//ROM_SCK <--> PC5
#define LCD_ROM_SCK_L GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0)
#define LCD_ROM_CS_H GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_PIN_4)		//ROM_CS <--> PC4
#define LCD_ROM_CS_L GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0)

#define LCD_ROM_OUT GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_6)

#define TEST_H GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1)		//DELAY时间测试管脚
#define TEST_L GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0)


//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
extern void LCD_PORT_init(void);

//延时
extern void LCD_delay(int n_ms);

//短延时
extern void LCD_delay_us(int n_us); 
 
// 写指令到LCD模块
extern void transfer_command_lcd(int data1);

// 写数据到LCD模块
extern void transfer_data_lcd(int data1);

//LCD模块初始化
extern void LCD_Init(void);

extern void lcd_address(unsigned int page,unsigned int column); 

//全屏清屏
extern void clear_screen(void);
 
//黑屏显示
extern void full_display(void); 

//显示128x64 点阵图像
extern void display_128x64(unsigned char *dp);

//显示32x32 点阵图像、ASCII, 或32x32 点阵的自造字符、其他图标
extern void display_graphic_32x32(unsigned char page,unsigned char column,unsigned char *dp); 

//显示16x16 点阵图像、汉字、生僻字或16x16 点阵的其他图标
extern void display_graphic_16x16(unsigned char page,unsigned char column,unsigned char *dp); 

//显示8x16 点阵图像、ASCII, 或8x16 点阵的自造字符、其他图标
extern void display_graphic_8x16(unsigned char page,unsigned char column,unsigned char *dp); 

//送指令到晶联讯字库IC
extern void send_command_to_ROM(unsigned char datu);

//从晶联讯字库IC 中取汉字或字符数据（1 个字节）
//extern static unsigned char get_data_from_ROM();

//从指定地址读出数据写到液晶屏指定（page,column)座标中
extern void get_and_write_16x16(unsigned long fontaddr,unsigned char page,unsigned char column,unsigned char inverse);

//从指定地址读出数据写到液晶屏指定（page,column)座标中
extern void get_and_write_8x16(unsigned long fontaddr,unsigned char page,unsigned char column,unsigned char inverse);

//在第 page行，第column列，显示一串 16x16 点阵汉字或 8x16 的 ASCII 字，是否反白显示
extern void display_GB2312_string(unsigned char page,unsigned char column,unsigned char *text,unsigned char inverse);

extern void display_string_5x8(unsigned char page,unsigned char column,unsigned char *text,unsigned char inverse);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __TM1638_H__

