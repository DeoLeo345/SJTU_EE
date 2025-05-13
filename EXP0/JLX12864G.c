#include "JLX12864G.h"


////使能GPIO
void LCD_PORT_init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);						//Enable PortC
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC));
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);						//Enable PortK
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK));
	  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);						//Enable PortB
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);						//Enable PortE
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));

    GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_0 | GPIO_PIN_1| GPIO_PIN_2);
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE,  GPIO_PIN_5 | GPIO_PIN_4);
	  GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_4 );
    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_6) ;	//PC6设置为输入

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);						//DELAY时间测试
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));	    //DELAY时间测试
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,  GPIO_PIN_1);    //DELAY时间测试
}
//延时
void LCD_delay(int n_ms)
{
    int j,k;
//	TEST_L;
    for(j=0; j<n_ms; j++)
        for(k=0; k<110; k++);
//	TEST_H;
}
//短延时
void LCD_delay_us(int n_us)
{
    int j,k;
    for(j=0; j<n_us; j++)
        for(k=0; k<1; k++);
}
//写指令到LCD 模块
void transfer_command_lcd(int data1)
{
    char i;
    LCD_CS_L;
    LCD_RS_L;
    for(i=0; i<8; i++)
    {
        LCD_SCK_L;
        //delay_us(10); //加少量延时
        if(data1&0X80) LCD_SDA_H;
        else LCD_SDA_L;
        //delay_us(10); //加少量延时
        data1=data1<<=1;
        LCD_SCK_H;
    }
    LCD_CS_H;
}
//写数据到LCD 模块
void transfer_data_lcd(int data1)
{
    char i;
    LCD_CS_L;
    LCD_RS_H;
    for(i=0; i<8; i++)
    {
        LCD_SCK_L;
//delay_us(10); //加少量延时
        if(data1&0x80) LCD_SDA_H;
        else LCD_SDA_L;
        LCD_SCK_H;
//delay_us(10); //加少量延时
        data1=data1<<=1;
    }
    LCD_CS_H;

}
//LCD 模块初始化
void LCD_Init()
{
//		LCD_PORT_init();
    LCD_RST_L; //低电平复位
    LCD_delay(100);
    LCD_RST_H; //复位完毕
    LCD_delay(100);
    transfer_command_lcd(0xe2); //软复位
    LCD_delay(5);
    transfer_command_lcd(0x2c); //升压步聚1
    LCD_delay(50);
    transfer_command_lcd(0x2e); //升压步聚2
    LCD_delay(50);
    transfer_command_lcd(0x2f); //升压步聚3
    LCD_delay(5);
    transfer_command_lcd(0x23); //粗调对比度，可设置范围0x20～0x27
    transfer_command_lcd(0x81); //微调对比度
    transfer_command_lcd(0x28); //微调对比度的值，可设置范围0x00～0x3f
    transfer_command_lcd(0xa2); //1/9 偏压比（bias）
    transfer_command_lcd(0xc8); //行扫描顺序：从上到下
    transfer_command_lcd(0xa0); //列扫描顺序：从左到右
    transfer_command_lcd(0x40); //起始行：第一行开始
    transfer_command_lcd(0xaf); //开显示
}
void lcd_address(unsigned int page,unsigned int column)
{
    column=column-0x01;
    transfer_command_lcd(0xb0+page-1); //设置页地址，每8 行为一页，全屏共64 行，被分成8 页
    transfer_command_lcd(0x10+(column>>4&0x0f)); //设置列地址的高4 位
    transfer_command_lcd(column&0x0f); //设置列地址的低4 位
}
//全屏清屏
void clear_screen()
{
    unsigned char i,j;
    for(i=0; i<9; i++)
    {
        transfer_command_lcd(0xb0+i);
        transfer_command_lcd(0x10);
        transfer_command_lcd(0x00);
        for(j=0; j<132; j++)
        {
            transfer_data_lcd(0x00);
        }
    }
}
//黑屏显示
void full_display()
{
    int i,j;
    for(i=0; i<8; i++)
    {
        lcd_address(i+1,0);
        for(j=0; j<128; j++)
        {
            transfer_data_lcd(0xff);
        }
    }
}
//显示128x64 点阵图像
void display_128x64(unsigned char *dp)
{
    unsigned int i,j;
    for(j=0; j<8; j++)
    {
        lcd_address(j+1,1);
        for (i=0; i<128; i++)
        {
            transfer_data_lcd(*dp); //写数据到LCD,每写完一个8 位的数据后列地址自动加1
            dp++;
        }
    }
}
//显示16x16 点阵图像、汉字、生僻字或16x16 点阵的其他图标
void display_graphic_16x16(unsigned char page,unsigned char column,unsigned char *dp)
{
    unsigned int i,j;
    for(j=0; j<2; j++)
    {
        lcd_address(page+j,column);
        for (i=0; i<16; i++)
        {
            transfer_data_lcd(*dp); //写数据到LCD,每写完一个8 位的数据后列地址自动加1
            dp++;
        }
    }
}
//显示8x16 点阵图像、ASCII, 或8x16 点阵的自造字符、其他图标
void display_graphic_8x16(unsigned char page,unsigned char column,unsigned char *dp)
{
    unsigned int i,j;
    for(j=0; j<2; j++)
    {
        lcd_address(page+j,column);
        for (i=0; i<8; i++)
        {
            transfer_data_lcd(*dp); //写数据到LCD,每写完一个8 位的数据后列地址自动加1
            dp++;
        }
    }
}
//显示5X8 点阵图像、ASCII, 或5x8 点阵的自造字符、其他图标
void display_graphic_5x8(unsigned char page,unsigned char column,unsigned char *dp)
{
    unsigned int i;
    lcd_address(page,column);
    for (i=0; i<6; i++)
    {
        transfer_data_lcd(*dp);
        dp++;
    }
}
//送指令到晶联讯字库IC
void send_command_to_ROM( unsigned char datu )
{
    unsigned char i;
    for(i=0; i<8; i++ )
    {
        LCD_ROM_SCK_L;
        LCD_delay_us(10);
        if(datu&0x80) LCD_ROM_IN_H;
        else LCD_ROM_IN_L;
        datu = datu<<1;
        LCD_ROM_SCK_H;
        LCD_delay_us(10);
    }
}
//从晶联讯字库IC 中取汉字或字符数据（1 个字节）
static unsigned char get_data_from_ROM( )
{
    unsigned char i;
    unsigned char ret_data=0;
    for(i=0; i<8; i++)
    {
        LCD_ROM_OUT_H;
        LCD_ROM_SCK_L;
//LCD_delay_us(1);
        ret_data=ret_data<<1;
        if( LCD_ROM_OUT )
            ret_data=ret_data+1;
        else
            ret_data=ret_data+0;
        LCD_ROM_SCK_H;
//LCD_delay_us(1);
    }
    return(ret_data);
}
//从指定地址读出数据写到液晶屏指定（page,column)座标中,是否反白显示
void get_and_write_16x16(unsigned long fontaddr,unsigned char page,unsigned char column,unsigned char inverse)
{
    unsigned char i,j,disp_data;
    LCD_ROM_CS_L;
    send_command_to_ROM(0x03);
    send_command_to_ROM((fontaddr&0xff0000)>>16); //地址的高8 位,共24 位
    send_command_to_ROM((fontaddr&0xff00)>>8); //地址的中8 位,共24 位
    send_command_to_ROM(fontaddr&0xff); //地址的低8 位,共24 位
    for(j=0; j<2; j++)
    {
        lcd_address(page+j,column);
        for(i=0; i<16; i++ )
        {
            disp_data=get_data_from_ROM();
            if (inverse==0) transfer_data_lcd(disp_data); //写数据到LCD,每写完1 字节的数据后列地址自动加1
            else transfer_data_lcd(~disp_data);
        }
    }
    LCD_ROM_CS_H;
}
//从指定地址读出数据写到液晶屏指定（page,column)座标中,是否反白显示
void get_and_write_8x16(unsigned long fontaddr,unsigned char page,unsigned char column,unsigned char inverse)
{
    unsigned char i,j,disp_data;
    LCD_ROM_CS_L;
    send_command_to_ROM(0x03);
    send_command_to_ROM((fontaddr&0xff0000)>>16); //地址的高8 位,共24 位
    send_command_to_ROM((fontaddr&0xff00)>>8); //地址的中8 位,共24 位
    send_command_to_ROM(fontaddr&0xff); //地址的低8 位,共24 位
    for(j=0; j<2; j++)
    {
        lcd_address(page+j,column);
        for(i=0; i<8; i++ )
        {
            disp_data=get_data_from_ROM();
            if (inverse==0) transfer_data_lcd(disp_data); //写数据到LCD,每写完1 字节的数据后列地址自动加1
            else transfer_data_lcd(~disp_data);
        }
    }
    LCD_ROM_CS_H;
}
//从指定地址读出数据写到液晶屏指定（page,column)座标中,是否反白显示
void get_and_write_5x8(unsigned long fontaddr,unsigned char page,unsigned char column,unsigned char inverse)
{
    unsigned char i,disp_data;
    LCD_ROM_CS_L;
    send_command_to_ROM(0x03);
    send_command_to_ROM((fontaddr&0xff0000)>>16); //地址的高8 位,共24 位
    send_command_to_ROM((fontaddr&0xff00)>>8); //地址的中8 位,共24 位
    send_command_to_ROM(fontaddr&0xff); //地址的低8 位,共24 位
    lcd_address(page,column);
    for(i=0; i<5; i++ )
    {
        disp_data=get_data_from_ROM();
        if (inverse==0) transfer_data_lcd(disp_data); //写数据到LCD,每写完1 字节的数据后列地址自动加1
        else transfer_data_lcd(~disp_data);
    }
    LCD_ROM_CS_H;
}
//****************************************************************
unsigned long fontaddr=0;
void display_GB2312_string(unsigned char page,unsigned char column,unsigned char *text,unsigned char inverse)
{
    unsigned char i= 0;
    while((text[i]>0x00))
    {
        if(((text[i]>=0xb0) &&(text[i]<=0xf7))&&(text[i+1]>=0xa1))
        {
//国标简体（GB2312）汉字在晶联讯字库IC 中的地址由以下公式来计算：
//Address = ((MSB - 0xB0) * 94 + (LSB - 0xA1)+ 846)*32+ BaseAdd;BaseAdd=0
//由于担心8 位单片机有乘法溢出问题，所以分三部取地址
            fontaddr = (text[i]- 0xb0)*94;
            fontaddr += (text[i+1]-0xa1)+846;
            fontaddr = (unsigned long)(fontaddr*32);
            get_and_write_16x16(fontaddr,page,column,inverse); //从指定地址读出数据写到液晶屏指定（page,column)座标中
            i+=2;
            column+=16;
        }
        else if(((text[i]>=0xa1) &&(text[i]<=0xa3))&&(text[i+1]>=0xa1))
        {
//国标简体（GB2312）15x16 点的字符在晶联讯字库IC 中的地址由以下公式来计算：
//Address = ((MSB - 0xa1) * 94 + (LSB - 0xA1))*32+ BaseAdd;BaseAdd=0
//由于担心8 位单片机有乘法溢出问题，所以分三部取地址
            fontaddr = (text[i]- 0xa1)*94;
            fontaddr += (text[i+1]-0xa1);
            fontaddr = (unsigned long)(fontaddr*32);
            get_and_write_16x16(fontaddr,page,column,inverse); //从指定地址读出数据写到液晶屏指定（page,column)座标中
            i+=2;
            column+=16;
        }
        else if((text[i]>=0x20) &&(text[i]<=0x7e))
        {
            fontaddr = (text[i]- 0x20);
            fontaddr = (unsigned long)(fontaddr*16);
            fontaddr = (unsigned long)(fontaddr+0x3cf80);
            get_and_write_8x16(fontaddr,page,column,inverse); //从指定地址读出数据写到液晶屏指定（page,column)座标中
            i+=1;
            column+=8;
        }
        else
            i++;
    }
}
void display_string_5x8(unsigned char page,unsigned char column,unsigned char *text,unsigned char inverse)
{
    unsigned char i= 0;
    while((text[i]>0x00))
    {
        if((text[i]>=0x20) &&(text[i]<=0x7e))
        {
            fontaddr = (text[i]- 0x20);
            fontaddr = (unsigned long)(fontaddr*8);
            fontaddr = (unsigned long)(fontaddr+0x3bfc0);
            get_and_write_5x8(fontaddr,page,column,inverse); //从指定地址读出数据写到液晶屏指定（page,column)座标中
            i+=1;
            column+=6;
        }
        else
            i++;
    }
}
//===============main program===================
//void main(void)
//{
//    while(1)
//    {
//        initial_lcd();
//        clear_screen(); //clear all dots
//        display_128x64(bmp1);
//        LCD_delay(2000);
//        display_GB2312_string(1,1,"12864,带中文字库"); //在第1 页，第1 列，显示一串16x16 点阵汉字或8x16 的ASCII 字
//        display_GB2312_string(3,1,"16X16 简体汉字库,"); //显示一串16x16 点阵汉字或8x16 的ASCII 字.以下雷同
//        display_GB2312_string(5,1,"或8X16 点阵ASCII,");
//        display_GB2312_string(7,1,"或5x8 点阵ASCII 码");
//        LCD_delay(4000);
//        clear_screen();
//        display_GB2312_string(1,1,"晶联讯成立于二零");
//        display_GB2312_string(3,1,"零四年十一月七日");
//        display_GB2312_string(5,1,"主要生产液晶模块");
//        display_GB2312_string(7,1,"品质至上真诚服务");
//        LCD_delay(4000);
//        display_GB2312_string(1,1,"GB2312 简体字库及");
//        display_GB2312_string(3,1,"有图型功能，可自");
//        display_GB2312_string(5,1,"编大字或图像或生");
//        display_GB2312_string(7,1,"僻字，例如： ");
//        display_graphic_16x16(7,97,jiong1); //在第7页，第81列显示单个自编生僻汉字“囧”
//        display_graphic_16x16(7,113,lei1); //显示单个自编生僻汉字"畾“
//        LCD_delay(4000);
//        clear_screen();
//        display_GB2312_string(1,1,"<!@#$%^&*()_-+]/"); //在第1 页，第1 列，显示一串16x16 点阵汉字或8*16 的ASCII 字
//        display_string_5x8(3,1,"<!@#$%^&*()_-+]/;.,?[");//在第3 页，第1 列，显示一串5x8 点阵的ASCII 字
//        display_string_5x8(4,1,"JLX electronics Co., ");//显示一串5x8 点阵的ASCII 字
//        display_string_5x8(5,1,"Ltd. established at ");//显示一串5x8 点阵的ASCII 字
//        display_string_5x8(6,1,"year 2004.Focus LCM. ");//显示一串5x8 点阵的ASCII 字
//        display_string_5x8(7,1,"TEL:0755-29784961 ");//显示一串5x8 点阵的ASCII 字
//        display_string_5x8(8,1,"FAX:0755-29784964 ");//显示一串5x8 点阵的ASCII 字
//        LCD_delay(4000);
//        display_GB2312_string(1,1,"啊阿埃挨哎唉哀皑"); //在第1 页，第1 列，显示一串16x16 点阵汉字或8x16 的ASCII 字
//        display_GB2312_string(3,1,"癌蔼矮艾碍爱隘鞍"); //显示一串16x16 点阵汉字或8x16 的ASCII 字.以下雷同
//        display_GB2312_string(5,1,"氨安俺按暗岸胺案");
//        display_GB2312_string(7,1,"肮昂盎凹敖熬翱袄");
//        LCD_delay(4000);
//        display_GB2312_string(1,1,"鬟鬣麽麾縻麂麇麈");
//        display_GB2312_string(3,1,"麋麒鏖麝麟黛黜黝");
//        display_GB2312_string(5,1,"黠黟黢黩黧黥黪黯");
//        display_GB2312_string(7,1,"鼢鼬鼯鼹鼷鼽鼾齄");
//        LCD_delay(4000);
//    }
//}
