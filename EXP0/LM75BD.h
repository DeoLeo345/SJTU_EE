//*****************************************************************************
//
// LM75BD.h - Prototypes for the LM75BD driver.
//
// Copyright：2020-2021,上海交通大学电子工程系实验教学中心
// 
// Author: 上海交通大学电子工程系实验教学中心
// Version: 1.0.0.20210513 
// Date：2021-05-13
// History：
//
//*****************************************************************************

#ifndef __LM75BD_H__
#define __LM75BD_H__

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
#include "inc/hw_memmap.h"        // 基址宏定义
#include "inc/hw_types.h"         // 数据类型宏定义，寄存器访问函数
#include "inc/hw_i2c.h"
#include "driverlib/debug.h"      // 调试用
#include "driverlib/gpio.h"       // 通用IO口宏定义
#include "driverlib/pin_map.h"    // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/sysctl.h"	  // 系统控制定义
#include "driverlib/i2c.h"
#include "driverlib/fpu.h"

// LM75BD的地址，1 0 0 1 A2 A1 A0
#define LM75BD_ADR1   0x48     // A2 A1 A0 = 0 0 0    
#define LM75BD_ADR2   0x49     // A2 A1 A0 = 0 0 1 

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
// 初始化I2C0	
extern void I2C0Init(void);

// 时延函数
extern void Delay(uint32_t value);

// 读取LM75BD温度值
extern int16_t GetTemputerature(uint8_t addr);
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __LM75BD_H__
