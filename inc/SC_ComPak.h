//------------------------------------------------------------------------------
//  Purpose: 对上位机交互模块
//  Funtion: 实现对上位机数据接收，回复，打包，解包过程
//           数据包格式: 55 AA LenthH LenthL ~LenthH ~LenthL Userdata CHK
//           Example = { 55 AA 00 0F FF F0 00 01 07 83 F1 11 C1 EF 8F C4 00 }
//  Dependent: SC_PcCom_H
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef SC_ComPak_H
#define SC_ComPak_H
#define SC_COMPAKTYPE   unsigned char
//------------------------------------------------------------------------------
//Funtion: 模块初始化
//Input:  none
//OutPut: none
//Return: none
//Info  : none
//------------------------------------------------------------------------------
void SC_ComPak_Init( void );
//------------------------------------------------------------------------------
//Funtion: 接收上位机数据包
//Input:  none
//OutPut: PcPak - 接收到的上位机发送数据用户区
//Return: 接收到的上位机发送数据用户区长度
//Info  : none
//------------------------------------------------------------------------------
unsigned int SC_ComPak_RecievePak( SC_COMPAKTYPE *PcPak );
//------------------------------------------------------------------------------
//Funtion: 向上位机发送数据包
//Input:  PakLenth - 向上位机发送数据用户区长度
//OutPut: PcPak - 向上位机发送数据用户区数据
//Return: true of false
//Info  : none
//------------------------------------------------------------------------------
char SC_ComPak_SendPak( unsigned int PakLenth, SC_COMPAKTYPE *PcPak );
#endif
//--------------------------------------------------------- End Of File --------
