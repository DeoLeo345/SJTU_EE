//*****************************************************************************
//
// 头文件
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"        // 基址宏定义
#include "inc/hw_types.h"         // 数据类型宏定义，寄存器访问函数
#include "inc/hw_ints.h" 
#include "driverlib/debug.h"      // 调试用
#include "driverlib/gpio.h"       // 通用IO口宏定义
#include "driverlib/pin_map.h"    // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/sysctl.h"			// 系统控制定义
#include "driverlib/systick.h"    // SysTick Driver 原型
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver 原型
#include "driverlib/uart.h"
#include "driverlib/timer.h"
#include "driverlib/adc.h"        // 与ADC有关的定义 
#include "tm1638.h"               // 与控制TM1638芯片有关的函数
#include "JLX12864G.h"
#include "PWM.h"
#include "uifunctionexp.c"
#include "ADC.h"
#include "LM75BD.h"               // 与控制LM75BD芯片有关的函数

//*****************************************************************************
//
// 宏定义
//
//*****************************************************************************
#define SYSTICK_FREQUENCY		50	 // SysTick频率为50Hz，即循环定时周期20ms

#define V_T40ms	 2               // 40ms软件定时器溢出值，2个20ms
#define V_T100ms	5              // 0.1s软件定时器溢出值，5个20ms
#define V_T500ms	25             // 0.5s软件定时器溢出值，25个20ms
#define V_T1s	50                 // 1s软件定时器溢出值，50个20ms
#define V_T2s		100							 // 2s软件定时器溢出值，100个20ms
#define V_T3s   150							 // 3s软件定时器溢出值，150个20ms
#define V_T10s   500             // 10s软件定时器溢出值，500个20ms

//*****************************************************************************
//
// 函数原型声明
//
//*****************************************************************************
void GPIOInit(void);        // GPIO初始化
void SysTickInit(void);     // 设置SysTick中断 
void UARTInit(void);				// UART初始化
void UARTStringPut(uint32_t ui32Base, const char* cMessage);		// UART用到的函数
void DevicesInit(void);     // MCU器件初始化，注：会调用上述函数

//*****************************************************************************
//
// 变量定义
//
//*****************************************************************************

// 软件定时器计数
uint8_t clock40ms = 0;
uint8_t clock100ms = 0;
uint8_t clock500ms = 0;
uint8_t clock1s = 0;
uint16_t clock2s = 0;
uint16_t clock3s = 0;
uint16_t clock10s = 0;

// 软件定时器溢出标志
uint8_t clock40ms_flag = 0;
uint8_t clock100ms_flag = 0;
uint8_t clock500ms_flag = 0;
uint8_t clock1s_flag = 0;
uint8_t clock3s_flag = 0;
uint8_t NOKEY_clock2s_flag = 0;		// 用来作为显示“载频超范围”的定时
uint8_t NOKEY_clock10s_flag = 0;	// 用来作为取消反白的定时

// 测试用计数器
uint32_t test_counter = 0;				// 用来辅助稳定ADC模块测出来的电压

// 8位数码管显示的数字或字母符号
// 注：板上数码位从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t digit[8]={' ',' ',' ',' ',' ',' ',' ',' '};			// 虽然没什么用但是建议留着，因为温度测量模块里面多次用到了digit

// 8位小数点 1亮  0灭
// 注：板上数码位小数点从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t pnt = 0x44;		// 与digit同理，建议留着

// 8个LED指示灯状态，0灭，1亮
// 注：板上指示灯从左到右序号排列为7、6、5、4、3、2、1、0
//     对应元件LED8、LED7、LED6、LED5、LED4、LED3、LED2、LED1
uint8_t led[] = {1, 1, 1, 1, 1, 1, 1, 0};

// 当前按键值
uint8_t key_code = 0;

// 用于记录前一次按键检测时的键盘状态，0表示无键按下，1有键按下
volatile uint8_t key_state = 0;

// 按键操作有效标记，0代表无新操作，1代表有键操作
volatile uint8_t  key_flag = 0;

uint8_t  m107_flag = 0;		// 用来判断液晶屏状态是否是107（也就是确定反白）

// 系统时钟频率 
uint32_t g_ui32SysClock;	// 无需多言

// AIN2(PE1)  ADC采样值[0-4095]
uint32_t ui32ADC0Value;     

// AIN2电压值(单位为0.01V) [0.00-3.30]
double ui32ADC0Voltage;
double aveADC0Voltage;
uint32_t sumADC0Value;
//电压值定义
uint8_t volt[]={"3.300"};		// 设置初始值

// 返回温度值 1LSB=0.1℃
int16_t i16Temperature1;      
int16_t i16Temperature2;     
uint8_t temper[] = {"25.0 30.5"}; 	// 设置初始值

uint8_t ui8DigitRefresh = 0;  

char freqs[]={"AT+FRE=1080\r\n"};		// 设置初始值

//*****************************************************************************
//
// 主程序
//
//*****************************************************************************
int main(void)
{
	uint32_t ui32Freq = 3000;  			// 方波频率，这里是先进行一个初始化
	uint16_t ui16Temp1,ui16Temp2;		// 温度测量模块里面的变量
	
	DevicesInit();            // MCU器件初始化

	while (clock100ms < 3);   // 延时>60ms,等待TM1638上电完成
	TM1638_Init(); 						// 初始化
  LCD_Init();								// 初始化
	initial_act();						// 初始化

	//发射端FM模块，"\r\n"是必要的
	SysCtlDelay(200 * (g_ui32SysClock / 3000));  
  UARTStringPut(UART4_BASE, "AT+FRE=845\r\n ");  
  SysCtlDelay(200 * (g_ui32SysClock / 3000));  
  UARTStringPut(UART4_BASE, "AT+VOL=30\r\n");
	SysCtlDelay(200 * (g_ui32SysClock / 3000));  
  UARTStringPut(UART4_BASE, "AT+CAMPUS=1\r\n");		// 开校园广播模式，我也不知道有没有用QAQ
	
	while (1)
	{
		if (clock100ms_flag == 1)   // 检查0.1秒定时是否到
		{
			clock100ms_flag		= 0;
		}

		if (clock500ms_flag == 1)   // 检查0.5秒定时是否到
		{
			clock500ms_flag = 0;
		}
		
		// ADC采样部分
		if (clock40ms_flag == 1)    // 检查40ms秒定时是否到
    {
      clock40ms_flag = 0;
            
      ui32ADC0Value = ADC_Sample();   // 采样   
			
			// 接下来是通过计算计数24次然后取平均值的方式来让读取到的电压值更加稳定
			if(++test_counter >= 25)
			{
				test_counter = 0;
				aveADC0Voltage = sumADC0Value / 24 * 330 / 4095.0;
				sumADC0Value = 0;
			}
			else
			{
				sumADC0Value += ui32ADC0Value;
			}
			
			ui32ADC0Voltage = ui32ADC0Value * 330 / 4095.0;		// 注意在4095后加".0"实现强制转换
            
      volt[0] = (int)(aveADC0Voltage / 100) % 10+'0'; // 显示电压值个位数
      volt[2] = (int)(aveADC0Voltage / 10) % 10+'0';  // 显示电压值十分位数
      volt[3] = (int)aveADC0Voltage % 10+'0';         // 显示电压值百分位数 
			volt[4] = (int)(aveADC0Voltage*10) %10+'0';			// 显示电压值千分位数 
			if(ui_state==000) display_GB2312_string(1,5*8+1,volt,0);	// 当状态机状态为000的时候显示
    }
		
		// 温度测量与显示模块
		if (clock3s_flag == 1)      // 检查3秒定时是否到
		{
			clock3s_flag = 0;					// 清标志
			if(ui_state==000)					// 当状态机状态为000的时候显示
			{
				if(temper[0] == '0')	
					temper[0] = ' ';
				display_GB2312_string(3,5*8+1,temper,0);
				if(temper[5] == '0')	
					temper[5] = ' ';
				display_GB2312_string(3,5*8+1,temper,0);
			}
                
      i16Temperature1 = GetTemputerature(LM75BD_ADR2);               
      i16Temperature2 = GetTemputerature(LM75BD_ADR1);    // 这一段完全没变，都是示例代码
      
			ui8DigitRefresh = 1;
			
			if(i16Temperature1 < 0)
      {
        ui16Temp1 = -i16Temperature1;
        digit[0] = '-';
      }    
      else
      {
        ui16Temp1 = i16Temperature1;
        digit[0] = ' ';
      }
           
			temper[0] = ui16Temp1 / 100+'0';      	// 计算十位数
			temper[1] = ui16Temp1 / 10 % 10+'0'; 		// 计算个位数
			temper[3] = ui16Temp1 % 10+'0';         // 计算十分位数
						
			if(i16Temperature2 < 0)
      {
        ui16Temp2 = -i16Temperature2;
        digit[4] = '-';
      }    
      else
      {
        ui16Temp2 = i16Temperature2;
        digit[4] = ' ';
      }
				
			temper[5] = ui16Temp2 / 100+'0';      	// 计算十位数
			temper[6] = ui16Temp2 / 10 % 10+'0'; 		// 计算个位数
			temper[8] = ui16Temp2 % 10+'0';         // 计算十分位数
			
      ui8DigitRefresh = 0;
			PWMStart(ui32Freq);
    }
		
		ui32Freq = 9.25*ui16Temp1+300;	// 选择一路温度然后计算出相应的发射频率

		ui_state_proc(ui_state);
	}	 
}



//*****************************************************************************
//
// 函数原型：void GPIOInit(void)
// 函数功能：GPIO初始化。使能PortK，设置PK4,PK5为输出；使能PortM，设置PM0为输出。
//          （PK4连接TM1638的STB，PK5连接TM1638的DIO，PM0连接TM1638的CLK）
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void GPIOInit(void)
{
	// 配置TM1638芯片管脚
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);						// 使能端口 K	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)){};		// 等待端口 K准备完毕		
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);						// 使能端口 M	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)){};		// 等待端口 M准备完毕		
	
  // 设置端口 K的第4,5位（PK4,PK5）为输出引脚		PK4-STB  PK5-DIO
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4|GPIO_PIN_5);
	// 设置端口 M的第0位（PM0）为输出引脚   PM0-CLK
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);	
		
	// 配置JLX12864G屏幕管脚
	LCD_PORT_init();
}

//*****************************************************************************
// 
// 函数原型：SysTickInit(void)
// 函数功能：设置SysTick中断
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void SysTickInit(void)
{
	SysTickPeriodSet(g_ui32SysClock/SYSTICK_FREQUENCY); // 设置心跳节拍,定时周期20ms
	SysTickEnable();  			// SysTick使能
	SysTickIntEnable();			// SysTick中断允许
}

//*****************************************************************************
// 
// 函数原型：DevicesInit(void)
// 函数功能：CU器件初始化，包括系统时钟设置、GPIO初始化和SysTick中断设置
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void DevicesInit(void)
{
	// 使用外部25MHz主时钟源，经过PLL，然后分频为20MHz
	g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | 
	                                   SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 
	                                   20000000);
	FPULazyStackingEnable(); 
  FPUEnable();

	GPIOInit();             // GPIO初始化
	I2C0Init();             // I2C0初始化
	ADCInit();              // ADC初始化
	PWMInit();              // PWM初始化 
  SysTickInit();          // 设置SysTick中断
	UARTInit();             // UART初始化
  IntMasterEnable();			// 总中断允许
}

//***************************************************************************** 
// 
// 函数原型:void UARTStringPut(uint32_t ui32Base,const char *cMessage) 
// 函数功能:向UART模块发送字符串
// 函数参数:ui32Base:UART模块
//          cMessage:待发送字符串  
// 函数返回值：无 
// 
//***************************************************************************** 
void UARTStringPut(uint32_t ui32Base, const char* cMessage) 
{ 
    while (*cMessage != '\0') 
        UARTCharPut(ui32Base, *(cMessage++)); 
} 

void UARTInit(void) { 
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART4); 
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); 
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)); 
 
    GPIOPinConfigure(GPIO_PA2_U4RX); 
    GPIOPinConfigure(GPIO_PA3_U4TX); 
    
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_3); 
     
    UARTConfigSetExpClk(UART4_BASE, 
                        g_ui32SysClock, 
                        38400, 
                        (UART_CONFIG_WLEN_8 | 
                         UART_CONFIG_STOP_ONE | 
                         UART_CONFIG_PAR_NONE)); 
}

//*****************************************************************************
// 
// 函数原型：void SysTick_Handler(void)
// 函数功能：SysTick中断服务程序
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void SysTick_Handler(void)       // 定时周期为20ms
{
	// 40ms秒钟软定时器计数
	if (++clock40ms >= V_T40ms)
	{
		clock40ms_flag = 1; // 当40ms到时，溢出标志置1
		clock40ms = 0;
	}
	
	// 0.1秒钟软定时器计数
	if (++clock100ms >= V_T100ms)
	{
		clock100ms_flag = 1; // 当0.1秒到时，溢出标志置1
		clock100ms = 0;
	}
	
 	// 0.5秒钟软定时器计数
	if (++clock500ms >= V_T500ms)
	{
		clock500ms_flag = 1; // 当0.5秒到时，溢出标志置1
		clock500ms = 0;
	}
	
	// 0.1秒钟软定时器计数
	if (++clock1s >= V_T1s)
	{
		clock1s_flag = 1; // 当1秒到时，溢出标志置1
		clock1s = 0;
	}
	
	if (++clock3s >= V_T3s)
	{
		clock3s_flag = 1;
		clock3s = 0;
	}

	// 刷新全部数码管和LED指示灯
	if(ui8DigitRefresh == 0)
	TM1638_RefreshDIGIandLED(digit, pnt, led);

	// 检查当前键盘输入，0代表无键操作，1-9表示有对应按键
	// 键号显示在一位数码管上
	key_code = TM1638_Readkeyboard();
	
	// key_state用于记录前一次按键检测时的键盘状态，0表示无键按下，1有键按下
	switch (key_state)
	{
        case 0:   // 前一次按键检测时无键按下
            if (key_code > 0)   // 本次按键检测有键按下
            {
                key_state = 1;
                key_flag = 1;	
						
            }
		    break;
        case 1:    // 前一次按键检测时有键按下
            if (key_code == 0)  // 本次按键检测时无键按下
            {
                key_state = 0;
            }
            break;
        default:
            key_state = 0;
            break;
  }	
	
	ENTER_detect();
	DOWN_detect();
	UP_detect();
	INCREASE_detect();
	DECREASE_detect();
	
	//10秒定时
		if (key_code==0)    // 当没有按键按下时
	{
			if (++clock10s >= V_T10s)  	 	// 10秒钟软定时器计数
		{
			NOKEY_clock10s_flag = 1; 			// 当10秒到时，溢出标志置1
			clock10s = 0;
		}
		
		if (++clock2s >= V_T2s)  	 			// 2秒钟软定时器计数
		{
			NOKEY_clock2s_flag = 1; 			// 当2秒到时，溢出标志置1
			clock2s = 0;
		}
	}
	else                 // 当有按键按下时
	{
		clock10s = 0;
		clock2s = 0;
	}
	
}
