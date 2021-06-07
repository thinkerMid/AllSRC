//------------------------------------------------------------------------------
//  Purpose: 串口资源控制模块
//  Funtion: 提供基本的串口操作接口和一个中断处理机制
//           需要占用处理机 2 个串口中断源，其中一个为保留资源
//  Dependent：32F207 LIB
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef SC_PcCom_C
#define SC_PcCom_C
#include "SC_PcCom.h"
#include "gdstd.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_gpio.h"
#ifndef DPU431
uint8_t COM431 = 0;
#else
uint8_t COM431 = 1;
#endif
#define COMn 2
#define COMIntn 2
USART_TypeDef* PCCOM_USART[ COMn ] = { USART3,USART1 };
GPIO_TypeDef* PCCOM_TX_PORT[ COMn ] = { GPIOB,GPIOB };
GPIO_TypeDef* PCCOM_RX_PORT[ COMn ] = { GPIOB,GPIOB };
const uint32_t PCCOM_USART_CLK[ COMn ] = {RCC_APB1Periph_USART3,RCC_APB2Periph_USART1};
const uint32_t PCCOM_TX_PORT_CLK[ COMn ] = {RCC_AHB1Periph_GPIOB,RCC_AHB1Periph_GPIOB};
const uint32_t PCCOM_RX_PORT_CLK[ COMn ] = {RCC_AHB1Periph_GPIOB,RCC_AHB1Periph_GPIOB};
const uint16_t PCCOM_TX_PIN[COMn] = {GPIO_Pin_10,GPIO_Pin_6};
const uint16_t PCCOM_RX_PIN[COMn] = {GPIO_Pin_11,GPIO_Pin_7};
const uint8_t PCCOM_TX_PIN_SOURCE[COMn] = {GPIO_PinSource10,GPIO_PinSource6};
const uint8_t PCCOM_RX_PIN_SOURCE[COMn] = {GPIO_PinSource11,GPIO_PinSource7};
const uint8_t PCCOM_TX_AF[COMn] = {GPIO_AF_USART3,GPIO_AF_USART1};
const uint8_t PCCOM_RX_AF[COMn] = {GPIO_AF_USART3,GPIO_AF_USART1};
__IO  uint8_t PCCOM_RxFlag[ COMn ][ 2 ]  = { 0 }; //串口接收中断标识
__IO  uint8_t PCCOM_TxFlag[ COMn ] = { 0 };       //串口发送中断标识
//------------------------------------------------------------------------------
// Funtion: 串口 port 管脚配置
// Input  : none
// Output : none
// Return : none
// Info   : none
//------------------------------------------------------------------------------
static void PcComConfig( )
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /* Enable GPIO clock */
    RCC_AHB1PeriphClockCmd(PCCOM_TX_PORT_CLK[COM431] | PCCOM_RX_PORT_CLK[COM431], ENABLE);
    if(USART1== PCCOM_USART[COM431] || PCCOM_USART[COM431]==USART6)
    {
        RCC_APB2PeriphClockCmd(PCCOM_USART_CLK[COM431], ENABLE);
    }
    else
    {
        /* Enable UART clock */
        RCC_APB1PeriphClockCmd(PCCOM_USART_CLK[COM431], ENABLE);
    }
    /* Connect PXx to USARTx_Tx*/
    GPIO_PinAFConfig(PCCOM_TX_PORT[COM431], PCCOM_TX_PIN_SOURCE[COM431], PCCOM_TX_AF[COM431]);
    /* Connect PXx to USARTx_Rx*/
    GPIO_PinAFConfig(PCCOM_RX_PORT[COM431], PCCOM_RX_PIN_SOURCE[COM431], PCCOM_RX_AF[COM431]);
    /* Configure USART Tx as alternate function  */
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = PCCOM_TX_PIN[COM431];
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(PCCOM_TX_PORT[COM431], &GPIO_InitStructure);
    /* Configure USART Rx as alternate function  */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = PCCOM_RX_PIN[COM431];
    GPIO_Init(PCCOM_RX_PORT[COM431], &GPIO_InitStructure);
}
//------------------------------------------------------------------------------
// Funtion: 向PC串口缓冲区写一个字符数据并等待数据从串口发送完毕
// Input  : chbyte - 发送的数据
// Output : none
// Return : none
// Info   : 查询接口,请注意使用在中断中的效果
//------------------------------------------------------------------------------
void PCComSendByte(unsigned char chbyte )
{
    PCCOM_TxFlag[ COM431 ] = false;
    /*write a character to the USART */
    USART_SendData(PCCOM_USART[ COM431 ], (uint8_t) chbyte);
    /* Loop until the end of transmission */
    while ( (!PCCOM_TxFlag[ COM431 ]) &&
            (USART_GetFlagStatus(PCCOM_USART[ COM431 ],  USART_FLAG_TC ) == RESET))
    {
    }
}
//------------------------------------------------------------------------------
// Funtion: 向串口缓冲区写多个字符数据并等待数据从串口发送完毕
// Input  : chframe - 发送的数据 chLenth - 数据长度
// Output : none
// Return : none
// Info   : 查询接口,请注意使用在中断中的效果
//------------------------------------------------------------------------------
void PcComSendFrame(unsigned char *chframe, unsigned char chLenth )
{
    unsigned char i;
    for( i = 0; i < chLenth; i++ )
    {
        PCComSendByte( chframe[ i ] );
    }
}
//------------------------------------------------------------------------------
// Funtion: 一直等待接收一个数据并在接收到时从串口缓冲区读取这个数据并
//          清空缓冲区
// Input  : none
// Output : none
// Return : 接收数据
// Info   : 查询接口,请注意使用在中断中的效果
//------------------------------------------------------------------------------
unsigned char PCComReadByte()
{
    uint8_t dat;
    do
    {
        /*if ( ( PCCOM_USART[ COM431 ]->SR & USART_FLAG_ORE ) != RESET )
        {
            PCCOM_USART[ COM431 ]->SR = (uint16_t)~USART_FLAG_ORE;
            PCCOM_USART[ COM431 ]->DR;
            //USART_ReceiveData(PCCOM_USART[COM431]);
        }
        if ( ( PCCOM_USART[ COM431 ] -> SR & USART_FLAG_RXNE ) == RESET )
        {
            continue;
        }
        dat = USART_ReceiveData( PCCOM_USART[ COM431 ] );
        break;*/
        if( PCCOM_RxFlag[COM431][ 0 ] == SET )
        {
            dat = PCCOM_RxFlag[COM431][ 1 ];
            USART_ClearFlag( PCCOM_USART[ COM431 ], USART_FLAG_RXNE );
            break;
        }
        if (USART_GetFlagStatus(PCCOM_USART[COM431], USART_FLAG_ORE)!=RESET)
        {
            USART_ClearFlag(PCCOM_USART[COM431],USART_FLAG_ORE);
            USART_ReceiveData(PCCOM_USART[COM431]);
        }
        if(USART_GetFlagStatus(PCCOM_USART[COM431], USART_FLAG_RXNE) == SET)
        {
            dat = USART_ReceiveData(PCCOM_USART[COM431]);
            USART_ClearFlag( PCCOM_USART[ COM431 ], USART_FLAG_RXNE );
            break;
        }
    }
    while( 1 );
    return dat;
}
//------------------------------------------------------------------------------
// Funtion: 一定时间内接收一个数据并在接收到时从串口缓冲区读取这个数据
//          并清空缓冲区
// Input  : nWaitMS - 等待溢出时间记数( 时间单位 MS )
//          chByte - 接收的数据
// Output : none
// Return : false 表示没有接收到数据 true 表示接收到数据
// Info   : 查询接口,请注意使用在中断中的效果
//------------------------------------------------------------------------------
char PCComByte( unsigned char *chByte, unsigned int nWaitMS )
{
    uint8_t flag=false;
#ifdef VECT_TAB_NOR
    __IO uint32_t time=nWaitMS*130;//3736;
#else
#ifdef VECT_TAB_SRAM
    __IO uint32_t time=nWaitMS*2147;//3736;
#else
    __IO uint32_t time=nWaitMS*2491;//4285;
#endif
#endif
    do
    {
        if( PCCOM_RxFlag[COM431][ 0 ] == SET )
        {
            ( *chByte ) = PCCOM_RxFlag[COM431][ 1 ];
            USART_ClearFlag( PCCOM_USART[ COM431 ], USART_FLAG_RXNE );
            flag = true;
            break;
        }
        if (USART_GetFlagStatus(PCCOM_USART[COM431], USART_FLAG_ORE)!=RESET)
        {
            USART_ClearFlag(PCCOM_USART[COM431],USART_FLAG_ORE);
            USART_ReceiveData(PCCOM_USART[COM431]);
        }
        if(USART_GetFlagStatus(PCCOM_USART[COM431], USART_FLAG_RXNE) == SET)
        {
            ( *chByte ) = USART_ReceiveData(PCCOM_USART[COM431]);
            USART_ClearFlag( PCCOM_USART[ COM431 ], USART_FLAG_RXNE );
            flag = true;
            break;
        }
    }
    while( time-- );
    return flag;
}
//------------------------------------------------------------------------------
// Funtion: 配置并打开串口，所有收发操作前必须打开串口
// Input  : Baudrate - 波特率
// Output : none
// Return : 串口已被打开时返回 false
//          打开串口成功返回 true
// Info   : 请注意给出接收 FIFO 的长度，最好将 FIFO 设置为 1
//------------------------------------------------------------------------------
char PcComOpen( long Baudrate )
{
    USART_InitTypeDef USART_InitStructure;
    //初始化IO配置
    PcComConfig( );
    //参数配置
    USART_InitStructure.USART_BaudRate = Baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    /* USART configuration */
    USART_Init( PCCOM_USART[ COM431 ], &USART_InitStructure );
    /* Enable USART */
    USART_Cmd( PCCOM_USART[ COM431 ], ENABLE );
    //USART_ClearFlag(PCCOM_USART[0],USART_FLAG_TC);
    return true;
}
//------------------------------------------------------------------------------
// Funtion: 关闭当前串口
// Input  : port - 目标串口
// Output : none
// Return : none
// Info   : none
//------------------------------------------------------------------------------
void PcComClose( )
{
    /* Disable USART */
    USART_Cmd( PCCOM_USART[ COM431 ], DISABLE );
}
#endif
//--------------------------------------------------------- End Of File --------
