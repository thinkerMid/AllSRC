

//------------------------------------------------------------------------------
//  Purpose: 指令函数集 - 总线参数设置类
//  Funtion: 完成所有物理层总线参数设置函数，主要集中与 指令ID 6000 - 6100
//  Dependent:
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef FUN_SETLENPARA_C
#define FUN_SETLENPARA_C
#include "FUN_SetLinePara.h"
#include "FUN_Base.h"

//------------------------------------------------------------------------------
// Funtion: 设置通讯参数
// Input  : argc - 参数长度
//          argv - 参数缓冲区
//          argv[0]:通讯逻辑，
//          argv[1]:通讯电平
//          argv[2]:输出脚
//          argv[3]:输入脚
//          argv[4]:L线使能开关
//          argv[5]:DTS线脚位
//          argv[6]:通讯类型：通讯类型K线，通讯类型双线CAN，通讯类型单线CAN，通讯类型 PWM，通讯类型 VPW
// Output : ans - 回复到上位机的数据，正常返回FF 00, 异常返回FF 02
// Return : 返回的ans缓冲区的有效数据长度
// Info   : argv: 60,01,ff,ff,08,07,00,ff,00,91,
//                      1) 2) 3) 4) 5) 6) 7)
//          1) ECU 通信的电压逻辑，FF：正；00：负，他失败
//          2) ECU 通信的工作电压，FF:12V；00：5V，其他失败
//          3) 输出线，请参考X431 IO 脚定义
//          4) 输入线，请参考X431 IO 脚定义
//          5) L 线使能， FF：使能；00：不使能， 其他失败
//          6) DTS 通讯线, 00：DTS IO8, 01: DTS IO4: 02: DTS IO11, FF 或其他
//          7) 通迅类型设置, 00 K线, 01 双线CANBUS, 02 单线CANBUS, 03 PWM/VPW
//          以下情况程序中直接退出：
//          6 14 协议类型不是01()双线CAN)
//          3 11 协议类型不是01()双线CAN)
//          协议类型是 01，但是两个IO 相同 双线CAN
//          协议类型是 02，但是两个IO 不相同 单线CAN
//          不重复设置IO，与上次设置相同，退出
//          ans: ff,00, 固定回 ff 00 表示成功
//------------------------------------------------------------------------------
int SetIoParameter_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char len, i;

    //柳甲元将此行注释掉 *commandpos+=2;
    // 初始化继电器模块
    CMPHY_Relay_Init(&SC_RelayState);
    CMPHY_Relay_Reset();

    // 增加 解决一些IO出现的电平异常问题 2009-8-31 12:14
    // 6 14协议类型不是01()双线CAN)
    if ((argv[2] == 0x0a) && (argv[3] == 0x09) && (argv[6] != COMM_DOUBLE_CAN_TYPE))
    {
        PutDataToAns(szFOK, ans, sizeof(szFOK));
        return ID_FOK;
    }

    // 3 11协议类型不是01()双线CAN)
    if ((argv[2] == 0x06) && (argv[3] == 0x05) && (argv[6] != COMM_DOUBLE_CAN_TYPE))
    {
        PutDataToAns(szFOK, ans, sizeof(szFOK));
        return ID_FOK;
    }

    //协议类型是 01，但是两个IO相同 双线CAN
    if ((argv[2] == argv[3]) && (argv[6] == COMM_DOUBLE_CAN_TYPE))
    {
        PutDataToAns(szFOK, ans, sizeof(szFOK));
        return ID_FOK;
    }

    //协议类型是 02，但是两个IO不相同 单线CAN
    if ((argv[2] != argv[3]) && (argv[6] == COMM_SINGLE_CAN_TYPE))
    {
        PutDataToAns(szFOK, ans, sizeof(szFOK));
        return ID_FOK;
    }

    for (i = 0; i < 7; i++) //这里是什么意思 hejm
    {
        if (argv[i] != 0)
        {
            break;
        }
    }

    //CMPHY_Relay_Reset(); //ResetRelay();        // 2008-11-3
    for (len = 0; len < 7; len++)
    {
        switch (len)
        {
            case 0: // Positive and negative logic
                if (argv[0] == 0x00)
                {
                    SC_RelayState.LogicType = COMM_LOGIC_MINUS;                                                                                             //负逻辑;
                }
                else if (argv[0] == 0xff)
                {
                    SC_RelayState.LogicType = COMM_LOGIC_PLUS;                                                                                              //正逻辑;
                }
                else
                {
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
                }

                break;

            case 1: // logic level
                if (argv[1] == 0x00)
                {
                    SC_RelayState.LevelType = COMM_LEVEL_5V;
                }
                else if (argv[1] == 0xff)
                {
                    SC_RelayState.LevelType = COMM_LEVEL_12V;
                }
                else
                {
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
                }

                break;

            case 2: // output channel
                if (argv[2] <= 15)
                {
                    SC_RelayState.OutputChannel = argv[2];
                }
                else
                {
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
                }

                break;

            case 3: // input channel
                if (argv[3] <= 15)
                {
                    SC_RelayState.InputChannel = argv[3];
                }
                else
                {
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
                }

                break;

            case 4: // L line
                if (argv[4] == 0x00) // 关闭 L 线
                {
                    SC_RelayState.LLine = 0;
                }
                else if (argv[4] == 0xff) // 使能 L 线
                {
                    SC_RelayState.LLine = 1;
                }
                else
                {
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
                }

                break;

            case 5: // DTS line
                switch (argv[5])
                {
                    case 0x0:
                        SC_RelayState.Dts = COMM_DTS_IO8; // Dts DTS 8 号线
                        IOCTRLSelect(IOCTRL_XMINT, IOCTRL_OUTPUT | IOCTRL_USMODE);
                        IOCTRLSet(IOCTRL_LOW, 0);
                        break;

                    case 0x1:
                        SC_RelayState.Dts = COMM_DTS_IO4; //Dts DTS 4 号线
                        IOCTRLSelect(IOCTRL_XMINT, IOCTRL_OUTPUT | IOCTRL_USMODE);
                        IOCTRLSet(IOCTRL_LOW, 0);
                        break;

                    case 0x2:
                        SC_RelayState.Dts = COMM_DTS_IO11; // Dts DTS 11 号线
                        IOCTRLSelect(IOCTRL_XMINT, IOCTRL_OUTPUT | IOCTRL_USMODE);
                        IOCTRLSet(IOCTRL_LOW, 0);
                        break;

                    case 0xff:
                        SC_RelayState.Dts = 0;
                        IOCTRLSelect(IOCTRL_XMINT, IOCTRL_OUTPUT | IOCTRL_USMODE);
                        IOCTRLSet(IOCTRL_LOW, 1);
                        break;

                    default:
                        SC_RelayState.Dts = 0;
                        IOCTRLSelect(IOCTRL_XMINT, IOCTRL_OUTPUT | IOCTRL_USMODE);
                        IOCTRLSet(IOCTRL_LOW, 1);
                        PutDataToAns(szFNG, ans, sizeof(szFNG));
                        return ID_FNG;
                }

                break;

            case 6: // 通讯类型选择
                //RelayState.ucVCANType = 0;
                switch (argv[6])
                {
                    case 0x0: // 普通类型
                        SC_RelayState.CommType = argv[6];
                        break;

                    case 0x1: // 双线CAN
                        SC_RelayState.CommType = argv[6];
                        SC_RelayState.CANH = SC_RelayState.InputChannel;
                        SC_RelayState.CANL = SC_RelayState.OutputChannel;
                        break;

                    case 0x2: // 单线CAN
                        SC_RelayState.CommType = argv[6];
						SC_RelayState.CANH = SC_RelayState.InputChannel;
                        break;

                    //新下位机暂时不做J1708和RS232
                    default:
                        SC_RelayState.CommType = COMM_UART_TYPE;
                        break;
                }

                break;
        }
    }

    Keep_Link.rightState = TOOL_WAITING;                                                                                                                    //链路默认状态，防止在发链路时异常退出产生的问题

    if (timer_open_flag == 1) //
    {
        timer_open_flag = 0;                                                                                                                                // 定时器开关标识置0，防止在发链路时异常退出产生的问题
        TimerRelease(TIMER0);                                                                                                                               // 完全关闭定时器，防止在发链路时异常退出产生的问题
    }

    ComClose(COM_ECUPORT);
    CMPHY_Relay_Set(&SC_RelayState);
    ComOpen(COM_ECUPORT, &SC_com_portMode);
    PutDataToAns(szFOK, ans, sizeof(szFOK));
    return ID_FOK;
}


//------------------------------------------------------------------------------
// Funtion: 设置对ECU通讯串口波特率
// Input  : argc - 参数长度
//          argv - 参数缓冲区
//          argv[0]:值非0：决定波特率是由argv[1]到argv[2]组成，
//                  值为0：决定波特率是由argv[1]到argv[4]组成。
//    在argv[0]为0时：
//          argv[1]:波特率参数
//          argv[2]:波特率参数
//          argv[3]:波特率参数
//          argv[4]:波特率参数
//          argv[5]:字节校验方式
//    在argv[0]为非0时：
//          argv[1]:波特率参数
//          argv[2]:波特率参数
//          argv[3]:字节校验方式
// Output : ans - 回复到上位机的数据，正常返回FF 00, 异常返回FF 02
// Return : 返回的ans缓冲区的有效数据长度
// Info   : argv: 60,02,00,00,25,80,
//                      1) 2) 3) 4)
//          1) 如果为 0 则波特率为：1)2)3)4) - 0x00002580
//             如果不为 0 则波特率为：1)2)
//             60,02,D7,50,00 - 0xD750
//          send: ff,00, 固定回 ff 00 表示成功
//------------------------------------------------------------------------------
int SetEcuBaudRate_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned long baudrate;
    char num = 0;

    if (argv[0] == 0x00) // 2008-12-1
    {
        // 2008-12-1
        baudrate = argv[num++] *0x1000000;                                                                                                                  // 2008-12-1
        baudrate += argv[num++] *0x10000;                                                                                                                   // 2008-12-1
        baudrate += argv[num++] *0x100;                                                                                                                     // 2008-12-1
        baudrate += argv[num++] *0x1;                                                                                                                       // 2008-12-1
    } // 2008-12-1
    else
    {
        baudrate = argv[num++] *0x100;
        baudrate += argv[num++];
        baudrate = 65536 - baudrate;
        baudrate = (18432000 / 32) / baudrate;
    }

    SC_com_portMode.BaudRate = baudrate;
    SC_com_portMode.WordLength = Com_WordLength_8b;                                                                                                         //默认
    SC_com_portMode.StopBits = Com_StopBits_1;                                                                                                              //默认
    SC_com_portMode.Parity = Com_Parity_No;                                                                                                                 //默认

    switch (argv[num])
    {
        // Com_Parity_No - 无校验
        // Com_Parity_Even - 奇校验方式
        // Com_Parity_Odd - 偶校验方式
        case NULL: // No Parity
            SC_com_portMode.Parity = Com_Parity_No;
            break;

        case UART_EVEN_PARITY: // Even Parity
            SC_com_portMode.Parity = Com_Parity_Even;
            SC_com_portMode.WordLength = Com_WordLength_9b;
            break;

        case UART_ODD_PARITY: // Odd Parity
            SC_com_portMode.Parity = Com_Parity_Odd;
            SC_com_portMode.WordLength = Com_WordLength_9b;
            break;

        case UART_2STOPBITSFIX: // 2 Stop Bits
        case UART_2STOPBITSVAR: // 2 Stop Bits
            SC_com_portMode.Parity = Com_StopBits_2;
            break;

        default:
            PutDataToAns(szFNG, ans, sizeof(szFNG));
            return 1;
    }

    Keep_Link.rightState = TOOL_WAITING;                                                                                                                    //链路默认状态，防止在发链路时异常退出产生的问题

    if (timer_open_flag == 1) //
    {
        timer_open_flag = 0;                                                                                                                                // 定时器开关标识置0，防止在发链路时异常退出产生的问题
        TimerRelease(TIMER0);                                                                                                                               // 完全关闭定时器，防止在发链路时异常退出产生的问题
    }

    ComClose(COM_ECUPORT);                                                                                                                                  //防止在发链路时异常退出产生的问题，特别是bosch的读写中断未关闭，容易出问题。
    ComInit();
    ComOpen(COM_ECUPORT, &SC_com_portMode);
    PutDataToAns(szFOK, ans, sizeof(szFOK));
    return ID_FOK;
}


//------------------------------------------------------------------------------
// Funtion: 设置对ECU通讯时间参数
// Input  : argc - 参数长度
//          argv - 参数缓冲区
//          argv[0]:设置字节间隔参数
//          argv[1]:设置字节间隔参数
//          argv[2]:设置帧间隔参数
//          argv[3]:设置最大等待时间参数
//          argv[4]:设置最大等待时间参数 hejm
// Output : ans - 回复到上位机的数据，正常返回FF 00, 异常返回FF 02
// Return : 返回的ans缓冲区的有效数据长度
// Info   : argv: 60,03,00,1e,01,00,19
//                      1) 2) 3) 4)
//          1)2) 字节与字节间隔时间*10，精度0.1 毫秒. 0X001E表示3.0MS
//          3) 数据包之间时间间隔,单位毫秒/10, 不能为0, 01-表示10MS
//          4) 5 uint8_t：接收ECU 数据包最大等待时间/10，不能为0， 0X0019表示250MS
//          send: ff,00, 固定回 ff 00 表示成功
//------------------------------------------------------------------------------
int SetEcuCommTimeInterval_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char len;
    unsigned int temp;

    for (len = 0; len < 3; len++)
    {
        switch (len)
        {
            case 0: // 100us   60 03 00 32 02 00 0A
                SC_TagKWP2000.m_nBtyetime = argv[0] *256;
                SC_TagKWP2000.m_nBtyetime += argv[1];
                SC_TagKWP2000.m_nBtyetime *= 100;
                break;

            case 1:
                if (argv[2] == 0)
                {
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
                }

                SC_TagKWP2000.m_nFrameTime = argv[2] *100; // 2008-11-17, *10
                break;

            case 2:
                temp = argv[3] *0x100; // 2008-12-15, add

                //  temp = 0;
                temp += argv[4]; // 2008-12-15, add

                if (temp == 0)
                {
                    PutDataToAns(szFNG, ans, sizeof(szFNG));
                    return ID_FNG;
                }

                SC_TagKWP2000.m_Maxwaittime = temp * 10; // 2009-4-10, temp * 10
                break;
        }
    }

    Keep_Link.rightState = TOOL_WAITING;                                                                                                                    //链路默认状态，防止在发链路时异常退出产生的问题

    if (timer_open_flag == 1) //
    {
        timer_open_flag = 0;                                                                                                                                // 定时器开关标识置0，防止在发链路时异常退出产生的问题
        TimerRelease(TIMER0);                                                                                                                               // 完全关闭定时器，防止在发链路时异常退出产生的问题
    }

    PutDataToAns(szFOK, ans, sizeof(szFOK));
    return ID_FOK;
}


//------------------------------------------------------------------------------
// Funtion: 设置总线电平（高或低），多用于拉低拉高KWP协议
// Input  : argc - 参数长度
//          argv - 参数缓冲区
//          argv[0]:值为0表示拉低电平，值为0xff表示拉高电平
//          argv[1]:拉低或拉高电平的时间参数
//          argv[2]:拉低或拉高电平的时间参数
// Output : ans - 回复到上位机的数据，正常返回FF 00, 异常返回FF 02
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 6007, 00,00,19,
//------------------------------------------------------------------------------
int SetCommunicationLineVoltage_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char temp_1;
    unsigned int timer;

    temp_1 = argv[0];

    // 初始化 IO 口
    IOCTRLInit();
    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_MSMODE | IOCTRL_OUTPUT);
    timer = argv[1] *256 + argv[2];

    if (temp_1 == 0)
    {
        // 空闲电平拉高
        IOCTRLSet(IOCTRL_HIGH, 20);

        // 电平拉低
        IOCTRLSet(IOCTRL_LOW, timer);
    }
    else if (temp_1 == 0xff)
    {
        // 电平拉高
        timer = timer;
        IOCTRLSet(IOCTRL_HIGH, timer);
        ComOpen(COM_ECUPORT, &SC_com_portMode);//COM_ECUPORT = 1
    }
    else
    {
        PutDataToAns(szFNG, ans, sizeof(szFNG));
        return ID_FNG;
    }

    PutDataToAns(szFOK, ans, sizeof(szFOK));
    return ID_FOK;
}


//------------------------------------------------------------------------------
// Funtion: 设置连路保持
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : argv: 60,04,06,80,98,f1,01,3e,48,06,64
//                      1) 2) 3) 4) 5) 6) 7) 8) 9)
//          1) 链路命令长度(若为0，则终止链路)
//          2) 3) 4) 5) 6) 7) 链路数据包
//          8) 未知或无意义
//          9) 其值乘10 即：0x64*10=1000（ms）为链路间隔时间，若为0，则返回错误
//          send: ff,00, 固定回 ff 00 表示成功
//------------------------------------------------------------------------------
int SetLinkData_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char dataLength;
    unsigned int XOR_Lenth;

    //StopKeepLink(STOPLINKTIME,ALL);        //KEEPLINK_ACCESS);
    //柳甲元将此行注释掉 *commandpos += 2;        // 忽略0x6004
    // 初始化定时器
    receive_buff[260] = 0;

	//这里有差异 当timer_open_flag == 1的时候会有一个循环的延时 然后设置中断啥的 
    if (timer_open_flag == 1)
    {
        TimerStop(TIMER0);
    }

    dataLength = argv[0];
    Keep_Link.idleSendLen = dataLength;
    Keep_Link.linkState = LINK_OFF;

    if (dataLength != 0)
    {
        memcpy(Keep_Link.dataBuf, &argv[1], dataLength);

        //接收返回的长度
        //ziyingzhu 修改2010-1-27 15:45 允许只发不收模式
        Keep_Link.idleRecvLen = argv[dataLength + 1];

        //链路时间
        if (argv[dataLength + 2] == 0)
        {
            PutDataToAns(szFNG, ans, sizeof(szFNG));
            return ID_FNG;
        }

        Keep_Link.IdleTime = argv[dataLength + 2] *10;                                                                                                      // 上位机传下来为10ms,调整为1ms
        Keep_Link.linkType = KEEPLINK_ACCESS;                                                                                                               // 默认为普通链路

        //CANBUS协议类型数据
        if ((argv[1] == 0x55) && (argv[2] == 0xAA))
        {
            Keep_Link.linkType = KEEPLINK_CAN_GENERAL;

            if (((argv[6] &0x80) == 0) && (dataLength > 0x11)) //排除错误数据
            {
                PutDataToAns(szFNG, ans, sizeof(szFNG));
                return ID_FNG;
            }

            if ((argv[6] &0x80) && (dataLength > 0x13)) //排除错误数据
            {
                PutDataToAns(szFNG, ans, sizeof(szFNG));
                return ID_FNG;
            }

            //进行异或校验检查
            XOR_Lenth = argv[3] +2;

            if (checkXOR_Parity(XOR_Lenth, &argv[3]) == FALSE)
            {
                PutDataToAns(szFNG, ans, sizeof(szFNG));
                return ID_FNG;
            }

            //解包CANBUS数据
            setCanbusLinkDataToken();
        }

        //StartKeepLink(dwKeepLinkTime,eLinkType);
    }
    else
    {
        //停止链路
        if (timer_open_flag == 1)
        {
            TimerStop(TIMER0);
        }

        Keep_Link.linkType = NULL;
        PutDataToAns(szFOK, ans, sizeof(szFOK));
        return ID_FOK;
    }

    TimerInit();

    switch (Keep_Link.linkType)
    {
        case KEEPLINK_ACCESS:
            TimerConfig(TIMER0, 5);

            // 加载链路维持中断
            Keep_Link.rightState = TOOL_WAITING;
            TimerInterruptFuntionLoad(TIMER0, SC_setlinkkeep);
            TimerStart(TIMER0);
            timer_open_flag = 1;
            break;

        case KEEPLINK_CAN_GENERAL:
            Keep_Link.rightState = TOOL_WAITING;
            TimerConfig(TIMER0, 10);
            TimerInterruptFuntionLoad(TIMER0, SC_CML_ISO15765_Time);
            TimerStart(TIMER0);
            timer_open_flag = 1;
            break;

        default:
            break;
    }

    PutDataToAns(szFOK, ans, sizeof(szFOK));
    return ID_FOK;
}


//------------------------------------------------------------------------------
// Funtion: 设置总线电平（高或低），多用于拉低拉高KWP协议
// Input  : argc - 参数长度
//          argv - 参数缓冲区
//          argv[0]:值为0表示拉低电平，值为0xff表示拉高电平
//          argv[1]:拉低或拉高电平的时间参数
//          argv[2]:拉低或拉高电平的时间参数
// Output : ans - 回复到上位机的数据，正常返回FF 00, 异常返回FF 02
// Return : 返回的ans缓冲区的有效数据长度
// Info   : recv: 60,0e,40,00
//                      1) 2)
//          1): VPW接收脉宽时间（一般设置为48 到64 之间，建议58）
//          2): 通迅电平 01: CA_5V 00: CA_12V
//------------------------------------------------------------------------------
int SetVpwLogicLevel_1(int argc, unsigned char * argv, unsigned char * ans)
{
    //s_Parameter.dwVpwWaitTime = argv[0];
    switch (argv[1])
    {
        case 0x0:
            // s_Parameter.ucVpwLevelType = COMM_LEVEL_5V;  //VPW逻辑电压
            break;

        case 0xff:
            // s_Parameter.ucVpwLevelType = COMM_LEVEL_12V;
            break;

        default:
            PutDataToAns(szFNG, ans, sizeof(szFNG));
            return ID_FNG;
    }

    PutDataToAns(szFOK, ans, sizeof(szFOK));
    return ID_FOK;
}


//------------------------------------------------------------------------------
// Funtion: 设置链路保持且无需应答
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : recv: 600f 05 81 10 f1 3e 00 10 25
//                     1) 2) 3) 4) 5) 6) 7) 8)
//          1) 链路数据长度
//          2)3)4)5)6) 链路数据
//          7) 无效参数
//          8) 链路触发事件
//------------------------------------------------------------------------------
int SetLinkNoAnswer_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char dataLength;
    unsigned int XOR_Lenth;


    //StopKeepLink(STOPLINKTIME,ALL);        //KEEPLINK_ACCESS);
    //柳甲元将此行注释掉 *commandpos += 2;        // 忽略0x6004
    // 初始化定时器
    if (timer_open_flag == 1)
    {
        TimerStop(TIMER0);
    }

    dataLength = argv[0];
    Keep_Link.idleSendLen = dataLength;
    Keep_Link.linkState = LINK_OFF;

    if (dataLength != 0)
    {
        memcpy(Keep_Link.dataBuf, &argv[1], dataLength);

        //接收返回的长度
        //ziyingzhu 修改2010-1-27 15:45 允许只发不收模式
        Keep_Link.idleRecvLen = argv[dataLength + 1];

        //链路时间
        if (argv[dataLength + 2] == 0)
        {
            PutDataToAns(szFNG, ans, sizeof(szFNG));
            return ID_FNG;
        }

        Keep_Link.IdleTime = argv[dataLength + 2] *10;                                                                                                      // 上位机传下来为10ms,调整为1ms
        Keep_Link.linkType = KEEPLINK_ACCESS;                                                                                                               // 默认为普通链路

        //CANBUS协议类型数据
        if ((argv[1] == 0x55) && (argv[2] == 0xAA))
        {
            Keep_Link.linkType = KEEPLINK_CAN_GENERAL;

            if (((argv[6]&0x80) == 0) && (dataLength > 0x14)) //排除错误数据
            {
                PutDataToAns(szFNG, ans, sizeof(szFNG));
                return ID_FNG;
            }

            if (((argv[6]&0x80)) && (dataLength > 0x13)) //排除错误数据
            {
                PutDataToAns(szFNG, ans, sizeof(szFNG));
                return ID_FNG;
            }

            //进行异或校验检查
            XOR_Lenth = argv[3] +2;

            if (checkXOR_Parity(XOR_Lenth, &argv[3]) == FALSE)
            {
                PutDataToAns(szFNG, ans, sizeof(szFNG));
                return ID_FNG;
            }

            //解包CANBUS数据
            setCanbusLinkDataToken();
        }

        //StartKeepLink(dwKeepLinkTime,eLinkType);
    }
    else
    {
        //停止链路
        if (timer_open_flag == 1)
        {
            TimerStop(TIMER0);
        }

        Keep_Link.linkType = NULL;
        PutDataToAns(szFOK, ans, sizeof(szFOK));
        return ID_FOK;
    }

    TimerInit();

    switch (Keep_Link.linkType)
    {
        case KEEPLINK_ACCESS:
            TimerConfig(TIMER0, 5);

            // 加载链路维持中断
            Keep_Link.rightState = TOOL_WAITING;
            TimerInterruptFuntionLoad(TIMER0, SC_setlinkkeep);
            TimerStart(TIMER0);
            timer_open_flag = 1;
            break;

        case KEEPLINK_CAN_GENERAL:
            Keep_Link.rightState = TOOL_WAITING;
            TimerConfig(TIMER0, 10);
            TimerInterruptFuntionLoad(TIMER0, SC_CML_ISO15765_Time);
            TimerStart(TIMER0);
            timer_open_flag = 1;
            break;

        default:
            break;
    }

    PutDataToAns(szFOK, ans, sizeof(szFOK));
    return ID_FOK;
}


//------------------------------------------------------------------------------
// Funtion: 获取 smartbox 产品号
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : SendCmd: 55 aa f0 f8 00 05 00 27 01 60 20 6b
//          ReceiveCmd: 00 01 00 05 05 39 38 30 35 34 30 30 39 38 30 30 30 01 02 00 01
//------------------------------------------------------------------------------
int GetBoxInfo(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char SendData[] =
    {
        0x00, 0x01, 0x00, 0x05, 0x05, 0x39, 0x38, 0x30, 0x35, 0x34,
        0x30, 0x30, 0x39, 0x38, 0x30, 0x30, 0x30, 0x01, 0x02, 0x00,
        0x01, 0x00, 0x01
    };
    memmove(ans, SendData, sizeof(SendData));
    return sizeof(SendData);
}


//------------------------------------------------------------------------------
// Funtion: 设置CANBUS链路保持
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : recv: 60,0C,06,80,98,f1,01,3e,48,06,64
//                      1) 2) 3) 4) 5) 6) 7) 8) 9)
//          1) 链路命令长度(若为0，则终止链路)
//          2) 3) 4) 5) 6) 7) 链路数据包
//          8) 未知或无意义
//          9) 其值乘10 即：0x64*10=1000（ms）为链路间隔时间，若为0，则返回错误
//          send: ff,00, 固定回 ff 00 表示成功
//------------------------------------------------------------------------------
int SetCANBusLinkData_1(int argc, unsigned char * argv, unsigned char * ans)
{
    return SetLinkData_1(argc, argv, ans);
}


//------------------------------------------------------------------------------
// Funtion: 设置CANBUS链路保持
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : recv: 60,0D,06,80,98,f1,01,3e,48,06,64
//                      1) 2) 3) 4) 5) 6) 7) 8) 9)
//          1) 链路命令长度(若为0，则终止链路)
//          2) 3) 4) 5) 6) 7) 链路数据包
//          8) 未知或无意义
//          9) 其值乘10 即：0x64*10=1000（ms）为链路间隔时间，若为0，则返回错误
//          send: ff,00, 固定回 ff 00 表示成功
//------------------------------------------------------------------------------
int SetCANBus2LinkData_1(int argc, unsigned char * argv, unsigned char * ans)
{
    int temp_1;

    temp_1 = SetLinkData_1(argc, argv, ans);
    return temp_1;
}


//------------------------------------------------------------------------------
// Funtion: 设置Benz38接头通讯IO
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : recv: 60,16,06,
//                      1)
//          1) 通讯IO
//          send: ff,00, 表示成功
//                ff,02  表示设置失败
//------------------------------------------------------------------------------
int Benz38SelectLine_1(int argc, unsigned char * argv, unsigned char * ans)
{
    char temp, ioadd, i, connector_type = 0x80, rxdflg = 0;
    char position[] =
    {
        0xE1, 0xE1, 0xE1, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB,
        0xEC, 0xED, 0xEE, 0xEF, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA,
        0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5
    };

    // 初始化继电器模块
    CMPHY_Relay_Init(&SC_RelayState);
    CMPHY_Relay_Reset();
    ioadd = argv[0];

    if (ioadd > 38)
    {
        ans[0] = 0x00;
        ans[1] = 0x02;
        return 2;
    }

    SC_RelayState.LogicType = COMM_LOGIC_PLUS;
    SC_RelayState.LevelType = COMM_LEVEL_12V;
    SC_RelayState.OutputChannel = 0x02;
    SC_RelayState.InputChannel = 0x02;
    SC_RelayState.Dts = COMM_DTS_IO8;

    //benz 38 里面没有CAN 协议ziyingzhu 2009-5-25
    SC_RelayState.CommType = COMM_UART_TYPE;

    //ziyingzhu 增加.确保下次上位机给上位机发送6001命令时，强制更新2009-8-10 14:09
    //IoParameterArray[6] = 0xff;
    CMPHY_Relay_Set(&SC_RelayState);                                                                                                                        // 选择IO
    delay(10);
    IOCTRLSelect(IOCTRL_KLINEREAD, IOCTRL_MSMODE | IOCTRL_INPUT);

    if (IOCTRLGet() == IOCTRL_HIGH)
    {
        rxdflg = 0xff;
    }

    delay(10);

    if ((IOCTRLGet() == IOCTRL_LOW) && (rxdflg == 0))
    {
        SC_RelayState.OutputChannel = 0x01;
        SC_RelayState.InputChannel = 0x03;
        connector_type = 0x81;
        CMPHY_Relay_Set(&SC_RelayState);
    }
    else
    {
        SC_RelayState.OutputChannel = 0x01;
        SC_RelayState.InputChannel = 0x01;
        connector_type = 0x80;
        CMPHY_Relay_Set(&SC_RelayState);
    }

    delay(10);
    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
    IOCTRLSet(IOCTRL_LOW, 0);
    i = ioadd - 1;
    ioadd = position[i];

    //ziyingzhu 修改 2009-8-18 14:55
    //OutPin(ioLCTR,FASTIO,1);    // 关闭L线
    //OutPin(ioLCTRL,FASTIO,1);    // 关闭L线输入
    for (i = 0; i < 8; i++)
    {
        delayus(20);                                                                                                                                        // 2009-04-08, 10->20
        IOCTRLSelect(IOCTRL_XMINT, IOCTRL_USMODE | IOCTRL_OUTPUT);
        IOCTRLSet(IOCTRL_LOW, 0);
        temp = ioadd & 0x80;
        ioadd = ioadd << 1;
        IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);

        if (temp)
        {
            IOCTRLSet(IOCTRL_HIGH, 0);
        }
        else
        {
            IOCTRLSet(IOCTRL_LOW, 0);
        }

        delayus(50);                                                                                                                                        // 2009-04-08, 10->20
        IOCTRLSelect(IOCTRL_XMINT, IOCTRL_USMODE | IOCTRL_OUTPUT);
        IOCTRLSet(IOCTRL_HIGH, 0);
    }

    delayus(10);                                                                                                                                            // 2009-04-08, add
    IOCTRLSelect(IOCTRL_KLINESEND, IOCTRL_USMODE | IOCTRL_OUTPUT);
    IOCTRLSet(IOCTRL_HIGH, 0);

    //ziyingzhu 修改 2009-8-18 14:55
    //OutPin(ioLCTR,FASTIO,0);    // 使能L线
    //OutPin(ioLCTRL,FASTIO,0);    // 打开L线输入
    //PinSel(ioTXD,3);
    ans[0] = 0x02;
    ans[1] = 0xff;
    ans[2] = connector_type;
    return 3;
}


//------------------------------------------------------------------------------
// Funtion: 设置接头与主机通讯波特率（无效果）
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : recv: 60,19,00,00,25,00
//                      1) 2) 3) 4)
//          1)2)3)4)  通讯波特率
//          send: ff,00, 表示成功
//------------------------------------------------------------------------------
int SetConnectorBaudRate_1(int argc, unsigned char * argv, unsigned char * ans)
{
    ans[0] = 0xff;
    ans[1] = 0x00;
    return 2;
}


//------------------------------------------------------------------------------
// Funtion: 设置接头与主机通讯波特率（无效果）
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : recv: 60,1B,06,80,98,f1,01,3e,48,06,64
//                      1) 2) 3) 4) 5) 6) 7) 8) 9)
//          1) 链路命令长度(若为0，则终止链路)
//          2) 3) 4) 5) 6) 7) 链路数据包
//          8) 未知或无意义
//          9) 其值乘10 即：0x64*10=1000（ms）为链路间隔时间，若为0，则返回错误
//          send: ff,00, 固定回 ff 00 表示成功
//------------------------------------------------------------------------------
int SetBenzHMFLinkKeep_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char len;
    unsigned char argvPos = 0;

    set_time0Stop();
    Keep_Link.linkState = LINK_OFF;
	Keep_Link.chCount = 0;//IDA反汇编中多出这个字段的赋值

    if ((len = argv[argvPos++]) != 0)
    {
        Keep_Link.idleSendLen = len;
        memcpy(Keep_Link.dataBuf, &argv[argvPos], len);
        argvPos += len;
        argvPos++;                                                                                                                                          // 未知
        Keep_Link.IdleTime = (argv[argvPos++]) * 10;
        TimerInit();
        TimerConfig(TIMER0, 5);

        // 加载链路维持中断
        Keep_Link.rightState = TOOL_WAITING;
        TimerInterruptFuntionLoad(TIMER0, SC_setlinkkeep);
        timer_open_flag = 1;
        TimerStart(TIMER0);
    }

    PutDataToAns(szFOK, ans, sizeof(szFOK));
    return ID_FOK;
}


//------------------------------------------------------------------------------
// Funtion: 设置 多个 通讯 IO
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6021
//------------------------------------------------------------------------------
int SelectMultiIoLine(int argc, unsigned char * argv, unsigned char * ans)
{
    ans[0] = 0x00;
    ans[1] = 0x02;
    return 2;
}


//------------------------------------------------------------------------------
// Funtion:
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6021
//------------------------------------------------------------------------------
#define M3_STM32_DPU            0x02


int GetDownloadType(int argc, unsigned char * argv, unsigned char * ans)
{
    ans[0] = 0x00;
    ans[1] = M3_STM32_DPU;
    return 2;
}


#endif

//--------------------------------------------------------- End Of File --------
