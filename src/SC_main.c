//------------------------------------------------------------------------------
//  Purpose: smartbox 测试入口
//  Funtion:
//  Dependent:
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef SC_main_C
#define SC_main_C
#include "SC_cmdline.h"
#include "SC_ComPak.h"
#include "SC_main.h"
#include <gdstd.h>
//------------------------------------------------------------------------------
//Funtion: smartbox 测试入口
//Input:  none
//OutPut: none
//Return: none
//Info  : none
//------------------------------------------------------------------------------
SC_COMPAKTYPE PCPakR[ 1000 ];    //
SC_COMPAKTYPE PCPakL[ 1000 ];
void SC_main(  )
{
    int Paklenth;
    SC_ComPak_Init();
    while(1)
    {
        // Recieve
        Paklenth = SC_ComPak_RecievePak( PCPakR );
        // CMD proccess+
        if( true == CmdLineProcess( &Paklenth, PCPakR, PCPakL ) )
        {
            SC_ComPak_SendPak( Paklenth, PCPakL );
        }
    }
}
#endif
//--------------------------------------------------------- End Of File --------
