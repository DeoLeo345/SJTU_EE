//*****************************************************************************
//
// LM75BD.c - API for LM75BD.
//
// Copyright：2020-2021,上海交通大学电子工程系实验教学中心
// 
// Author:	上海交通大学电子工程系实验教学中心
// Version: 1.0.0.20210513 
// Date：2021-05-13
// History：
//
//*****************************************************************************

#include "LM75BD.h"
extern uint32_t g_ui32SysClock;    // 系统时钟


//******************************************************************************************
// 
// 函数原型：void I2C0Init(void)
// 函数功能：I2C0初始化（PB2-I2C0SCL，PB3-I2C0SDA）
// 函数参数：无
// 函数返回值：无
//
//******************************************************************************************
void I2C0Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinConfigure(GPIO_PB3_I2C0SDA);
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

    I2CMasterInitExpClk(I2C0_BASE, g_ui32SysClock, true);		//config I2C0 400k
	I2CMasterEnable(I2C0_BASE);	
}

//******************************************************************************************
// 
// 函数原型：void Delay(uint32_t value)
// 函数功能：时延函数
// 函数参数：uint32_t value：循环次数
// 函数返回值：无
//
//******************************************************************************************
void Delay(uint32_t value)
{
	uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++){};
}

//******************************************************************************************
// 
// 函数原型：int16_t GetTemputerature(uint8_t adr)
// 函数功能：读取LM75BD温度值
// 函数参数：uint8_t addr：LM75BD的I2C地址
// 函数返回值：读取的温度值（ 1LSB=0.1℃）
//
//******************************************************************************************
int16_t GetTemputerature(uint8_t addr)
{	
    
    uint16_t ui16Temputerature;  //温度值
	
    float Temputerature;         // 十进制温度值
    
    uint16_t Temp_LSB;    // 存放温度的低字节，高3位有效 
    uint16_t Temp_MSB;    // 存放温度的高字节
    
    // 设置Slave地址,并设置传输方式为主机从从机读数据
    I2CMasterSlaveAddrSet(I2C0_BASE, addr, true);  
    while(I2CMasterBusy(I2C0_BASE)) {};              

	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_BURST_RECEIVE_START);

    while(I2CMasterBusy(I2C0_BASE)) {};    
    
    if(I2CMasterErr(I2C0_BASE) == I2C_MASTER_ERR_NONE)
        Temp_MSB = I2CMasterDataGet(I2C0_BASE);  // 读取温度的高字节
     
    I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
  	while(I2CMasterBusy(I2C0_BASE)){};
        
    if(I2CMasterErr(I2C0_BASE) == I2C_MASTER_ERR_NONE)                   
        Temp_LSB = I2CMasterDataGet(I2C0_BASE);  // 读取温度的低字节,高3位有效
  
    Delay(100);    // 适当延迟，根据实际运行情况进行调整
    
	ui16Temputerature = (Temp_MSB << 8 ) + Temp_LSB;

	// 取温度数字量
	ui16Temputerature = ui16Temputerature >> 5; // 1LSB=0.125℃

	if(ui16Temputerature & 0x0400)  // 负温度
       Temputerature = -((ui16Temputerature^0x07FF ) + 1) * 0.125;
	else  // 正温度 
		Temputerature = 0.125 * ui16Temputerature;
    
    return ((int16_t)(Temputerature * 10));
}
