//------------------------------------------------------------------------------
//  Purpose: 对上位机交互模块
//  Funtion: 实现对上位机数据接收，回复，打包，解包过程
//           数据包格式: 55 AA LenthH LenthL ~LenthH ~LenthL Userdata CHK
//           Example = { 55 AA 00 0F FF F0 00 01 07 83 F1 11 C1 EF 8F C4 00 }
//  Dependent: SC_PcCom_H
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef SC_ComPak_C
#define SC_ComPak_C
#include "SC_PcCom.h"
#include "SC_ComPak.h"
#include <gdstd.h>
#define PAKLENTH TagPCPAK.Paklenth
#define SADDRESS TagPCPAK.Saddr
#define TADDRESS TagPCPAK.Taddr
#define COUNT   TagPCPAK.Count
static struct PCPAK_DATA    //模块数据结构
{
    unsigned int Paklenth;  //PC数据包长度
    unsigned char Saddr;    //本方地址
    unsigned char Taddr;    //目标地址
    unsigned char Count;    //计数
}   TagPCPAK;
//------------------------------------------------------------------------------
//Funtion: 模块初始化
//Input:  none
//OutPut: none
//Return: none
//Info  : none
//------------------------------------------------------------------------------
void SC_ComPak_Init( void )
{
    PAKLENTH = 0;
    PcComOpen( 19200 );
    PCComSendByte( 'O' );   // ON
    PCComSendByte( 'N' );
}
//------------------------------------------------------------------------------
//Funtion: 接收上位机数据包
//Input:  none
//OutPut: PcPak - 接收到的上位机发送数据用户区
//Return: 接收到的上位机发送数据用户区长度
//Info  : none
//------------------------------------------------------------------------------
unsigned int SC_ComPak_RecievePak( SC_COMPAKTYPE *PcPak )
{
    unsigned int i;
    unsigned char Temp;
    i = 0;
    PAKLENTH = 0;
    // 55,aa,f0,f8,00,13,04,27,01,61,05,ff,01,08,55,aa,04,60,02,00,03,65,00,03,fc,ab,
    Temp = 0;
    while( 0x55 != Temp )
    {
        Temp = PCComReadByte( );   // 0x55
    }
    PCComByte( &Temp, 10 );   // 0xAA
    PCComByte( &SADDRESS, 10 );    //addr
    PCComByte( &TADDRESS, 10 );    //addr
    PCComByte( &Temp, 10 );    //Lenth
    PAKLENTH = ( unsigned int )Temp * 256;
    PCComByte( &Temp, 10 );    //Lenth
    PAKLENTH += ( unsigned int )Temp;   //Lenth
    PCComByte( &COUNT, 10 );   //~lenth
    COUNT++;
    PCComByte( &Temp, 10 );   //27
    PCComByte( &Temp, 10 );   //01
    if( PAKLENTH > 3 )
    {
        PAKLENTH -= 3;
    }
    else
    {
        PAKLENTH = 0;
        return 0;
    }
    for( i = 0; i < PAKLENTH; i++ ) //DATA
    {
        if( false == PCComByte( &PcPak[ i ], 10 ) )
        {
            PAKLENTH = 0;
            return 0;
        }
    }
    PCComByte( &Temp, 10 );   //CHK
    return PAKLENTH;
}
//------------------------------------------------------------------------------
//Funtion: 向上位机发送数据包
//Input:  PakLenth - 向上位机发送数据用户区长度
//OutPut: PcPak - 向上位机发送数据用户区数据
//Return: true of false
//Info  : none
//------------------------------------------------------------------------------
char SC_ComPak_SendPak( unsigned int PakLenth, SC_COMPAKTYPE *PcPak )
{
    unsigned int Chksum = 0;
    unsigned char Temp, TempL;
    unsigned int i;
    // 55,aa,f0,f8,00,13,04,67,01,61,05,ff,01,08,55,aa,04,60,02,00,03,65,00,03,fc,ab,
    PCComSendByte( 0x55 );  //55
    PCComSendByte( 0xAA );  //AA
    PAKLENTH = PakLenth + 3;
    // ADDRESS
    PCComSendByte( TADDRESS );
    Chksum ^= TADDRESS;
    PCComSendByte( SADDRESS );
    Chksum ^= SADDRESS;
    // Lenth
    Temp = ((PAKLENTH & 0xff00) >> 8);
    PCComSendByte( Temp );
    Chksum ^= Temp;
    Temp = ( unsigned char )(PAKLENTH & 0x00ff);
    PCComSendByte( Temp );
    Chksum ^= Temp;
    // Count
    PCComSendByte( COUNT );
    Chksum ^= COUNT;
    // 67 01
    PCComSendByte( 0x67 );   //67
    Chksum ^= 0x67;
    PCComSendByte( 0x01 );   //01
    Chksum ^= 0x01;
    i = 0;
    while( i < PakLenth )   // user data
    {
        PCComSendByte( PcPak[ i ] );
        Chksum ^= PcPak[ i ];
        i++;
    }
    // CHK
    PCComSendByte( Chksum );
    return true;
}
#endif
//--------------------------------------------------------- End Of File --------
