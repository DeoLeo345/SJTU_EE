//单界面菜单示例，反白表示游标位置
//可由底板3*3按键控制
//   ⑴   ⑵   ⑶
//   ⑷   ⑸   ⑹
//   ⑺   ⑻   ⑼
// ⑸表示enter
// ⑷⑹控制光标位置
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h" 
#include "inc/hw_types.h" 
#include "driverlib/pin_map.h"  
#include "driverlib/sysctl.h"	
#include "JLX12864G.h"

#define KEYTMR_OF  10
extern uint32_t g_ui32SysClock;
extern uint8_t key_code;
extern uint8_t clock1s_flag;
extern uint8_t NOKEY_clock2s_flag;
extern uint8_t NOKEY_clock10s_flag;
extern uint8_t m107_flag;
extern char freqs[];
extern void UARTStringPut(uint32_t ui32Base, const char* cMessage);

struct struct_act
{
	unsigned char num;
	unsigned char *str[20];
	unsigned char x[20],y[20],inverse[20];//x:page,y:column,inverse:反白
} a[4];//定义了一个a0结构体
struct struct_act *act[4];//定义了一个指针

unsigned char a0_s0[]="电压:";
unsigned char a0_s1[]="温度:";
unsigned char a0_s2[]="V";
unsigned char a0_s3[]="℃";


unsigned char a1_s0[]="载频：";
unsigned char a1_s1[]="1";
unsigned char a1_s2[]="0";
unsigned char a1_s3[]="1";
unsigned char a1_s4[]=".";
unsigned char a1_s5[]="7";
unsigned char a1_s6[]="MHz";
unsigned char a1_s7[]="确定";

unsigned char a2_s0[]="载频超范围！";
unsigned char a3_s0[]="请等待!";

int freqxxx_ = 1;
int freqxx_ = 0;
int freqx_ = 1;
int freq_x = 7;	


unsigned int ui_state=0;  //状态号

unsigned int key_ENTER_state=0;
unsigned int key_ENTER_prestate=0;
unsigned int ENTER_key_timer=0;
unsigned int key_ENTER_flag=0; 

unsigned int key_DOWN_state=0;
unsigned int key_DOWN_prestate=0;
unsigned int key_DOWN_timer=0;
unsigned int key_DOWN_flag=0; 

unsigned int key_UP_state=0;
unsigned int key_UP_prestate=0;
unsigned int key_UP_timer=0;
unsigned int key_UP_flag=0; 

unsigned int key_INCREASE_state=0;
unsigned int key_INCREASE_prestate=0;
unsigned int key_INCREASE_timer=0;
unsigned int key_INCREASE_flag=0; 

unsigned int key_DECREASE_state=0;
unsigned int key_DECREASE_prestate=0;
unsigned int key_DECREASE_timer=0;
unsigned int key_DECREASE_flag=0; 

void ENTER_detect(void)
{												
	switch(key_ENTER_state)
	{
		case 0:
			if(key_code==5)
			{key_ENTER_state=1; key_ENTER_flag=1;} break;
		case 1:
			if (key_code!=5)
			{key_ENTER_state=0;} break;
		default: {key_ENTER_state=0;} break;//默认key_ENTER_state是0，也就是没有被按下
			
	}
}

void DOWN_detect(void)
{
	if (key_code==6) ///////////////////	 DOWN也就是right
	{
		key_DOWN_prestate=key_DOWN_state;		
		key_DOWN_state=0;
		if (key_DOWN_prestate==1) key_DOWN_flag=1;
		
	}
	else
	{
		key_DOWN_prestate = key_DOWN_state; 
		key_DOWN_state=1;	
	}

}


void UP_detect(void)
{
	switch(key_UP_state)
	{
		case 0:
			if(key_code==4)
			{key_UP_state=1; key_UP_flag=1;} break;
		case 1:
			if (key_code!=4)
			{key_UP_state=0;} break;
		default: {key_UP_state=0;} break;
			
	}
}


void INCREASE_detect(void)
{
	if (key_code==2) ///////////////////	 INCREASE
	{
		key_INCREASE_prestate=key_INCREASE_state;		
		key_INCREASE_state=0;
		if (key_INCREASE_prestate==1) 
		{	key_INCREASE_flag=1;	key_INCREASE_timer =0;	}
		else if (key_INCREASE_prestate==0)
		{
			if 	(++key_INCREASE_timer>=KEYTMR_OF)
			{ key_INCREASE_flag=1; key_INCREASE_timer=0;}  
		}
	}
	else
	{
		key_INCREASE_prestate = key_INCREASE_state; 
		key_INCREASE_state=1;
		key_INCREASE_timer=0;	
	}
}

void DECREASE_detect(void)
{
	if (key_code==8) ///////////////////	 DECREASE	
	{
		key_DECREASE_prestate=key_DECREASE_state;		
		key_DECREASE_state=0;
		if (key_DECREASE_prestate==1) 
		{	key_DECREASE_flag=1;	key_DECREASE_timer =0;	}
		else if (key_DECREASE_prestate==0)
		{
			if 	(++key_DECREASE_timer>=KEYTMR_OF)
			{ key_DECREASE_flag=1; key_DECREASE_timer=0;}  
		}
	}
	else
	{
		key_DECREASE_prestate = key_DECREASE_state; 
		key_DECREASE_state=1;
		key_DECREASE_timer=0;	
	}
}


void display_ui_act(unsigned int i)
{		
	unsigned int j=0;
	clear_screen();

	for (j=0;j<act[i]->num;j++) 
	{
		display_GB2312_string(act[i]->x[j],(act[i]->y[j]-1)*8+1,act[i]->str[j],act[i]->inverse[j]);		
	}
}



void initial_act(void)
{
	// act000
	a[0].num=4;
	a[0].str[0]=a0_s0; a[0].x[0]=1;  a[0].y[0]=1;  a[0].inverse[0]=0; 
	a[0].str[1]=a0_s1; a[0].x[1]=3;  a[0].y[1]=1;  a[0].inverse[1]=0;	
	a[0].str[2]=a0_s2; a[0].x[2]=1;  a[0].y[2]=11;  a[0].inverse[2]=0; 
	a[0].str[3]=a0_s3; a[0].x[3]=3;  a[0].y[3]=15;  a[0].inverse[3]=0;
	act[0]=&a[0];

	display_ui_act(0);
	
	// act100
	a[1].num=8;
	a[1].str[0]=a1_s0; a[1].x[0]=1;  a[1].y[0]=1;  a[1].inverse[0]=0; 
	a[1].str[1]=a1_s1; a[1].x[1]=1;  a[1].y[1]=6;  a[1].inverse[1]=0;		
	a[1].str[2]=a1_s2; a[1].x[2]=1;  a[1].y[2]=7;  a[1].inverse[2]=0;
	a[1].str[3]=a1_s3; a[1].x[3]=1;  a[1].y[3]=8;  a[1].inverse[3]=0;
	a[1].str[4]=a1_s4; a[1].x[4]=1;  a[1].y[4]=9;  a[1].inverse[4]=0;
	a[1].str[5]=a1_s5; a[1].x[5]=1;  a[1].y[5]=10;  a[1].inverse[5]=0;
	a[1].str[6]=a1_s6; a[1].x[6]=1;  a[1].y[6]=11;  a[1].inverse[6]=0;
	a[1].str[7]=a1_s7; a[1].x[7]=7;  a[1].y[7]=13;  a[1].inverse[7]=0;
	act[1]=&a[1];

	// act200
	a[2].num=1;
	a[2].str[0]=a2_s0; a[2].x[0]=1;  a[2].y[0]=1;  a[2].inverse[0]=0; 	
	act[2]=&a[2];
	
	// act111		这是一个特殊的状态，起到一个延时的作用，并且会显示“请等待！”三个字
	a[3].num=1;
	a[3].str[0]=a3_s0; a[3].x[0]=1;  a[3].y[0]=1;  a[3].inverse[0]=0; 	
	act[3]=&a[3];

}

void ui_proc000(void)
{

	if(key_code!=0)// 任意键按下时
	{
		key_UP_flag=0;key_DOWN_flag=0;key_INCREASE_flag=0;key_DECREASE_flag=0;key_ENTER_flag=0;
		display_ui_act(1);
		act[1]->inverse[1]=1; 
		display_GB2312_string(act[1]->x[1],(act[1]->y[1]-1)*8+1,act[1]->str[1],act[1]->inverse[1]);
		ui_state=101;// 切换状态到101
	}
	if (clock1s_flag == 1)      // 检查1秒定时是否到
		{
			clock1s_flag		= 0;
		}

}

void ui_proc100(void)
{

	if(key_code!=0)// 任意键按下时
	{
		key_UP_flag=0;key_DOWN_flag=0;key_INCREASE_flag=0;key_DECREASE_flag=0;key_ENTER_flag=0;
		act[1]->inverse[1]=1; 
		display_GB2312_string(act[1]->x[1],(act[1]->y[1]-1)*8+1,act[1]->str[1],act[1]->inverse[1]);
		ui_state=101;// 切换状态到101
	}

}

void ui_proc101(void)
{
	if(key_UP_flag)
	{
		key_UP_flag=0;
		ui_state = 107;
		act[1]->inverse[7] = 1;
		display_GB2312_string(act[1]->x[7],(act[1]->y[7]-1)*8+1,act[1]->str[7],act[1]->inverse[7]);
		// 取消反白效果
		act[1]->inverse[1] = 0;// 先改参数
		display_GB2312_string(act[1]->x[1],(act[1]->y[1]-1)*8+1,act[1]->str[1],act[1]->inverse[1]);	// 再送显示
	}
	else if (key_DOWN_flag)
	{
		key_DOWN_flag=0;
		ui_state = 102;
		act[1]->inverse[2] = 1;
		display_GB2312_string(act[1]->x[2],(act[1]->y[2]-1)*8+1,act[1]->str[2],act[1]->inverse[2]);
		act[1]->inverse[1] = 0;// 先改参数
		display_GB2312_string(act[1]->x[1],(act[1]->y[1]-1)*8+1,act[1]->str[1],act[1]->inverse[1]);	// 再送显示
	}
	else if (key_INCREASE_flag)
	{
		key_INCREASE_flag = 0;
		++freqxxx_;								
		if(freqxxx_ > 9)	freqxxx_ = 0;	
		a1_s1[0] = freqxxx_ + '0';
		display_GB2312_string(act[1]->x[1],(act[1]->y[1]-1)*8+1,act[1]->str[1],act[1]->inverse[1]);
		ui_state = 101;
	}
		else if (key_DECREASE_flag)
	{
		key_DECREASE_flag = 0;
		--freqxxx_;								
		if(freqxxx_ < 0)	freqxxx_ = 9;	
		a1_s1[0] = freqxxx_ + '0';
		display_GB2312_string(act[1]->x[1],(act[1]->y[1]-1)*8+1,act[1]->str[1],act[1]->inverse[1]);
		ui_state = 101;
	}

	else if(key_ENTER_flag)
	{
		key_ENTER_flag=0;
	}
	
	if(NOKEY_clock10s_flag==1)		
	{
		NOKEY_clock10s_flag = 0;
		act[1]->inverse[1]=0; 
		display_GB2312_string(act[1]->x[1],(act[1]->y[1]-1)*8+1,act[1]->str[1],act[1]->inverse[1]);
		ui_state=100;							
	}

}

void ui_proc102(void)
{
	if(key_UP_flag)
	{
		key_UP_flag = 0;
		ui_state = 101;
		act[1]->inverse[1] = 1;
		display_GB2312_string(act[1]->x[1],(act[1]->y[1]-1)*8+1,act[1]->str[1],act[1]->inverse[1]);	
		act[1]->inverse[2] = 0; 
		display_GB2312_string(act[1]->x[2],(act[1]->y[2]-1)*8+1,act[1]->str[2],act[1]->inverse[2]);	
	}
	
	else if(key_DOWN_flag)
	{
		key_DOWN_flag = 0;
		ui_state = 103;
		act[1]->inverse[3] = 1;	
		display_GB2312_string(act[1]->x[3],(act[1]->y[3]-1)*8+1,act[1]->str[3],act[1]->inverse[3]);
		act[1]->inverse[2] = 0;
		display_GB2312_string(act[1]->x[2],(act[1]->y[2]-1)*8+1,act[1]->str[2],act[1]->inverse[2]);
	}
	else if(key_INCREASE_flag)
	{
		key_INCREASE_flag = 0;
		++freqxx_;							
		if(freqxx_ > 9)	freqxx_ = 0;	
		a1_s2[0] = freqxx_ + '0';
		display_GB2312_string(act[1]->x[2],(act[1]->y[2]-1)*8+1,act[1]->str[2],act[1]->inverse[2]);
		ui_state = 102;
	}
	else if(key_DECREASE_flag)
	{
		key_DECREASE_flag = 0;
		--freqxx_;
		if(freqxx_ < 0)	freqxx_ = 9;
		a1_s2[0] = freqxx_ + '0';
		display_GB2312_string(act[1]->x[2],(act[1]->y[2]-1)*8+1,act[1]->str[2],act[1]->inverse[2]);
		ui_state = 102;
	}
	else if(key_ENTER_flag)
	{
		key_ENTER_flag=0;
	}
	if(NOKEY_clock10s_flag==1)		
	{
		NOKEY_clock10s_flag = 0;
		act[1]->inverse[2]=0; 
		display_GB2312_string(act[1]->x[2],(act[1]->y[2]-1)*8+1,act[1]->str[2],act[1]->inverse[2]);
		ui_state=100;							
	}
}

void ui_proc103(void)
{
	if(key_UP_flag)
	{
		key_UP_flag = 0;
		ui_state = 102;
		act[1]->inverse[2] = 1;
		display_GB2312_string(act[1]->x[2],(act[1]->y[2]-1)*8+1,act[1]->str[2],act[1]->inverse[2]);	
		act[1]->inverse[3] = 0; 
		display_GB2312_string(act[1]->x[3],(act[1]->y[3]-1)*8+1,act[1]->str[3],act[1]->inverse[3]);	
	}
	
	else if(key_DOWN_flag)
	{
		key_DOWN_flag = 0;
		ui_state = 105;
		act[1]->inverse[5] = 1;	
		display_GB2312_string(act[1]->x[5],(act[1]->y[5]-1)*8+1,act[1]->str[5],act[1]->inverse[5]);
		act[1]->inverse[3] = 0; 
		display_GB2312_string(act[1]->x[3],(act[1]->y[3]-1)*8+1,act[1]->str[3],act[1]->inverse[3]);
	}
	else if(key_INCREASE_flag)
	{
		key_INCREASE_flag = 0;
		++freqx_;							
		if(freqx_ > 9)	freqx_ = 0;	
		a1_s3[0] = freqx_ + '0';
		display_GB2312_string(act[1]->x[3],(act[1]->y[3]-1)*8+1,act[1]->str[3],act[1]->inverse[3]);
		ui_state = 103;
		
		
	}
	else if(key_DECREASE_flag)
	{
		key_DECREASE_flag = 0;
		--freqx_;							
		if(freqx_ < 0)	freqx_ = 9;	
		a1_s3[0] = freqx_ + '0';
		display_GB2312_string(act[1]->x[3],(act[1]->y[3]-1)*8+1,act[1]->str[3],act[1]->inverse[3]);
		ui_state = 103;
	}
	else if(key_ENTER_flag)
	{
		key_ENTER_flag=0;
	}
	if(NOKEY_clock10s_flag==1)		
	{
		NOKEY_clock10s_flag = 0;
		act[1]->inverse[3]=0; 
		display_GB2312_string(act[1]->x[3],(act[1]->y[3]-1)*8+1,act[1]->str[3],act[1]->inverse[3]);
		ui_state=100;							
	}
}

void ui_proc105(void)
{
	if(key_UP_flag)
	{
		key_UP_flag = 0;
		ui_state = 103;
		act[1]->inverse[3] = 1;
		display_GB2312_string(act[1]->x[3],(act[1]->y[3]-1)*8+1,act[1]->str[3],act[1]->inverse[3]);	
		act[1]->inverse[5] = 0; 
		display_GB2312_string(act[1]->x[5],(act[1]->y[5]-1)*8+1,act[1]->str[5],act[1]->inverse[5]);	
	}
	
	else if(key_DOWN_flag)
	{
		key_DOWN_flag = 0;
		ui_state = 107;
		act[1]->inverse[7] = 1;	
		display_GB2312_string(act[1]->x[7],(act[1]->y[7]-1)*8+1,act[1]->str[7],act[1]->inverse[7]);
		act[1]->inverse[5] = 0; 
		display_GB2312_string(act[1]->x[5],(act[1]->y[5]-1)*8+1,act[1]->str[5],act[1]->inverse[5]);
	}
	else if(key_INCREASE_flag)
	{
		key_INCREASE_flag = 0;
		++freq_x;							
		if(freq_x > 9)	freq_x = 0;	
		a1_s5[0] = freq_x + '0';
		display_GB2312_string(act[1]->x[5],(act[1]->y[5]-1)*8+1,act[1]->str[5],act[1]->inverse[5]);
		ui_state = 105;
	}
	else if(key_DECREASE_flag)
	{
		key_DECREASE_flag = 0;
		--freq_x;							
		if(freq_x < 0)	freq_x = 9;	
		a1_s5[0] = freq_x + '0';
		display_GB2312_string(act[1]->x[5],(act[1]->y[5]-1)*8+1,act[1]->str[5],act[1]->inverse[5]);
		ui_state = 105;
	}
	else if(key_ENTER_flag)
	{
		key_ENTER_flag=0;
	}
	if(NOKEY_clock10s_flag==1)		
	{
		NOKEY_clock10s_flag = 0;
		act[1]->inverse[5]=0; 
		display_GB2312_string(act[1]->x[5],(act[1]->y[5]-1)*8+1,act[1]->str[5],act[1]->inverse[5]);
		ui_state=100;							
	}
}

void ui_proc107(void)
{
	if(key_ENTER_flag && ((freqxxx_*1000+freqxx_*100+freqx_*10+freq_x > 1080 )|| (freqxxx_*1000+freqxx_*100+freqx_*10+freq_x < 880)))
	{
		key_ENTER_flag=0;
		m107_flag=0;
		clear_screen();
		act[1]->inverse[7] = 0;
		
		display_ui_act(2);
		ui_state = 200;
	}
	
	else if(key_ENTER_flag && ((freqxxx_*1000+freqxx_*100+freqx_*10+freq_x <= 1080 )&& (freqxxx_*1000+freqxx_*100+freqx_*10+freq_x >= 880)))
	{
		key_ENTER_flag=0;
		m107_flag=1;
		freqs[7]=freqxxx_+'0';
		freqs[8]=freqxx_+'0';
		freqs[9]=freqx_+'0';
		freqs[10]=freq_x+'0';
		SysCtlDelay(200 * (g_ui32SysClock / 3000));  
    UARTStringPut(UART4_BASE, freqs);  
		clear_screen();
		act[1]->inverse[7] = 0;
		display_GB2312_string(act[1]->x[7],(act[1]->y[7]-1)*8+1,act[1]->str[7],act[1]->inverse[7]);
		display_ui_act(3);
		ui_state = 111;
	}
	
	
	else if(key_UP_flag)
	{
		key_UP_flag = 0;
		ui_state = 105;
		act[1]->inverse[5] = 1;
		display_GB2312_string(act[1]->x[5],(act[1]->y[5]-1)*8+1,act[1]->str[5],act[1]->inverse[5]);	
		act[1]->inverse[7] = 0; 
		display_GB2312_string(act[1]->x[7],(act[1]->y[7]-1)*8+1,act[1]->str[7],act[1]->inverse[7]);	
	}
	
	else if(key_DOWN_flag)
	{
		key_DOWN_flag = 0;
		ui_state = 101;
		act[1]->inverse[1] = 1;	
		display_GB2312_string(act[1]->x[1],(act[1]->y[1]-1)*8+1,act[1]->str[1],act[1]->inverse[1]);
		act[1]->inverse[7] = 0; 
		display_GB2312_string(act[1]->x[7],(act[1]->y[7]-1)*8+1,act[1]->str[7],act[1]->inverse[7]);
	}
	else if(key_INCREASE_flag)
	{
		key_INCREASE_flag = 0;
	}
	else if(key_DECREASE_flag)
	{
		key_DECREASE_flag = 0;
	}
	
	if(NOKEY_clock2s_flag==1)			
	{
		NOKEY_clock2s_flag = 0;
	}
	
	if(NOKEY_clock10s_flag==1)		
	{
		NOKEY_clock10s_flag = 0;
		act[1]->inverse[7]=0; 
		display_GB2312_string(act[1]->x[7],(act[1]->y[7]-1)*8+1,act[1]->str[7],act[1]->inverse[7]);
		ui_state=100;							
	}
}


void ui_proc200(void)	
{
	if(key_UP_flag)
	{
		key_UP_flag = 0;
	}
	else if(key_DOWN_flag)
	{
		key_DOWN_flag = 0;
	}
	else if(key_INCREASE_flag)
	{
		key_INCREASE_flag = 0;
	}
	else if(key_DECREASE_flag)
	{
		key_DECREASE_flag = 0;
	}
	if(NOKEY_clock10s_flag==1)			
	{
		NOKEY_clock10s_flag = 0;
	}
	if(NOKEY_clock2s_flag == 1)
	{
		NOKEY_clock2s_flag = 0;
		clear_screen();
		display_ui_act(1);
		act[1]->inverse[1]=1; 
		display_GB2312_string(act[1]->x[1],(act[1]->y[1]-1)*8+1,act[1]->str[1],act[1]->inverse[1]);
		ui_state = 101;
	}
}

void ui_proc111(void)	
{
	if(key_UP_flag)
	{
		key_UP_flag=0;
	}
	else if (key_DOWN_flag)
	{
		key_DOWN_flag=0;
	}
	else if (key_INCREASE_flag)
	{
		key_INCREASE_flag=0;
	}
	else if (key_DECREASE_flag)
	{
		key_DECREASE_flag=0;
	}
	else if(key_ENTER_flag)
	{
		key_ENTER_flag=0;
	}
	if(NOKEY_clock2s_flag==1)			
	{
		NOKEY_clock2s_flag = 0;
		clear_screen();
		display_ui_act(0);
		ui_state = 000;
	}
}


void ui_state_proc(unsigned int ui_state)
{
	switch (ui_state)
		{
			case 000: ui_proc000(); break;
			case 100: ui_proc100(); break;
			case 101: ui_proc101(); break;
			case 102: ui_proc102(); break;
			case 103: ui_proc103();	break;
			case 105: ui_proc105();	break;
			case 107: ui_proc107();	break;
			case 200: ui_proc200();	break;
		  case 111: ui_proc111();	break;
			
			default: break;
		}

}
