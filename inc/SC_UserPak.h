//------------------------------------------------------------------------------
//  Purpose: Smartbox 下位机数据包用户区解包模块
//  Funtion: 对输入的下位机用户区数据进行解包输出
//           数据包格式: 2字节指令ID 多字节指令参数
//           Example = { 60 02 00 00 00 00 00 00 }
//  Dependent:
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef SC_UserPak_H
#define SC_UserPak_H
typedef struct //触发参数
{
    unsigned int PakCmdID;        //2字节指令ID
    unsigned int PakLenth;        //数据包长度
    unsigned char *Pakdata;       //多字节指令参数
}   SC_UserPak;
//------------------------------------------------------------------------------
// Funtion: 构建一个回复数据包
// Input  : UserPak - 用户数据包
// Output : Temp - 输出的原始数据
// Return : 原始数据长度
// Info   : PakCmdID = 6002 PakLenth = 2 Pakdata = { FF 00 }
//          Temp = { FF 00 } return = 2
//------------------------------------------------------------------------------
unsigned int SC_UserPak_CreateANS( SC_UserPak *UserPak, unsigned char *Temp );
//------------------------------------------------------------------------------
// Funtion: 解包一个请求数据包
// Input  : Temp - 输出的原始数据
// Output : UserPak - 用户数据包
// Return : none
// Info   : Temp = { 60 02 00 00 00 00 00 00 }
//          PakCmdID = 6002 PakLenth = 6 Pakdata = { 00 00 00 00 00 00 }
//------------------------------------------------------------------------------
void SC_UserPak_SolveREQ( unsigned char *Temp, SC_UserPak *UserPak );
#endif
//--------------------------------------------------------- End Of File --------
