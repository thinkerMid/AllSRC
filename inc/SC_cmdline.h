//------------------------------------------------------------------------------
//  Purpose: 指令功能解释模块
//  Funtion: 实现对上位机已接收数据包指令解释和功能函数调用
//  Dependent: SC_UserPak
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef SC_cmdline_H
#define SC_cmdline_H
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
//*****************************************************************************
//
//! Defines the value that is returned if the command is not found.
//
//*****************************************************************************
#define CMDLINE_BAD_CMD         (-1)
//*****************************************************************************
//
//! Defines the value that is returned if there are too many arguments.
//
//*****************************************************************************
#define CMDLINE_TOO_MANY_ARGS   (-2)
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
//------------------------------------------------------------------------------
//Funtion: 指令解析
//Input:  pcCmdLine - 输入指令
//        CmdLen - 输入指令长度
//OutPut: ans - 指令返回结果
//        CmdLen - 输出指令长度
//Return: true , CMDLINE_BAD_CMD, CMDLINE_TOO_MANY_ARGS
//Info  : none
//------------------------------------------------------------------------------
    int CmdLineProcess( int *CmdLen, unsigned char *pcCmdLine, unsigned char *ans );
#ifdef __cplusplus
}
#endif
#endif
//--------------------------------------------------------- End Of File --------
