//------------------------------------------------------------------------------
//  Purpose: 指令功能解释模块
//  Funtion: 实现对上位机已接收数据包指令解释和功能函数调用
//  Dependent: SC_UserPak
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef SC_cmdline_C
#define SC_cmdline_C
#include <string.h>
#include "SC_cmdline.h"
#include "SC_UserPak.h"
#include "SC_cmdfunc.h"
//*****************************************************************************
//
// Defines the maximum number of arguments that can be parsed.
//
//*****************************************************************************
#define MAX_ARGS                16
SC_UserPak UserPak[ MAX_ARGS ];
//------------------------------------------------------------------------------
//Funtion: 指令解析
//Input:  pcCmdLine - 输入指令
//        CmdLen - 输入指令长度
//OutPut: ans - 指令返回结果
//        CmdLen - 输出指令长度
//Return: true , CMDLINE_BAD_CMD, CMDLINE_TOO_MANY_ARGS
//Info  : none
//------------------------------------------------------------------------------
int CmdLineProcess( int *CmdLen, unsigned char *pcCmdLine, unsigned char *ans )
{
    int Pos = 0;
    unsigned char Cmdnumb;
    int Len;
    int FunID,FunID1;
    Len = (*CmdLen);
    Cmdnumb = 0;
    //60 07 00 00 19 60 07 ff 00 19 61 09 ff 01 05 81 10 f1 81 03
    while( Pos < Len )
    {
        if( (Len - Pos) < 3 && (Len > 2) )
        {
            ans[ 0 ] = 0xff;
            ans[ 1 ] = 0x02;
            (*CmdLen) = 2;
            break;
        }
        SC_UserPak_SolveREQ( &pcCmdLine[ Pos ], &UserPak[ Cmdnumb ] );
        // 长度为剩余
        if( 0 == UserPak[ Cmdnumb ].PakLenth )
        {
            Pos = Len - 2;
        }
        else
        {
            Pos += UserPak[ Cmdnumb ].PakLenth;
        }

        // 检查参数 60 07
        if( (UserPak[ Cmdnumb ].PakCmdID < 0x6000) ||
                (UserPak[ Cmdnumb ].PakCmdID > 0x6300) )
        {
            break;
        }
        // 获取 函数指针位置
        FunID = UserPak[ Cmdnumb ].PakCmdID & 0xff;
        FunID1 = ((UserPak[ Cmdnumb ].PakCmdID & 0x0f00) >> 8);
        // 检查 FunID
        if( FunID >= 0x30 )
        {
            break;
        }
        // 跳转至相应函数
        if( g_sCmdTable[ FunID1 ][ FunID ] )
        {
            (*CmdLen) = g_sCmdTable[ FunID1 ][ FunID ]
                        ( UserPak[ Cmdnumb ].PakLenth, UserPak[ Cmdnumb ].Pakdata, ans );
        }
        else
        {
            ans[ 0 ] = 0xff;
            ans[ 1 ] = 0x02;
            (*CmdLen) = 2;
        }
        // 指令计数加
        Cmdnumb++;
        Pos += 2;
        if( Cmdnumb > MAX_ARGS )
        {
            break;
        }
    }
    return 1;
}
#endif
//--------------------------------------------------------- End Of File --------
