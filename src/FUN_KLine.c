

//------------------------------------------------------------------------------
//  Purpose: 指令函数集 - 总线通讯类（K Line）
//  Funtion: 完成所有物理层总线参数设置函数，主要集中与 指令ID 6100 - 6200
//  Dependent:
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef FUN_KLINE_C
#define FUN_KLINE_C
#include "FUN_KLine.h"
#include "FUN_Base.h"

//------------------------------------------------------------------------------
// Funtion: BOSCH地址码触发函数  bosch协议专用
// Input  : argc - 参数长度
//          argv - 指令参数缓冲区
//          argv[0]:确定在发送完地址码后是否禁能L线，为0表示禁能L线，非0表示不禁能L线
//          argv[1]:地址码值
// Output : ans - 回复到上位机的数据
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、当KW2为0x8f时，返回00 00
//                4、当KW2非0x8f，并正常接收版本信息数据时，ans的格式为：总帧数 第一帧数据  第二帧数据......第N帧数据
//                如ans[] = {00,04,0f,01,f6,b0,36,42,39,30,36,30,33,32,41,20,20,03,17,03,f6,32,2e,30,4c,20,32,56,20,45,
//                           55,32,20,20,20,20,20,20,20,20,20,03,07,05,f6,35,32,31,36,03,08,07,f6,00,00,06,00,00,03}
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 6006
//------------------------------------------------------------------------------
int SetBosch_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned int baudrate, i,j, dataLenth = 0;
    unsigned char Keyword[3] =
    {
        0
    };
    SC_PAKSEND PakSend;
    SC_PAKRECV PakRecv;

    PakSend.Pakdata = (unsigned char *)calloc(100, sizeof(char));
    PakSend.PakLenth = 4;
    PakSend.Pakdata[0] = 0x03;
    PakSend.Pakdata[1] = 0x00;
    PakSend.Pakdata[2] = 0x09;
    PakSend.Pakdata[3] = 0x03;

    //设置链路数据包
    SC_TagKWP1281.m_chHoldDataLen = 4;
    SC_TagKWP1281.m_chHoldDig[0] = 0x03;
    SC_TagKWP1281.m_chHoldDig[1] = 0x00;
    SC_TagKWP1281.m_chHoldDig[2] = 0x09;
    SC_TagKWP1281.m_chHoldDig[3] = 0x03;
    PakRecv.Pakdata = (unsigned char *)calloc(100, sizeof(char));
    TimerRelease(TIMER0);

    //ComClose(COM_ECUPORT);
    SC_TagKWP1281.m_Idletime = 600;
    SC_TagKWP1281.m_chHoldDataLen = 4;
    SC_TagKWP1281.m_SendFinishFlag = 1;

    //    TimerStop( TIMER0 );
    //发送5bps的时候使能L线
    EnableLLine_1();

    //SendToECU(argv[1],NULL);            // 以5波特率发送地址码

	for (j = 0;  j < 2;  j++)
	{
		IOCTRLInit();
	    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
	    sendAddressToEcu(argv[1]);

	    if (argv[0])//*Argv & 0x80
	    {
	        EnableLLine_1();//发送5bps的时候使能L线
	        SetRelayPara(0x01, INPUTCHANNEL);
	        SetRelayPara(0x04, OUTPUTCHANNEL);
	        CMPHY_Relay_Set(&SC_RelayState);
	    }
	    else
	    {
	        //在进行正常的帧数据收发前，关闭L线，否则从L线发往ECU的数据可能对通信产生影响。ljy_2011-12-28
	        DisableLLine_1();
	    }

	    // 计算波特率
	    IOCTRLSelect(IOCTRL_KLINEREAD, IOCTRL_USMODE | IOCTRL_INPUT);
	    baudrate = autoCheckBaudRate();
		if ( Baudrate )
      		break;
	}
   

    if (baudrate == 0)
    {
        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    SC_com_portMode.BaudRate = baudrate;

    //TagKWP1281.Ecuport.BaudRate = 10400;
    // 初始化 Com 模块
    ComOpen(COM_ECUPORT, &SC_com_portMode);

    //delay(2); //为了防止收第一个关键字时接收到55H的数据造成错误，故在此延时。
    // KB1 - KBn
    for (i = 0; i < 2; i++)
    {
        if (FALSE == ComByte(&Keyword[i], 5000))
        {
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }
    }

	if ( (*Argv & 0xF) != 10 )
    	DisableLLine_1();

    delay_ms(30);

    // KB 取反发回
    ComSendByte(~Keyword[1]);
    ans[0] = 0;
    ans[1] = 0;
    dataLenth = 2;

    if (Keyword[1] == 0x8F)
    {
        uint8_t tmp;
        ComByte(&tmp, 1000);
    }
    else
    {
        TimerInit();
        SC_TagKWP1281.m_nMScount = 0; //定时器中断计数初始化为0
        SC_TagKWP1281.m_chCount = 0; //初始化接收字符数为0
        Keep_Link.rightState = TOOL_READING;

        // 加载链定时器中断
        if (baudrate < 5000)
        {
            TimerConfig(TIMER0, 1);
            SC_TagKWP1281.m_nBtyetime = 6;
            SC_TagKWP1281.m_nFrameTime = 30;
            SC_TagKWP1281.m_ReserveTime = 3;
        }
        else
        {
            TimerConfig(TIMER0, 1);
            SC_TagKWP1281.m_nBtyetime = 2;
            SC_TagKWP1281.m_nFrameTime = 22;
            SC_TagKWP1281.m_ReserveTime = 2;
        }

        SC_TagKWP1281.m_LinkFG = true;
        timer_open_flag = 1;
        TimerInterruptFuntionLoad(TIMER0, SC_CML_KWP1281_Time);

        // 加载串口中断
        ComInterruptFuntionLoad(SC_CML_KWP1281_Comread, COM_INTERREAD);
        ComInterruptFuntionLoad(SC_CML_KWP1281_Comsend, COM_INTERSEND);
        TimerStart(TIMER0);

        // 接收回复
        ComReadSign(true);

        if (FALSE == SC_CML_KWP1281_Recv(&PakRecv))
        {
            free((void *) (PakSend.Pakdata));
            free((void *) (PakRecv.Pakdata));
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        while (1)
        {
            if ((PakRecv.Pakdata[2] == 0x09) || (PakRecv.Pakdata[2] == 0xF7))
            {
                break;
            }

            if (PakRecv.PakLenth < 3)
            {
                break;
            }
            else
            {
                ans[1] ++;

                for (i = 0; i < PakRecv.PakLenth; i++)
                {
                    ans[dataLenth++] = PakRecv.Pakdata[i];
                }
            }

            if (FALSE == SC_CML_KWP1281_Send(&PakSend))
            {
                free((void *) (PakSend.Pakdata));
                free((void *) (PakRecv.Pakdata));
                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                return ID_FBREAK;
            }

            if (FALSE == SC_CML_KWP1281_Recv(&PakRecv))
            {
                free((void *) (PakSend.Pakdata));
                free((void *) (PakRecv.Pakdata));
                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                return ID_FBREAK;
            }
        }

        free((void *) (PakSend.Pakdata));
        free((void *) (PakRecv.Pakdata));
    }

    return dataLenth;
}


//------------------------------------------------------------------------------
// Funtion:  send linkeep to ecu,ecu no answer
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据 返回 FF 00
// Return : 回复到上位机的数据长度
// Info   : 601b   此函数未做任何动作
//------------------------------------------------------------------------------
int SetLinkNoAns_1(int argc, unsigned char * argv, unsigned char * ans)
{
    PutDataToAns(szFOK, ans, sizeof(szFOK));
    return ID_FOK;
}


//-------------------------------------------------------------------------------
//wabco ABS send one frame to ecu and receive one frame,but each times read or send byte
//command word;6121
//ziyingzhu 对此函数进行了较大修改，尚未验证
//------------------------------------------------------------------------------
int WabcoAbsSendOneAndReicveOneFrame_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char argvPos = 0;
    unsigned char ansPos = 0;
    unsigned char sendframenum, sendlen, rcvlen, rcvbyte;

    ans[ansPos++] = 0;
    set_time0Stop();

    //发送帧数
    if ((sendframenum = argv[argvPos++]) == 0x0)
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        PutDataToAns(szFNG, ans, sizeof(szFNG));
        return ID_FNG;
    }

    ans[ansPos++] = sendframenum;

    while (sendframenum--)
    {
        //发送的每帧长度
        if ((sendlen = argv[argvPos++]) == 0x0)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            PutDataToAns(szFNG, ans, sizeof(szFNG));
            return ID_FNG;
        }

        //接收的每帧长度
        if ((rcvlen = argv[argvPos++]) == 0x0)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            PutDataToAns(szFNG, ans, sizeof(szFNG));
            return ID_FNG;
        }

        ans[ansPos++] = rcvlen - 1;

        while (sendlen-- > 1)
        {
            ComSendByte(argv[argvPos++]);

            if (ComByte(&rcvbyte, 500) == FALSE)
            {
                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                return ID_FBREAK;
            }

            delayus(SC_TagKWP2000.m_nBtyetime);
        }

        ComSendByte(argv[argvPos++]);

        while (rcvlen-- > 1)
        {
            if (ComByte(&rcvbyte, 1500) == FALSE)
            {
                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                return ID_FBREAK;
            }

            ComSendByte(rcvbyte);
            ans[ansPos++] = rcvbyte;
        }

        if (ComByte(&rcvbyte, 1500) == FALSE)
        {
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        //如果是单帧的话，这里是否需要延时
        delay(SC_TagKWP2000.m_nFrameTime); // 2008-11-17, Delay100us -> Sleep
    }

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    return ansPos;
}


//------------------------------------------------------------------------------
// Funtion: wabco ABS send one frame to ecu and receive Multi-frame,but each times read or send byte
// Input  : argc - 参数长度
//          argv - 指令参数缓冲区
//          argv[0]:发送到ECU的一帧数据的长度len
//          argv[1] ~ argv[len]:发送到ECU的数据
// Output : ans - 回复到上位机的数据
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、ans的格式为：总帧数 帧长度 帧数据
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 6122
//------------------------------------------------------------------------------
int WabcoAbsSendOneAndReicveMultiFrame_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char argvPos = 0;
    unsigned char ansLen = 0;
    unsigned char sendlen, rcvlen = 0, rcvbyte, temp;

    set_time0Stop();

    if ((sendlen = argv[0]) == 0x0)
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        PutDataToAns(szFNG, ans, sizeof(szFNG));
        return ID_FNG;
    }

    argvPos = 1;

    while (sendlen-- > 1)
    {
        ComSendByte(argv[argvPos++]);

        if (ComByte(&rcvbyte, 1000) == FALSE)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        delayus(SC_TagKWP2000.m_nBtyetime);
    }

    ComSendByte(argv[argvPos++]);
    ans[0] = 0;
    ans[1] = 1;
    ansLen = 3;

    do
    {
        if (ComByte(&rcvbyte, 2500) == FALSE)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        ComSendByte(rcvbyte);
        ans[ansLen++] = rcvbyte;
        temp = rcvbyte;
        rcvlen++;

        if (ComByte(&rcvbyte, 1000) == FALSE)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }
    }
    while(temp != 0x04);

    ans[2] = rcvlen;

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    return ansLen;
}


//----------------------------------------------------------------------------------------------------
//wabco ABS send one frame to ecu and receive Multi-frame,but each times read or send byte
//command word;6122
//
//----------------------------------------------------------------------------------------------------

/*int WabcoAbsSendOneAndReicveMultiFrame_1( int argc, unsigned char *argv, unsigned char *ans )
{
    unsigned char argvPos = 0;
    unsigned char ansPos = 0;
    unsigned char  sendframelen,rcvbyte,temp;
    ans[ansPos++] = 0;
    ans[ansPos++] = 1;
    ans[2] = 0;
    ansPos = 3;
    set_time0Stop();
    if((sendframelen = argv[argvPos++]) == 0x0)
    {
        return ReturnAndStartKeepLink(ID_FNG);
    }
    while(sendframelen-- > 1)
    {
        ComSendByte( argv[argvPos++] );
        if(ComByte(&rcvbyte,500)==FALSE)
        {
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }
        delayus(SC_TagKWP2000.m_nBtyetime);
    }
    ComSendByte( argv[argvPos++] );
    do
    {
        if(ComByte(&rcvbyte,2500)==FALSE)
        {
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }
        ComSendByte(rcvbyte);
        ans[ansPos++] = rcvbyte;
        temp = rcvbyte;
        ans[2]++;
        if(ReadFromECU(&rcvbyte,1000)==FALSE)
        {
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }
    }while(temp!=0x04);
    if(timer_open_flag == 1)
        TimerStart( TIMER0 );
    return ansPos;
}*/
//------------------------------------------------------------------------------
// Funtion: send two frame and receive multiframe from ecu,know length of receive'frame
// Input  : argc - 参数长度
//          argv - 指令参数缓冲区
//          argv[0]:发送的第一帧数据的长度len
//          argv[1]~argv[len]:发送的第一帧数据
//          argv[len + 1]:发送的第二帧数据的长度len
//          argv[len + 2 。。。]:送的第二帧数据
// Output : ans - 回复到上位机的数据
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、ans的格式为：总帧数 帧长度 帧数据
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 6125 用于对读取故障码有比较严格的时间要求的汽车, 发送第一个命令并收取ECU数据后,
//               第二个命令必须在很短时间内就发出,ECU才会响应. 所有收到的数据存放到接收缓冲区.
//------------------------------------------------------------------------------
int QuickSendTwoFrameReceiveMultiFrame_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned int temp, len = 0, argvPos, ansLen;
    unsigned char rcvbyte, stopFlag = 0;

    set_time0Stop();

    //发送第一帧长度
    if ((len = argv[0]) == 0x0)
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        PutDataToAns(szFNG, ans, sizeof(szFNG));
        return ID_FNG;
    }

    argvPos = 1;

    while (len-- > 1)
    {
        ComSendByte(argv[argvPos++]);
        delayus(SC_TagKWP2000.m_nBtyetime);
    }

    ComSendByte(argv[argvPos++]);

    //接收第一帧长度
    len = argv[argvPos++];
    temp = len;
    ans[0] = 0;
    ans[1] = 0;
    ansLen = 2;
    ans[ansLen++] = len; //接收的第一帧数据的长度

    while (len-- > 0)
    {
        if (ComByte(&rcvbyte, 300) == FALSE)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        ans[ansLen++] = rcvbyte; //接收的第一帧数据
    }

    ans[1] ++;

    // 快速发出的第二帧长度
    if ((len = argv[argvPos++]) == 0x0)
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        PutDataToAns(szFNG, ans, sizeof(szFNG));
        return ID_FNG;
    }

    while (len-- > 1)
    {
        ComSendByte(argv[argvPos++]);
        delayus(SC_TagKWP2000.m_nBtyetime);
    }

    ComSendByte(argv[argvPos++]);

    //后续帧的接收长度，每帧都相同
    len = argv[argvPos++];
    temp = len;
    ans[ansLen++] = len; //接收的第二帧数据的长度

    while (len-- > 0)
    {
        if (ComByte(&rcvbyte, 500) == FALSE)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        ans[ansLen++] = rcvbyte; //接收的第二帧数据
    }

    ans[1] ++;

    do
    {
        len = temp;

        while (len-- > 0)
        {
            if (ComByte(&rcvbyte, 500) == FALSE)
            {
                stopFlag = 1;
                break;
            }

            ans[ansLen++] = rcvbyte; //接收的第N帧数据
        }

        if (stopFlag == 1)
        {
            break;
        }

        ans[ansLen++] = temp; //接收的第三帧数据的长度
        ans[1] ++;
    }
    while(1);

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    return ansLen;
}


//*****************************************************************************
//    功  能：发一帧数据到ECU并接收一帧ECU的应答，一帧数据长度大于
//            256字节.
//    参  数：argc 发送数据长度
//            argv     发送缓冲区指针,格式为:数据长度高位
//                        （1BYTE）数据长度低位（1BYTE）, 数据内容,
// Output : ans - 回复到上位机的数据
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、ans的格式为：总帧数 帧长度 帧数据
// Return : 返回的ans缓冲区的有效数据长度
//    0x6126    用于长安铃木车型
//*****************************************************************************
int OneToOne_SendLongDataToEcuGetLongAnswer_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char rcvbyte;
    unsigned int sendframesize, rcvlen, argvPos = 0, ansLen = 0;

    set_time0Stop();
    sendframesize = argv[0] *0x100;
    ComSendByte(argv[0]);
    delayus(SC_TagKWP2000.m_nBtyetime);
    sendframesize += argv[1];
    ComSendByte(argv[1]);
    delayus(SC_TagKWP2000.m_nBtyetime);

    if (sendframesize < 2)
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        PutDataToAns(szFNG, ans, sizeof(szFNG));
        return ID_FNG;
    }

    argvPos = 2;
    sendframesize -= 2;

    do
    {
        ComSendByte(argv[argvPos++]);
        delayus(SC_TagKWP2000.m_nBtyetime);
    }
    while(--sendframesize > 1);

    ComSendByte(argv[argvPos++]);

    //一帧数据
    ans[0] = 0;
    ans[1] = 1;
    ans[2] = 0;
    ansLen = 3;
    ans[ansLen++] = 0x01;
    ans[2] ++;

    if (ComByte(&rcvbyte, 300) == FALSE)
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    ans[ansLen++] = rcvbyte;
    ans[2] ++;
    rcvlen = rcvbyte * 0x100;

    if (ComByte(&rcvbyte, 300) == FALSE)
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    ans[ansLen++] = rcvbyte;
    ans[2] ++;
    rcvlen += rcvbyte;

    if (rcvlen < 2)
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    rcvlen -= 2;

    while (rcvlen--)
    {
        if (ComByte(&rcvbyte, 300) == FALSE)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        ans[ansLen++] = rcvbyte;
        ans[2] ++;
    }

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    return ansLen;
}


//------------------------------------------------------------------------------
// Funtion: BOSCH地址码触发函数  bosch协议专用
// Input  : argc - 参数长度
//          argv - 指令参数缓冲区
//          argv[0]:发送地址码模式
//     模式1
//          argv[1]:地址码值
//          argv[2]:是否使能L线标识，最高位为1则使能L线，否则禁能L线
//          argv[3]:是否自动计算波特率，为0则使用设定的波特率，为0xff则自动计算波特率
//          argv[4]:无意义
//          argv[5]:KW2和KW2取反之间的时间延时参数，毫秒级
//          argv[6]:判定是否KW2取反发回，为0则不取反发回，为0XFF则取反发回
//          argv[7]:如argv[6]非0，argv[7]为0表示必须要收到ECU发来的地址码取反字节，如argv[6]为0，argv[7]为0表示
//                  发完地址码后不再进行任何收发动作，如argv[7]非0，则argv[7]表示要从ECU接受的数据的帧数
//          argv[8]~arg[8+argv[7]-1]:如argv[7]为0，这些字节无意义,如argv[7]非0，这些字节分别表示接收的每一帧数据的长度
//     模式2
//          argv[1]:地址码值
//          argv[2]:是否使能L线标识，最高位为1则使能L线，否则禁能L线
//          argv[3]:最高位为1表示需要计算地址码的波特率，并作为计算波特率的一个参数，否则表示地址码波特率为200.
//          argv[4]:argv[3]最高位为1时，argv[4]为计算波特率的一个参数，argv[3]最高位不为1时，argv[4]无意义
//          argv[5]:无意义
//          argv[6]:KW2和KW2取反之间的时间延时参数，毫秒级
//          argv[7]:判定是否KW2取反发回，为0则不取反发回，为0XFF则取反发回
//          argv[8]:如argv[7]非0，argv[8]为0表示必须要收到ECU发来的地址码取反字节，如argv[7]为0，argv[8]为0表示
//                  发完地址码后不再进行任何收发动作，如argv[8]非0，则argv【8]表示要从ECU接受的数据的帧数
//          argv[9]~arg[9+argv[8]-1]:如argv[8]为0，这些字节无意义,如argv[8]非0，这些字节分别表示接收的每一帧数据的长度
// Output : ans - 回复到上位机的数据
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、当KW2为0x8f时，返回00 00
//                4、当KW2非0x8f，并正常接收版本信息数据时，ans的格式为：总帧数 第一帧数据  第二帧数据......第N帧数据
//                如ans[] = {00,04,0f,01,f6,b0,36,42,39,30,36,30,33,32,41,20,20,03,17,03,f6,32,2e,30,4c,20,32,56,20,45,
//                           55,32,20,20,20,20,20,20,20,20,20,03,07,05,f6,35,32,31,36,03,08,07,f6,00,00,06,00,00,03}
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 601a
//------------------------------------------------------------------------------
int SendAddressCodeTimeAdjustable_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char OBD_FID1, rcvbyte;
    unsigned int i, frameLen, framenum, argvLen, ansPostion = 0, baudrate;
    unsigned char k = 0, keyWord[3] =
    {
        0
    };
    OBD_FID1 = argv[2];

    switch (argv[0])
    {
        //-------------------- mode 1 ----------------
        case 1:
            switch (argv[3])
            {
                case 0x00: //已知波特率
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                    sendAddressToEcu(argv[1]); // 发送地址码
                    DisableLLine_1();

                    if (OBD_FID1 & 0x80)
                    {
                        EnableLLine_1();
                    }

                    // 初始化 Com 模块
                    ComOpen(COM_ECUPORT, &SC_com_portMode);

                    if ((FALSE == ComByte(&k, 2550)) || (k != 0x55))
                    {
                        delay(4000);
                        IOCTRLInit();
                        IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                        sendAddressToEcu(argv[1]);
                        ComOpen(COM_ECUPORT, &SC_com_portMode);

                        if ((FALSE == ComByte(&k, 2550)) || (k != 0x55))
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            return ID_FBREAK;
                        }
                    }

                    break;

                case 0xff: //自动计算0x55波特率
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);

                    //发送5bps地址码
                    sendAddressToEcu(argv[1]);
                    DisableLLine_1();

                    if (OBD_FID1 & 0x80)
                    {
                        EnableLLine_1();
                    }

                    // 10S 内等待一个 0x55 起始位
                    SC_com_portMode.BaudRate = 0;

                    // 计算波特率
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINEREAD, IOCTRL_USMODE | IOCTRL_INPUT);
                    SC_com_portMode.BaudRate = autoCheckBaudRate();

                    if (SC_com_portMode.BaudRate == 0)
                    {
                        delay(4000);
                        IOCTRLInit();
                        IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);

                        //延时500毫秒
                        IOCTRLSet(IOCTRL_HIGH, 500);

                        //delay(500);
                        //发送5bps地址码
                        sendAddressToEcu(argv[1]);
                        DisableLLine_1();

                        // 计算波特率
                        IOCTRLInit();
                        IOCTRLSelect(IOCTRL_KLINEREAD, IOCTRL_USMODE | IOCTRL_INPUT);
                        SC_com_portMode.BaudRate = autoCheckBaudRate();

                        if (SC_com_portMode.BaudRate == 0)
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            return ID_FBREAK;
                        }
                    }

                    //TagKWP2000.Ecuport.BaudRate = 10400;
                    // 设置 Com 模块
                    //delay(1);//为了防止收第一个关键字时接收到55H的数据造成错误，故在此延时。
                    ComOpen(COM_ECUPORT, &SC_com_portMode);
                    break;

                default:
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
            }

            rcvbyte = 0;

            if (ComByte(&rcvbyte, 500) == FALSE)
            {
                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                return ID_FBREAK;
            }

            if (ComByte(&rcvbyte, 500) == FALSE)
            {
                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                return ID_FBREAK;
            }

            //延时argv[5]
            IOCTRLSet(IOCTRL_HIGH, argv[5]);

            //是否取反发回
            switch (argv[6])
            {
                case 0x00:
                    if (argv[7] == 0x00)
                    {
                        PutDataToAns(szFOK, ans, sizeof(szFOK));

                        //在进行正常的帧数据收发前，关闭L线，否则从L线发往ECU的数据可能对通信产生影响。ljy_2011-12-28
                        DisableLLine_1();
                        return ID_FOK;
                    }

                    break;

                case 0xff:
                    rcvbyte = 255 - rcvbyte;
                    ComSendByte(rcvbyte);

                    if (argv[7] == 0x00)
                    {
                        //一般是地址码的取反发回
                        if (ComByte(&rcvbyte, 300) == FALSE)
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            return ID_FBREAK;
                        }

                        PutDataToAns(szFOK, ans, sizeof(szFOK));

                        //在进行正常的帧数据收发前，关闭L线，否则从L线发往ECU的数据可能对通信产生影响。ljy_2011-12-28
                        DisableLLine_1();
                        return ID_FOK;
                    }

                    break;

                default:
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
            }

            //在进行正常的帧数据收发前，关闭L线，否则从L线发往ECU的数据可能对通信产生影响。ljy_2011-12-28
            framenum = argv[7]; // 保存帧数
            ans[ansPostion++] = 0;
            ans[ansPostion++] = framenum;
            argvLen = 8;

            for (i = 0; i < framenum; i++)
            {
                if ((frameLen = argv[argvLen++]) == 0)
                {
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
                }

                ans[ansPostion++] = frameLen;

                if (ComByte(&rcvbyte, 1000) == FALSE)
                {
                    PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                    return ID_FBREAK;
                }

                ans[ansPostion++] = rcvbyte; //保存帧数据
                frameLen--;

                while (frameLen-- > 0)
                {
                    if (ComByte(&rcvbyte, 100) == FALSE)
                    {
                        break;
                    }

                    ans[ansPostion++] = rcvbyte; //保存帧数据
                }
            }

            break;

        case 2:
            if (argv[3] &0x80)
            {
                baudrate = argv[3] *256 + argv[4];
                baudrate = 65536 - baudrate;
                baudrate = (18432000 / 32) / baudrate;
                IOCTRLInit();
                IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                sendAddressCodeToEcuOnAnyBaudrate(argv[1], baudrate);
                ComOpen(COM_ECUPORT, &SC_com_portMode);

                if ((FALSE == ComByte(&k, 2550)) || (k != 0x55))
                {
                    delay(4000);
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                    sendAddressCodeToEcuOnAnyBaudrate(argv[1], baudrate);
                    ComOpen(COM_ECUPORT, &SC_com_portMode);

                    if ((FALSE == ComByte(&k, 2550)) || (k != 0x55))
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        return ID_FBREAK;
                    }
                }
            }
            else
            {
                IOCTRLInit();
                IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                sendAddressCodeToEcuOnAnyBaudrate(argv[1], 200);
                ComOpen(COM_ECUPORT, &SC_com_portMode);

                if ((FALSE == ComByte(&k, 2550)) || (k != 0x55))
                {
                    delay(4000);
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                    sendAddressCodeToEcuOnAnyBaudrate(argv[1], 200);
                    DisableLLine_1();

                    if (OBD_FID1 & 0x80)
                    {
                        EnableLLine_1();
                    }

                    ComOpen(COM_ECUPORT, &SC_com_portMode);

                    if ((FALSE == ComByte(&k, 2550)) || (k != 0x55))
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        return ID_FBREAK;
                    }
                }
            }

            for (i = 0; i < 2; i++)
            {
                if (FALSE == ComByte(&keyWord[i], 5000))
                {
                    PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                    return ID_FBREAK;
                }
            }

            if (argv[7] == 0)
            {
                if (argv[8] == 0)
                {
                    PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                    return ID_FBREAK;
                }
            }
            else
            {
                if (argv[7] != 0xff)
                {
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
                }

                delay(34);
                ComSendByte(~keyWord[1]);

                if (argv[8] == 0)
                {
                    if (FALSE == ComByte(&k, 5000))
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        return ID_FBREAK;
                    }

                    PutDataToAns(szFOK, ans, sizeof(szFOK));
                    return ID_FOK;
                }
            }

            // 接收回复
            ansPostion = 0;
            ans[ansPostion++] = 0;
            ans[ansPostion++] = argv[8]; //接收的帧数
            argvLen = 9;

            for (i = 0; i < argv[8]; i++)
            {
                if (argv[argvLen] == 0)
                {
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
                }

                ans[ansPostion++] = argv[argvLen];

                //将接收到的数据放入ANS中
                for (k = 0; k < argv[argvLen]; k++)
                {
                    if (FALSE == ComByte(&ans[ansPostion++], 1000))
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        return ID_FBREAK;
                    }
                }

                argvLen++;
            }

            break;

        default:
            PutDataToAns(szFNG, ans, sizeof(szFNG));
            return ID_FNG;
    }

    return ansPostion;
}


//------------------------------------------------------------------------------
// Funtion: BOSCH地址码触发函数  bosch协议专用 接受五个关键字，对第二个关键字取反发回
// Input  : argc - 参数长度
//          argv - 指令参数缓冲区
//          argv[0]:地址码值
// Output : ans - 回复到上位机的数据
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、ans的格式为：总帧数 第一帧数据  第二帧数据......第N帧数据
//                如ans[] = {00,04,0f,01,f6,b0,36,42,39,30,36,30,33,32,41,20,20,03,17,03,f6,32,2e,30,4c,20,32,56,20,45,
//                           55,32,20,20,20,20,20,20,20,20,20,03,07,05,f6,35,32,31,36,03,08,07,f6,00,00,06,00,00,03}
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 6015
//------------------------------------------------------------------------------
int FiatBoschSystem_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned int i, dataLenth;
    unsigned char kw, j, flag_L_Line = 0;
    SC_PAKSEND PakSend;
    SC_PAKRECV PakRecv;

    TimerRelease(TIMER0);
    flag_L_Line = CheckLLine();
    SC_TagKWP1281.m_Idletime = 600;
    SC_TagKWP1281.m_SendFinishFlag = 1;
    EnableLLine_1();
    IOCTRLInit();
    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
    sendAddressToEcu(argv[0]); // 以5波特率发送地址码

    if (flag_L_Line == 1)
    {
        EnableLLine_1();
    }
    else
    {
        DisableLLine_1();
    }

    IOCTRLSet(IOCTRL_HIGH, 20);
    IOCTRLSelect(IOCTRL_KLINEREAD, IOCTRL_USMODE | IOCTRL_INPUT);
    SC_com_portMode.BaudRate = autoCheckBaudRate();

    //TagKWP1281.Ecuport.BaudRate = 10400;
    // 初始化 Com 模块
    ComOpen(COM_ECUPORT, &SC_com_portMode);
    delay(10); // 不用延时

    if (FALSE == ComByte(&j, 1000)) //kw1
    {
        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    delay(10);

    if (FALSE == ComByte(&j, 1000)) //kw2
    {
        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    kw = j;
    delay(10);

    if (FALSE == ComByte(&j, 1000)) //kw3
    {
        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    delay(10);

    if (FALSE == ComByte(&j, 1000)) //kw4
    {
        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    delay(10);

    if (FALSE == ComByte(&j, 1000)) //kw5
    {
        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    ComSendByte(~kw);//对接收的第二个字节取反发送
    ans[0] = 0;
    ans[1] = 0;
    dataLenth = 2;
    SC_TagKWP1281.m_nMScount = 0; //定时器中断计数初始化为0
    SC_TagKWP1281.m_chCount = 0; //初始化接收字符数为0
    Keep_Link.rightState = TOOL_READING;
    PakSend.Pakdata = (unsigned char *)calloc(100, sizeof(char));
    PakSend.PakLenth = 4;
    PakSend.Pakdata[0] = 0x03;
    PakSend.Pakdata[1] = 0x00;
    PakSend.Pakdata[2] = 0x09;
    PakSend.Pakdata[3] = 0x03;

    //设置链路数据包
    SC_TagKWP1281.m_chHoldDataLen = 4;
    SC_TagKWP1281.m_chHoldDig[0] = 0x03;
    SC_TagKWP1281.m_chHoldDig[1] = 0x00;
    SC_TagKWP1281.m_chHoldDig[2] = 0x09;
    SC_TagKWP1281.m_chHoldDig[3] = 0x03;
    PakRecv.Pakdata = (unsigned char *)
    calloc(100, sizeof(char));

    // 加载链定时器中断
    TimerInit();

    if (SC_com_portMode.BaudRate < 5000)
    {
        TimerConfig(TIMER0, 1);
        SC_TagKWP1281.m_nBtyetime = 6;
        SC_TagKWP1281.m_nFrameTime = 30;
        SC_TagKWP1281.m_ReserveTime = 3;
    }
    else
    {
        TimerConfig(TIMER0, 1);
        SC_TagKWP1281.m_nBtyetime = 2;
        SC_TagKWP1281.m_nFrameTime = 22;
        SC_TagKWP1281.m_ReserveTime = 2;
    }

    TimerInterruptFuntionLoad(TIMER0, SC_CML_KWP1281_Time);

    // 加载串口中断
    ComInterruptFuntionLoad(SC_CML_KWP1281_Comread, COM_INTERREAD);
    ComInterruptFuntionLoad(SC_CML_KWP1281_Comsend, COM_INTERSEND);
    timer_open_flag = 1;
    TimerStart(TIMER0);

    // 接收回复
    ComReadSign(true);

    if (FALSE == SC_CML_KWP1281_Recv(&PakRecv))
    {
        free((void *) (PakSend.Pakdata));
        free((void *) (PakRecv.Pakdata));
        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    while (1)
    {
        if ((PakRecv.Pakdata[2] == 0x09) || (PakRecv.Pakdata[2] == 0xF7))
        {
            break;
        }

        if (PakRecv.PakLenth < 3)
        {
            break;
        }
        else
        {
            ans[1] ++;

            for (i = 0; i < PakRecv.PakLenth; i++)
            {
                ans[dataLenth++] = PakRecv.Pakdata[i];
            }
        }

        if (FALSE == SC_CML_KWP1281_Send(&PakSend))
        {
            free((void *) (PakSend.Pakdata));
            free((void *) (PakRecv.Pakdata));
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        if (FALSE == SC_CML_KWP1281_Recv(&PakRecv))
        {
            free((void *) (PakSend.Pakdata));
            free((void *) (PakRecv.Pakdata));
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }
    }

    free((void *) (PakSend.Pakdata));
    free((void *) (PakRecv.Pakdata));
    return dataLenth;
}


//------------------------------------------------------------------------------
// Funtion: BOSCH链路维持函数  bosch协议专用
// Input  : argc - 参数长度
//          argv - 指令参数缓冲区
//          此函数不需要参数
// Output : ans - 回复到上位机的数据
//                返回FF 00
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 6008
//------------------------------------------------------------------------------
int SetBoschLink_1(int argc, unsigned char * argv, unsigned char * ans)
{
    //TimerInit( );
    //TimerConfig( TIMER0, 2 );
    SC_TagKWP1281.m_Idletime = 600;

    // 加载链路维持中断
    //TimerInterruptFuntionLoad( TIMER0, SC_CML_KWP1281_Time );
    // 设置链路数据
    SC_TagKWP1281.m_chHoldDig[0] = 0x03;
    SC_TagKWP1281.m_chHoldDig[1] = 0x00;
    SC_TagKWP1281.m_chHoldDig[2] = 0x09;
    SC_TagKWP1281.m_chHoldDig[3] = 0x03;

    switch (argv[0])
    {
        case 0x00:
            //        SC_TagKWP1281.m_LinkFG = false;
            //        SC_TagKWP1281.m_chHoldDataLen = 0;
            if (timer_open_flag == 1)
            {
                while (Keep_Link.rightState != TOOL_WAITING) //防止发生链路维持正在发送的时候定时器被关闭
                {
                    ;
                }

                TimerStop(TIMER0);
            }

            break;

        case 0xff:
            SC_TagKWP1281.m_LinkFG = true;
            SC_TagKWP1281.m_chHoldDataLen = 4;
            break;

        default:
            PutDataToAns(szFNG, ans, sizeof(szFNG));
            return ID_FNG;
    }

    Keep_Link.rightState = TOOL_WAITING;
    PutDataToAns(szFOK, ans, sizeof(szFOK));
    return ID_FOK;
}


//-------------------------------------------------------------------
// Set5BpsParameter for roverland
// command word:0x6010
//-------------------------------------------------------------------
//int SetSendAdressCodeRoverland_1( int argc, unsigned char *argv, unsigned char *ans )
//{
//    uUARTMODE uartmode;
//    DWORD baudrate,point,i;
//    uint8_t rcvbyte,temp,group,addr5bps;//,linkcount=0;
//    uartmode.mode = 0;
//
//    SC_com_portMode.WordLength = Com_WordLength_8b;     //默认
//    SC_com_portMode.StopBits = Com_StopBits_1;          //默认
//    SC_com_portMode.Parity = Com_Parity_No;             //默认
//
//    switch(argv[0])
//    {
//    //-------------------- mode 1 ----------------
//    case 1:
//        addr5bps = argv[1];
//        switch(argv[2])
//        {
//        case 0x00:                        // 已知波特率
//            Sleep(500);
//            IOCTRLInit();
//            IOCTRLSelect( IOCTRL_KLINESEND, IOCTRL_USMODE|IOCTRL_OUTPUT );
//            sendAddressToEcu(addr5bps);    // 发送地址码
//
//            DisableLLine();
//
//            InitUART(COM3,s_Parameter.dwBaudRate,
//                *s_Parameter.p_uartmode);
//            Uart3RXDHandler = IRQ_Uart3RXD;
//            temp = ReadFromECU(&rcvbyte,2550);    // 接收
//            if(temp == FALSE || rcvbyte != 0xAA)
//            {
//                Sleep(4000);
//                InitUART(COM3,5,uartmode);        // 以5波特率发送请求
//                SendToECU(addr5bps,NULL);
//                InitUART(COM3,s_Parameter.dwBaudRate,
//                    *s_Parameter.p_uartmode);
//                temp = ReadFromECU(&rcvbyte,2550);    //
//                if(temp == FALSE || rcvbyte != 0xAA)
//                    return ID_FBREAK;
//            }
//            break;
//        case 0xff:                        // 未知波特率，发送地址码后自动检测波特率
//            Sleep(500);
//            SendToECU(`,NULL);
//
//            DisableLLine();
//
//            if((baudrate = CheckBaudRate(4000))==0)    // 自动监测波特率，4秒超时
//            {
//                Sleep(4000);
//                SendToECU(addr5bps,NULL);
//                if((baudrate = CheckBaudRate(4000))==0)
//                    return ID_FNG;// 自动监测波特率，4秒超时
//            }
//            InitUART(COM3,baudrate,*s_Parameter.p_uartmode);
//            break;
//        default:
//            return ID_FBREAK;
//        }
//        //Sleep(2);
//        if(ReadFromECU(&rcvbyte,1000)==FALSE)
//            return ID_FBREAK;    // 接收
//        if(ReadFromECU(&rcvbyte,2000)==FALSE)
//            return ID_FBREAK;    // 接收
//        Sleep(35);
//        temp = argv[3];
//        if(argv[4] == 0)
//        {
//            switch(temp)
//            {
//            case 0xff:
//                SendToECU(~rcvbyte,NULL);
//                if(ReadFromECU(&rcvbyte,6)==FALSE)
//                    return ID_FBREAK;    // 接收
//            case 0x00:
//                (*commandpos)++;
//                break;
//            default:
//                return ID_FBREAK;
//                break;
//            }
//        }
//        else
//        {
//            group = rcvbyte;        // 保存帧数
//            point = 0;
//            for(i=0;i<group;i++)
//            {
//                index[i].point = point;
//                if((temp = argv[5]) == 0)
//                    return ID_FNG;
//                index[i].len = temp;            // 保存长度
//                while(temp-->0)
//                {
//                    ReadFromECU(&rcvbyte,20);
//                    s_FrameToPC.Buff[point++] = rcvbyte;    //保存帧数据
//                }
//                Sleep(30);
//            }
//            SendMultiFrameToPC(s_FrameToPC.Buff,index,group);
//        }
//        break;
//    //------------------ mode 2 ----------------
//    case 2:       //nobody use it
//        break;
//    default:
//        PutDataToAns(szFNG, ans, sizeof(szFNG));
//        return ID_FNG;
//    }
//    return 0xff;
//}
//}
//------------------------------------------------------------------------------
// Funtion: BOSCH发多帧收多帧函数，在发多帧时，每发一帧，只收一帧  bosch协议专用
// Input  : argc - 参数长度
//          argv - 指令参数缓冲区
//          argv[0]:数据流开关
//          argv[1]:待发送的数据的总帧数
//          argv[2]~argv[n]:第一帧命令，第二帧命令......第N帧命令
// Output : ans - 回复到上位机的数据
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、并正常接收版本信息数据时，ans的格式为：总帧数 第一帧数据  第二帧数据......第N帧数据
//                如ans[] = {00,04,0f,01,f6,b0,36,42,39,30,36,30,33,32,41,20,20,03,17,03,f6,32,2e,30,4c,20,32,56,20,45,
//                           55,32,20,20,20,20,20,20,20,20,20,03,07,05,f6,35,32,31,36,03,08,07,f6,00,00,06,00,00,03}
// Return : 返回的ans缓冲区的有效数据长度
// info:0x610f
//------------------------------------------------------------------------------
int BoschSendDataToEcuGetAnswer_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char frameLen, i;
    unsigned int argvPos, dataLenth = 0;
    SC_PAKSEND Paksend;
    SC_PAKRECV PakRecv;

    PakRecv.Pakdata = (unsigned char *)calloc(264, sizeof(char));
    Paksend.Pakdata = (unsigned char *)calloc(264, sizeof(char));

    //关闭链路
    if (SC_TagKWP1281.m_LinkFG == true)
    {
        SC_TagKWP1281.m_chHoldDataLen = 0;
    }

    SC_TagKWP1281.m_nMScount = 0;

    //帧数
    frameLen = argv[1];
    ans[0] = 0;
    dataLenth = 1;
    argvPos = 2;
    ans[dataLenth++] = frameLen;

    while (TOOL_WAITING != Keep_Link.rightState)
    {
        if (SC_TagKWP1281.m_nMScount > 2500)
        {
            return FALSE; // 接收字节等待超时
        }
    }

    while (frameLen--)
    {
        //每一帧的长度
        Paksend.PakLenth = argv[argvPos] +1;

        //每一帧的数据
        for (i = 0; i < Paksend.PakLenth; i++)
        {
            Paksend.Pakdata[i] = argv[argvPos++];
        }

        if (SC_CML_KWP1281_Send(&Paksend) == FALSE)
        {
            free((void *) (PakRecv.Pakdata));
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        if (SC_CML_KWP1281_Recv(&PakRecv) == FALSE)
        {
            free((void *) (PakRecv.Pakdata));
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        for (i = 0; i < PakRecv.PakLenth; i++)
        {
            ans[dataLenth++] = PakRecv.Pakdata[i];
        }
    }

    //启动链路
    if (SC_TagKWP1281.m_LinkFG == true)
    {
        SC_TagKWP1281.m_chHoldDataLen = 4;
    }

    free((void *) (PakRecv.Pakdata));
    free((void *) (Paksend.Pakdata));
    return dataLenth;
}


//------------------------------------------------------------------------------
// Funtion: BOSCH发一帧收多帧函数  bosch协议专用
// Input  : argc - 参数长度
//          argv - 指令参数缓冲区
//          argv[0]:BOSCH命令的帧头
//          argv[1]:BOSCH命令计数器
//          argv[2]~ argv[argv[0] - 1]:BOSCH命令功能ID
//          argv[argv[0]]:BOSCH命令结束字节03
//          argv[argv[0]+1]:判定是否接收多帧，非1表示收多帧，为1表示收单帧
// Output : ans - 回复到上位机的数据
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、并正常接收版本信息数据时，ans的格式为：总帧数 第一帧数据  第二帧数据......第N帧数据
//                如ans[] = {00,04,0f,01,f6,b0,36,42,39,30,36,30,33,32,41,20,20,03,17,03,f6,32,2e,30,4c,20,32,56,20,45,
//                           55,32,20,20,20,20,20,20,20,20,20,03,07,05,f6,35,32,31,36,03,08,07,f6,00,00,06,00,00,03}
// Return : 返回的ans缓冲区的有效数据长度
// 0x610a
//------------------------------------------------------------------------------
int BoschSendDataToEcuGetMultiFrameAnswer_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned int temp;
    unsigned char i, dataLenth;
    SC_PAKSEND Paksend;
    SC_PAKRECV PakRecv;

    PakRecv.Pakdata = (unsigned char *)
    calloc(264, sizeof(char));
    Paksend.Pakdata = (unsigned char *)
    calloc(264, sizeof(char));

    //关闭链路
    if (SC_TagKWP1281.m_LinkFG == true)
    {
        SC_TagKWP1281.m_chHoldDataLen = 0;
    }

    SC_TagKWP1281.m_nMScount = 0;
    temp = argv[0] +1;
    ans[0] = 0;
    ans[1] = 0;
    dataLenth = 2;

    timer_open_flag = 1; //mod by hejm

    while (TOOL_WAITING != Keep_Link.rightState)
    {
        if (SC_TagKWP1281.m_nMScount > 2500)
        {
            return FALSE; // 接收字节等待超时
        }
    }

    //接受多帧
    if (argv[temp] != 1)
    {
        // mode 0
        Paksend.PakLenth = argv[0] +1;

        for (i = 0; i < Paksend.PakLenth; i++)
        {
            Paksend.Pakdata[i] = argv[i];
        }

        //Paksend.Pakdata =  &argv[0];
        if (SC_CML_KWP1281_Send(&Paksend) == FALSE)
        {
            free((void *) (Paksend.Pakdata));
            free((void *) (PakRecv.Pakdata));
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        if (SC_CML_KWP1281_Recv(&PakRecv) == FALSE)
        {
            free((void *) (Paksend.Pakdata));
            free((void *) (PakRecv.Pakdata));
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        ans[1] ++;

        for (i = 0; i < PakRecv.PakLenth; i++)
        {
            ans[dataLenth++] = PakRecv.Pakdata[i];
        }

        Paksend.PakLenth = 4;
        Paksend.Pakdata[0] = 0x03;
        Paksend.Pakdata[1] = 0x00;
        Paksend.Pakdata[2] = 0x09;
        Paksend.Pakdata[3] = 0x03;

        while (1)
        {
            if (SC_CML_KWP1281_Send(&Paksend) == FALSE)
            {
                free((void *) (Paksend.Pakdata));
                free((void *) (PakRecv.Pakdata));
                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                return ID_FBREAK;
            }

            if (SC_CML_KWP1281_Recv(&PakRecv) == FALSE)
            {
                free((void *) (Paksend.Pakdata));
                free((void *) (PakRecv.Pakdata));
                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                return ID_FBREAK;
            }

            if ((PakRecv.Pakdata[2] == 0x09) || (PakRecv.Pakdata[2] == 0xF7))
            {
                break;
            }

            if (PakRecv.PakLenth < 3)
            {
                break;
            }
            else
            {
                ans[1] ++;

                for (i = 0; i < PakRecv.PakLenth; i++)
                {
                    ans[dataLenth++] = PakRecv.Pakdata[i];
                }
            }
        }
    }

    //只接受单帧
    else
    {
        // mode 1
        Paksend.PakLenth = argv[0] +1;

        for (i = 0; i < Paksend.PakLenth; i++)
        {
            Paksend.Pakdata[i] = argv[i];
        }

        if (SC_CML_KWP1281_Send(&Paksend) == FALSE)
        {
            free((void *) (Paksend.Pakdata));
            free((void *) (PakRecv.Pakdata));
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        if (SC_CML_KWP1281_Recv(&PakRecv) == FALSE)
        {
            free((void *) (Paksend.Pakdata));
            free((void *) (PakRecv.Pakdata));
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        ans[1] ++;

        for (i = 0; i < PakRecv.PakLenth; i++)
        {
            ans[dataLenth++] = PakRecv.Pakdata[i];
        }
    }

    //启动链路
    if (SC_TagKWP1281.m_LinkFG == true)
    {
        SC_TagKWP1281.m_chHoldDataLen = 4;
    }

    free((void *) (PakRecv.Pakdata));
    free((void *) (Paksend.Pakdata));
    return dataLenth;
}


//------------------------------------------------------------------------------
// Funtion: 地址码触发函数，用于发送地址码，通过55H计算波特率，接收KW1,KW2,
//          发送KW2取反。常用于地址码ISO协议、地址码KWP协议、地址码单字节协议
// Input  : argc - 参数长度
//          argv - 参数
//          argv[0]:地址码发送模式。值为1表示模式1，值为2表示模式2，值为3表示模式3
//     模式1  发送5波特率地址码
//          argv[1]:地址码值
//          argv[2]:判断是否自动计算波特率。为0表示不自动计算，为0xFF表示自动计算波特率
//          argv[3]:判断是否将KW2取反发回到ECU。为0表示不取反发回，为0xff表示取反发回
//          argv[4]:如argv[3]非0，argv[4]为0表示必须要收到ECU发来的地址码取反字节，如argv[3]为0，argv[4]为0表示
//                  发完地址码后不再进行任何收发动作，如argv[4]非0，则argv[4]表示要从ECU接受的数据的帧数
//          argv[5]~arg[5+argv[4]-1]:如argv[4]为0，这些字节无意义,如argv[4]非0，这些字节分别表示接收的每一帧数据的长度
//--------------------------------------------------
//     模式2 发送设定的地址码或200bps地址码
//          argv[1]:地址码值
//          argv[2]:最高位为1表示需要计算地址码的波特率，并作为计算波特率的一个参数，否则表示地址码波特率为200.
//          argv[3]:argv[2]最高位为1时，argv[3]为计算波特率的一个参数，argv[2]最高位不为1时，argv[3]无意义
//          argv[4]:判断是否将KW2取反发回到ECU。为0表示不取反发回，为0xff表示取反发回
//          argv[5]:如argv[4]非0，argv[5]为0表示必须要收到ECU发来的地址码取反字节，如argv[4]为0，argv[5]为0表示
//                  发完地址码后不再进行任何收发动作，如argv[5]非0，则argv[5]表示要从ECU接受的数据的帧数
//          argv[6]~arg[6+argv[5]-1]:如argv[4]为0，这些字节无意义,如argv[5]非0，这些字节分别表示接收的每一帧数据的长度
//--------------------------------------------------
//     模式3 发送200bps地址码
//          argv[1]:地址码值
//          argv[2]:判断是否自动计算波特率。为0表示不自动计算，为0xFF表示自动计算波特率
//          argv[3]:判断是否将KW2取反发回到ECU。为0表示不取反发回，为0xff表示取反发回
//          argv[4]:如argv[3]非0，argv[4]为0表示必须要收到ECU发来的地址码取反字节，如argv[3]为0，argv[4]为0表示
//                  发完地址码后不再进行任何收发动作，如argv[4]非0，则argv[4]表示要从ECU接受的数据的帧数
//          argv[5]~arg[5+argv[4]-1]:如argv[4]为0，这些字节无意义,如argv[4]非0，这些字节分别表示接收的每一帧数据的长度
// Output : ans - 回复到上位机的数据:
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、如在ECU发完KW1,KW2后，还有帧数据接收，则，ans的内容为：接收总帧数 第一帧数据长度 第一帧数据
//                   第二帧数据长度 第二帧数据......第n帧数据长度，第n帧数据(总帧数用两个字节表示)
//                   如ans[] = {0x00,0x02,0x05,0x81,0x10,0xf1,0x81,0x03,0x06，0x82,0x10,0xf1,0x04,0x05,0x08c}
//                4、如在ECU发完KW1,KW2后，无帧数据接收，则返回FF 00
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 6005
//------------------------------------------------------------------------------
int Set5BpsParameter_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned int i, argvPosition, dataLenth = 0, baudrate;
    unsigned char k, j, addrCode, keyWord[3] =
    {
        0
    };
    delay(5);

    //index[group].point = rcvpos;
    //60,05,01,00,ff,ff,00,00,64
    //模式
    TimerRelease(TIMER0);

    switch (argv[0])
    {
        //-------------------- mode 1 发送5bps地址码----------------
        case 1:
            // 发送5波特地址码
            switch (argv[2])
            {
                case 0x00:
                    delay(500);
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);

                    //发送5bps地址码
                    sendAddressToEcu(argv[1]);
					
                    if ( *argv == 1 )//加了个判断 拉低的条件 lh
        				DisableLLine_1();
                    ComOpen(COM_ECUPORT, &SC_com_portMode);

                    if ((FALSE == ComByte(&k, 2550)) || (k != 0x55))
                    {
                        delay(4000);
                        IOCTRLInit();
                        IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                        sendAddressToEcu(argv[1]);
                        ComOpen(COM_ECUPORT, &SC_com_portMode);

                        if ((FALSE == ComByte(&k, 2550)) || (k != 0x55))
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            return ID_FBREAK;
                        }
                    }

                    break;

                case 0xff:
                    delay(500);
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                    sendAddressToEcu(argv[1]);
					
					 if ( *argv == 1 )//加了个判断 拉低的条件 lh
        				DisableLLine_1();

                    // 10S 内等待一个 0x55 起始位
                    SC_com_portMode.BaudRate = 0;

                    // 计算波特率
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINEREAD, IOCTRL_USMODE | IOCTRL_INPUT);
                    SC_com_portMode.BaudRate = autoCheckBaudRate();

                    if (SC_com_portMode.BaudRate == 0)
                    {
                        delay(4000);
                        IOCTRLInit();
                        IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);

                        //延时500毫秒
                        IOCTRLSet(IOCTRL_HIGH, 500);

                        //delay(500);
                        //发送5bps地址码
                        sendAddressToEcu(argv[1]);

                        // 10S 内等待一个 0x55 起始位
                        // 计算波特率
                        IOCTRLInit();
                        IOCTRLSelect(IOCTRL_KLINEREAD, IOCTRL_USMODE | IOCTRL_INPUT);
                        SC_com_portMode.BaudRate = autoCheckBaudRate();

                        if (SC_com_portMode.BaudRate == 0)
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            return ID_FBREAK;
                        }
                    }

                    //TagKWP2000.Ecuport.BaudRate = 10400;
                    // 设置 Com 模块
                    //delay(1);//为了防止收第一个关键字时接收到55H的数据造成错误，故在此延时。
                    ComOpen(COM_ECUPORT, &SC_com_portMode);
                    break;

                default:
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
            }

            for (i = 0; i < 2; i++)
            {
                if (FALSE == ComByte(&keyWord[i], 5000))
                {
                    PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                    return ID_FBREAK;
                }
            }

            // KB 取反 60 05 01 46 FF FF 00 00 B2
            if (argv[3] != 0)
            {
                if (argv[3] == 0xff)
                {
                    delay(34);
                    ComSendByte(~keyWord[1]);
                }

                //接收帧数为0的话，再读一个字节,此读到的字节一般为为地址码(取反)发回
                if (argv[4] == 0x00)
                {
                    if (ComByte(&j, 300) == FALSE)
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        return ID_FBREAK;
                    }

                    PutDataToAns(szFOK, ans, sizeof(szFOK));
                    return ID_FOK;
                }
            }

            if (argv[4] == 0)
            {
                PutDataToAns(szFOK, ans, sizeof(szFOK));
                return ID_FOK;
            }
            else
            {
                // 接收回复
                dataLenth = 0;
                ans[dataLenth++] = 0;
                argvPosition = 5;
                ans[dataLenth++] = argv[4]; //接收的帧数

                for (i = 0; i < argv[4]; i++)
                {
                    if (argv[argvPosition] == 0)
                    {
                        PutDataToAns(szFNG, ans, sizeof(szFNG));
                        return ID_FNG;
                    }

                    ans[dataLenth++] = argv[argvPosition];

                    //将接收到的数据放入ANS中
                    for (k = 0; k < argv[argvPosition]; k++)
                    {
                        if (FALSE == ComByte(&ans[dataLenth++], 1000))
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            return ID_FBREAK;
                        }
                    }

                    argvPosition++;
                }
            }

            //在进行正常的帧数据收发前，关闭L线，否则从L线发往ECU的数据可能对通信产生影响。ljy_2011-12-28
            break;

        case 2:
            addrCode = argv[1];
            delay(500);
            IOCTRLInit();
            IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);

            if (argv[2] &0x80)
            {
                baudrate = argv[2] *256 + argv[3];
                baudrate = 65536 - baudrate;
                baudrate = (18432000 / 32) / baudrate; //波特率这里相除是什么意思? hejm
                sendAddressCodeToEcuOnAnyBaudrate(addrCode, baudrate);
                ComOpen(COM_ECUPORT, &SC_com_portMode);

                if ((FALSE == ComByte(&k, 2550)) || (k != 0x55))
                {
                    delay(4000);
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                    sendAddressCodeToEcuOnAnyBaudrate(addrCode, baudrate);
                    ComOpen(COM_ECUPORT, &SC_com_portMode);

                    if ((FALSE == ComByte(&k, 2550)) || (k != 0x55))
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        return ID_FBREAK;
                    }
                }
            }
            else
            {
                sendAddressCodeToEcuOnAnyBaudrate(addrCode, 200);
                ComOpen(COM_ECUPORT, &SC_com_portMode);

                if ((FALSE == ComByte(&k, 2550)) || (k != 0x55))
                {
                    delay(4000);
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                    sendAddressCodeToEcuOnAnyBaudrate(addrCode, 200);
                    DisableLLine_1();
                    ComOpen(COM_ECUPORT, &SC_com_portMode);

                    if ((FALSE == ComByte(&k, 2550)) || (k != 0x55))
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        return ID_FBREAK;
                    }
                }
            }

            for (i = 0; i < 2; i++)
            {
                if (FALSE == ComByte(&keyWord[i], 5000))
                {
                    PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                    return ID_FBREAK;
                }
            }

            if (argv[4] == 0)
            {
                if (argv[5] == 0)
                {
                    PutDataToAns(szFOK, ans, sizeof(szFOK));
                    return ID_FOK;
                }
            }
            else
            {
                if (argv[4] != 0xff)
                {
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
                }

                delay(34);
                ComSendByte(~keyWord[1]);

                if (argv[5] == 0)
                {
                    if (FALSE == ComByte(&k, 5000))
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        return ID_FBREAK;
                    }

                    PutDataToAns(szFOK, ans, sizeof(szFOK));
                    return ID_FOK;
                }
            }

            // 接收回复
            dataLenth = 0;
            ans[dataLenth++] = 0;
            argvPosition = 6;
            ans[dataLenth++] = argv[5]; //接收的帧数

            for (i = 0; i < argv[5]; i++)
            {
                if (argv[argvPosition] == 0)
                {
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
                }

                ans[dataLenth++] = argv[argvPosition];

                //将接收到的数据放入ANS中
                for (k = 0; k < argv[argvPosition]; k++)
                {
                    if (FALSE == ComByte(&ans[dataLenth++], 1000))
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        return ID_FBREAK;
                    }
                }

                argvPosition++;
            }

            break;

        case 3:
            addrCode = argv[1];
            delay(500);

            switch (argv[2])
            {
                //60 05 03 B8 FF FF 01 10
                case 0xff: //自动识别波特率
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                    sendAddressCodeToEcuOnAnyBaudrate(addrCode, 200);
                    EnableLLine_1();
                    SC_com_portMode.BaudRate = 0;

                    // 计算波特率
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINEREAD, IOCTRL_USMODE | IOCTRL_INPUT);
                    SC_com_portMode.BaudRate = autoCheckBaudRate();

                    if (SC_com_portMode.BaudRate == 0)
                    {
                        delay(4000);
                        IOCTRLInit();
                        IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);

                        //延时500毫秒
                        IOCTRLSet(IOCTRL_HIGH, 500);

                        //delay(500);
                        //发送5bps地址码
                        sendAddressCodeToEcuOnAnyBaudrate(addrCode, 200);
                        IOCTRLInit();
                        IOCTRLSelect(IOCTRL_KLINEREAD, IOCTRL_USMODE | IOCTRL_INPUT);

                        // 计算波特率
                        SC_com_portMode.BaudRate = autoCheckBaudRate();

                        if (SC_com_portMode.BaudRate == 0)
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            return ID_FBREAK;
                        }
                    }

                    ComOpen(COM_ECUPORT, &SC_com_portMode);
                    break;

                case 0x00: //指定波特率
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                    sendAddressCodeToEcuOnAnyBaudrate(addrCode, 200);
                    DisableLLine_1();
                    ComOpen(COM_ECUPORT, &SC_com_portMode);

                    if ((FALSE == ComByte(&k, 2550)) || (k != 0x55))
                    {
                        delay(4000);
                        IOCTRLInit();
                        IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                        sendAddressCodeToEcuOnAnyBaudrate(addrCode, 200);
                        ComOpen(COM_ECUPORT, &SC_com_portMode);

                        if ((FALSE == ComByte(&k, 2550)) || (k != 0x55))
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            return ID_FBREAK;
                        }
                    }

                    break;

                default:
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
            }

            for (i = 0; i < 2; i++)
            {
                if (FALSE == ComByte(&keyWord[i], 5000))
                {
                    PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                    return ID_FBREAK;
                }
            }

            switch (argv[3])
            {
                case 0x00:
                    if (argv[4] == 0)
                    {
                        PutDataToAns(szFOK, ans, sizeof(szFOK));
                        return ID_FOK;
                    }

                    break;

                case 0xff:
                    delay(34);
                    ComSendByte(~keyWord[1]);

                    if (argv[4] == 0)
                    {
                        if (FALSE == ComByte(&k, 1000))
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            return ID_FBREAK;
                        }

                        PutDataToAns(szFOK, ans, sizeof(szFOK));
                        return ID_FOK;
                    }

                    break;

                default:
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
            }

            // 接收回复
            dataLenth = 0;
            ans[dataLenth++] = 0;
            argvPosition = 5;
            ans[dataLenth++] = argv[4]; //接收的帧数

            for (i = 0; i < argv[4]; i++)
            {
                if (argv[argvPosition] == 0)
                {
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
                }

                ans[dataLenth++] = argv[argvPosition];

                //将接收到的数据放入ANS中
                for (k = 0; k < argv[argvPosition]; k++)
                {
                    if (FALSE == ComByte(&ans[dataLenth++], 1000))
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        return ID_FBREAK;
                    }
                }

                argvPosition++;
            }

            break;

        default:
            PutDataToAns(szFNG, ans, sizeof(szFNG));
            return ID_FNG;
    }

    return dataLenth;
}


//------------------------------------------------------------------------------
// Funtion: 地址码触发函数，用于发送地址码，通过55H计算波特率，接收KW1,KW2,
//          发送KW2取反。常用于地址码ISO协议、地址码KWP协议、地址码单字节协议
// Input  : argc - 参数长度
//          argv - 参数
//          argv[0]:地址码发送模式。值为1表示模式1，值为2表示模式2，值为3表示模式3
//          argv[1]:地址码值
//          argv[2]:控制L线是否禁能
//     模式1  发送5波特率地址码
//          argv[3]:判断是否自动计算波特率。为0表示不自动计算，为0xFF表示自动计算波特率
//          argv[4]:无意义
//          argv[5]:KW2到KW2取反之间的时间延迟
//          argv[6]:判断是否将KW2取反发回到ECU。为0表示不取反发回，为0xff表示取反发回
//          argv[7]:如argv[6]非0，argv[7]为0表示必须要收到ECU发来的地址码取反字节，如argv[6]为0，argv[7]为0表示
//                  发完地址码后不再进行任何收发动作，如argv[7]非0，则argv[7]表示要从ECU接受的数据的帧数
//          argv[8]~arg[8+argv[7]-1]:如argv[7]为0，这些字节无意义,如argv[7]非0，这些字节分别表示接收的每一帧数据的长度
//--------------------------------------------------
//     模式2 发送设定的地址码或200bps地址码
//          argv[3]:最高位为1表示需要计算地址码的波特率，并作为计算波特率的一个参数，否则表示地址码波特率为200.
//          argv[4]:argv[3]最高位为1时，argv[4]为波特率计算参数，否则无意义
//          argv[5]:无意义
//          argv[6]:KW2到KW2取反之间的时间延迟
//          argv[7]:判断是否将KW2取反发回到ECU。为0表示不取反发回，为0xff表示取反发回
//          argv[8]:如argv[7]非0，argv[8]为0表示必须要收到ECU发来的地址码取反字节，如argv[7]为0，argv[8]为0表示
//                  发完地址码后不再进行任何收发动作，如argv[7]非0，则argv[8]表示要从ECU接受的数据的帧数
//          argv[9]~arg[9+argv[8]-1]:如argv[8]为0，这些字节无意义,如argv[8]非0，这些字节分别表示接收的每一帧数据的长度
//--------------------------------------------------
// Output : ans - 回复到上位机的数据:
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、如在ECU发完KW1,KW2后，还有帧数据接收，则，ans的内容为：接收总帧数 第一帧数据长度 第一帧数据
//                   第二帧数据长度 第二帧数据......第n帧数据长度，第n帧数据(总帧数用两个字节表示)
//                   如ans[] = {0x00,0x02,0x05,0x81,0x10,0xf1,0x81,0x03,0x06，0x82,0x10,0xf1,0x04,0x05,0x08c}
//                4、如在ECU发完KW1,KW2后，无帧数据接收，则返回FF 00
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 601c
//------------------------------------------------------------------------------
int AddressCodeWayAdjustTime_benz_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char rcvlen, rcvframenum, addr_code, obdfid, rcvbyte = 0;
    unsigned char anspos = 0;
    unsigned int baudrate, argvpos;

    addr_code = argv[1];
    obdfid = argv[2];

    //60 1C 01 20 FF FF 00 00 00 01 12
    //60 1C 02 20 FF FF C4 00 00 00 01 12
    switch (argv[0])
    {
        case 1: // mode 1, 以5波特率发送，已知波特率接收
            switch (argv[3])
            {
                case 0x00: // 已知波特率
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                    sendAddressToEcu(addr_code);

                    if (obdfid & 0x80)
                    {
                        EnableLLine_1();
                    }
                    else
                    {
                        DisableLLine_1();
                    }

                    ComOpen(COM_ECUPORT, &SC_com_portMode);

                    if ((ComByte(&rcvbyte, 2550) == FALSE) || (rcvbyte != 0x55))
                    {
                        delay(4000);
                        IOCTRLInit();
                        IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);

                        //延时500毫秒
                        IOCTRLSet(IOCTRL_HIGH, 500);
                        sendAddressToEcu(addr_code);
                        ComOpen(COM_ECUPORT, &SC_com_portMode);

                        if (ComByte(&rcvbyte, 2550) == FALSE)
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            return ID_FBREAK; // 自动监测波特率，4秒超时
                        }

                        if (rcvbyte != 0x55)
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            return ID_FBREAK; // 自动监测波特率，4秒超时
                        }
                    }

                case 0xff: // 自动识别波特率
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);

                    //延时500毫秒
                    IOCTRLSet(IOCTRL_HIGH, 500);
                    sendAddressCodeToEcuOnAnyBaudrate(argv[1], 5);

                    if (obdfid & 0x80)
                    {
                        EnableLLine_1();
                    }
                    else
                    {
                        DisableLLine_1();
                    }

                    // 计算波特率
                    IOCTRLSelect(IOCTRL_KLINEREAD, IOCTRL_USMODE | IOCTRL_INPUT);
                    SC_com_portMode.BaudRate = autoCheckBaudRate();

                    if (SC_com_portMode.BaudRate == 0) // 自动监测波特率，4秒超时
                    {
                        delay(4000);
                        IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                        sendAddressCodeToEcuOnAnyBaudrate(argv[1], 5);

                        // 计算波特率
                        IOCTRLSelect(IOCTRL_KLINEREAD, IOCTRL_USMODE | IOCTRL_INPUT);
                        SC_com_portMode.BaudRate = autoCheckBaudRate();

                        if (SC_com_portMode.BaudRate == 0)
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            return ID_FBREAK; // 自动监测波特率，4秒超时
                        }
                    }

                    // 设置 Com 模块
                    ComOpen(COM_ECUPORT, &SC_com_portMode);
                    argvpos = 4;
                    break;

                default:
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
            }

            break;

        case 2: // mode 2  02 20 FF FF C4 00 00 00 01 12
            delay(500); // 延时500ms

            if (argv[3] &0x80)
            {
                baudrate = argv[3] *0x100;
                baudrate += argv[4];
                baudrate = 65536 - baudrate;
                baudrate = (18432000 / 32) / baudrate;
                IOCTRLInit();
                IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);

                //延时500毫秒
                IOCTRLSet(IOCTRL_HIGH, 500);
                sendAddressCodeToEcuOnAnyBaudrate(addr_code, baudrate);

                //            delay(10);
                ComOpen(COM_ECUPORT, &SC_com_portMode);

                if ((ComByte(&rcvbyte, 2550) == FALSE) || (rcvbyte != 0x55))
                {
                    delay(4000);
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                    sendAddressCodeToEcuOnAnyBaudrate(addr_code, baudrate);
                    ComOpen(COM_ECUPORT, &SC_com_portMode);

                    if (ComByte(&rcvbyte, 2550) == FALSE)
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        return ID_FBREAK; // 自动监测波特率，4秒超时
                    }

                    if (rcvbyte != 0x55)
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        return ID_FBREAK; // 自动监测波特率，4秒超时
                    }
                }
            }
            else
            {
                IOCTRLInit();
                IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                sendAddressCodeToEcuOnAnyBaudrate(addr_code, 200);

                if (obdfid & 0x80)
                {
                    EnableLLine_1();
                }

                ComOpen(COM_ECUPORT, &SC_com_portMode);

                if ((ComByte(&rcvbyte, 2550) == FALSE) || (rcvbyte != 0x55))
                {
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);

                    //延时500毫秒
                    delay(4000);
                    IOCTRLSet(IOCTRL_HIGH, 500);
                    sendAddressCodeToEcuOnAnyBaudrate(addr_code, 200);

                    if (obdfid & 0x80)
                    {
                        EnableLLine_1();
                    }

                    ComOpen(COM_ECUPORT, &SC_com_portMode);

                    if (ComByte(&rcvbyte, 2550) == FALSE)
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        return ID_FBREAK; // 自动监测波特率，4秒超时
                    }

                    if (rcvbyte != 0x55)
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        return ID_FBREAK; // 自动监测波特率，4秒超时
                    }
                }
            }

            argvpos = 5;
            break;

        default:
            PutDataToAns(szFNG, ans, sizeof(szFNG));
            return ID_FNG;
    }

    argvpos++;

    if (ComByte(&rcvbyte, 1000) == FALSE)
    {
        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK; // 自动监测波特率，4秒超时
    }

    if (ComByte(&rcvbyte, 2000) == FALSE)
    {
        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK; // 自动监测波特率，4秒超时
    }

    delay(argv[argvpos++]);

    switch (argv[argvpos++])
    {
        case 0:
            if ((rcvframenum = argv[argvpos++]) == 0)
            {
                PutDataToAns(szFOK, ans, sizeof(szFOK));
                return ID_FOK;
            }

            break;

        case 0xff:
            ComSendByte(~rcvbyte);

            if ((rcvframenum = argv[argvpos++]) == 0)
            {
                if (ComByte(&rcvbyte, 300) == FALSE)
                {
                    PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                    return ID_FBREAK; // 自动监测波特率，4秒超时
                }

                PutDataToAns(szFOK, ans, sizeof(szFOK));
                return ID_FOK;
            }

            break;

        default:
            PutDataToAns(szFNG, ans, sizeof(szFNG));
            return ID_FNG;
    }

    ans[anspos++] = 0;
    ans[anspos++] = rcvframenum;

    do
    {
        if ((rcvlen = argv[argvpos++]) == 0)
        {
            PutDataToAns(szFNG, ans, sizeof(szFNG));
            return ID_FNG;
        }

        ans[anspos++] = rcvlen;

        do
        {
            if (ComByte(&rcvbyte, 300) == FALSE)
            {
                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                return ID_FBREAK; // 自动监测波特率，4秒超时
            }

            ans[anspos++] = rcvbyte;
        }
        while(--rcvlen);
    }
    while(--rcvframenum);

    return anspos;
}


//------------------------------------------------------------------------------
// Funtion: BOSCH地址码触发函数  bosch协议专用
// Input  : argc - 参数长度
//          argv - 指令参数缓冲区
//          argv[0]:一帧数据发送的次数
//          argv[1]:待发送的命令的帧数
//          argv[2]到指令末尾：发送的一帧数据的长度  发送的一帧的数据，一直下去直到指令末尾
// Output : ans - 回复到上位机的数据
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、成功则返回 FF 00
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 6104 Only send data to ecu not receive anyone data from ecu
//------------------------------------------------------------------------------
int OnlySendToEcu_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char argvpos;
    unsigned char CANCONNECTOR = false, sendtime, framenum;
    unsigned char sendlen;

    set_time0Stop();

    if ((argv[3] == 0x55) && (argv[4] == 0xAA))
    {
        CANCONNECTOR = true;
    }

    //发送次数
    if ((sendtime = argv[0]) == 0) // 读取第一个字节，以送次数
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        PutDataToAns(szFNG, ans, sizeof(szFNG));
        return ID_FNG;
    }

    //01 01 FD 00 FA
    while (sendtime--)
    {
        argvpos = 1;

        //帧数
        if ((framenum = argv[argvpos++]) == 0) // 获取通讯帧数
        {
            PutDataToAns(szFNG, ans, sizeof(szFNG));

            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            return ID_FNG;
        }

        while (framenum--)
        {
            //每帧长度
            if ((sendlen = argv[argvpos++]) == 0) // 获取通讯帧长
            {
                PutDataToAns(szFNG, ans, sizeof(szFNG));

                if (timer_open_flag == 1)
                {
                    TimerStart(TIMER0);
                }

                return ID_FNG;
            }

            while (sendlen--)
            {
                if (CANCONNECTOR == true)
                {
                    setCanbusNormalDataToken(&argv[argvpos + 2]);
                    argvpos += sendlen;
                }
                else
                {
                    ComSendByte(argv[argvpos++]);

                    //ziyingzhu 修改 2009-10-29 14:32 每帧单个字节的时候，不需要字节间隔
                    if (sendlen > 1)
                    {
                        delayus(SC_TagKWP2000.m_nBtyetime);
                    }
                }
            }

            delay(SC_TagKWP2000.m_nFrameTime);
        }
    }

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    PutDataToAns(szFOK, ans, sizeof(szFOK));
    return ID_FOK;
}


//------------------------------------------------------------------------------
// Funtion: 地址码触发函数，用于发送地址码，通过55H计算波特率，接收KW1,KW2,
//          发送KW2取反。常用于地址码ISO协议、地址码KWP协议、地址码单字节协议
// Input  : argc - 参数长度
//          argv - 参数
//          argv[0]:地址码发送模式。值为1表示模式1，值为2表示模式2，值为3表示模式3
//          argv[1]:地址码值
//     模式1  发送5波特率地址码
//          argv[2]:判断是否自动计算波特率。为0表示不自动计算，为0xFF表示自动计算波特率
//          argv[3]:判断是否将KW2取反发回到ECU。为0表示不取反发回，为0xff表示取反发回
//          argv[4]:如argv[3]非0，argv[4]为0表示必须要收到ECU发来的地址码取反字节，如argv[3]为0，argv[4]为0表示
//                  发完地址码后不再进行任何收发动作，如argv[3]非0，argv[4]非0，则argv[4]表示要从ECU接受的数据的帧数
//          argv[5]~arg[5+argv[4]-1]:如argv[4]为0，这些字节无意义,如argv[4]非0，这些字节分别表示接收的每一帧数据的长度
//--------------------------------------------------
//     模式2 发送设定的地址码或200bps地址码
//          argv[2]:最高位为1表示需要计算地址码的波特率，并作为计算波特率的一个参数，否则表示地址码波特率为200.
//          argv[3]:argv[2]最高位为1时，argv[3]为波特率计算参数，否则无意义
//          argv[4]:判断是否将KW2取反发回到ECU。为0表示不取反发回，为0xff表示取反发回
//          argv[5]:如argv[4]非0，argv[5]为0表示必须要收到ECU发来的地址码取反字节，如argv[4]为0，argv[5]为0表示
//                  发完地址码后不再进行任何收发动作，如argv[4]非0，则argv[5]表示要从ECU接受的数据的帧数
//          argv[6]~arg[6+argv[5]-1]:如argv[5]为0，这些字节无意义,如argv[5]非0，这些字节分别表示接收的每一帧数据的长度
//--------------------------------------------------
// Output : ans - 回复到上位机的数据:
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、如在ECU发完KW1,KW2后，还有帧数据接收，则，ans的内容为：接收总帧数 第一帧数据长度 第一帧数据
//                   第二帧数据长度 第二帧数据......第n帧数据长度，第n帧数据(总帧数用两个字节表示)
//                   如ans[] = {0x00,0x02,0x05,0x81,0x10,0xf1,0x81,0x03,0x06，0x82,0x10,0xf1,0x04,0x05,0x08c}
//                4、如在ECU发完KW1,KW2后，无帧数据接收，则返回FF 00
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 601d
//------------------------------------------------------------------------------
int AddressCodeCommunicationWay_Lline_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char rcvbyte;
    unsigned int rcvframenum, argvLen, rcvlen, ansposition;

    delay(5);

    switch (argv[0])
    {
        case 1: // mode 1, 以5波特率发送，已知波特率接收
            IOCTRLInit();
            IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);

            //发送5bps地址码
            sendAddressToEcu(argv[1]);

            // 计算波特率
            IOCTRLInit();
            IOCTRLSelect(IOCTRL_KLINEREAD, IOCTRL_USMODE | IOCTRL_INPUT);
            SC_com_portMode.BaudRate = autoCheckBaudRate();

            //TagKWP2000.Ecuport.BaudRate = 10400;
            // 设置 Com 模块
            ComOpen(COM_ECUPORT, &SC_com_portMode);
            break;

        default:
            PutDataToAns(szFNG, ans, sizeof(szFNG));
            return ID_FNG;
    }

    if (ComByte(&rcvbyte, 1000) == FALSE)
    {
        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    if (ComByte(&rcvbyte, 2000) == FALSE)
    {
        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    delay(35);

    //是否取反发回
    switch (argv[3])
    {
        case 0:
            if ((rcvframenum = argv[4]) == 0)
            {
                PutDataToAns(szFOK, ans, sizeof(szFOK));

                //在进行正常的帧数据收发前，关闭L线，否则从L线发往ECU的数据可能对通信产生影响。ljy_2011-12-28
                DisableLLine_1();
                return ID_FOK;
            }

            break;

        case 0xff:
            rcvbyte = 255 - rcvbyte;
            ComSendByte(rcvbyte);

            if ((rcvframenum = argv[4]) == 0)
            {
                if (ComByte(&rcvbyte, 300) == FALSE)
                {
                    PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                    return ID_FBREAK;
                }

                PutDataToAns(szFOK, ans, sizeof(szFOK));

                //在进行正常的帧数据收发前，关闭L线，否则从L线发往ECU的数据可能对通信产生影响。ljy_2011-12-28
                DisableLLine_1();
                return ID_FOK;
            }

            break;

        default:
            PutDataToAns(szFNG, ans, sizeof(szFNG));
            return ID_FNG;
    }

    //在进行正常的帧数据收发前，关闭L线，否则从L线发往ECU的数据可能对通信产生影响。ljy_2011-12-28
    DisableLLine_1();
    ans[0] = rcvframenum;
    argvLen = 5;
    ansposition = 2;
    ans[1] = rcvframenum;

    do
    {
        if ((rcvlen = argv[argvLen++]) == 0)
        {
            PutDataToAns(szFNG, ans, sizeof(szFNG));
            return ID_FNG;
        }

        ans[ansposition++] = rcvlen;

        do
        {
            if (ComByte(&rcvbyte, 1000) == FALSE)
            {
                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                return ID_FBREAK;
            }

            ans[ansposition++] = rcvbyte;
        }
        while(--rcvlen);
    }
    while(--rcvframenum);

    ans[0] = 0;
    return ansposition;
}


//------------------------------------------------------------------------------
// Funtion: BOSCH地址码触发函数  bosch协议专用
// Input  : argc - 参数长度
//          argv - 指令参数缓冲区
//          argv[0]:地址码值
// Output : ans - 回复到上位机的数据
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、当KW2为0x8f时，返回00 00
//                4、当KW2非0x8f，并正常接收版本信息数据时，ans的格式为：总帧数 第一帧数据  第二帧数据......第N帧数据
//                如ans[] = {00,04,0f,01,f6,b0,36,42,39,30,36,30,33,32,41,20,20,03,17,03,f6,32,2e,30,4c,20,32,56,20,45,
//                           55,32,20,20,20,20,20,20,20,20,20,03,07,05,f6,35,32,31,36,03,08,07,f6,00,00,06,00,00,03}
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 6014
//------------------------------------------------------------------------------
int ShangHuanBoschSystem_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned int i, dataLenth = 0;
    unsigned char j, flag_L_Line = 0;
    SC_PAKRECV PakRecv;
    SC_PAKRECV PakSend;

    delay(50);
    TimerRelease(TIMER0);
    flag_L_Line = CheckLLine();
    EnableLLine_1();
    IOCTRLInit();
    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
    sendAddressToEcu(argv[0]); // 以5波特率发送地址码

    if (flag_L_Line == 1)
    {
        EnableLLine_1();
    }
    else
    {
        DisableLLine_1();
    }

    // 计算波特率
    IOCTRLInit();
    IOCTRLSelect(IOCTRL_KLINEREAD, IOCTRL_USMODE | IOCTRL_INPUT);
    SC_com_portMode.BaudRate = autoCheckBaudRate();

    // 设置 Com 模块
    ComOpen(COM_ECUPORT, &SC_com_portMode);

    if (FALSE == ComByte(&j, 1000)) // 接收第一个字节
    {
        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    if (FALSE == ComByte(&j, 2000)) //接收第二个字节
    {
        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    delay(22);
    j = 255 - j;
    ComSendByte(j);
    SC_TagKWP1281.m_nMScount = 0; //定时器中断计数初始化为0
    SC_TagKWP1281.m_chCount = 0; //初始化接收字符数为0
    Keep_Link.rightState = TOOL_READING;

    // 加载串口中断
    ComInterruptFuntionLoad(SC_CML_KWP1281_Comread, COM_INTERREAD);
    ComInterruptFuntionLoad(SC_CML_KWP1281_Comsend, COM_INTERSEND);
    TimerInit();

	//上面这些设置中断的函数顺序不一样 应该不影响程序的运行
	ComReadSign(1);//IDA反汇编多的一个函数 lh 

    if (SC_com_portMode.BaudRate < 5000)
    {
        TimerConfig(TIMER0, 1);
        SC_TagKWP1281.m_nBtyetime = 6;
        SC_TagKWP1281.m_nFrameTime = 30;
        SC_TagKWP1281.m_ReserveTime = 3;
    }
    else
    {
        TimerConfig(TIMER0, 1);
        SC_TagKWP1281.m_nBtyetime = 3;
        SC_TagKWP1281.m_nFrameTime = 22;
        SC_TagKWP1281.m_ReserveTime = 2;
    }

    // 加载链路维持中断
    TimerInterruptFuntionLoad(TIMER0, SC_CML_KWP1281_Time);
    TimerStart(TIMER0);
    timer_open_flag = 1;

    //设置收版本信息时发送的数据包
    PakRecv.Pakdata = (unsigned char *)calloc(264, sizeof(char));
    PakSend.Pakdata = (unsigned char *)calloc(100, sizeof(char));
    PakSend.PakLenth = 4;
    PakSend.Pakdata[0] = 0x03;
    PakSend.Pakdata[1] = 0x00;
    PakSend.Pakdata[2] = 0x09;
    PakSend.Pakdata[3] = 0x03;

    //设置链路数据包
    SC_TagKWP1281.m_chHoldDataLen = 4;
    SC_TagKWP1281.m_chHoldDig[0] = 0x03;
    SC_TagKWP1281.m_chHoldDig[1] = 0x00;
    SC_TagKWP1281.m_chHoldDig[2] = 0x09;
    SC_TagKWP1281.m_chHoldDig[3] = 0x03;

    if (FALSE == SC_CML_KWP1281_Recv(&PakRecv))
    {
        free((void *) (PakSend.Pakdata));
        free((void *) (PakRecv.Pakdata));
        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    dataLenth = 2;
    ans[0] = 0;
    ans[1] = 0;

    while (1)
    {
        if ((PakRecv.Pakdata[2] == 0x09) || (PakRecv.Pakdata[2] == 0xF7))
        {
            break;
        }

        if (PakRecv.PakLenth < 3)
        {
            break;
        }
        else
        {
            ans[1] ++;

            for (i = 0; i < PakRecv.PakLenth; i++)
            {
                ans[dataLenth++] = PakRecv.Pakdata[i];
            }
        }

        if (FALSE == SC_CML_KWP1281_Send(&PakSend))
        {
            free((void *) (PakSend.Pakdata));
            free((void *) (PakRecv.Pakdata));
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        if (FALSE == SC_CML_KWP1281_Recv(&PakRecv))
        {
            free((void *) (PakSend.Pakdata));
            free((void *) (PakRecv.Pakdata));
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }
    }

    free((void *) (PakRecv.Pakdata));
    free((void *) (PakSend.Pakdata));
    return dataLenth;
}


//------------------------------------------------------------------------------
// Funtion: LANDROVER ABS 地址码触发函数
//  tool -> 地址码   ECU->  55H  KW1  KE2
//  tool -> byte1    ECU->  0x42
//  tool -> byte2    ECU->  byte3
//  tool -> byte3    ECU->  byte4
//  tool -> byte4    ECU->  byte5
//  tool -> byte5    ECU->  byte6
//  完成触发
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6011
//------------------------------------------------------------------------------
int LandroverABSEnter_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char j;

    IOCTRLInit();
    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);

    // 发送地址码
    sendAddressToEcu(argv[0]);
    DisableLLine_1();
    ComOpen(COM_ECUPORT, &SC_com_portMode);

    if (FALSE == ComByte(&j, 2550)) // 接收第一个字节
    {
        IOCTRLInit();
        IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);

        // 发送地址码
        sendAddressToEcu(argv[0]);
        ComOpen(COM_ECUPORT, &SC_com_portMode);

        if (FALSE == ComByte(&j, 2550))
        {
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }
    }

    if (FALSE == ComByte(&j, 1000)) //接收第二个字节
    {
        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    if (FALSE == ComByte(&j, 2000)) //接收第三个字节
    {
        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    if (argv[1] != 0x00)
    {
        delay(39);
        ComSendByte(argv[2]);

        if ((FALSE == ComByte(&j, 800)) || (j != 0x42))
        {
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        ComSendByte(argv[3]);
        delay(2);

        if (FALSE == ComByte(&j, 300))
        {
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        ComSendByte(j);

        if (FALSE == ComByte(&j, 300))
        {
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        ComSendByte(j);

        if (FALSE == ComByte(&j, 300))
        {
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        ComSendByte(j);

        if (FALSE == ComByte(&j, 300))
        {
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }
        else
        {
            PutDataToAns(szFOK, ans, sizeof(szFOK));
            return ID_FOK;
        }
    }
    else
    {
        //强制关闭L线//在进行正常的帧数据收发前，关闭L线，否则从L线发往ECU的数据可能对通信产生影响。ljy_2011-12-28
        PutDataToAns(szFOK, ans, sizeof(szFOK));
        return ID_FOK;
    }
}


//-----------------------------------------------------------------------------
// Funtion: 类似于Set5BpsParameter   对ECU发来的关键字不取反发回
// Input  : argc - 参数长度
//          argv - 参数
//          argv[0]:地址码
// Output : ans - 回复到上位机的数据格式 总帧数 帧长度 帧数据
//          ans[] = { 00 01 02 08 09}
// Return : 返回ans的有效数据长度
// Info   : 6009
//-----------------------------------------------------------------------------
int BoschFiat_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char j;
    unsigned char anslen = 0;

    IOCTRLInit();
    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);

    //延时500毫秒
    //delay(500);
    sendAddressToEcu(argv[0]);
    IOCTRLSelect(IOCTRL_KLINEREAD, IOCTRL_USMODE | IOCTRL_INPUT);

    // 10S 内等待一个 0x55 起始位
    DisableLLine_1();
    SC_com_portMode.BaudRate = autoCheckBaudRate();

    if (SC_com_portMode.BaudRate == 0)
    {
        delay(40000);
        IOCTRLInit();
        IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);

        //延时500毫秒
        //delay(500);
        sendAddressToEcu(argv[0]);
        IOCTRLSelect(IOCTRL_KLINEREAD, IOCTRL_USMODE | IOCTRL_INPUT);

        // 10S 内等待一个 0x55 起始位
        SC_com_portMode.BaudRate = autoCheckBaudRate();

        if (SC_com_portMode.BaudRate == 0)
        {
            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }
    }

    //TagKWP2000.Ecuport.BaudRate = 10400;
    // 初始化 Com 模块
    ComOpen(COM_ECUPORT, &SC_com_portMode);

    // KB1 - KBn
    if (FALSE == ComByte(&j, 1000))
    {
        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    ans[anslen++] = 0;
    ans[anslen++] ++;
    ans[anslen++] = 2;
    ans[anslen++] = j;

    if (FALSE == ComByte(&j, 2000))
    {
        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    //在进行正常的帧数据收发前，关闭L线，否则从L线发往ECU的数据可能对通信产生影响。ljy_2011-12-28
    ans[anslen++] = j;
    return anslen;
}


//------------------------------------------------------------------------------
// Funtion: 发送地址码到ECU，返回六个字节，对第三个字节取返发回。
// Input  : argc - 参数长度
//          argv - 指令参数缓冲区
//          argv[0]:地址码发送模式
//          argv[1]:地址码值
//          argv[2]:判断是否自动计算波特率，为0表示使用已知波特率，为0xff表示自动计算波特率
//          argv[3]:判断是否继续从ECU接受数据，为0表示不接收数据，为0xff表示继续接受数据
//          argv[4]:指定从ECU接收的数据的总帧数
// Output : ans - 回复到上位机的数据
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、不接收ECU的帧数据，则返回FF 00
//                4、当从ECU接受指定的帧数的数据时，但不接收五个关键字时，ans的格式为：总帧数(总帧数固定用两个字节表示) 第一帧长度 第一帧数据
//                   第二帧长度 第二帧数据......第N帧长度 第N帧数据
//                   如ans[] = {00,04,0f,0E,f6,b0,36,42,39,30,36,30,33,32,41,20,20,05,17,16,f6,32,2e,30,4c,20,32,56,20,45,
//                              55,32,20,20,20,20,20,20,20,20,20,04,07,06,f6,35,32,31,36,19,08,07,f6,00,00,06,00,00,03}
//                   当从ECU接受指定的帧数的数据时，且接收五个关键字时，ans的格式为：总帧数(总帧数固定用两个字节表示) 第一帧长度 第一帧数据
//                   第二帧长度 第二帧数据......第N帧长度 第N帧数据
//                   如ans[] = {00,05,05,01,02,03,04,05,0f,0E,f6,b0,36,42,39,30,36,30,33,32,41,20,20,05,17,16,f6,32,2e,30,4c,20,32,56,20,45,
//                              55,32,20,20,20,20,20,20,20,20,20,04,07,06,f6,35,32,31,36,19,08,07,f6,00,00,06,00,00,03}
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 600a
//-------------------------------------------------------------------
// SetAddressFiat :发送地址码到ECU，返回六个字节，对第三个字节取返发回。
// 0x600a   0k 20080531
//-------------------------------------------------------------------
int SetAddressFiat_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned int i, framenum, framelenth, anslen;
    unsigned char k, j;

    ans[0] = 0;
    ans[1] = 0;

    switch (argv[0])
    {
        //-------------------- mode 1 ----------------
        case 1:
            //-------------------- mode 2 ----------------
        case 2: // 2008-12-8, add
            switch (argv[2])
            {
                case 0x00:
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);

                    //发送5bps地址码
                    delay(500);
                    sendAddressToEcu(argv[1]);
                    DisableLLine_1();
                    ComOpen(COM_ECUPORT, &SC_com_portMode);

                    if ((FALSE == ComByte(&k, 2550)) || (k != 0x55))
                    {
                        delay(4000);
                        IOCTRLInit();
                        IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                        sendAddressToEcu(argv[1]);
                        ComOpen(COM_ECUPORT, &SC_com_portMode);

                        if ((FALSE == ComByte(&k, 2550)) || (k != 0x55))
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            return ID_FBREAK;
                        }
                    }

                    break;

                case 0xff:
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                    delay(500);

                    //发送5bps地址码
                    sendAddressToEcu(argv[1]);
                    DisableLLine_1();

                    // 10S 内等待一个 0x55 起始位
                    SC_com_portMode.BaudRate = 0;

                    // 计算波特率
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINEREAD, IOCTRL_USMODE | IOCTRL_INPUT);
                    SC_com_portMode.BaudRate = autoCheckBaudRate();

                    if (SC_com_portMode.BaudRate == 0)
                    {
                        delay(4000);
                        IOCTRLInit();
                        IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);

                        //延时500毫秒
                        IOCTRLSet(IOCTRL_HIGH, 500);

                        //delay(500);
                        //发送5bps地址码
                        sendAddressToEcu(argv[1]);
                        IOCTRLInit();
                        IOCTRLSelect(IOCTRL_KLINEREAD, IOCTRL_USMODE | IOCTRL_INPUT);

                        // 计算波特率
                        SC_com_portMode.BaudRate = autoCheckBaudRate();

                        if (SC_com_portMode.BaudRate == 0)
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            return ID_FBREAK;
                        }
                    }

                    //TagKWP2000.Ecuport.BaudRate = 10400;
                    // 设置 Com 模块
                    //delay(1);//为了防止收第一个关键字时接收到55H的数据造成错误，故在此延时。
                    ComOpen(COM_ECUPORT, &SC_com_portMode);
                    break;

                default:
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
            }

            //接受五个关键字
            for (i = 0; i < 5; i++)
            {
                if (FALSE == ComByte(&j, 2000))
                {
                    PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                    return ID_FBREAK;
                }

                if (1 == i)
                {
                    k = j;
                }
            }

            switch (argv[3])
            {
                case 0x00:
                    //强制关闭L线//在进行正常的帧数据收发前，关闭L线，否则从L线发往ECU的数据可能对通信产生影响。ljy_2011-12-28
                    DisableLLine_1();
                    PutDataToAns(szFOK, ans, sizeof(szFOK));
                    return ID_FOK;

                case 0xff:
                    DisableLLine_1();
                    delay(5);
                    k = 255 - k;
                    ComSendByte(k);

                    //强制关闭L线//在进行正常的帧数据收发前，关闭L线，否则从L线发往ECU的数据可能对通信产生影响。ljy_2011-12-28
                    j = 0;
                    framenum = argv[4];

                    if (framenum == 0)
                    {
                        PutDataToAns(szFNG, ans, sizeof(szFNG));
                        return ID_FNG;
                    }

                    ans[1] = framenum;
                    anslen = 2;

                    for (i = 0; i < framenum; i++)
                    {
                        if (FALSE == ComByte(&j, 500))
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            return ID_FBREAK;
                        }

                        framelenth = j;
                        ans[anslen++] = j + 1;
                        ans[anslen++] = j;

                        //将接收到的数据放入ANS中
                        for (k = 0; k < framelenth; k++)
                        {
                            if (FALSE == ComByte(&j, 300))
                            {
                                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                                return ID_FBREAK;
                            }

                            ans[anslen++] = j;
                        }
                    }

                    break;

                default:
                    break;
            }

            break;

        //-------------------- mode 3 ----------------
        case 3: // 2008-12-8, add
            switch (argv[2])
            {
                case 0x00:
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);

                    //发送5bps地址码
                    delay(500);
                    sendAddressToEcu(argv[1]);
                    DisableLLine_1();
                    ComOpen(COM_ECUPORT, &SC_com_portMode);

                    if ((FALSE == ComByte(&k, 2550)) || (k != 0x55))
                    {
                        delay(4000);
                        IOCTRLInit();
                        IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                        sendAddressToEcu(argv[1]);
                        ComOpen(COM_ECUPORT, &SC_com_portMode);//COM_ECUPORT = 1

                        if ((FALSE == ComByte(&k, 2550)) || (k != 0x55))
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            return ID_FBREAK;
                        }
                    }

                    break;

                case 0xff:
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
                    delay(500);

                    //发送5bps地址码
                    sendAddressToEcu(argv[1]);
                    DisableLLine_1();

                    // 10S 内等待一个 0x55 起始位
                    SC_com_portMode.BaudRate = 0;

                    // 计算波特率
                    IOCTRLInit();
                    IOCTRLSelect(IOCTRL_KLINEREAD, IOCTRL_USMODE | IOCTRL_INPUT);
                    SC_com_portMode.BaudRate = autoCheckBaudRate();

                    if (SC_com_portMode.BaudRate == 0)
                    {
                        delay(4000);
                        IOCTRLInit();
                        IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);

                        //延时500毫秒
                        IOCTRLSet(IOCTRL_HIGH, 500);

                        //delay(500);
                        //发送5bps地址码
                        sendAddressToEcu(argv[1]);
                        IOCTRLInit();
                        IOCTRLSelect(IOCTRL_KLINEREAD, IOCTRL_USMODE | IOCTRL_INPUT);

                        // 计算波特率
                        SC_com_portMode.BaudRate = autoCheckBaudRate();

                        if (SC_com_portMode.BaudRate == 0)
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            return ID_FBREAK;
                        }
                    }

                    //TagKWP2000.Ecuport.BaudRate = 10400;
                    // 设置 Com 模块
                    //delay(1);//为了防止收第一个关键字时接收到55H的数据造成错误，故在此延时。
                    ComOpen(COM_ECUPORT, &SC_com_portMode);
                    break;

                default:
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
            }

            //接受五个关键字
            //case2与case3的区别就在于case3把五个关键字也作为返回的数据了
            anslen = 3;

            for (i = 0; i < 5; i++)
            {
                if (FALSE == ComByte(&j, 2000))
                {
                    PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                    return ID_FBREAK;
                }

                ans[anslen++] = j;

                if (1 == i)
                {
                    k = j;
                }
            }

            ans[1] = 1;
            ans[2] = 5;

            //强制关闭L线//在进行正常的帧数据收发前，关闭L线，否则从L线发往ECU的数据可能对通信产生影响。ljy_2011-12-28
            DisableLLine_1();

            if (argv[3] != 0xff)
            {
                PutDataToAns(szFOK, ans, sizeof(szFOK));
                return ID_FOK;
            }
            else
            {
                // 接收回复
                j = 0;
                framenum = argv[4];

                if (framenum < 2)
                {
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
                }

                ans[1] += framenum;

                for (i = 1; i < framenum; i++)
                {
                    if (FALSE == ComByte(&j, 500))
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        return ID_FBREAK;
                    }

                    framelenth = j;
                    ans[anslen++] = j + 1;
                    ans[anslen++] = j;

                    //将接收到的数据放入ANS中
                    for (k = 0; k < framelenth; k++)
                    {
                        if (FALSE == ComByte(&j, 300))
                        {
                            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                            return ID_FBREAK;
                        }

                        ans[anslen++] = j;
                    }
                }
            }

            break;

        default:
            PutDataToAns(szFNG, ans, sizeof(szFNG));
            return ID_FNG;
    }

    return anslen;
}


//------------------------------------------------------------------------------
//函数名称:ReceiveOneKwpFrameFromECU
//函数功能:从ECU获取一帧信息
//函数的参数:buff接受缓冲区
//          :size接受的总的长度
//          :overtime接受时帧与帧之间的间隔，字节与字节之间的间隔
//          :speci_data.0x7f,0x78所对应的字节内容
//函数的返回值:true  成功
//            false 失败
//与LPC2368相比，这里的函数直接把0x7f,0x78直接
//在这里实现的是接受一帧kwp指令，在指令中没有关闭和开启链路保持。
//------------------------------------------------------------------------------
unsigned int ReceiveOneKwpFrameFromECU(unsigned char * buff, unsigned int * size,
    unsigned int overtime,
    unsigned char * speci_data)
{
    unsigned int i = 0;
    unsigned int Time = 0;
    unsigned int length = 0; //数据区长度 + 校验字节
    unsigned char PakType = 0;

    *size = 0;

    //------------------------------------------------------------------
    Time = overtime; //接受第一帧等待时间是，最大等待时间

    if (FALSE == ComByte(&buff[0], Time)) //读取第一个字节。可以通过读取的第一个字节来确定类型和长度
    {
        return FALSE;
    }

    //-------------------------------------------------------------------
    Time = overtime;
    PakType = buff[0] &0xc0; //数据包类型

    //if( CMPISO14230_FMT_RESERVE == PakType )//0x40
    if (PakType == 0x40)
    {
        return FALSE;
    }
    else if ((PakType >= 0x80) && ! (buff[0] &0x3f)) // 80 10 f1 01 21 sum
    {
        // C0 10 f1 02 21 sum
        length = 0;

        for (i = 1; i <= length + 4; i++) //要接受校验位
        {
            if (FALSE == ComByte(&buff[i], Time))
            {
                return FALSE;
            }

            if (i == 3)
            {
                length = buff[3];
            }
        }

        speci_data[0] = buff[4];
        speci_data[1] = buff[6];
        (*size) = length + 5;
    }
    else if (PakType >= 0x80) // 81 10 f1 21 sum, C1 10 f1 21 sum
    {
        length = buff[0] &0x3f;

        for (i = 1; i <= length + 3; i++) //要接受校验位
        {
            if (FALSE == ComByte(&buff[i], Time))
            {
                return FALSE;
            }
        }

        speci_data[0] = buff[3];
        speci_data[1] = buff[5];
        (*size) = length + 4;
    }
    else if (! (buff[0] &0x3f)) // 00 01 21 sum
    {
        length = 0;

        for (i = 1; i <= length + 2; i++) //要接受校验位
        {
            if (FALSE == ComByte(&buff[i], Time))
            {
                return FALSE;
            }

            if (i == 1)
            {
                length = buff[1];
            }
        }

        speci_data[0] = buff[2];
        speci_data[1] = buff[4];
        *size = length + 3;
    }
    else //0x02,0x21,0x01 0xcs
    {
        length = buff[0];

        for (i = 1; i <= length + 1; i++)
        {
            if (FALSE == ComByte(&buff[i], Time))
            {
                return FALSE;
            }
        }

        speci_data[0] = buff[1];
        speci_data[1] = buff[3];
        *size = length + 2;
    }

    return TRUE;
}


//------------------------------------------------------------------------------
// Funtion: KWP协议发多帧收多帧，对于发送的每一帧都有一帧的应答.
// Input  : argc - 参数长度
//          argv - 发送次数+发送帧数+发送第一帧的长度+发送帧的内容……
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6109
//1.发送给上位机的数据格式是: 0x00 ,0x00, 0xlen 一帧长度 一帧内容 。。。。。。
//                            -    ----------   ---------------------
//                            |        |                 |记录的是实际的内容。
//                            |        |帧数记录。
//                            |数据标志位，（0xff表示的是设置）。
//2.argv的缓冲区的数据格式：
//  发送次数 +  发送帧数 + 发送第一帧长度 + 发送第一帧内容
//3.实现的功能是：发多帧收多帧，并且在发送的过程中是发一帧收一帧。
//4.在函数中能够对0X7f,0x78进行处理。
//5.函数例程:
//req: 82 10 F1 21 01 A5
//ans: 82 f1 10 61 01 cc
//req: 80 10 F1 02 21 02 A6
//ans: 80 f1 10 03 7f 02 78 ef
//req: 00 02 21 01 24
//ans: 00 02 61 01 64
//req: 02 21 01 24
//ans: 02 61 01 64
//------------------------------------------------------------------------------
int KWPSendDataToEcuGetAnswer_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned int frame_number = argv[1]; //发送的帧数
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int frame_length = 0;
    unsigned int frame_pos = 2;
    static unsigned char buff[200];
    unsigned int receive_len = 0;
    unsigned char speci_data[2] =
    {
        0
    };

    //用于存储0x7f,0x78.
    unsigned short receive_number = 0; //用两个字节来表示长度
    unsigned char * ans_head = &ans[0]; //接受缓冲区的帧数的存储位置。
    unsigned int receive_total_length = 0;
    unsigned char check_byte = 0;
	unsigned char packData;

    ans = ans + 2; //0x00 0x00 0xlen 0x

    if (argc < 0)
    {
        memset(ans, 0, strlen(ans));
        ans_head[0] = 0xff;
        ans_head[1] = 0x02;
        return 0x02;
    }

    set_time0Stop();

//ida反汇编分析 多出一个循环接收接收的判读
	if ( ComByte_sub_80B19E2(packData, 0) == 1 )
	{
		do
		{
			if ( ComByte_sub_80B19E2(packData, 5) != 1 )
			  break;
			Count = step++;
		}
		while ( Count < 0x33 );                   // 一直循环收0x33 次 
	}
	

    for (i = 0; i < frame_number; i++)
    {
        frame_length = argv[frame_pos];

        for (j = 0; j < frame_length - 1; j++) //注意每帧的长度里面包含了校验位
        {
            ComSendByte(argv[frame_pos + j + 1]);
            check_byte += argv[frame_pos + j + 1]; //计算累加校验和
            delayus(SC_TagKWP2000.m_nBtyetime);
        }

        {
            //发送校验位
            ComSendByte(check_byte);
            check_byte = 0;
        }

        while (1)
        {
            if (ReceiveOneKwpFrameFromECU(&buff[1], &receive_len, SC_TagKWP2000.m_Maxwaittime, speci_data) == 0)
            {
                //memset(ans, 0, sizeof(ans));
                ans_head[0] = 0xff;
                ans_head[1] = 0x02;
                return 0x02;
            }

            if ((speci_data[0] == 0x7f) && (speci_data[1] == 0x78))
            {
                continue;
            }
            else
            {
                break;
            }
        };

        buff[0] = receive_len;
        memmove(ans, buff, receive_len + 1);
        receive_number++;
        ans = ans + receive_len + 1;
        frame_pos += (frame_length + 1); //获取长度所在的位置.
        receive_total_length += receive_len + 1;
    }

    ans_head[0] = 0x00;
    ans_head[1] = receive_number & 0x00ff;

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    return receive_total_length + 2;
}


//------------------------------------------------------------------------------
// Funtion: KWP协议发一帧收多帧
// Input  : argc - 参数长度
//          argv - 0xlen,0x内容,最大等待时间
// Output : ans  - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 610b
//1.发送给上位机的数据格式是: 0x00 ,0x00, 0xlen 一帧长度 一帧内容 。。。。。。
//                            -    ----------   ---------------------
//                            |        |                 |记录的是实际的内容。
//                            |        |帧数记录。
//                            |数据标志位，（0xff表示的是设置）。
//2.argv的缓冲区的数据格式：
//  发送次数 +  发送帧数 +  + 发送第一帧长度 + 发送第一帧内容
//3.实现的功能是：发多帧收多帧，并且在发送的过程中是发一帧收一帧。
//4.在函数中能够对0X7f,0x78进行处理。
//5.函数例程：
//req: 82 10 F1 21 03 A7
//ans: 1n 82 f1 10 61 01 A6
//ans: 1n 82 f1 10 61 02 A6
//ans: 1n 82 f1 10 61 02 A6
//ans:    82 f1 10 61 02 cc

// 这里只是发送一帧数据 hejm
// 所以argv的缓冲区的数据格式是：
// 帧长度 + 帧内容
//------------------------------------------------------------------------------
int KWPSendOneAndReceiveMultiFrame_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned int send_length = argv[0];
    unsigned int i = 0;
    unsigned int time = 0;
    unsigned char buff[200] =
    {
        0
    };
    unsigned char receive_number = 0;
    unsigned int receive_length = 0;
    unsigned char speci_data[2] =
    {
        0
    };
    unsigned char check_byte = 0;
    unsigned char * ans_head = ans;
    unsigned int receive_total_length = 0;

    ans = ans + 2;
    set_time0Stop();

    for (i = 0; i < send_length - 1; i++)
    {
        ComSendByte(argv[i + 1]);
        check_byte += argv[i + 1];
        delayus(SC_TagKWP2000.m_nBtyetime); //现在默认的时间是字节与字节之间的时间间隔是5ms

        //delay(1);
    }

    {
        //发送校验位
        ComSendByte(check_byte);
        check_byte = 0;
    }

    time = CalculateTimeOut(argv[i + 2], 10) * 10;

    //delay(1);
    while (ReceiveOneKwpFrameFromECU(&buff[1], &receive_length, time, speci_data) == true) //注意在这里接受的帧数是
    {
        if (speci_data[0] == 0x7f && speci_data[1] == 0x78)
        {
            time = 5000;
            continue;
        }

        buff[0] = receive_length;
        memmove(ans, buff, receive_length + 1);
        ans = ans + receive_length + 1;
        receive_number++;
        receive_total_length += receive_length + 1;
		if (receive_total_length > 0xE10 )//add by lh ida反编译代码添加200708 
		{
			ans_head[0] = 0;
			ans_head[1] = receive_number;
			if (timer_open_flag == 1)
				TimerStart(TIMER0);
			return receive_total_length+2;
		}
    }

    if (receive_number == 0)
    {
        ans_head[0] = 0xff;
        ans_head[1] = 0x02;
        return 2;
    }
    else
    {
        ans_head[0] = 0x00;
        ans_head[1] = receive_number & 0x00ff;

        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        return receive_total_length + 2;
    }
}


//------------------------------------------------------------------------------
// Funtion: KWP协议发多帧收多帧。每帧的长度可以超过0xff
// Input  : argc - 参数长度
//          argv - 0xlen,0x内容,最大等待时间
// Output : ans  - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6211
//1.发送给上位机的数据格式是: 0x00 ,0x00, 0xlen 一帧长度 一帧内容.......
//                            -    ----------   ---------------------
//                            |        |                 |记录的是实际的内容。
//                            |        |帧数记录。
//                            |数据标志位，（0xff表示的是设置）。
//2.argv的缓冲区的数据格式：
//  发送次数 +  发送帧数  + 发送第一帧长度高位 + 发送第一帧长度低位 + 第一帧内容 ......
//3.实现的功能是：发多帧收多帧，长度可以超过0xff.
//4.在函数中能够对0X7f,0x78进行处理。
//5.函数例程：
//req: 82 10 F1 22 01 A5
//ans: 82 f1 10 62 01 cc
//req: 80 10 F1 02 22 02 A6
//ans: 80 f1 10 03 77 02 78 ef
//req: 00 02 22 01 24
//------------------------------------------------------------------------------
int KWPSendDataToEcuGetAnswer_more_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned int send_number = argv[1]; //发送的帧数
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int send_length = 0;
    unsigned int send_pos = 2;
    unsigned char buff[200];
    unsigned int receive_len = 0;
    unsigned char speci_data[2] =
    {
        0
    };

    //用于存储0x7f,0x78.
    unsigned short receive_number = 0; //用两个字节来表示长度
    unsigned char * ans_head = &ans[0]; //接受缓冲区的帧数的存储位置。
    unsigned int receive_total_length = 0;
    unsigned char check_byte = 0;

    //ans = ans + 3;
    //修改2012-12-23
    ans = ans + 2;
    set_time0Stop();

    for (i = 0; i < send_number; i++)
    {
        send_length = argv[send_pos] *0x100 + argv[send_pos + 1];

        for (j = 0; j < send_length - 1; j++)
        {
            ComSendByte(argv[send_pos + j + 2]);
            check_byte += argv[send_pos + j + 2]; //计算累加校验和
            delayus(SC_TagKWP2000.m_nBtyetime);
        }

        {
            ComSendByte(check_byte);
            check_byte = 0;
        }

        while (1)
        {
            if (ReceiveOneKwpFrameFromECU(&buff[1], &receive_len, SC_TagKWP2000.m_Maxwaittime, speci_data) == 0)
            {
                //memset(ans, 0, sizeof(ans));
                ans_head[0] = 0xff;
                ans_head[1] = 0x02;
                return 0x02;
            }

            if ((speci_data[0] == 0x7f) && (speci_data[1] == 0x78))
            {
                continue;
            }
            else
            {
                break;
            }
        }

        send_pos += send_length + 2;
        buff[0] = receive_len;
        memmove(ans, buff, receive_len + 1);
        receive_number++;
        ans = ans + receive_len + 1;
        receive_total_length += receive_len + 1;
    }

    ans_head[0] = 0;

    //ans_head[1] = receive_number/0x100;
    ans_head[1] = receive_number & 0x00ff;

    //ans = ans_head ;
    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    return receive_total_length + 2;
}


//------------------------------------------------------------------------------
// 函数说明:KWPSendOneFrameAndReceiveMultiFrame_Volvo
// Input  : argc - 参数长度
//          argv - 0xlen,0x内容,最大等待时间
// Output : ans  - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6212
//1.发送给上位机的数据格式是: 0x00 ,0x00, 0xlen 一帧长度 一帧内容.......
//                            -    ----------   ---------------------
//                            |        |                 |记录的是实际的内容。
//                            |        |帧数记录。
//                            |数据标志位，（0xff表示的是设置）。
//2.argv的缓冲区的数据格式：
//  发送长度 +  发送内容  + 最大等待时间 //0x03 0x01,0x02,0x03,
//3.实现的功能是：发一帧收多帧,主要是针对VOLVO 的0x83
//4.在函数中未对0X7f,0x78进行处理。
//5.函数的实现和610B发一帧收多帧是相同的.
//6.类KWP协议！！！主要是针对类KWP协议。
//   Req:83 40 13 AE 1B 9F
//   Ans:94 13 40 EE 1B 00 27 03 E7 03 E8 00 00 FF FF FF FF FF FF FF FF FF E3
//   Ans:94 13 40 EE 1B 00 23 03 E8 03 E8 00 00 FF FF FF FF FF FF FF FF FF E0
//７.函数调作用的方法：
//   argv[] = {6,0x83,0x40,0x13,0xAE,0x1B,0x9F,200};
//   KWPSendOneFrameAndReceiveMultiFrame_Volvo(0,aegv,ans);
/////////////////////////////////////////////
//这是周四要问的问题，VOLVO的kwp协议的格式是。
//这里没有处理0X7F,0X78的问题。
//------------------------------------------------------------------------------
int KWPSendOneFrameAndReceiveMultiFrame_Volvo_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned int send_length = argv[0];
    unsigned int i = 0;
    unsigned int time = 0;
    unsigned char receive_number = 0;
    unsigned int receive_length = 0;
    unsigned int remain_length = 0;
    unsigned char check_byte = 0;
    unsigned char * ans_head = ans;
    unsigned int receive_total_length = 0;

    //ans = ans + 3;
    //修改
    ans = ans + 2;
    set_time0Stop(); //停止链路保持  0x03,0x01,0x01,0x01,0x02
    time = CalculateTimeOut(argv[send_length + 1], 10) * 10; //最大等待时间的判断.

    for (i = 0; i < send_length - 1; i++)
    {
        ComSendByte(argv[i + 1]);
        check_byte += argv[i + 1];
        delayus(SC_TagKWP2000.m_nBtyetime);
    }

    {
        //发送校验位
        ComSendByte(check_byte);
        check_byte = 0;
    }

    while (1)
    {
        if (false == ComByte(&receive_buff[1], time)) //对接受失败进行判断
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            ans_head[0] = 0x00;

            //ans_head[1] = receive_number/0x100;
            ans_head[1] = receive_number & 0x00ff;
            return receive_total_length + 2;
        }
        else
        {
            i = 2;

            if (receive_buff[1] == 0x80) //req:0x80,0x11,0xf1,0x02,0x21,0x02,0xcs
            {
                for (i = 2; i < 5; i++)
                {
                    ComByte(&receive_buff[i], time); //2,3,4
                }

                remain_length = receive_buff[i - 1] +1; //还需要加的长度
                receive_length = remain_length + 4; //总长度
            }
            else if (receive_buff[1] > 0x80) //Req:0x83,0x40,0x13,0xAE,0x1B,0x9F
            {
                remain_length = (receive_buff[1] &0x7f) + 2;
                receive_length = remain_length + 1;
            }
            else //req:0x02,0x21,0x01,0x24
            {
                remain_length = (receive_buff[1] &0x7f) + 1;
                receive_length = remain_length + 1;
            }

            while (remain_length--)
            {
                if (false == ComByte(&receive_buff[i++], time)) //判断接受失败
                {
                    if (timer_open_flag == 1)
                    {
                        TimerStart(TIMER0);
                    }

                    ans_head[0] = 0xff;
                    ans_head[1] = 0x02;
                    return 0x02;
                }
            }

            receive_buff[0] = receive_length; //0    1    2    3    4   5    6
            memmove(ans, receive_buff, receive_length + 1); //0x02 0x01 0x02 0x03 0x01 0x02 0x03 0x04 0x01 0x02 0x03 0x04
            ans = ans + receive_length + 1;
            receive_total_length += receive_length + 1;
            receive_number++;
        }
    }
}


//------------------------------------------------------------------------------
// 函数说明:KWPSendOneAndReceiveMultiFrame_Toyota
// Input  : argc - 参数长度
//          argv - 0xlen,0x内容,最大等待时间
// Output : ans  - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6214
//1.发送给上位机的数据格式是: 0x00 ,0x00, 0xlen 一帧长度 一帧内容.......
//                            -    ----------   ---------------------
//                            |        |                 |记录的是实际的内容。
//                            |        |帧数记录。
//                            |数据标志位，（0xff表示的是设置）。
//2.argv的缓冲区的数据格式：
//  发送繁忙帧个数１　+ 发送繁忙帧个数２ +  长度　+ 内容 + 最大等待时间 //0x03 0x01,0x02,0x03,
//3.实现的功能是：toyota,发一帧收多针
//                实现的功能是：对接受到0X7F个数进行判断，
//4.在函数中能够对0X7f的个数进行判断，当接收的0x7f的个数大于计数的个数时表示接受失败。
//5.与其他的KWP协议相比主要是针对0x7f进行判断
//req: 82 11 f1 21 06 00
//ans: 1n 82 11 f1 7f 06 00
//ans: 1n 82 11 f1 7f 06 00
//ans: 82 11 f1 61 06 00
//6.函数调作用的方法：
//   command_data6[] = {0,4,6,0x82,0x11,0xf1,0x21,0x06,0x00,200};
//   KWPSendOneAndReceiveMultiFrame_Toyota(0,command_data6,ans6);
//7.
//------------------------------------------------------------------------------
int KWPSendOneAndReceiveMultiFrame_Toyota_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned int send_length = 0;
    unsigned int send_length_pos = 0;
    unsigned int i = 0;
    unsigned char receive_number = 0;
    unsigned int receive_length = 0;
    unsigned char check_byte = 0;
    unsigned char * ans_head = ans;
    unsigned int receive_total_length = 0;
    unsigned int number_7f = 0;
    unsigned char speci_data[2] =
    {
        0
    };

    //用于存储0x7f,0x78.
    unsigned char time;

    //ans = ans + 3;
    ans = ans + 2;
    number_7f = argv[0] *0x100 + argv[1]; //0X7F的个数
    send_length_pos = 2;
    send_length = argv[send_length_pos];

    //time = argv [  send_length + 1 + 2];//最大等待时间的判断.
    time = CalculateTimeOut(argv[send_length + 1 + 2], 10) * 10;
    set_time0Stop(); //停止链路保持  0x03,0x01,0x01,0x01,0x02

    for (i = 0; i < send_length - 1; i++)
    {
        ComSendByte(argv[i + send_length_pos + 1]);
        check_byte += argv[i + send_length_pos + 1];
        delayus(SC_TagKWP2000.m_nBtyetime); //现在默认的时间是字节与字节之间的时间间隔是5ms
    }

    {
        ComSendByte(check_byte);
        check_byte = 0;
    }

    while (1)
    {
        if (ReceiveOneKwpFrameFromECU(&receive_buff[1], &receive_length, time, speci_data) == false) //if(ReceiveOneKwpFrameFromECU(&receive_buff[1],&receive_length,600,speci_data)==false)
        {
            if ((receive_number == 0)) //||(receive_length!=0))//第一次接受失败
            {
                if (timer_open_flag == 1)
                {
                    TimerStart(TIMER0);
                }

                ans_head[0] = 0xff;
                ans_head[1] = 0x02;
                return 2;
            }
            else
            {
                if (timer_open_flag == 1)
                {
                    TimerStart(TIMER0);
                }

                ans_head[0] = 0x00;

                //ans_head[1] = receive_number/0x100;
                ans_head[1] = receive_number & 0x00ff;
                return receive_total_length + 2;
            }
        }
        else
        {
            if (speci_data[0] == 0x7f)
            {
                number_7f--;

                if (number_7f == 0)
                {
                    if (receive_number == 0)
                    {
                        if (timer_open_flag == 1)
                        {
                            TimerStart(TIMER0);
                        }

                        ans_head[0] = 0xff;
                        ans_head[1] = 0x02;
                        return 2;
                    }
                    else
                    {
                        if (timer_open_flag == 1)
                        {
                            TimerStart(TIMER0);
                        }

                        ans_head[0] = 0x00;

                        //ans_head[1] = receive_number/0x100;
                        ans_head[1] = receive_number & 0x00ff;
                        return receive_total_length + 2;
                    }
                }

                continue;
            }
            else
            {
                receive_number++;
                receive_buff[0] = receive_length; //
                memmove(ans, receive_buff, receive_length + 1); //长度 + 内容
                ans += receive_length + 1; //0x02,0x01,0x02,0x03,0x0x01,0x02,0x03,0x02
                receive_total_length += receive_length + 1; //总的长度
            }
        }
    }
}


//----------------------------------------------------------------------------
//函数名称:int KWP_SendDataToEcuGetAnswer_Benz()
// Input  : argc - 参数长度
//          argv - 0xlen,0x内容,最大等待时间
// Output : ans  - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6220
//1.发送给上位机的数据格式是: 0x00 ,0x00, 0xlen 一帧长度 一帧内容.......
//                            -    ----------   ---------------------
//                            |        |                 |记录的是实际的内容。
//                            |        |帧数记录。
//                            |数据标志位，（0xff表示的是设置）。
//2.argv的缓冲区的数据格式：
//  发送次数　+ 发送帧数 +  长度　+ 内容 + 最大等待时间 //0x03 0x01,0x02,0x03,
//3.实现的功能是： 在普通的KWP协议里面对
//                 0x7f进行处理:
//                     0x78->接受繁忙,连续接受多次，每次接受的数据不可以用
//                     0x21/0xa0->重复发送，限制重复的次数是20次
//                     0x23->重复接受。
//                 实现的是发一帧收一帧。
//4.
//5.与其他的KWP协议相比主要是针对0x7f进行判断
//req: 82 11 f1 21 06 00
//ans: 1n 82 11 f1 7f 06 00
//ans: 1n 82 11 f1 7f 06 00
//ans: 82 11 f1 61 06 00
//6.函数调作用的方法：
//  unsigned char command_data7[] = { 0xff,0x04,
//                                          0x06,0x82,0x10,0xF1,0x22,0x07,0xA5,
//                                           0x07,0x80,0x10,0xF1,0x02,0x22,0x07,0xA8,
//                                           0x05,0x00,0x02,0x22,0x07,0x64,
//                                           0x04,0x02,0x22,0x07,0x24};
//   KWP_SendDataToEcuGetAnswer_Benz(0,command_data7,ans7);
//------------------------------------------------------------------------------
int KWP_SendDataToEcuGetAnswer_Benz_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned int send_length = 0;
    unsigned int send_length_pos = 0;
    unsigned char send_number = 0;
    unsigned int j = 0;
    unsigned int time = 0;
    unsigned char receive_number = 0;
    unsigned int receive_length = 0;
    unsigned char check_byte = 0;
    unsigned char * ans_head = ans;
    unsigned int receive_total_length = 0;
    unsigned char speci_data[2] =
    {
        0
    };

    //用于存储0x7f,0x78.
    unsigned char error_count = 0;
    unsigned char buffer_0x7f_0x23[50] =
    {
        0
    };

    //ans = ans + 3;//发送次数 + 帧数 + 帧长度 + 内容
    ans = ans + 2;
    send_number = argv[1];
    send_length_pos = 2; //考虑一下是不是构建结构体。
    time = SC_TagKWP2000.m_Maxwaittime;
    set_time0Stop();

    do
    {
        send_length = argv[send_length_pos];

        for (j = 0; j < send_length - 1; j++)
        {
            ComSendByte(argv[send_length_pos + j + 1]); //长度所在的位置加1
            check_byte += argv[send_length_pos + j + 1]; //计算累加校验和
            delayus(SC_TagKWP2000.m_nBtyetime);
        }

        {
            //delay( SC_TagKWP2000.m_nBtyetime );
            ComSendByte(check_byte);
            check_byte = 0;
        } //发送一帧数据

        do
        {
            if (ReceiveOneKwpFrameFromECU(&receive_buff[1], &receive_length, time, speci_data) == false)
            {
                if ((speci_data[0] == 0x7f) && ((speci_data[1] == 0x23))) //这里有一种情况可能会出现问题。
                {
                    receive_number++;
                    memmove(ans, buffer_0x7f_0x23, receive_length + 1);
                    ans += receive_length + 1;
                    receive_total_length += receive_length + 1;
                    send_length_pos += (argv[send_length_pos] +1); //下一帧的位置
                    break; //开始发送下一帧数据
                }
                else
                {
                    ans_head[0] = 0xff;
                    ans_head[1] = 0x02;

                    if (timer_open_flag == 1)
                    {
                        TimerStart(TIMER0);
                    }

                    return 2;
                }
            }
            else
            {
                if (speci_data[0] == 0x7f)
                {
                    switch (speci_data[1])
                    {
                        case 0x78: //重复接受
                            time = 5000; //等待时间加长
                            continue;

                        case 0x21: //重复发送
                        case 0xa0:
                            error_count++;

                            if (error_count <= 20) //继续发送并且不进行保存。
                            {
                                break;
                            }
                            else //保存并发送下一帧
                            {
                                error_count = 0;
                                receive_number++;
                                receive_buff[0] = receive_length;
                                memmove(ans, receive_buff, receive_length + 1);
                                ans += receive_length + 1;
                                receive_total_length += receive_length + 1;
                                send_length_pos += (argv[send_length_pos] +1); //下一帧的位置
                            }

                            break;

                        case 0x23: //表示的是ECU已经正确的接受到了数据但是，但是程序正在进行中还没有完成，
                            {
                                time = 2000;
                                memset(buffer_0x7f_0x23, 0, sizeof(buffer_0x7f_0x23));
                                receive_buff[0] = receive_length;
                                memmove(buffer_0x7f_0x23, receive_buff, receive_length + 1);
                                break;
                            }

                        default:
                            break;
                    }
                }
            }
        }
        while((speci_data[0] == 0x7f && speci_data[1] == 0x78));

        //只有在0x7f,0x78的时候才进行重复的接受。
        if ((speci_data[0] == 0x7f && speci_data[1] == 0x23) || (speci_data[0] == 0x7f && speci_data[1] == 0x21) || (speci_data[0] == 0x7f &&
             speci_data[1] == 0xa0)) //表示继续发送这一帧
        {
            ;
        }
        else
        {
            time = SC_TagKWP2000.m_Maxwaittime;
            receive_number++;
            receive_buff[0] = receive_length;
            memmove(ans, receive_buff, receive_length + 1);
            ans += receive_length + 1;
            receive_total_length += receive_length + 1;
            send_length_pos += argv[send_length_pos] +1; //下一帧的位置
        }
    }
    while(receive_number < send_number);

    ans_head[0] = 0x00;

    //ans_head[1] = receive_number/0x100;
    ans_head[1] = receive_number & 0x00ff;

    //ans = ans_head;
    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    return receive_total_length + 2;
}


//------------------------------------------------------------------------------
//函数名称:int KWP_SendDataToEcuGetMultiFrameAnswer_Benz()
// Input  : argc - 参数长度
//          argv - 0xlen,0x内容,最大等待时间
// Output : ans  - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6221
//1.发送给上位机的数据格式是: 0x00 ,0x00, 0xlen 一帧长度 一帧内容.......
//                            -    ----------   ---------------------
//                            |        |                 |记录的是实际的内容。
//                            |        |帧数记录。
//                            |数据标志位，（0xff表示的是设置）。
//2.argv的缓冲区的数据格式：
//  长度　+ 内容 + 最大等待时间 //0x03 0x01,0x02,0x03,
//3.实现的功能是： 在普通的KWP协议里面对
//                 0x7f进行处理:
//                     0x78/0x23->接受繁忙,连续接受多次，每次接受的数据不可以用
//                     0x21->重复发送，限制重复的次数是20次
//                 实现的是发一帧收多帧。
//4.
//5.与其他的KWP协议相比主要是针对0x7f进行判断
//req: 82 11 f1 21 06 00
//ans: 1n 82 11 f1 7f 06 00
//ans: 1n 82 11 f1 7f 06 00
//ans: 82 11 f1 61 06 00
//6.函数调作用的方法：
//   command_data6[] = {0,4,6,0x82,0x11,0xf1,0x21,0x06,0x00,200};
//   KWPSendOneAndReceiveMultiFrame_Toyota(0,command_data6,ans6);
//------------------------------------------------------------------------------
int KWP_SendDataToEcuGetMultiFrameAnswer_Benz_1(int argc, unsigned char * argv,
    unsigned char * ans)
{
    unsigned int send_length = 0; //发送长度
    unsigned int send_length_pos = 0; //发送长度位置
    unsigned int j = 0;
    unsigned int time = 0;
    unsigned char receive_number = 0; //接受帧数
    unsigned int receive_length = 0; //接受长度
    unsigned char check_byte = 0; //校验位
    unsigned char * ans_head = ans;
    unsigned int receive_total_length = 0;
    unsigned char speci_data[2] =
    {
        0
    };

    //用于存储0x7f,0x78.
    //ans = ans + 3;//发送次数 + 帧数 + 帧长度 + 内容  0x03,0x01,0x02,0x03,0xdd
    ans = ans + 2;
    send_length_pos = 0;
    time = CalculateTimeOut(argv[send_length_pos + 1], 10) * 10;
    set_time0Stop();

    do
    {
        send_length = argv[send_length_pos];

        for (j = 0; j < send_length - 1; j++)
        {
            ComSendByte(argv[send_length_pos + j + 1]); //长度所在的位置加1
            check_byte += argv[send_length_pos + j + 1]; //计算累加校验和
            delayus(SC_TagKWP2000.m_nBtyetime);
        }

        {
            //delay( SC_TagKWP2000.m_nBtyetime );
            ComSendByte(check_byte);
            check_byte = 0;
        }

        while (1)
        {
            if (ReceiveOneKwpFrameFromECU(&receive_buff[1], &receive_length, time, speci_data) == false)
            {
                if ((receive_number == 0) || (receive_length != 0))
                {
                    ans_head[0] = 0xff;
                    ans_head[1] = 0x00;

                    if (timer_open_flag == 1)
                    {
                        TimerStart(TIMER0);
                    }

                    return 2;
                }
                else
                {
                    ans_head[0] = 0x00;

                    //ans_head[ 1 ] = receive_number/0x100;
                    ans_head[1] = receive_number & 0x00ff;

                    if (timer_open_flag == 1)
                    {
                        TimerStart(TIMER0);
                    }

                    return receive_total_length + 2;
                }
            }

            if (speci_data[0] == 0x7f)
            {
                if ((speci_data[1] == 0x78) || (speci_data[1] == 0x23))
                {
                    time = 5000; //延长时间进行接收
                    continue;
                }
                else if (speci_data[1] == 0x21)
                {
                    break;
                }
            }
            else
            {
                receive_number++; //帧数
                receive_buff[0] = receive_length; //长度
                receive_length++;
                memmove(ans, receive_buff, receive_length);
                ans += receive_length;
                receive_total_length += receive_length;
            }
        }
    }
    while(1);
}


//----------------------------------------------------------------------------
//函数名称:int KWPSendDataToEcuGetAnswer_BMW()
// Input  : argc - 参数长度
//          argv - 0xlen,0x内容,最大等待时间
// Output : ans  - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6224
//1.发送给上位机的数据格式是: 0x00 ,0x00, 0xlen 一帧长度 一帧内容.......
//                            -    ----------   ---------------------
//                            |        |                 |记录的是实际的内容。
//                            |        |帧数记录。
//                            |数据标志位，（0xff表示的是设置）。
//2.argv的缓冲区的数据格式：
//  发送次数 + 发送帧数 + 长度　+ 内容 + 最大等待时间 //0x03 0x01,0x02,0x03,
//3.实现的功能是： 在普通的KWP协议里面对
//                 0x7f进行处理:
//                     0x78/0x23->接受繁忙,连续接受多次，每次接受的数据不可以用
//                     0x21->重复发送，限制重复的次数是20次
//                 实现的是发一帧收多帧。
//5.与其他的KWP协议相比主要是针对0x7f进行判断
//req: 82 11 f1 21 06 00
//ans: 1n 82 11 f1 7f 06 00
//ans: 1n 82 11 f1 7f 06 00
//ans: 82 11 f1 61 06 00
//6.函数调作用的方法：
//   command_data6[] = {0,4,6,0x82,0x11,0xf1,0x21,0x06,0x00,200};
//   KWPSendOneAndReceiveMultiFrame_Toyota(0,command_data6,ans6);
//------------------------------------------------------------------------------
int KWPSendDataToEcuGetAnswer_BMW_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned int frame_number = argv[1]; //发送的帧数
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int frame_length = 0;
    unsigned int frame_pos = 2;
    unsigned int receive_len = 0;
    unsigned char speci_data[2] =
    {
        0
    };

    //用于存储0x7f,0x78.
    unsigned short receive_number = 0; //用两个字节来表示长度
    unsigned char * ans_head = &ans[0]; //接受缓冲区的帧数的存储位置。
    unsigned int receive_total_length = 0;
    unsigned char check_byte = 0;
    unsigned int time = SC_TagKWP2000.m_Maxwaittime;

    //ans = ans + 3;//0x00 0x00 0xlen 0x
    ans = ans + 2;

    if (argc < 0)
    {
        //memset(ans, 0, sizeof(ans));
        ans_head[0] = 0xff;
        ans_head[1] = 0x02;
        return 0x02;
    }

    set_time0Stop();

    for (i = 0; i < frame_number; i++)
    {
        frame_length = argv[frame_pos];

        for (j = 0; j < frame_length - 1; j++) //注意每帧的长度里面包含了校验位
        {
            ComSendByte(argv[frame_pos + j + 1]);
            check_byte += argv[frame_pos + j + 1]; //计算累加校验和
            delayus(SC_TagKWP2000.m_nBtyetime);
        }

        {
            //发送校验位
            //delay( SC_TagKWP2000.m_nBtyetime );
            ComSendByte(check_byte);
            check_byte = 0;
        }

        while (1)
        {
            if (ReceiveOneKwpFrameFromECU(&receive_buff[1], &receive_len, time, speci_data) == 0)
            {
                //memset(ans, 0, sizeof(ans));
                ans_head[0] = 0xff;
                ans_head[1] = 0x02;
                return 0x02;
            }

            if ((speci_data[0] == 0x7f) && (speci_data[1] == 0x78))
            {
                time = 5000;
                continue;
            }
            else
            {
                time = SC_TagKWP2000.m_Maxwaittime;
                break;
            }
        }

        receive_buff[0] = receive_len;
        memmove(ans, receive_buff, receive_len + 1);
        receive_number++;
        ans = ans + receive_len + 1;
        frame_pos += (frame_length + 1); //获取长度所在的位置.
        receive_total_length += receive_len + 1;
    }

    ans_head[0] = 0x00;

    //ans_head[ 1 ] = receive_number/0x100;
    ans_head[1] = receive_number & 0x00ff;

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    return receive_total_length + 2;
}


//------------------------------------------------------------------------------
//函数名称:int ISO_SendDataToEcuGetMultiFrameAnswer()
// Input  : argc - 参数长度
//          argv - 0xlen,0x内容,最大等待时间
// Output : ans  - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6107
//1.发送给上位机的数据格式是: 0x00 ,0x00, 0xlen 一帧长度 一帧内容.......
//                            -    ----------   ---------------------
//                            |        |                 |记录的是实际的内容。
//                            |        |帧数记录。
//                            |数据标志位，（0xff表示的是设置）。
//2.argv的缓冲区的数据格式：
//  长度　+ 内容 + 最大等待时间 //0x03 0x01,0x02,0x03,
//3.实现的功能:ISO_9141协议的接受和发送。
//             主要实现的是：发送并接受,直到没有接受数据为止。
//4,协议举例:
//   req:68 6a f1 be 81
//   ans:48 6b c0 fe 00 01 72
//--------------------------------------------------------------------------------
int ISO_SendDataToEcuGetMultiFrameAnswer_1(int argc, unsigned char * argv,
    unsigned char * ans)
{
    unsigned int i = 0;
    unsigned int frame_length = 0;
    unsigned int frame_pos = 0;
    unsigned char buff[200] =
    {
        0
    };
    unsigned int receive_len = 0;
    unsigned short receive_number = 0; //用两个字节来表示长度
    unsigned char * ans_head = &ans[0]; //接受缓冲区的帧数的存储位置。
    unsigned int receive_total_length = 0;

    //unsigned char check_byte = 0;
    unsigned int time;

    frame_pos = 0;
    ans = ans + 2;
    frame_length = argv[frame_pos];
    time = CalculateTimeOut(argv[frame_length + 1], 10) * 10;

    if (argc < 0)
    {
        //memset(ans, 0, sizeof(ans));
        ans_head[0] = 0xff;
        ans_head[1] = 0x02;
        return 0x02;
    }

    set_time0Stop();

    for (i = 0; i < frame_length; i++)
    {
        ComSendByte(argv[frame_pos + i + 1]);

        //check_byte += argv [ frame_pos + i + 1 ];//计算累加校验和
        delayus(SC_TagKWP2000.m_nBtyetime);
    }

    /*{
        ComSendByte(check_byte);
        check_byte = 0;
    }*/
    i = 1;

    while (1)
    {
        if (false == ComByte(&buff[i], time)) //接受完毕
        {
            {
                //存储最后一帧数据
                receive_len = i - 1;
                buff[0] = receive_len;
                memmove(ans, buff, receive_len + 1);
                receive_total_length += receive_len;
            }

            if (receive_number == 0)
            {
                ans_head[0] = 0xff;
                ans_head[1] = 0x02;
                return 0x02;
            }

            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            ans_head[0] = 0x00;
            ans_head[1] = receive_number & 0x00ff;
            return receive_total_length + 3;
        }
        else
        {
            //buff里面的内容是:48,6b,10,e4,02,ff,cc,43,53,
            if ((buff[i] == 0x6b) && (buff[i - 1] == 0x48)) //在这里每次接收成功的时候都要进行比较的话效率不会很高的哦
            {
                if (receive_number == 0) //接受的第一帧
                {
                    i++;
                    receive_number++;
                    continue;
                }

                receive_len = i - 2; //上一帧的长度
                buff[0] = receive_len;
                memmove(ans, buff, receive_len + 1);
                receive_total_length += receive_len;
                ans += receive_len + 1;
                receive_number++;
                memmove(&buff[1], &buff[i - 1], 2);
                i = 3;
            }
            else
            {
                i++;
            }
        }
    }
}


//------------------------------------------------------------------------------
//函数名称:int ISO_SendDataToEcuGetAnswer()
// Input  : argc - 参数长度
//          argv - 0xlen,0x内容,最大等待时间
// Output : ans  - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6110
//1.发送给上位机的数据格式是: 0x00 ,0x00, 0xlen 一帧长度 一帧内容.......
//                            -    ----------   ---------------------
//                            |        |                 |记录的是实际的内容。
//                            |        |帧数记录。
//                            |数据标志位，（0xff表示的是设置）。
//2.argv的缓冲区的数据格式：
//  发送的次数 + 帧数 + 每帧的长度 + 每帧的内容 +长度　+ 内容
//3.实现的功能:ISO_9141协议的接受和发送。
//             主要实现的是：发多帧收多帧,对于发送的每一帧都有一帧的应答。
//4,协议举例:
//   req:68 6a f1 be 81
//   ans:48 6b c0 fe 00 01 72
//5.注意函数实现的功能和函数：IsoInitSendDataToEcuGetAnswer,IsoInitSendDataToEcuGetAnswer,
//  IsoAddSendDataToEcuGetAnswer.组合使用的方式是一样的。
//------------------------------------------------------------------------------
int ISO_SendDataToEcuGetAnswer_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int send_length = 0;
    unsigned int send_count = 0;
    unsigned int send_number = 0;
    unsigned int send_length_pos = 0;
    unsigned char buff[200] =
    {
        0
    };
    unsigned int receive_len = 0;
    unsigned short receive_number = 0; //用两个字节来表示长度
    unsigned char * ans_head = &ans[0]; //接受缓冲区的帧数的存储位置。
    unsigned int receive_total_length = 0;

    //unsigned char check_byte = 0;
    unsigned int time = 500;

    send_count = argv[0]; //发送次数
    send_number = argv[1]; //发送帧数
    send_length_pos = 2;
    ans = ans + 2;

    if ((argc < 0) || (send_count == 0) || (send_number == 0))
    {
        //memset(ans, 0, sizeof(ans));
        ans_head[0] = 0xff;
        ans_head[1] = 0x02;
        return 0x02;
    }

    set_time0Stop();

    for (j = 0; j < send_number; j++)
    {
        send_length = argv[send_length_pos];

        for (i = 0; i < send_length; i++)
        {
            ComSendByte(argv[send_length_pos + i + 1]);

            //check_byte += argv [ send_length_pos + i + 1 ];//计算累加校验和
            delayus(SC_TagKWP2000.m_nBtyetime);
        }

        /*{//发送校验位
            //delay( SC_TagKWP2000.m_nBtyetime );
            ComSendByte(check_byte);
            check_byte = 0;
        }*/
        i = 1;

        while (1)
        {
            if (false == ComByte(&buff[i], time)) //接受完毕
            {
                if (0 == i)
                {
                    ans[0] = 0xff;
                    ans[1] = 0x02;
                    return 2;
                }

                receive_len = i - 1;
                buff[0] = receive_len;
                receive_number++;
                memmove(ans, buff, receive_len + 1);
                ans += (receive_len + 1);
                send_length_pos += send_length + 1;
                receive_total_length += receive_len;
                break;
            }
            else
            {
                i++;
                time = 100;
            }
        }
    }

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    ans_head[0] = 0x00;

    //ans_head[ 1 ] = receive_number / 0x100;
    ans_head[1] = receive_number & 0x00ff;
    return receive_total_length + 2;
}


//------------------------------------------------------------------------------
//函数名称:int OneToOne_SendOneByteToEcuGetAnswerLM()
// Input  : argc - 参数长度
//          argv - 0xlen,0x内容,最大等待时间
// Output : ans  - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6128
//1.发送给上位机的数据格式是: 0x00 ,0x00, 0xlen 一帧长度 一帧内容.......
//                            -    ----------   ---------------------
//                            |        |                 |记录的是实际的内容。
//                            |        |帧数记录。
//                            |数据标志位，（0xff表示的是设置）。
//2.argv的缓冲区的数据格式：
//  发送的次数 + 帧数 + 每帧的长度 + 每帧的内容 +长度　+ 内容
//3.实现的功能:ISO_9141协议的接受和发送。
//             主要实现的是：发多帧收多帧,对于发送的每一帧都有一帧的应答。
//4,协议举例:
//   req:68 6a f1 be 81
//   ans:48 6b c0 fe 00 01 72
//5.注意函数实现的功能和函数：IsoInitSendDataToEcuGetAnswer,IsoInitSendDataToEcuGetAnswer,
//  IsoAddSendDataToEcuGetAnswer.组合使用的方式是一样的。
//------------------------------------------------------------------------------
int OneToOne_SendOneByteToEcuGetAnswerLM_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char rcvbyte;
    int sendframesize, rcvlen, rcvpos = 1;
    int i = 0;

    sendframesize = argv[0];

    if (sendframesize < 1)
    {
        ans[0] = 0xff;
        ans[1] = 0x02;
        return 2;
    }

    sendframesize--;
    i = 1;
    set_time0Stop();

    while (sendframesize--)
    {
        ComSendByte(argv[i++]);

        if (ComByte(&rcvbyte, 1000) == false)
        {
            ans[0] = 0xff;
            ans[1] = 0x02;
            return 2;
        }

        delayus(SC_TagKWP2000.m_nBtyetime);
    }

    ComSendByte(argv[i++]);

    if (ComByte(&rcvbyte, 1000) == false)
    {
        ans[0] = 0xff;
        ans[1] = 0x02;
        return 2;
    }

    //一帧数据
    ans[rcvpos++] = 0x01;

    if (ComByte(&rcvbyte, 1000) == false)
    {
        ans[0] = 0xff;
        ans[1] = 0x02;
        return 2;
    }

    ans[rcvpos++] = rcvbyte;

    if (ComByte(&rcvbyte, 1000) == false)
    {
        ans[0] = 0xff;
        ans[1] = 0x02;
        return 2;
    }

    ans[rcvpos++] = rcvbyte;
    rcvlen = rcvbyte + 1;
    sendframesize = rcvlen + 4;

    while (rcvlen--)
    {
        if (ComByte(&rcvbyte, 1000) == false)
        {
            ans[0] = 0xff;
            ans[1] = 0x02;
            return 2;
        }

        ans[rcvpos++] = rcvbyte;
    }

    ans[0] = 0;

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    return sendframesize;
}


//------------------------------------------------------------------------------
// Funtion: KWP通讯时，可过滤指定ID
// Input  : argc - 参数长度
//          argv - 指令参数缓冲区
//          argv[0]:recmode
//          argv[1]:发送一帧数数据的长度len
//          argv[2]~argv[len+1]:发送往ECU的一帧有效数据
//          argv[3]:目标地址
//          argv[4]:源地址
//          argv[len+2]:timebuff
// Output : ans - 回复到上位机的数据
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、ans的格式为：总帧数 帧长度 帧数据
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 6129 用于下线检测的指令，诊断中未使用
//------------------------------------------------------------------------------
int KWPSendOneAndReceiveMultiFrameHasFilter_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char framelen, i;
    unsigned char busyid[2] =
    {
        0
    };
    unsigned char buff[300];
    unsigned int timebuff, waitingtime, rcvframesize, argvPos, ansLen;
    unsigned char targetaddr[2], sourceaddr[2];
    unsigned char recmode;

    set_time0Stop();
    recmode = argv[0];

    if (recmode == 0)
    {
        targetaddr[0] = argv[3];
        sourceaddr[0] = argv[4];
    }

    //Send Data
    argvPos = 1;

    if ((framelen = argv[argvPos++]) == 0)
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        PutDataToAns(szFNG, ans, sizeof(szFNG));
        return ID_FNG;
    }

    do
    {
        ComSendByte(argv[argvPos++]);
        delayus(SC_TagKWP2000.m_nBtyetime);
    }
    while(--framelen > 1);

    ComSendByte(argv[argvPos++]);
    timebuff = 300;
    waitingtime = timebuff;
    ans[0] = 0;
    ans[1] = 0;
    ansLen = 2;

    //Recice Data
    do
    {
        if (ReceiveOneKwpFrameFromECUHasMode(recmode, buff, &rcvframesize, waitingtime) == FALSE)
        {
            if (ans[1] == 0)
            {
                if (timer_open_flag == 1)
                {
                    TimerStart(TIMER0);
                }

                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                return ID_FBREAK;
            }

            if (rcvframesize != 0) // 非正常断线
            {
                if (timer_open_flag == 1)
                {
                    TimerStart(TIMER0);
                }

                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                return ID_FBREAK;
            }

            break;
        }

        //Filter
        if (recmode == 0)
        {
            //send:  8x targetaddr sourceaddr ...
            //recv   8x sourceaddr targetaddr ...
            sourceaddr[1] = buff[1];
            targetaddr[1] = buff[2];

            if ((targetaddr[0] != targetaddr[1]) || (sourceaddr[0] != sourceaddr[1]))
            {
                continue;
            }
        }

        GetKwpFrameBusyFlag(buff, busyid);

        if ((busyid[0] == 0x7f) && (busyid[1] == 0x78))
        {
            waitingtime = 5000; // 遇忙帧，等待时间加长
        }
        else
        {
            ans[1] ++;
            ans[ansLen++] = rcvframesize;
            i = 0;

            while (rcvframesize--)
            {
                ans[ansLen++] = buff[i++];
            }
        }
    }
    while(1);

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    return ansLen;
}


//------------------------------------------------------------------------------
// Funtion: send one frame to ecu and receive multiframe know nothing of frame
// Input  : argc - 参数长度
//          argv - 指令参数缓冲区
//          argv[0]:发往ECU的一帧数据的长度: len
//          argv[1] ~ argv[len]:发往ECU的一帧数据
//     后面的三个字节有合起来有三种情况
//     case 1:
//          argv[len + 1]: 非0表示从ECU接受一帧数据的长度
//     case 2:
//          argv[len + 1]为0，argv[len + 2]为0表示接受数据直到碰到指定的字节argv[len + 3]后，再接收一个字节。
//     case 3:
//          argv[len + 1]为0，argv[len + 2]非0表示以特定的规则从ECU接收数据，这个规则的关键字为argv[len + 3]
// Output : ans - 回复到上位机的数据
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、ans的格式为：总帧数 帧长度 帧数据
//                如ans[] = {00,01,0f,01,f6,b0,36,42,39,30,36,30,33,32,41,20,20,03}
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 6111
//------------------------------------------------------------------------------
int SendOneFrameDataToEcuGetAnyFrameAnswer_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned int len = 0, ansLen = 0, argvPos = 0;
    unsigned char rcvbyte, kw1;
    unsigned char temp = 0;

    set_time0Stop();

    //检测串口是否处于空闲状态
    if (CheckIoBusy(40, 5) == true)
    {
        if (CheckIoBusy(20, 8) == true)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            PutDataToAns(szFNG, ans, sizeof(szFNG));
            return ID_FNG;
        }
    }

    if ((len = argv[0]) == 0x0)
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        PutDataToAns(szFNG, ans, sizeof(szFNG));
        return ID_FNG;
    }

    argvPos = 1;

    while (len-- > 1)
    {
        ComSendByte(argv[argvPos++]);
        delayus(SC_TagKWP2000.m_nBtyetime);
    }

    ComSendByte(argv[argvPos++] ++); //send fininsh
    ans[0] = 0;
    ans[1] = 1;
    ans[2] = 0;
    ansLen = 3;

    //自动判断长度
    if ((len = argv[argvPos++]) == 0x0)
    {
        //指定接收的字节
        if (argv[argvPos] == 0x0) // 2008-12-4, remove "++" after "(*commandpos)"
        {
            argvPos++;
            kw1 = argv[argvPos++];

            do
            {
                if (ComByte(&rcvbyte, 2000) == FALSE)
                {
                    PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                    return ID_FBREAK;
                }

                ans[ansLen++] = rcvbyte;
                ans[2] ++;
            }
            while(rcvbyte != kw1);

            if (ComByte(&rcvbyte, 2000) == FALSE)
            {
                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                return ID_FBREAK;
            }

            ans[ansLen++] = rcvbyte;
            ans[2] ++;
        }
        else
        {
            //长度位置
            temp = argv[argvPos++];
            len = 0;

            //增加的值
            kw1 = argv[argvPos++];

            while (temp-- > 0)
            {
                if (ComByte(&rcvbyte, 2000) == FALSE)
                {
                    PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                    return ID_FBREAK;
                }

                ans[ansLen++] = rcvbyte;
                ans[2] ++;
            }

            temp = rcvbyte; // 2008-12-4, add
            temp -= kw1;
            temp -= ans[2];

            while (temp-- > 0)
            {
                if (ComByte(&rcvbyte, 2000) == FALSE)
                {
                    PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                    return ID_FBREAK;
                }

                ans[ansLen++] = rcvbyte;
                ans[2] ++;
            }
        }
    }

    //指定长度
    else
    {
        //ziyingzhu 修改 2009-9-10 22:52 会导致上传的结果多了一个字节，错位
        ans[2] = len;

        while (len-- > 0)
        {
            if (ComByte(&rcvbyte, 2000) == FALSE)
            {
                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                return ID_FBREAK;
            }

            ans[ansLen++] = rcvbyte;
        }
    }

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    return ansLen;
}


//------------------------------------------------------------------------------
//函数名称:int FordIsoSendOneAndReceiveMultiFrame()
// Input  : argc - 参数长度
//          argv - 0xlen,0x内容,最大等待时间
// Output : ans  - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6116
//1.发送给上位机的数据格式是: 0x00 ,0x00, 0xlen 一帧长度 一帧内容.......
//                            -    ----------   ---------------------
//                            |        |                 |记录的是实际的内容。
//                            |        |帧数记录。
//                            |数据标志位，（0xff表示的是设置）。
//2.argv的缓冲区的数据格式：
//   长度 + 内容 　+ 接受的最大等待时间
//3.实现的功能:ISO_9141协议的接受和发送。
//             主要实现的是:发送一帧内容收多帧的情况
//4,协议举例:
//   req:68 6a f1 be 81
//   ans:48 6b c0 fe 00 01 72
//5.发送和接受的数据格式不是很清楚
//--------------------------------------------------------------------------------
int FordIsoSendOneAndReceiveMultiFrame_1(int argc, unsigned char * argv,
    unsigned char * ans)
{
    unsigned int i = 0;
    unsigned int send_length = 0;
    unsigned int send_length_pos = 0;
    unsigned char buff[200] =
    {
        0
    };
    unsigned int receive_len = 0;
    unsigned short receive_number = 0; //用两个字节来表示长度
    unsigned char * ans_head = &ans[0]; //接受缓冲区的帧数的存储位置。
    unsigned int receive_total_length = 0;
    unsigned char check_byte = 0;
    unsigned int time;

    send_length_pos = 0;

    //ans = ans + 3;
    ans = ans + 2;
    send_length = argv[send_length_pos];
    time = CalculateTimeOut(argv[send_length + 1], 10) * 10;

    if (argc < 0)
    {
        //memset(ans, 0, sizeof(ans));
        ans_head[0] = 0xff;
        ans_head[1] = 0x02;
        return 0x02;
    }

    set_time0Stop();

    for (i = 0; i < send_length - 1; i++)
    {
        ComSendByte(argv[send_length_pos + i + 1]);
        check_byte += argv[send_length_pos + i + 1]; //计算累加校验和
        delayus(SC_TagKWP2000.m_nBtyetime);
    }

    //delay( SC_TagKWP2000.m_nBtyetime );
    ComSendByte(check_byte);

    while (1)
    {
        i = 1;
		if (receive_total_length > 0xE10)
		{
			*ans = 0;
        	ans[1] = receive_len;
        	if ( timer_open_flag == 1 )
          		TimerStart_sub_8086158(0);
			return SingleFrameCount + receive_total_length + 2;
		}

		
        if (false == ComByte(&buff[i], time)) //接受完毕
        {
            if (receive_number == 0)
            {
                if (timer_open_flag == 1)
                {
                    TimerStart(TIMER0);
                }

                ans_head[0] = 0xff;
                ans_head[1] = 0x02;
                return 2;
            }
            else
            {
                if (timer_open_flag == 1)
                {
                    TimerStart(TIMER0);
                }

                ans_head[0] = 0x00;
                ans_head[1] = receive_number & 0x00ff;
                return receive_total_length + 2 + receive_number;
            }
        }

        receive_len = (buff[i++] >> 4) + 1;
        buff[0] = receive_len;

        while (--receive_len)
        {
            if (false == ComByte(&buff[i], time)) //接受完毕
            {
                if (timer_open_flag == 1)
                {
                    TimerStart(TIMER0);
                }

                ans_head[0] = 0xff;
                ans_head[1] = 0x02;
                return 2;
            }
            else
            {
                i++;
            }
        }

        receive_number++;//这个是接收的帧的数目吗?
        memmove(ans, buff, buff[0] +1);
        ans += (buff[0] +1);
        receive_total_length += buff[0];
    }
}


//------------------------------------------------------------------------------
// Funtion: send one frame to ecu and receive multiframe know nothing of frame
// Input  : argc - 参数长度
//          argv - 指令参数缓冲区
//          argv[0]:数据流标志
//          argv[1]:发送帧数
//          argv[2]:接收帧数
//          argv[3]:检测 08 55 a3开关, 如果为0，则检测
//     根据帧数后面字节循环依次为：headid : 检验从ECU所收数据的帧头是否匹配
//                                 busidletime: 检测总线空闲最大等待时间
//                                 framelen；发送的一帧数据的长度len
//                                 framedata:长度为len的一帧数据
// Output : ans - 回复到上位机的数据
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、ans的格式为：总帧数 帧长度 帧数据
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 6117
//------------------------------------------------------------------------------
int HoldenNormalRingLinkSendOneAndOneFrame_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char sendnum, rcvnum, framelen, rcvbyte, checkid_sw, headid, busidletime;
    unsigned int ansLen = 0, argvPos, i = 0, checkFlag = 0;
    unsigned char ansTemp[255] =
    {
        0
    },
    lenth = 0, checkByte[3] =
    {
        0x08, 0x55, 0xa3
    };
    set_time0Stop();
    delay(10);

    //ResetUART3();
    if (argv[0] == 0)
    {
        ; // DATASTREAM_FG = true;
    }

    sendnum = argv[1]; // 发送帧数
    rcvnum = argv[2]; // 接收帧数
    checkid_sw = argv[3]; // 检测 08 55 a3开关, 如果为0，则检测
    ans[0] = 0;
    ans[1] = 0;
    ansLen = 2;
    argvPos = 4;

    do
    {
        if (checkid_sw == 0x0)
        {
            do
            {
                for (i = 0; i < 3; i++)
                {
                    if (ComByte(&rcvbyte, 500) == FALSE)
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        return ID_FBREAK;
                    }

                    if (rcvbyte != checkByte[i])
                    {
                        checkFlag = 0;
                        break;
                    }

                    checkFlag = 1;
                }

                if (checkFlag == 0)
                {
                    continue;
                }
                else
                {
                    break;
                }
            }
            while(1);
        }

        headid = argv[argvPos++];
        busidletime = argv[argvPos++];

        if (checkid_sw == 0x0)
        {
            delay(busidletime);
        }
        else
        {
            if (CheckIoBusy(40, busidletime) == true)
            {
                argvPos -= 2;
                continue;
            }
        }

        framelen = argv[argvPos++];

        while (framelen-- > 1)
        {
            ComSendByte(argv[argvPos++]);
            delayus(SC_TagKWP2000.m_nBtyetime);
        }

        ComSendByte(argv[argvPos++]);

        if (rcvnum-- > 0) // while -> if, zcl
        {
            if (ComByte(&rcvbyte, 500) == FALSE)
            {
                if (timer_open_flag == 1)
                {
                    TimerStart(TIMER0);
                }

                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                return ID_FBREAK;
            }

            if (rcvbyte != headid)
            {
                if (timer_open_flag == 1)
                {
                    TimerStart(TIMER0);
                }

                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                return ID_FBREAK;
            }

            ansTemp[lenth++] = rcvbyte;

            if (ComByte(&rcvbyte, 500) == FALSE)
            {
                if (timer_open_flag == 1)
                {
                    TimerStart(TIMER0);
                }

                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                return ID_FBREAK;
            }

            ansTemp[lenth++] = rcvbyte;
            framelen = rcvbyte - 0x54;

            while (framelen-- > 0)
            {
                if (ComByte(&rcvbyte, 500) == FALSE)
                {
                    if (timer_open_flag == 1)
                    {
                        TimerStart(TIMER0);
                    }

                    PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                    return ID_FBREAK;
                }

                ansTemp[lenth++] = rcvbyte;
            }

            delay(SC_TagKWP2000.m_nFrameTime);
        }

        ans[1] ++;
        ans[ansLen++] = lenth;

        for (i = 0; i < lenth; i++)
        {
            ans[ansLen++] = ansTemp[i];
        }

        lenth = 0;
    }
    while(--sendnum);

    return ansLen;
}


//------------------------------------------------------------------------------
// Funtion:  6118+00/FF+FID（过滤的帧头ID）+CS说明：该模块属于只收模式的一种类型，上位机告诉要
//                接受的以什么字节开头的数据帧; 数据第二个字节-54H是接收数据的长度。
// Input  : argc - 参数长度
//          argv - 指令参数缓冲区
//          argv[0]:数据流标志
//          argv[1]:fid 指定所接受的数据帧以什么字节开头的
// Output : ans - 回复到上位机的数据
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、ans的格式为：总帧数 帧长度 帧数据
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 6118
//------------------------------------------------------------------------------
int HoldenOnlyKnowHeadOfFrame_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char fid, temp, len = 0, rcvbyte, framelen, tempbuff[5] =
    {
        0
    };
    unsigned int i, try_count = 0, ansLen;

    set_time0Stop();
    delay(10);

    //ResetUART3();
    if (argv[0] == 0)
    {
        ; //DATASTREAM_FG = true;
    }

    if ((fid = argv[1]) == 0)
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        PutDataToAns(szFNG, ans, sizeof(szFNG));
        return ID_FNG;
    }

    do
    {
        if (ComByte(&rcvbyte, 1000) == FALSE)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        if (rcvbyte != 0x08)
        {
            continue;
        }

        if (ComByte(&rcvbyte, 300) == FALSE)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        if (rcvbyte != 0x55)
        {
            continue;
        }

        if (ComByte(&rcvbyte, 300) == FALSE)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        if (rcvbyte != 0xa3)
        {
            continue;
        }

        break;
    }
    while(1);

    try_count = 0;

    do
    {
        if (ComByte(&rcvbyte, 1000) == FALSE)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        temp = rcvbyte;
        tempbuff[0] = rcvbyte;

        if (rcvbyte != fid)
        {
            if (ComByte(&rcvbyte, 300) == FALSE)
            {
                if (timer_open_flag == 1)
                {
                    TimerStart(TIMER0);
                }

                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                return ID_FBREAK;
            }

            tempbuff[1] = rcvbyte;
            len = rcvbyte - 0x54;

            for (i = 0; i < len; i++)
            {
                if (ComByte(&rcvbyte, 300) == FALSE)
                {
                    if (timer_open_flag == 1)
                    {
                        TimerStart(TIMER0);
                    }

                    PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                    return ID_FBREAK;
                }

                if (i < 3)
                {
                    tempbuff[i + 2] = rcvbyte;
                }
            }

            if ((tempbuff[0] == 0x08) && (tempbuff[1] == 0x55) && (tempbuff[2] == 0xaa))
            {
                if (try_count++ >= 2)
                {
                    if (timer_open_flag == 1)
                    {
                        TimerStart(TIMER0);
                    }

                    PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                    return ID_FBREAK;
                }
            }
        }
    }
    while((temp != fid));

    ans[0] = 0;
    ans[1] = 1;
    ansLen = 3;
    ans[ansLen++] = rcvbyte;
    ans[2] ++;

    if (ComByte(&rcvbyte, 500) == FALSE)
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    ans[ansLen++] = rcvbyte;
    ans[2] ++;
    framelen = rcvbyte - 0x54;

    while (framelen--)
    {
        if (ComByte(&rcvbyte, 500) == FALSE)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
            return ID_FBREAK;
        }

        ans[ansLen++] = rcvbyte;
        ans[2] ++;
    }

    return ansLen;
}


//------------------------------------------------------------------------------
// Funtion:  6119 针对SUBRU老协议的链路，其格式和6105的格式相同，其受数据的高电平的时间为12。
// Input  : argc - 参数长度
//          argv - 指令参数缓冲区
//          argv[0]:数据流开关标志
//          argv[1]:指定从ECU接收数据的帧数
//          argv[2]:发送往ECU的有效数据的长度
//          argv[3]~argv[len + 2]:发送往ECU的有效数据
//     后面的字节情况:
//          case1: argv[len + 3]非0:表示从ECU接收固定长度的数据
//          case2: argv[len + 3]为0，argv[len + 4]非0:argv[len + 4]和argv[len + 5]都为一种特殊接收数据的方式的关键字
//          case3: argv[len + 3]为0，argv[len + 4]为0，另外一种特殊接收数据的方式
// Output : ans - 回复到上位机的数据
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、ans的格式为：总帧数 帧长度 帧数据
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 6119
//------------------------------------------------------------------------------
int SubruOldProtocol_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned int temp, len = 0;
    unsigned char R0, B, rcvbyte, framenum;
    unsigned int ansLen = 0, argvPos = 0, ansPos = 0;

    set_time0Stop();

    if (CheckIoBusy(80, 8) == true)
    {
        PutDataToAns(szFNG, ans, sizeof(szFNG));
        return ID_FNG;
    }

    if (argv[0] == 0x0) //数据流开关
    {
        ; //DATASTREAM_FG = true;
    }

    if ((framenum = argv[1]) == 0x0)
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        PutDataToAns(szFNG, ans, sizeof(szFNG));
        return ID_FNG;
    }

    if ((len = argv[2]) == 0x0)
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        PutDataToAns(szFNG, ans, sizeof(szFNG));
        return ID_FNG;
    }

    argvPos = 3;

    do
    {
        ComSendByte(argv[argvPos++]);
        delayus(SC_TagKWP2000.m_nBtyetime);
    }
    while(--len > 1);

    ComSendByte(argv[argvPos++]);
    ans[0] = 0;
    ans[1] = framenum;
    ansLen = 2;

    do
    {
        //if(CheckIoBusy(60,10)==true)
        //    return ID_FBREAK;        // 2008-12-4, ID_FNG->ID_FBREAK
        //UART_FCR(3) = 0x02;
        if ((temp = argv[argvPos++]) != 0x0)
        {
            // 接收固定长度
            ans[ansLen++] = temp; // 接收长度

            do
            {
                if (ComByte(&rcvbyte, 500) == FALSE)
                {
                    PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                    return ID_FBREAK;
                }

                ans[ansLen++] = rcvbyte;
            }
            while(--temp > 0);
        }
        else
        {
            if ((temp = argv[argvPos++]) != 0x0)
            {
                ansPos = ansLen++; // 计算长度
                len = 0;
                B = argv[argvPos++];

                do
                {
                    if (ComByte(&rcvbyte, 500) == FALSE)
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        return ID_FBREAK;
                    }

                    ans[ansLen++] = rcvbyte;
                    len++;
                }
                while(--temp > 0);

                rcvbyte -= B;
                rcvbyte -= len;
                R0 = rcvbyte;

                do
                {
                    if (ComByte(&rcvbyte, 500) == FALSE)
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        return ID_FBREAK;
                    }

                    ans[ansLen++] = rcvbyte;
                    len++;
                }
                while(--R0 > 0);

                ans[ansPos] = len;
            }
            else
            {
                len = 0;
                ansPos = ansLen++;

                do
                {
                    if (ComByte(&rcvbyte, 500) == FALSE)
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        return ID_FBREAK;
                    }

                    ans[ansLen++] = rcvbyte;
                    len++;
                }
                while(rcvbyte != 0);

                if (ComByte(&rcvbyte, 500) == FALSE)
                {
                    PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                    return ID_FBREAK;
                }

                ans[ansLen++] = rcvbyte;
                len++;
                ans[ansPos] = len;
            }
        }
    }
    while(--framenum > 0);

    delay(SC_TagKWP2000.m_nFrameTime); // 2008-11-17, Delay100us -> Sleep

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    return ansLen;
}


//------------------------------------------------------------------------------
// Funtion:  OPEL/SAAB 发一帧收多帧.
// Input  : argc - 参数长度
//          argv - 指令参数缓冲区
//          argv[0]:数据流开关标志
//          argv[1]:检测总线是否空闲的时间参数
//          argv[2]:发送往ECU的有效数据的长度 len
//          argv[3]~argv[len + 2]:发送往ECU的有效数据
//     后面的字节情况:
//          case1: argv[len + 3]非0:表示从ECU接收固定长度的数据
//          case2: argv[len + 3]为0，argv[len + 4]非0:argv[len + 4]和argv[len + 5]都为一种特殊接收数据的方式的关键字
//          case3: argv[len + 3]为0，argv[len + 4]为0，另外一种特殊接收数据的方式
// Output : ans - 回复到上位机的数据
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、ans的格式为：总帧数 帧长度 帧数据
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 611c   注意，本函数也要求在PC有新指令下来之前，下位机必须与ECU反复通讯。此功能还未能实现
//------------------------------------------------------------------------------
int SendOneFrameDataToEcuGetAnyFrameAnswer_Check_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned int temp, argvPos = 0, len = 0, ansLen = 0;
    unsigned char R0, B, rcvbyte, busytime, bFrameUploaded = FALSE;
    unsigned char RepeatTime, checkNum = 0;

    set_time0Stop();
    RepeatTime = argv[0]; //反复和ECU通讯给上位机传送结果，还是只传送一次
    busytime = argv[1]; // 第二个字节，判断高电平的时间
    checkNum = 50 / busytime + 5;

    if (CheckIoBusy(checkNum, busytime) == TRUE) // 2008-12-5, 128->500
    {
        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    if ((len = argv[2]) == 0x0)
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
        return ID_FBREAK;
    }

    argvPos = 3;

    while (len-- > 1)
    {
        ComSendByte(argv[argvPos++]);
        delayus(SC_TagKWP2000.m_nBtyetime);
    }

    ComSendByte(argv[argvPos++]);

    //backupcommandpos = argvPos;
    ans[0] = 0;
    ans[1] = 1;
    ansLen = 3;

    //接收上位机指定的固定长度
    if ((temp = argv[argvPos++]) != 0x0)
    {
        ans[2] = temp; // 接收长度

        do
        {
            if (ComByte(&rcvbyte, 500) == FALSE)
            {
                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                return ID_FBREAK;
            }

            ans[ansLen++] = rcvbyte;
        }
        while(--temp > 0);
    }

    //自动判断
    else
    {
        //位置
        if ((temp = argv[argvPos++]) != 0x0)
        {
            len = 0;

            //增加长度
            B = argv[argvPos++];

            do
            {
                if (ComByte(&rcvbyte, 500) == FALSE)
                {
                    if (bFrameUploaded == FALSE)
                    {
                        PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                        return ID_FBREAK;
                    }
                    else
                    {
                        return 0xff;
                    }
                }

                ans[ansLen++] = rcvbyte;
                len++;
            }
            while(--temp > 0);

            rcvbyte -= B;
            rcvbyte -= len;
            R0 = rcvbyte;

            do
            {
                if (ComByte(&rcvbyte, 500) == FALSE)
                {
                    PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                    return ID_FBREAK;
                }

                ans[ansLen++] = rcvbyte;
                len++;
            }
            while(--R0 > 0);

            ans[2] = temp;
        }
        else
        {
            temp = argv[argvPos++];
            len = 0;

            do
            {
                if (ComByte(&rcvbyte, 500) == FALSE)
                {
                    PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                    return ID_FBREAK;
                }

                ans[ansLen++] = rcvbyte;
                len++;
            }
            while(rcvbyte != 0);

            if (ComByte(&rcvbyte, 500) == FALSE)
            {
                PutDataToAns(szFBREAK, ans, sizeof(szFBREAK));
                return ID_FBREAK;
            }

            ans[ansLen++] = rcvbyte;
            len++;
            ans[2] = len;
        }
    }

    delay(SC_TagKWP2000.m_nFrameTime);
    return ansLen;
}


//------------------------------------------------------------------------------
// Funtion:
// Input  : argc - 参数长度
//          argv - 数据格式
//         (0x61 + 0x13 ) + 发送次数 + 帧数 + 发送长度 + 发送内容 .
//         在链路层中发送次数和帧数默认的设置的是1
// 实例   : 0x61 0x13 0x01 0x01 0x11 0x55 0xaa 0x0b 0X61 0X03 0x08 0xID1 0XID2 0X03
//          0X21 0X01 0X02 0X00 0X00 0X00 0X00 0XXX
// Output : ans - 这个函数只是进行数据的发送
// Return : 0xff
// Info   : 6113
// 在使用这个函数的时候要注意的是：
// 1.接受时间的确认。
// 2.发送的帧数和发送的次数在链路层默认的是1.
//------------------------------------------------------------------------------
int CanbusOnlySendDataToEcu_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned int send_length = 0;
    unsigned char send_number = 0;
    unsigned char send_timers = 0;
    unsigned char i = 3;

    send_timers = argv[0];
    send_number = argv[1];
    send_length = argv[2];

    if ((send_timers == 0) || (send_number == 0) || (send_length == 0))
    {
        ans[0] = 0xff;
        ans[1] = 0x02;
        return 2;
    }

    set_time0Stop();

    while (send_length--)
    {
        ComSendByte(argv[i++]);
        delayus(SC_TagKWP2000.m_nBtyetime);
    }

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    ans[0] = 0xff;
    ans[1] = 0x00;
    return 2;
}


//------------------------------------------------------------------------------
// Funtion: lanrdrover send one frame and receive multiframe until timeout
// Input  : argc - 参数长度
//          argv - 数据格式
// 实例   : 55,aa,00,0a,ff,f5,62,0b,11,80,0c,f4,
// Output : ans - ff,00
// Return : 0xff
// Info   : 620b
//------------------------------------------------------------------------------
int LandroverSendAndReceiveFault_1(int argc, unsigned char * argv,
    unsigned char * ans)
{
    int temp, rcvpos = 3, len = 0;
    unsigned char rcvbyte = 0, i = 0;

    temp = argv[0];

    if (temp != 0x11)
    {
        ans[0] = 0xff;
        ans[1] = 0x00;
        return 2;
    }

    set_time0Stop();
    ComSendByte(temp);

    if (ComByte(&rcvbyte, 500) == false)
    {
        ans[0] = 0xff;
        ans[1] = 0x02;
        return 2;
    }

    if (rcvbyte != 0x11)
    {
        ans[0] = 0xff;
        ans[1] = 0x02;
        return 2;
    }

    temp = argv[1];

    if (temp != 0x80)
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        ans[0] = 0xff;
        ans[1] = 0x00;
        return 2;
    }

    delay(13);
    ComSendByte(temp);
    len = argv[2];
    ans[0] = 0;
    ans[1] = 1;
    ans[2] = len;
    i = 0;

    while (i < 2)
    {
        if (ComByte(&rcvbyte, 500) == false)
        {
            ans[0] = 0xff;
            ans[1] = 0x02;
            return 2;
        }

        ComSendByte(rcvbyte);
        i++;
    }

    while (len-- > 0)
    {
        if (ComByte(&rcvbyte, 500) == false)
        {
            ans[0] = 0xff;
            ans[1] = 0x02;
            return 2;
        }

        ans[rcvpos++] = rcvbyte;
        ComSendByte(rcvbyte);
    }

    if (ComByte(&rcvbyte, 500) == false)
    {
        ans[0] = 0xff;
        ans[1] = 0x02;
        return 2;
    }

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    return rcvpos;
}


//------------------------------------------------------------------------------
// Funtion: lanrdrover send one frame and receive multiframe until timeout
// Input  : argc - 参数长度
//          argv - 数据格式
// 实例   : 55,aa,00,0a,ff,f5,62,0b,11,80,0c,f4,
// Output : ans - ff,00
// Return : 0xff
// Info   : 6219
//------------------------------------------------------------------------------
int SysGazSendBankSingleFrame_1(int argc, unsigned char * argv, unsigned char * ans)
{
    int rcvpos = 2, Lenpos;
    unsigned char rcvbyte, cmdframenum, cmdframelen, rcvframenum = 0, rcvframelen = 0;
    int Sendpos = 0;

    set_time0Stop();

    if ((cmdframenum = argv[1]) == 0)
    {
        ans[0] = 0xff;
        ans[1] = 0x02;
        return 2;
    }

    Sendpos = 2;

    while (cmdframenum-- > 0)
    {
        if ((cmdframelen = argv[Sendpos++]) == 0)
        {
            ans[0] = 0xff;
            ans[1] = 0x02;
            return 2;
        }

        while (cmdframelen-- > 1)
        {
            ComSendByte(argv[Sendpos++]);
            delayus(SC_TagKWP2000.m_nBtyetime);
        }

        ComSendByte(argv[Sendpos++]);
        rcvframelen = 0;
        Lenpos = rcvpos;
        rcvpos++;

        if (ComByte(&rcvbyte, SC_TagKWP2000.m_Maxwaittime) == false)
        {
            ans[0] = 0xff;
            ans[1] = 0x02;
            return 2;
        }

        rcvframelen++;
        ans[rcvpos] = rcvbyte;
        rcvpos++;

        do
        {
            if (ComByte(&rcvbyte, 500) == false)
            {
                ans[0] = 0xff;
                ans[1] = 0x02;
                return 2;
            }

            ans[rcvpos] = rcvbyte;
            rcvframelen++;
            rcvpos++;
        }
        while(rcvbyte != 0x0d);

        ans[Lenpos] = rcvframelen;
    }

    ans[0] = 0;
    ans[1] = argv[1];

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    return rcvpos;
}


//------------------------------------------------------------------------------
// Funtion: 发一帧接收多帧
// Input  : argc - 参数长度
//          argv - 数据格式
// Output : ans - ff,00
// Return : 0xff
// Info   : 6222
//          55,aa,00,11,ff,ee,62,22,06,c1,13,f0,81,45,45,00,14,64,95,
//          00,01,07,83,f0,13,c1,e9,8f,bf,
//------------------------------------------------------------------------------
int SendOneFrameToEcuGetMultiFrameAnswer_1(int argc, unsigned char * argv,
    unsigned char * ans)
{
    unsigned char rcvbyte;
    int sendframesize, rcvlen, rcvpos = 3, waitingtime;

    set_time0Stop();

    // 06,c1,13,f0,81,45,45,00,14,64
    sendframesize = argv[0];

    if (sendframesize < 1)
    {
        ans[0] = 0xff;
        ans[1] = 0x01;
        return 2;
    }

    rcvlen = 0;

    // 可以加入一个连续的高电平判断
    while (ComByte(&rcvbyte, 2) == true)
    {
        if (rcvlen++ == 10)
        {
            ans[0] = 0xff;
            ans[1] = 0x02;
            return 2;
        }
    }

    rcvpos = 1;
    sendframesize--;

    while (sendframesize--)
    {
        ComSendByte(argv[rcvpos++]);
        delayus(SC_TagKWP2000.m_nBtyetime);
    }

    ComSendByte(argv[rcvpos++]);
    rcvlen = argv[rcvpos++] *256;
    rcvlen += argv[rcvpos++];
    waitingtime = CalculateTimeOut(argv[rcvpos], 10) * 10;
    rcvpos = 3;

    if (rcvlen == 0)
    {
        while (1)
        {
            if (ComByte(&rcvbyte, waitingtime) == false)
            {
                break;
            }

            ans[rcvpos++] = rcvbyte;
        }
    }
    else
    {
        while (rcvlen)
        {
            if (ComByte(&rcvbyte, waitingtime) == false)
            {
                break;
            }

            ans[rcvpos++] = rcvbyte;
            rcvlen--;
        }
    }

    ans[0] = 0;
    ans[1] = 1;
    ans[2] = rcvpos - 3;

    if (3 == rcvpos)
    {
        ans[0] = 0xff;
        ans[1] = 0x02;
        return 2;
    }

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    return rcvpos;
}


#endif

//--------------------------------------------------------- End Of File --------
