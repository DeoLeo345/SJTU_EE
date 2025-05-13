//*****************************************************************************
//
// ͷ�ļ�
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"        // ��ַ�궨��
#include "inc/hw_types.h"         // �������ͺ궨�壬�Ĵ������ʺ���
#include "inc/hw_ints.h" 
#include "driverlib/debug.h"      // ������
#include "driverlib/gpio.h"       // ͨ��IO�ں궨��
#include "driverlib/pin_map.h"    // TM4Cϵ��MCU��Χ�豸�ܽź궨��
#include "driverlib/sysctl.h"			// ϵͳ���ƶ���
#include "driverlib/systick.h"    // SysTick Driver ԭ��
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver ԭ��
#include "driverlib/uart.h"
#include "driverlib/timer.h"
#include "driverlib/adc.h"        // ��ADC�йصĶ��� 
#include "tm1638.h"               // �����TM1638оƬ�йصĺ���
#include "JLX12864G.h"
#include "PWM.h"
#include "uifunctionexp.c"
#include "ADC.h"
#include "LM75BD.h"               // �����LM75BDоƬ�йصĺ���

//*****************************************************************************
//
// �궨��
//
//*****************************************************************************
#define SYSTICK_FREQUENCY		50	 // SysTickƵ��Ϊ50Hz����ѭ����ʱ����20ms

#define V_T40ms	 2               // 40ms�����ʱ�����ֵ��2��20ms
#define V_T100ms	5              // 0.1s�����ʱ�����ֵ��5��20ms
#define V_T500ms	25             // 0.5s�����ʱ�����ֵ��25��20ms
#define V_T1s	50                 // 1s�����ʱ�����ֵ��50��20ms
#define V_T2s		100							 // 2s�����ʱ�����ֵ��100��20ms
#define V_T3s   150							 // 3s�����ʱ�����ֵ��150��20ms
#define V_T10s   500             // 10s�����ʱ�����ֵ��500��20ms

//*****************************************************************************
//
// ����ԭ������
//
//*****************************************************************************
void GPIOInit(void);        // GPIO��ʼ��
void SysTickInit(void);     // ����SysTick�ж� 
void UARTInit(void);				// UART��ʼ��
void UARTStringPut(uint32_t ui32Base, const char* cMessage);		// UART�õ��ĺ���
void DevicesInit(void);     // MCU������ʼ����ע���������������

//*****************************************************************************
//
// ��������
//
//*****************************************************************************

// �����ʱ������
uint8_t clock40ms = 0;
uint8_t clock100ms = 0;
uint8_t clock500ms = 0;
uint8_t clock1s = 0;
uint16_t clock2s = 0;
uint16_t clock3s = 0;
uint16_t clock10s = 0;

// �����ʱ�������־
uint8_t clock40ms_flag = 0;
uint8_t clock100ms_flag = 0;
uint8_t clock500ms_flag = 0;
uint8_t clock1s_flag = 0;
uint8_t clock3s_flag = 0;
uint8_t NOKEY_clock2s_flag = 0;		// ������Ϊ��ʾ����Ƶ����Χ���Ķ�ʱ
uint8_t NOKEY_clock10s_flag = 0;	// ������Ϊȡ�����׵Ķ�ʱ

// �����ü�����
uint32_t test_counter = 0;				// ���������ȶ�ADCģ�������ĵ�ѹ

// 8λ�������ʾ�����ֻ���ĸ����
// ע����������λ�������������Ϊ4��5��6��7��0��1��2��3
uint8_t digit[8]={' ',' ',' ',' ',' ',' ',' ',' '};			// ��Ȼûʲô�õ��ǽ������ţ���Ϊ�¶Ȳ���ģ���������õ���digit

// 8λС���� 1��  0��
// ע����������λС����������������Ϊ4��5��6��7��0��1��2��3
uint8_t pnt = 0x44;		// ��digitͬ����������

// 8��LEDָʾ��״̬��0��1��
// ע������ָʾ�ƴ������������Ϊ7��6��5��4��3��2��1��0
//     ��ӦԪ��LED8��LED7��LED6��LED5��LED4��LED3��LED2��LED1
uint8_t led[] = {1, 1, 1, 1, 1, 1, 1, 0};

// ��ǰ����ֵ
uint8_t key_code = 0;

// ���ڼ�¼ǰһ�ΰ������ʱ�ļ���״̬��0��ʾ�޼����£�1�м�����
volatile uint8_t key_state = 0;

// ����������Ч��ǣ�0�������²�����1�����м�����
volatile uint8_t  key_flag = 0;

uint8_t  m107_flag = 0;		// �����ж�Һ����״̬�Ƿ���107��Ҳ����ȷ�����ף�

// ϵͳʱ��Ƶ�� 
uint32_t g_ui32SysClock;	// �������

// AIN2(PE1)  ADC����ֵ[0-4095]
uint32_t ui32ADC0Value;     

// AIN2��ѹֵ(��λΪ0.01V) [0.00-3.30]
double ui32ADC0Voltage;
double aveADC0Voltage;
uint32_t sumADC0Value;
//��ѹֵ����
uint8_t volt[]={"3.300"};		// ���ó�ʼֵ

// �����¶�ֵ 1LSB=0.1��
int16_t i16Temperature1;      
int16_t i16Temperature2;     
uint8_t temper[] = {"25.0 30.5"}; 	// ���ó�ʼֵ

uint8_t ui8DigitRefresh = 0;  

char freqs[]={"AT+FRE=1080\r\n"};		// ���ó�ʼֵ

//*****************************************************************************
//
// ������
//
//*****************************************************************************
int main(void)
{
	uint32_t ui32Freq = 3000;  			// ����Ƶ�ʣ��������Ƚ���һ����ʼ��
	uint16_t ui16Temp1,ui16Temp2;		// �¶Ȳ���ģ������ı���
	
	DevicesInit();            // MCU������ʼ��

	while (clock100ms < 3);   // ��ʱ>60ms,�ȴ�TM1638�ϵ����
	TM1638_Init(); 						// ��ʼ��
  LCD_Init();								// ��ʼ��
	initial_act();						// ��ʼ��

	//�����FMģ�飬"\r\n"�Ǳ�Ҫ��
	SysCtlDelay(200 * (g_ui32SysClock / 3000));  
  UARTStringPut(UART4_BASE, "AT+FRE=845\r\n ");  
  SysCtlDelay(200 * (g_ui32SysClock / 3000));  
  UARTStringPut(UART4_BASE, "AT+VOL=30\r\n");
	SysCtlDelay(200 * (g_ui32SysClock / 3000));  
  UARTStringPut(UART4_BASE, "AT+CAMPUS=1\r\n");		// ��У԰�㲥ģʽ����Ҳ��֪����û����QAQ
	
	while (1)
	{
		if (clock100ms_flag == 1)   // ���0.1�붨ʱ�Ƿ�
		{
			clock100ms_flag		= 0;
		}

		if (clock500ms_flag == 1)   // ���0.5�붨ʱ�Ƿ�
		{
			clock500ms_flag = 0;
		}
		
		// ADC��������
		if (clock40ms_flag == 1)    // ���40ms�붨ʱ�Ƿ�
    {
      clock40ms_flag = 0;
            
      ui32ADC0Value = ADC_Sample();   // ����   
			
			// ��������ͨ���������24��Ȼ��ȡƽ��ֵ�ķ�ʽ���ö�ȡ���ĵ�ѹֵ�����ȶ�
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
			
			ui32ADC0Voltage = ui32ADC0Value * 330 / 4095.0;		// ע����4095���".0"ʵ��ǿ��ת��
            
      volt[0] = (int)(aveADC0Voltage / 100) % 10+'0'; // ��ʾ��ѹֵ��λ��
      volt[2] = (int)(aveADC0Voltage / 10) % 10+'0';  // ��ʾ��ѹֵʮ��λ��
      volt[3] = (int)aveADC0Voltage % 10+'0';         // ��ʾ��ѹֵ�ٷ�λ�� 
			volt[4] = (int)(aveADC0Voltage*10) %10+'0';			// ��ʾ��ѹֵǧ��λ�� 
			if(ui_state==000) display_GB2312_string(1,5*8+1,volt,0);	// ��״̬��״̬Ϊ000��ʱ����ʾ
    }
		
		// �¶Ȳ�������ʾģ��
		if (clock3s_flag == 1)      // ���3�붨ʱ�Ƿ�
		{
			clock3s_flag = 0;					// ���־
			if(ui_state==000)					// ��״̬��״̬Ϊ000��ʱ����ʾ
			{
				if(temper[0] == '0')	
					temper[0] = ' ';
				display_GB2312_string(3,5*8+1,temper,0);
				if(temper[5] == '0')	
					temper[5] = ' ';
				display_GB2312_string(3,5*8+1,temper,0);
			}
                
      i16Temperature1 = GetTemputerature(LM75BD_ADR2);               
      i16Temperature2 = GetTemputerature(LM75BD_ADR1);    // ��һ����ȫû�䣬����ʾ������
      
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
           
			temper[0] = ui16Temp1 / 100+'0';      	// ����ʮλ��
			temper[1] = ui16Temp1 / 10 % 10+'0'; 		// �����λ��
			temper[3] = ui16Temp1 % 10+'0';         // ����ʮ��λ��
						
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
				
			temper[5] = ui16Temp2 / 100+'0';      	// ����ʮλ��
			temper[6] = ui16Temp2 / 10 % 10+'0'; 		// �����λ��
			temper[8] = ui16Temp2 % 10+'0';         // ����ʮ��λ��
			
      ui8DigitRefresh = 0;
			PWMStart(ui32Freq);
    }
		
		ui32Freq = 9.25*ui16Temp1+300;	// ѡ��һ·�¶�Ȼ��������Ӧ�ķ���Ƶ��

		ui_state_proc(ui_state);
	}	 
}



//*****************************************************************************
//
// ����ԭ�ͣ�void GPIOInit(void)
// �������ܣ�GPIO��ʼ����ʹ��PortK������PK4,PK5Ϊ�����ʹ��PortM������PM0Ϊ�����
//          ��PK4����TM1638��STB��PK5����TM1638��DIO��PM0����TM1638��CLK��
// ������������
// ��������ֵ����
//
//*****************************************************************************
void GPIOInit(void)
{
	// ����TM1638оƬ�ܽ�
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);						// ʹ�ܶ˿� K	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)){};		// �ȴ��˿� K׼�����		
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);						// ʹ�ܶ˿� M	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)){};		// �ȴ��˿� M׼�����		
	
  // ���ö˿� K�ĵ�4,5λ��PK4,PK5��Ϊ�������		PK4-STB  PK5-DIO
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4|GPIO_PIN_5);
	// ���ö˿� M�ĵ�0λ��PM0��Ϊ�������   PM0-CLK
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);	
		
	// ����JLX12864G��Ļ�ܽ�
	LCD_PORT_init();
}

//*****************************************************************************
// 
// ����ԭ�ͣ�SysTickInit(void)
// �������ܣ�����SysTick�ж�
// ������������
// ��������ֵ����
//
//*****************************************************************************
void SysTickInit(void)
{
	SysTickPeriodSet(g_ui32SysClock/SYSTICK_FREQUENCY); // ������������,��ʱ����20ms
	SysTickEnable();  			// SysTickʹ��
	SysTickIntEnable();			// SysTick�ж�����
}

//*****************************************************************************
// 
// ����ԭ�ͣ�DevicesInit(void)
// �������ܣ�CU������ʼ��������ϵͳʱ�����á�GPIO��ʼ����SysTick�ж�����
// ������������
// ��������ֵ����
//
//*****************************************************************************
void DevicesInit(void)
{
	// ʹ���ⲿ25MHz��ʱ��Դ������PLL��Ȼ���ƵΪ20MHz
	g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | 
	                                   SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 
	                                   20000000);
	FPULazyStackingEnable(); 
  FPUEnable();

	GPIOInit();             // GPIO��ʼ��
	I2C0Init();             // I2C0��ʼ��
	ADCInit();              // ADC��ʼ��
	PWMInit();              // PWM��ʼ�� 
  SysTickInit();          // ����SysTick�ж�
	UARTInit();             // UART��ʼ��
  IntMasterEnable();			// ���ж�����
}

//***************************************************************************** 
// 
// ����ԭ��:void UARTStringPut(uint32_t ui32Base,const char *cMessage) 
// ��������:��UARTģ�鷢���ַ���
// ��������:ui32Base:UARTģ��
//          cMessage:�������ַ���  
// ��������ֵ���� 
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
// ����ԭ�ͣ�void SysTick_Handler(void)
// �������ܣ�SysTick�жϷ������
// ������������
// ��������ֵ����
//
//*****************************************************************************
void SysTick_Handler(void)       // ��ʱ����Ϊ20ms
{
	// 40ms������ʱ������
	if (++clock40ms >= V_T40ms)
	{
		clock40ms_flag = 1; // ��40ms��ʱ�������־��1
		clock40ms = 0;
	}
	
	// 0.1������ʱ������
	if (++clock100ms >= V_T100ms)
	{
		clock100ms_flag = 1; // ��0.1�뵽ʱ�������־��1
		clock100ms = 0;
	}
	
 	// 0.5������ʱ������
	if (++clock500ms >= V_T500ms)
	{
		clock500ms_flag = 1; // ��0.5�뵽ʱ�������־��1
		clock500ms = 0;
	}
	
	// 0.1������ʱ������
	if (++clock1s >= V_T1s)
	{
		clock1s_flag = 1; // ��1�뵽ʱ�������־��1
		clock1s = 0;
	}
	
	if (++clock3s >= V_T3s)
	{
		clock3s_flag = 1;
		clock3s = 0;
	}

	// ˢ��ȫ������ܺ�LEDָʾ��
	if(ui8DigitRefresh == 0)
	TM1638_RefreshDIGIandLED(digit, pnt, led);

	// ��鵱ǰ�������룬0�����޼�������1-9��ʾ�ж�Ӧ����
	// ������ʾ��һλ�������
	key_code = TM1638_Readkeyboard();
	
	// key_state���ڼ�¼ǰһ�ΰ������ʱ�ļ���״̬��0��ʾ�޼����£�1�м�����
	switch (key_state)
	{
        case 0:   // ǰһ�ΰ������ʱ�޼�����
            if (key_code > 0)   // ���ΰ�������м�����
            {
                key_state = 1;
                key_flag = 1;	
						
            }
		    break;
        case 1:    // ǰһ�ΰ������ʱ�м�����
            if (key_code == 0)  // ���ΰ������ʱ�޼�����
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
	
	//10�붨ʱ
		if (key_code==0)    // ��û�а�������ʱ
	{
			if (++clock10s >= V_T10s)  	 	// 10������ʱ������
		{
			NOKEY_clock10s_flag = 1; 			// ��10�뵽ʱ�������־��1
			clock10s = 0;
		}
		
		if (++clock2s >= V_T2s)  	 			// 2������ʱ������
		{
			NOKEY_clock2s_flag = 1; 			// ��2�뵽ʱ�������־��1
			clock2s = 0;
		}
	}
	else                 // ���а�������ʱ
	{
		clock10s = 0;
		clock2s = 0;
	}
	
}
