//------------------------------------------------------------------------------
//  Purpose: 指令函数集 - 总线通讯类（ ONETOONE）
//  Funtion: 完成所有物理层总线参数设置函数，主要集中与 指令ID 6105
//  Dependent:
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
//涂敏
#ifndef FUN_ONETOONE_C
#define FUN_ONETOONE_C
//柳甲元
#include "FUN_OneToOne.h"
#include "FUN_Base.h"
//------------------------------------------------------------------------------
// Funtion: OneToOne模式与ECU通信
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6105
//61 05 FF 01 07 41 28 07 22 07 99 00 00 03 00 4C
//------------------------------------------------------------------------------
int OneToOne_SendDataToEcuGetAnswer_1( int argc, unsigned char *argv, unsigned char *ans )
{
    unsigned int ansPos = 0, i;
    unsigned char anslenth = 0;
    unsigned int len = 0;
    unsigned char temp_1,R0,B,rcvbyte,framenum,framecount=0;
    unsigned char CANCONNECTOR = FALSE;
    unsigned int argpos;
    unsigned char ansTemp[256] = {0};
    unsigned char framelen ;
    set_time0Stop();
    ans[ansPos++] = 0;
    if((argv[3] == 0x55) && (argv[4] == 0xAA))
    {
        CANCONNECTOR = TRUE;
        if(SC_RelayState.CommType == COMM_UART_TYPE)
        {
            SC_RelayState.CommType = COMM_DOUBLE_CAN_TYPE;
        }
    }

    //61,05,ff,01,08,55,aa,04,60,02,ff,04,9d,00,03,fc,92
    if((framenum = argv[1]) == 0x0)
    {
        if(timer_open_flag == 1)
        {
            TimerStart( TIMER0 );
        }
        PutDataToAns(szFNG, ans, sizeof(szFNG));
        return ID_FNG;
    }
    ans[ansPos++] = framenum;
    ans[ansPos] = 0;
    argpos = 2;
    //61 05 FF 01 05 20 05 00 02 D9 00 02 00
    do
    {
        //发送数据
        if((temp_1 = argv[argpos++]) == 0x0)    // 第三个字节，帧长度
        {
            if(timer_open_flag == 1)
            {
                TimerStart( TIMER0 );
            }
            PutDataToAns(szFNG, ans, sizeof(szFNG));
            return ID_FNG;
        }
        framelen = temp_1;
        while(framelen)
        {
            framelen--;
            if(CANCONNECTOR == TRUE)
            {
                setCanbusNormalDataToken(&argv[argpos + 2]); //跳过55aa hejm
                argpos += temp_1;
                if(argv[6] == 0x60)
                {
                    switch(argv[7])
                    {
                    case 1:
                    case 2:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 0x0A:
                    case 0x0b:
                        ans[1] = 0x01;
                        ans[2] = 0x05;
                        ans[3] = 0x55;
                        ans[4] = 0xaa;
                        ans[5] = 0x01;
                        ans[6] = 0x00;
                        ans[7] = 0x01;
                        ansPos = 8;
                        return ansPos;
                    default:
                        break;
                    }
                }
                break;
            }
            else
            {
                ComSendByte(argv[argpos++]);
                if(framelen != 0)
                {
                    delayus(SC_TagKWP2000.m_nBtyetime);
                }
            }
        }
        //接收数据,指定接收长度  61,05,ff,01,01,1a,01,2e
        if((temp_1 = argv[argpos++]) != 0x0)
        {
            // 接收固定长度
            ans[ansPos++] = temp_1;     // 接收长度
            do
            {
                if(CANCONNECTOR == TRUE)
                {
                    anslenth = SC_CML_ISO15765_Recv(&ans[ansPos + 1], temp_1);
                    ans[ansPos] = anslenth;
                    ansPos += anslenth + 1;
                    if(anslenth == 0)
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        if(timer_open_flag == 1)
                        {
                            TimerStart( TIMER0 );
                        }
                        return ID_FBREAK;
                    }
                    break;
                }
                else
                {
                    //SC_TagKWP2000.m_Maxwaittime = 1000;
                    if(ComByte( &rcvbyte, SC_TagKWP2000.m_Maxwaittime )==FALSE)
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        if(timer_open_flag == 1)
                        {
                            TimerStart( TIMER0 );
                        }
                        return ID_FBREAK;
                    }
                }
                ans[ansPos++] = rcvbyte;
            }
            while(--temp_1);
        }
        //接收数据,自动识别
        else
        {
            //回复的数据中包含长度信息
            if((temp_1 = argv[argpos++]) != 0x0)
            {
                if(CANCONNECTOR == TRUE)
                {
                    anslenth = SC_CML_ISO15765_Recv(&ans[ansPos + 1], temp_1);
                    ans[ansPos] = anslenth;
                    ansPos += anslenth + 1;
                    if(anslenth == 0)
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        if(timer_open_flag == 1)
                        {
                            TimerStart( TIMER0 );
                        }
                        return ID_FBREAK;
                    }
                }
                else
                {
                    len = 0;        // 计算长度
                    B = argv[argpos++];
                    do
                    {
                        if(ComByte(&rcvbyte,SC_TagKWP2000.m_Maxwaittime)==FALSE)
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            if(timer_open_flag == 1)
                            {
                                TimerStart( TIMER0 );
                            }
                            return ID_FBREAK;
                        }
                        ansTemp[len ++] = rcvbyte;
                    }
                    while(--temp_1);
                    rcvbyte -= B;
                    rcvbyte -= len;
                    R0 = rcvbyte;
                    if((R0==0x00) && (CANCONNECTOR != TRUE))   //lyc add 20110118
                    {
                        ans[ansPos++] = len;
                        for(i = 0; i < len; i++)
                        {
                            ans[ansPos++] = ansTemp[i];
                        }
                        continue;
                    }
                    do
                    {
                        if(ComByte(&rcvbyte,500)==FALSE)
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            if(timer_open_flag == 1)
                            {
                                TimerStart( TIMER0 );
                            }
                            return ID_FBREAK;
                        }
                        ansTemp[len++] = rcvbyte;
                    }
                    while(--R0 > 0);
                    ans[ansPos++] = len;
                    for(i = 0; i < len; i++)
                    {
                        ans[ansPos++] = ansTemp[i];
                    }
                }
            }
            //到特定字节结束
            else
            {
                // R0 = 37
                if(CANCONNECTOR == TRUE)
                {
                    anslenth = SC_CML_ISO15765_Recv(&ans[ansPos + 1], temp_1);
                    ans[ansPos] = anslenth;
                    ansPos += anslenth + 1;
                    if(anslenth == 0)
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        if(timer_open_flag == 1)
                        {
                            TimerStart( TIMER0 );
                        }
                        return ID_FBREAK;
                    }
                }
                else
                {
                    R0 = argv[argpos++];
                    len = 0;
                    do
                    {
                        if(ComByte(&rcvbyte,SC_TagKWP2000.m_Maxwaittime)==FALSE)
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            if(timer_open_flag == 1)
                            {
                                TimerStart( TIMER0 );
                            }
                            return ID_FBREAK;
                        }
                        ansTemp[len++] = rcvbyte;
                    }
                    while(rcvbyte != R0);
                    if(ComByte(&rcvbyte,500)==FALSE)
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        if(timer_open_flag == 1)
                        {
                            TimerStart( TIMER0 );
                        }
                        return ID_FBREAK;
                    }
                    //ansTemp[len++] = rcvbyte;
                    ans[ansPos++] = len;
                    for(i = 0; i < len; i++)
                    {
                        ans[ansPos++] = ansTemp[i];
                    }
                }
            }
        }
        delay(SC_TagKWP2000.m_nFrameTime);          // 2008-11-17, Delay100us -> Sleep
        framecount++;
    }
    while(framecount < framenum);
    if(timer_open_flag == 1)
    {
        TimerStart( TIMER0 );
    }
    return ansPos;
}
#endif
//--------------------------------------------------------- End Of File --------
