﻿//------------------------------------------------------------------------------
//  Purpose: 指令函数集 - 总线通讯类（K Line）
//  Funtion: 完成所有物理层总线参数设置函数，主要集中与 指令ID 6100 - 6200
//  Dependent:
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef FUN_ONETOONE_H
#define FUN_ONETOONE_H
//------------------------------------------------------------------------------
// Funtion: OneToOne模式与ECU通信
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6105
//61 05 FF 01 07 41 28 07 22 07 99 00 00 03 00 4C
//------------------------------------------------------------------------------
int OneToOne_SendDataToEcuGetAnswer_1( int argc, unsigned char *argv, unsigned char *ans );
#endif
//--------------------------------------------------------- End Of File --------