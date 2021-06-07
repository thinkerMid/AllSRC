

//------------------------------------------------------------------------------
//  Purpose: 指令函数集通用函数 （中断函数和特殊函数，对上层提供支持）
//  Funtion: 对 FUN_CAN FUN_KLine FUN_SetLinePara 提供支持
//  Dependent:
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef FUN_Base_C
#define FUN_Base_C
#include "FUN_Base.h"
unsigned char szFOK[4] =
{
    0xff, 0x00
};


unsigned char szFNG[4] =
{
    0xff, 0x01
};


unsigned char szFBREAK[4] =
{
    0xff, 0x02
};


unsigned char szFNOWAIT[4] =
{
    0xff, 0x03
};


unsigned char szErrorFrame[7] =
{
    0x00, 0x06, 0xff, 0xf9, 0xff, 0xff
};


unsigned char timer_open_flag = 0;
unsigned char LinkFaultTime = 0;
COM_PORTMODE SC_com_portMode;
CMPHY_Relay SC_RelayState; //继电器相关数据参数
SC_CML_ISO14230 SC_TagKWP2000;
SC_CML_ISO15765 SC_TagISO15765;
SC_CML_KWP1281 SC_TagKWP1281;
SC_CMLISO15765para CAN_SetPara;
CAN_CONFIGPARA PHY_CAN_SetPara;
CAN_RXMessage Can_RXM;
SC_CMLISO15765ENTERFRAME CanEnterFrame;
SET_LINK_KEEP Keep_Link;
unsigned char receive_buff[261] =
{
    0
};


//-- Defined of struct ---------------------------------------------------------
#define CMPISO14230_FMT_ADR_LEN         0x80  //(0x80,ADR,ADR,LEN,SID,DATA,CHK)
#define CMPISO14230_FMT_ADR             0x81  //(0x81,ADR,ADR,SID,DATA,CHK)
#define CMPISO14230_FMT                 0x01  //(0x01,SID,DATA,CHK)
#define CMPISO14230_FMT_LEN             0x00  //(0x00,LEN,SID,DATA,CHK)
#define CMPISO14230_FMT_ADR_LEN_FUN     0xc0  //(0xC0,ADR,ADR,LEN,SID,DATA,CHK)
#define CMPISO14230_FMT_ADR_FUN         0xc1  //(0xC1,ADR,ADR,SID,DATA,CHK)
#define CMPISO14230_FMT_RESERVE         0x40  //(0x40.....)

//------------------------------------------------------------------------------
//函数名称:CANUnPacket
//函数的功能:实现的是把接收到得canbus数据包,以数组的形式进行存储
//函数的参数:buff转换后的存储区
//           candata从can总线接受的数据
//函数的返回值：接受的数据长度.
//------------------------------------------------------------------------------
unsigned char CANUnPacket(unsigned char * buff, CAN_RXMessage rx_data)
{
    unsigned int length = 0;
    unsigned int i = 0;
    unsigned int temp = 0;

    if ((rx_data.IDE == CAN_Id_Standard)) //说明是标准帧
    {
        temp = (rx_data.StdId << 5);
        buff[i++] = (rx_data.RTR << 6) | (rx_data.DLC & 0x0f); //帧头
        buff[i++] = (temp / 0x100);
        buff[i++] = (temp & 0x00ff);
        length = length + 3;
    }
    else //说明是扩展帧.
    {
        temp = rx_data.ExtId << 3;
        buff[i++] = (rx_data.RTR << 6) | (rx_data.DLC) | (0x80); //帧头
        buff[i++] = temp / 0x1000000;
        buff[i++] = temp / 0x10000;
        buff[i++] = temp / 0x100;
        buff[i++] = (temp & 0xff);
        length = length + 5;
    }

    buff = &buff[i];

    for (i = 0; i < rx_data.DLC; i++)
    {
        buff[i] = rx_data.Data[i];
    }

    return (length + rx_data.DLC);
}


//------------------------------------------------------------------------------
//函数名称:ReadOneFrameFromCANBuf_Ext
//函数的参数:*ch接受的数据
//           IDNum接受的ID数
//           specialdata 特殊数字//包括0x30,0x7f,0x78
//           bretMode模式字:
//             1.表示的是:ch里面的格式是：0x55,0xaa,len,0x08,0xid1,0xid2,,,,,
//               这里有校验位。对于没有校验位的处理，要在程序中进行修改。
//             2.表示的是:ch里面的格式是：data(除去数据去里面的0x21,0x22,.0x2f等的值)
//             3.表示的是:ch里面的格式是：0x08,0xid1,0xid2,data0,data1,,,,,,,.
//           timeout:接受超时。
//函数的返回值：0表示接受失败 ;
//              非0表示的是返回的是数据区的长度.
//------------------------------------------------------------------------------
unsigned int ReadOneFrameFromCANBuf_Ext(unsigned char * ch, unsigned char IDNum,
    unsigned char * specialdata,
    unsigned char bretMode,
    unsigned int timeout)
{
    unsigned int i = 0;
    unsigned int length = 0; //读取一帧的长度
    unsigned char btemp[30] =
    {
        0x55, 0xaa, 0x0b
    };
    unsigned int sum = 0;
    CAN_RXMessage rx_data =
    {
        0, //StdId
        0, //Extid
        0, //IDE
        0, //RTR
        0, //DLC
        {
            0
        },

        //Data[8]
    };
    if (CANRecv(&rx_data, timeout) == false)
    {
        return false;
    } //接受一帧canbus数据

    length = CANUnPacket(ch, rx_data);

    switch (IDNum)
    {
        case 1:
            specialdata[0] = ch[3];
            specialdata[1] = ch[4];
            specialdata[2] = ch[6];
            specialdata[3] = ch[7]; //can 30 st
            break;

        case 3:
            specialdata[0] = ch[4];
            specialdata[1] = ch[5];
            specialdata[2] = ch[7];
            specialdata[3] = ch[6]; //can 30 st
            break;

        case 4:
            specialdata[0] = ch[5];
            specialdata[1] = ch[6];
            specialdata[2] = ch[8];
            specialdata[3] = ch[7];
            break;

        default: // 08 FD E0 02 7f sid 78
            specialdata[0] = ch[3]; //这里可能存放的是0X7F,
            specialdata[1] = ch[4]; //这里可能存放的是0xsid
            specialdata[2] = ch[6]; //这里可能存放的是0x78.
            specialdata[3] = ch[5];
            break;
    }

    switch (bretMode)
    {
        case 1: //设置位0x55,0xaa,0xlen 0x08,0xid1,0xid2,data...cs
            memcpy(btemp + 3, ch, length); //
            btemp[2] = length;
            memcpy(ch, btemp, length + 4);

            for (i = 0; i < length; i++)
            {
                sum ^= ch[i + 3]; //校验位是从数据位开始进行计算的
            }

            ch[length + 3] = sum;

            //length = length + 4;6202 这里没有进行校验位
            length = length + 3;

            break; /*这里是否需要校验位*/
            /*---------------------------------------------------
                                在这里长度返回采用的是整个数据的长度而不是实际的长度
                                所以在进行用长度进行判断接受的时候可能出现长度小于0
                                ----------------------------------------------------*/

        case 2: //只返回有效数据字节(不包括ID)
            switch (IDNum)
            {
                case 3:
                    length -= 4; //ch ->0x08 0xid1 0xid2 0xid3 data;
                    memcpy(btemp, ch + 4, length);
                    memcpy(ch, btemp, length); //ch -> data;
                    break;

                case 4:
                    length -= 5;
                    memcpy(btemp, ch + 5, length);
                    memcpy(ch, btemp, length);
                    break;

                default:
                    length -= 3;
                    memcpy(btemp, ch + 3, length); //ch->08 id1 id2 ....
                    memcpy(ch, btemp, length); //ch ->data

                    //这里面包含了0x21,0x22,0x23等等。
                    break;
            }

            break;

        case 3:
            break;

        default:
            break;
    }

    return length;
}


//-------------------------------------------------------------------------------
//函数的功能是:发送一帧数据到ECU
//函数参数: uint8_t *BUFF  发送缓冲区 0x08,0xid1,0xid2,0x.......
//          uint8_t  IDNum 发送的ID个数//在实际的处理中只有扩展帧和标准帧两种.
//          uint8_t *specialdata 用来存储标志位
//函数的返回值：true/false
//--------------------------------------------------------------------------------
unsigned int CANSendOneFrame_Ext(unsigned char * buff, unsigned char IDNum,
    unsigned char * specialdata)
{
    CAN_TXMessage Canmsg;
    unsigned int head = 0;
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int Temp;

    head = buff[i++];

    if (! (head & 0x80)) //说明是标准帧
    {
        Canmsg.IDE = CAN_Id_Standard; //说明是标准帧
        Temp = buff[i++] *0x100;
        Temp += buff[i++];
        Temp >>= 5; //注意在这里GX3是进行了
        Canmsg.StdId = Temp;
        Canmsg.RTR = head & 0x40;
        Canmsg.DLC = head & 0x3f;

        for (j = 0; j < Canmsg.DLC; j++)
        {
            Canmsg.Data[j] = buff[i++];
        }
    }
    else //说明是扩展帧
    {
        Canmsg.IDE = CAN_Id_Extended; //说明是标准帧
        Canmsg.ExtId = buff[i++] *0x1000000;
        Canmsg.ExtId += buff[i++] *0x10000;
        Canmsg.ExtId += buff[i++] *0x100;
        Canmsg.ExtId += buff[i++];
        Canmsg.ExtId >>= 3;
        Canmsg.RTR = head & 0x40;
        Canmsg.DLC = head & 0x3f;

        for (j = 0; j < Canmsg.DLC; j++)
        {
            Canmsg.Data[j] = buff[i++];
        }
    }

    if (IDNum == 3)
    {
        specialdata[0] = Canmsg.Data[1]; //0x10
        specialdata[1] = Canmsg.Data[2]; //bn
        specialdata[2] = Canmsg.Data[3]; //st
    }
    else
    {
        specialdata[0] = Canmsg.Data[0]; //0x10
        specialdata[1] = Canmsg.Data[1]; //bn
        specialdata[2] = Canmsg.Data[2]; //st
    }

    return CANSend(&Canmsg);
}


//------------------------------------------------------------------------------
// Funtion: 关闭定时器，清空计数器
// Input  : 无
// Output : 无
// Return : void
// Info   :
//------------------------------------------------------------------------------
void set_time0Stop(void)
{
    Keep_Link.timeCount = 0;
    SC_TagISO15765.m_nMScount = 0;
    SC_TagKWP1281.m_nMScount = 0;

    if (timer_open_flag == 1)
    {
        while (Keep_Link.rightState != TOOL_WAITING) //防止发生链路维持正在发送的时候定时器被关闭
        {
            ;
        }

        if (Keep_Link.linkState == LINK_OPEN)
        {
            delay(Keep_Link.IdleTime / 2);
        }

        Keep_Link.linkState = LINK_OFF;
        TimerStop(TIMER0);
    }
}


//------------------------------------------------------------------------------
// Funtion: 设置回复给上位机的数据包
// Input  : data - 回复给上位机的数据
//          Ans - 回复给上位机的数据包
//          dataLenth - 数据包的长度
// Output : ans - 回复到上位机的数据
// Return : 无
// Info   :
//------------------------------------------------------------------------------
void PutDataToAns(unsigned char * data, unsigned char * Ans, int dataLenth)
{
    while (dataLenth--)
    {
        * (Ans++) = * (data++);
    }
}


//------------------------------------------------------------------------------
// Funtion: 设置继电器参数
// Input  : relayVal - 要修改的参数的值
//          relayType - 要修改的参数的类型
// Return : 回复到上位机的数据长度
// Info   :
//------------------------------------------------------------------------------
int SetComPara(unsigned char comVal, unsigned char comType)
{
    switch (comType)
    {
        case BAUDRATE:
            SC_com_portMode.BaudRate = comVal;
            break;

        case WORDLENGTH:
            SC_com_portMode.WordLength = comVal;
            break;

        case STOPBITS:
            SC_com_portMode.StopBits = comVal;
            break;

        case PARITY:
            SC_com_portMode.Parity = comVal;
            break;

        default:
            break;
    }

    return 0;
}


//------------------------------------------------------------------------------
// Funtion: 设置继电器参数
// Input  : relayVal - 要修改的参数的值
//          relayType - 要修改的参数的类型
// Return : 回复到上位机的数据长度
// Info   :
//------------------------------------------------------------------------------
int SetRelayPara(unsigned char relayVal, unsigned char relayType)
{
    switch (relayType)
    {
        case INPUTCHANNEL:
            SC_RelayState.InputChannel = relayVal;
            break;

        case OUTPUTCHANNEL:
            SC_RelayState.OutputChannel = relayVal;
            break;

        case CAN_H:
            SC_RelayState.CANH = relayVal;
            break;

        case CAN_L:
            SC_RelayState.CANL = relayVal;
            break;

        case COMMTYPE:
            SC_RelayState.CommType = relayVal;
            break;

        case LOGICTYPE:
            SC_RelayState.LogicType = relayVal;
            break;

        case LEVELTYPE:
            SC_RelayState.LevelType = relayVal;
            break;

        case LLINE:
            SC_RelayState.LLine = relayVal;
            break;

        case DTS:
            SC_RelayState.Dts = relayVal;
            break;

        default:
            break;
    }

    return 0;
}


//------------------------------------------------------------------------------
// Funtion: 进行异或校验检查
// Input  : datalenth - 进行校验的数据长度
//          dataPos - 进行校验的数据头指针
// Output :
// Return : 校验正确返回TRUE,否则返回FALSE
// Info   :
//------------------------------------------------------------------------------
int checkXOR_Parity(int datalenth, unsigned char * dataPos)
{
    unsigned char temp = 0;

    while (datalenth--)
    {
        temp ^= * (dataPos++);
    }

    if (temp)
    {
        return FALSE;
    }
    else
    {
        return true;
    }
}


//------------------------------------------------------------------------------
// Funtion: 将整帧的CANBUS链路维持数据解包放入指定的结构体中
// Input  : 无
// Output : 无
// Return : void
// Info   : Keep_Link.dataBuf 55,aa,0f,61,01,88,c6,d0,87,88,02,3e,80,ff,ff,ff,ff,ff,bd,0d,c8,44
//------------------------------------------------------------------------------
void setCanbusLinkDataToken()
{
    unsigned char * temp_1;
    unsigned int StandardID = 0, i;
    unsigned int ExtendedID = 0;
    unsigned char dataLenth = 0;

    temp_1 = &Keep_Link.dataBuf[3];

    switch (*temp_1)
    {
        case 0x61:
            temp_1++;

            switch (*temp_1)
            {
                case 0x01:
                case 0x02:
                case 0x03:
                    temp_1++;
                    dataLenth = (*temp_1) & 0x0f;
                    SC_TagISO15765.m_chHoldDig.DLC = dataLenth;

                    if (* (temp_1) & 0x40)
                    {
                        SC_TagISO15765.m_chHoldDig.RTR = CAN_RTR_Remote; //远程帧
                    }
                    else
                    {
                        SC_TagISO15765.m_chHoldDig.RTR = CAN_RTR_Data; //数据帧
                    }

                    if ((* (temp_1++)) & 0x80) //判断帧头
                    {
                        SC_TagISO15765.m_chHoldDig.IDE = CAN_Id_Extended; // 扩展帧格式
                        ExtendedID += (* (temp_1++)) * 0x1000000;
                        ExtendedID += (* (temp_1++)) * 0x10000;
                        ExtendedID += (* (temp_1++)) * 0x100;
                        ExtendedID += * (temp_1++);
                        SC_TagISO15765.m_chHoldDig.ExtId = ExtendedID >> 3;
                    }
                    else
                    {
                        SC_TagISO15765.m_chHoldDig.IDE = CAN_Id_Standard; // 标准帧格式
                        StandardID += * (temp_1++) * 0x100;
                        StandardID += * (temp_1++);
                        SC_TagISO15765.m_chHoldDig.StdId = StandardID >> 5;
                    }

                    i = 0;

                    while (dataLenth--)
                    {
                        //有效数据
                        SC_TagISO15765.m_chHoldDig.Data[i++] = * (temp_1++);
                    }

                    break;

                default:
                    break;
            }

        default:
            break;
    }
}


//------------------------------------------------------------------------------
// Funtion: 普通协议链路保持中断处理函数
// Input  : 无
// Output : 无
// Return : void
// Info   :
//------------------------------------------------------------------------------
void SC_setlinkkeep(void)
{
    Keep_Link.timeCount++;

    if (TOOL_SENDING == Keep_Link.rightState) //发送状态
    {
        //字节间距满
        if (Keep_Link.timeCount >= (SC_TagKWP2000.m_nBtyetime / 5000))
        {
            Keep_Link.timeCount = 0;

            if (0 == Keep_Link.idleSendLen)
            {
                return;
            }

            // 发送完毕
            if (Keep_Link.chCount >= Keep_Link.idleSendLen)
            {
                Keep_Link.rightState = TOOL_WAITING;
                Keep_Link.timeCount = 0;
                return;
            }

            // 发送一个字节
            ComSendByte(Keep_Link.dataBuf[Keep_Link.chCount]);
            Keep_Link.chCount++;
        }
    }
    else if (TOOL_WAITING == Keep_Link.rightState) //等待状态
    {
        if ((Keep_Link.timeCount > (int) (Keep_Link.IdleTime / 5)) && Keep_Link.IdleTime) // 握手等待时间满
        {
            Keep_Link.linkState = LINK_OPEN;
            Keep_Link.timeCount = 0;
            Keep_Link.chCount = 0;

            //发送第一字节
            ComSendByte(Keep_Link.dataBuf[Keep_Link.chCount]);
            Keep_Link.chCount++;
            Keep_Link.rightState = TOOL_SENDING; //发送数据更新就绪
        }
    }
}


//------------------------------------------------------------------------------
// Funtion: 使能L线
// Return : 无
// Info   :
//------------------------------------------------------------------------------
void EnableLLine_1(void)
{
    SC_RelayState.LLine = 1;
    CMPHY_Relay_Set(&SC_RelayState);
}


//------------------------------------------------------------------------------
// Funtion: 禁能L线
// Return : 无
// Info   :
//------------------------------------------------------------------------------
void DisableLLine_1(void)
{
    SC_RelayState.LLine = 0;
    CMPHY_Relay_Set(&SC_RelayState);
}


//------------------------------------------------------------------------------
// Funtion: 检查L线是否打开
// Return : L线打开返回1，否则返回0
// Info   :
//------------------------------------------------------------------------------
unsigned char CheckLLine(void)
{
    return SC_RelayState.LLine;
}


//------------------------------------------------------------------------------
// Funtion: KWP1281 定时器中断
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 600d
//------------------------------------------------------------------------------
void SC_CML_KWP1281_Time(void)
{
    SC_TagKWP1281.m_nMScount++;

    // 发送链路保持 或者 功能命令专用
    if (TOOL_SENDING == Keep_Link.rightState) //发送状态
    {
        //字节间距满， 两次发送字节的间隔为4ms
        if (SC_TagKWP1281.m_chCount == 0)
        {
            if (SC_TagKWP1281.m_nMScount > SC_TagKWP1281.m_nFrameTime)
            {
                // 发送一个字节
                if (SC_TagKWP1281.m_Senddata)
                {
                    ComReadSign(false);
                    SC_TagKWP1281.m_nMScount = 0;

                    if (SC_TagKWP1281.m_chCount == 1) //命令计数器
                    {
                        ComSendByteInIsr(SC_TagKWP1281.m_Framecount);
                    }
                    else
                    {
                        ComSendByteInIsr(SC_TagKWP1281.m_Senddata[SC_TagKWP1281.m_chCount]);
                    }

                    SC_TagKWP1281.m_chCount++;
                    SC_TagKWP1281.m_SendFinishFlag = 0;
                }
            }
        }
        else
        {
            if (SC_TagKWP1281.m_nMScount >= SC_TagKWP1281.m_nBtyetime)
            {
                // 发送一个字节
                if (SC_TagKWP1281.m_Senddata)
                {
                    if (SC_TagKWP1281.m_SendFinishFlag == 1)
                    {
                        ComReadSign(false);
                        SC_TagKWP1281.m_nMScount = 0;

                        if (SC_TagKWP1281.m_chCount == 1) //命令计数器
                        {
                            ComSendByteInIsr(SC_TagKWP1281.m_Framecount);
                        }
                        else
                        {
                            ComSendByteInIsr(SC_TagKWP1281.m_Senddata[SC_TagKWP1281.m_chCount]);
                        }

                        SC_TagKWP1281.m_chCount++;
                        SC_TagKWP1281.m_SendFinishFlag = 0;
                    }
                }
            }
        }
    }

    // 将ECU发过来的字节取反后发回专用
    else if (TOOL_RECEREVERSE == Keep_Link.rightState)
    {
        //字节间距满
        if (SC_TagKWP1281.m_nMScount >= SC_TagKWP1281.m_ReserveTime) //if( SC_TagKWP1281.m_nMScount >= ( SC_TagKWP1281.m_ReserveTime) / 2 )
        {
            if (SC_TagKWP1281.m_SendFinishFlag == 1) //防止低波特率时，一个字节在一个定时器时间单位内没有发完的情况发生。
            {
                // 发送取反后的字节
                SC_TagKWP1281.m_nMScount = 0;
                ComReadSign(false);
                ComSendByteInIsr(255 - SC_TagKWP1281.m_Reverse);
                SC_TagKWP1281.m_SendFinishFlag = 0;
            }
        }
    }
    else if (TOOL_WAITING == Keep_Link.rightState) //等待状态
    {
        if ((SC_TagKWP1281.m_nMScount > SC_TagKWP1281.m_Idletime) && SC_TagKWP1281.m_Idletime) // 握手等待时间满
        {
            ComReadSign(false);
            SC_TagKWP1281.m_nMScount = 0;
            SC_TagKWP1281.m_chCount = 0;

            //发送第一字节
            SC_TagKWP1281.m_Senddata = SC_TagKWP1281.m_chHoldDig;
            SC_TagKWP1281.m_Lenth = SC_TagKWP1281.m_chHoldDataLen; //链路保持字节长度

            if (SC_TagKWP1281.m_Lenth)
            {
                ComSendByteInIsr(SC_TagKWP1281.m_Senddata[SC_TagKWP1281.m_chCount]);
                SC_TagKWP1281.m_chCount++;
                Keep_Link.rightState = TOOL_SENDING; //发送数据更新就绪
                SC_TagKWP1281.m_SendFinishFlag = 0;
            }

            //ComReadSign( true );
        }
    }
}


//------------------------------------------------------------------------------
// Funtion: KWP1281 协议串口发送中断函数,串口发送完一个字节后产生的中断
// Input  : none
// Output : none
// Return : none
// Info   : 该函数仅对内使用
//------------------------------------------------------------------------------
void SC_CML_KWP1281_Comsend(void)
{
    // 清 0 计数器
    // 确认为 发送状态
    if (TOOL_SENDING == Keep_Link.rightState)
    {
        // 发送完毕
        if (SC_TagKWP1281.m_chCount == SC_TagKWP1281.m_Lenth)
        {
            SC_TagKWP1281.m_Senddata = 0;
            SC_TagKWP1281.m_chCount = 0;
            Keep_Link.rightState = TOOL_READING;
        }
        else
        {
            Keep_Link.rightState = TOOL_SENDREVERSE;
            TimerReset(TIMER0);
        }
    }

    // 接收取反时进入
    else if (TOOL_RECEREVERSE == Keep_Link.rightState)
    {
        Keep_Link.rightState = TOOL_READING;
    }

    SC_TagKWP1281.m_SendFinishFlag = 1;
    SC_TagKWP1281.m_nMScount = 0;
    ComReadSign(true);
}


//------------------------------------------------------------------------------
// Funtion: KWP1281 协议串口接收中断函数
// Input  : none
// Output : none
// Return : none
// Info   : 该函数仅对内使用
//------------------------------------------------------------------------------
void SC_CML_KWP1281_Comread(void)
{
    // 清 0 计数器
    SC_TagKWP1281.m_nMScount = 0;

    //SC_TagKWP1281.m_chCount++;
    // 确认为 接收状态
    if (TOOL_READING == Keep_Link.rightState)
    {
        //处于接收状态，收到的数据将在定时器中断取反发送
        SC_TagKWP1281.m_Reverse = ReadByteInIsr();
        SC_TagKWP1281.m_chTemp[SC_TagKWP1281.m_chCount] = SC_TagKWP1281.m_Reverse;
        SC_TagKWP1281.m_chCount++;

        if (SC_TagKWP1281.m_chCount == 1) // ECU回复的第一个字节
        {
            SC_TagKWP1281.m_Lenth = SC_TagKWP1281.m_Reverse + 1; //计算帧长度
        }

        if (SC_TagKWP1281.m_chCount == 2) //ECU回复的第二个字节，命令计数器
        {
            SC_TagKWP1281.m_Framecount = SC_TagKWP1281.m_Reverse + 1; //保存命令计数器的数值
        }

        if (SC_TagKWP1281.m_chCount == SC_TagKWP1281.m_Lenth) //接收完毕
        {
            //ComSendByteInIsr( 255 - SC_TagKWP1281.m_Reverse );
            //ComReadSign( true );
            //            if((SC_TagKWP1281.m_chTemp[0] = 0x03) && (SC_TagKWP1281.m_chTemp[2] == 0x09))
            Keep_Link.rightState = TOOL_WAITING;

            //            else
            //                SC_TagKWP1281.m_chState = TOOL_WAITSEND;
            //SC_TagKWP1281.m_chTemp[ SC_TagKWP1281.m_chCount ] = 0x03;
            SC_TagKWP1281.m_chCount = 0;
        }
        else
        {
            Keep_Link.rightState = TOOL_RECEREVERSE;
        }
    }

    // 发送取反时进入
    else if (TOOL_SENDREVERSE == Keep_Link.rightState)
    {
        Keep_Link.rightState = TOOL_SENDING;
    }

    TimerReset(TIMER0);
    SC_TagKWP1281.m_nMScount = 0;
}


//recmode = 0 : normal --- 80, 8x, 0x
//          1 : C0
//          2 : 00 xx(>80)
unsigned char ReceiveOneKwpFrameFromECUHasMode(unsigned char recmode, unsigned char * buff, unsigned int * size, unsigned int overtime)
{
    unsigned char rcvbyte, mark;
    unsigned int datalen, rcvlen = 0;

    //Get First Byte
    (*size) = 0;
    datalen = 10;

    do
    {
        if (ComByte(&rcvbyte, overtime) == FALSE)
        {
            return FALSE;
        }

        if (recmode == 1)
        {
            if (rcvbyte == 0xC0)
            {
                break;
            }
        }
        else if (recmode == 2)
        {
            if (rcvbyte == 0)
            {
                break;
            }
        }
        else
        {
            if (rcvbyte != 0)
            {
                break;
            }
        }
    }
    while(--datalen);

    if (datalen == 0)
    {
        return FALSE;
    }

    //Get Data Length
    * (buff++) = rcvbyte;
    (*size) ++;
    mark = rcvbyte;

    switch (recmode)
    {
        case 1:
            if (mark == 0xC0)
            {
                rcvlen = 3;

                do
                {
                    if (ComByte(&rcvbyte, 300) == FALSE)
                    {
                        return FALSE;
                    }

                    * (buff++) = rcvbyte;
                    (*size) ++;
                }
                while(--rcvlen);

                datalen = rcvbyte + 1;
            }

            break;

        case 2:
            if (mark == 0x00)
            {
                rcvlen = 10;

                do
                {
                    if (ComByte(&rcvbyte, 300) == FALSE)
                    {
                        return FALSE;
                    }

                    if (rcvbyte != 0)
                    {
                        * (buff++) = rcvbyte;
                        (*size) ++;
                        break;
                    }
                }
                while(--rcvlen);

                if (rcvlen == 0)
                {
                    return FALSE;
                }

                datalen = rcvbyte + 1;
            }

            break;

        default:
            if (mark == 0x80)
            {
                rcvlen = 3;

                do
                {
                    if (ComByte(&rcvbyte, 300) == FALSE)
                    {
                        return FALSE;
                    }

                    * (buff++) = rcvbyte;
                    (*size) ++;
                }
                while(--rcvlen);

                datalen = rcvbyte + 1;
            }
            else if (mark > 0x80)
            {
                datalen = (mark & 0x3f) + 4 - 1;
            }
            else
            {
                datalen = (mark & 0x3f) + 2 - 1;
            }

            break;
    }

    //Get Data
    while (datalen--)
    {
        if (ComByte(&rcvbyte, 300) == FALSE)
        {
            return FALSE;
        }

        * (buff++) = rcvbyte;
        (*size) ++;
    }

    return TRUE;
}


unsigned char GetKwpFrameBusyFlag(unsigned char * buff, unsigned char * busyid)
{
    switch (buff[0])
    {
        case 0x00:
            busyid[0] = buff[2];
            busyid[1] = buff[4];
            break;

        case 0x80:
            busyid[0] = buff[4];
            busyid[1] = buff[6];
            break;

        default:
            if (buff[0] > 0x80)
            {
                busyid[0] = buff[3];
                busyid[1] = buff[5];
            }
            else
            {
                busyid[0] = buff[1];
                busyid[1] = buff[3];
            }

            break;
    }

    return TRUE;
}


//------------------------------------------------------------------------------
// Funtion: KWP1281 发送一个数据包
// Input  : PakSend - 待发送数据
// Output : none
// Return : 发送总线仲裁失败时返回 false
// Info   : none
//------------------------------------------------------------------------------
int SC_CML_KWP1281_Send(SC_PAKSEND * PakSend)
{
    while (TOOL_WAITING != Keep_Link.rightState) //选择在收到数据之后 到 发送链路第一个字节之前，这之间的时间点发送数据。
    {
        if (SC_TagKWP1281.m_nMScount > 2500)
        {
            return FALSE; // 接收字节等待超时
        }
    } //跳出while 循环即为发送完毕

    set_time0Stop();

    //
    //    // 仲裁时间
    //    if( TRUE == ComByte( &temp_1, SC_TagKWP1281.m_nFrameTime - 2 ) )
    //    {
    //        // 仲裁失败
    //        return FALSE;
    //    }
    //
    // 构造待发送数据包
    if (NULL != PakSend)
    {
        SC_TagKWP1281.m_Lenth = PakSend->PakLenth;
        SC_TagKWP1281.m_Senddata = PakSend->Pakdata;
    }
    else
    {
        return true;
    }

    // 设置为发送状态由定时器发出
    SC_TagKWP1281.m_chCount = 0;
    SC_TagKWP1281.m_nMScount = 0;
    Keep_Link.rightState = TOOL_SENDING;

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    while ((TOOL_SENDING == Keep_Link.rightState) || (Keep_Link.rightState == TOOL_SENDREVERSE))
    {
        if (SC_TagKWP1281.m_nMScount > 2500)
        {
            return FALSE;
        }
    }

    return true;
}


//------------------------------------------------------------------------------
// Funtion: KWP1281 接收一个数据包
// Input  :
// Output : Lenth - 数据长度
//          Data - 数据
// Return : 应答码
// Info   : 当返回 Lenth 为
//------------------------------------------------------------------------------
int SC_CML_KWP1281_Recv(SC_PAKRECV * Pakrecv)
{
    unsigned char i;

    while (TOOL_WAITING != Keep_Link.rightState) //||(TOOL_RECEREVERSE == SC_TagKWP1281.m_chState ))
    {
        if (SC_TagKWP1281.m_nMScount > 2500)
        {
            return FALSE;
        }
    }

    set_time0Stop();

    // 解包过程
    Pakrecv->PakLenth = SC_TagKWP1281.m_Lenth;

    for (i = 0; i < Pakrecv->PakLenth; i++)
    {
        Pakrecv->Pakdata[i] = SC_TagKWP1281.m_chTemp[i];
    }

    SC_TagKWP1281.m_nMScount = 0;
    Keep_Link.rightState = TOOL_WAITING;

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    return 1;
}


//------------------------------------------------------------------------------
// Funtion: 释放动态申请的缓冲区
// Input  : 参数1：PAKRECV结构体指针，参数2：num结构体数量
// Output : none
// Return : 无
// Info   :
//------------------------------------------------------------------------------
void freeBuffer(SC_PAKRECV * PakRecv, unsigned int num)
{
    int i = 0;

    for (i = 0; i < num; i++)
    {
        free((void *) ((&PakRecv[i])->Pakdata));
    }
}


//------------------------------------------------------------------------------
// Funtion: 计算ISO91941里面的延时
// Input  : 参数1：A B 延时参数
// Output : none
// Return : 无
// Info   :
//------------------------------------------------------------------------------
unsigned int CalculateTimeOut(unsigned int A, unsigned int B)
{
    if (B == 0)
    {
        return 250;
    }

    if (A / B == 0) //小于10
    {
        return 10;
    }

    if (A / B >= 25) //大于250
    {
        return 250;
    }

    if (A % B != 0)
    {
        return ((A / B) * 10 + 5);
    }

    return ((A / B) * 10);
}


//------------------------------------------------------------------------------
// Funtion: 采用模拟方式发送5bps地址
// Input  : 参数1：PAKRECV结构体指针，参数2：num结构体数量
// Output : none
// Return : 无
// Info   :
//------------------------------------------------------------------------------
void sendAddressToEcu(unsigned char fiveBpsAddr)
{
    ComClose(COM_ECUPORT);
    IOCTRLSet(IOCTRL_HIGH, 100000);

    // 起始位
    IOCTRLSet(IOCTRL_LOW, 200000);

    // 8个数据位
    if ((fiveBpsAddr & 0x01) != 0)
    {
        IOCTRLSet(IOCTRL_HIGH, 200000);
    }
    else
    {
        IOCTRLSet(IOCTRL_LOW, 200000);
    }

    if ((fiveBpsAddr & 0x02) != 0)
    {
        IOCTRLSet(IOCTRL_HIGH, 200000);
    }
    else
    {
        IOCTRLSet(IOCTRL_LOW, 200000);
    }

    if ((fiveBpsAddr & 0x04) != 0)
    {
        IOCTRLSet(IOCTRL_HIGH, 200000);
    }
    else
    {
        IOCTRLSet(IOCTRL_LOW, 200000);
    }

    if ((fiveBpsAddr & 0x08) != 0)
    {
        IOCTRLSet(IOCTRL_HIGH, 200000);
    }
    else
    {
        IOCTRLSet(IOCTRL_LOW, 200000);
    }

    if ((fiveBpsAddr & 0x10) != 0)
    {
        IOCTRLSet(IOCTRL_HIGH, 200000);
    }
    else
    {
        IOCTRLSet(IOCTRL_LOW, 200000);
    }

    if ((fiveBpsAddr & 0x20) != 0)
    {
        IOCTRLSet(IOCTRL_HIGH, 200000);
    }
    else
    {
        IOCTRLSet(IOCTRL_LOW, 200000);
    }

    if ((fiveBpsAddr & 0x40) != 0)
    {
        IOCTRLSet(IOCTRL_HIGH, 200000);
    }
    else
    {
        IOCTRLSet(IOCTRL_LOW, 200000);
    }

    if ((fiveBpsAddr & 0x80) != 0)
    {
        IOCTRLSet(IOCTRL_HIGH, 200000);
    }
    else
    {
        IOCTRLSet(IOCTRL_LOW, 200000);
    }

    // 结束位
    IOCTRLSet(IOCTRL_HIGH, 1);
}


//-----------------------------------------------------------------------------
//  自动计算波特率
//  bosch协议中 发送5 bps地址码后根据ECU回复的第一个字节计算通信波特率
//------------------------------------------------------------------------------
int32_t autoCheckBaudRate(void)
{
    return get_auto_baud();
}


//------------------------------------------------------------------------------
// Funtion: 采用模拟方式发送任意波特率的地址码
// Input  : 参数1：地址码   参数2：波特率
// Output : none
// Return : 无
// Info   :
//------------------------------------------------------------------------------
void sendAddressCodeToEcuOnAnyBaudrate(unsigned char fiveBpsAddr,
    unsigned int baudrate)
{
    unsigned int time = 0;

    ComClose(COM_ECUPORT);
    time = (unsigned int) (1000000 / baudrate);
    IOCTRLSet(IOCTRL_HIGH, 100);

    // 起始位
    IOCTRLSet(IOCTRL_LOW, time);

    // 8个数据位
    if ((fiveBpsAddr & 0x01) != 0)
    {
        IOCTRLSet(IOCTRL_HIGH, time);
    }
    else
    {
        IOCTRLSet(IOCTRL_LOW, time);
    }

    if ((fiveBpsAddr & 0x02) != 0)
    {
        IOCTRLSet(IOCTRL_HIGH, time);
    }
    else
    {
        IOCTRLSet(IOCTRL_LOW, time);
    }

    if ((fiveBpsAddr & 0x04) != 0)
    {
        IOCTRLSet(IOCTRL_HIGH, time);
    }
    else
    {
        IOCTRLSet(IOCTRL_LOW, time);
    }

    if ((fiveBpsAddr & 0x08) != 0)
    {
        IOCTRLSet(IOCTRL_HIGH, time);
    }
    else
    {
        IOCTRLSet(IOCTRL_LOW, time);
    }

    if ((fiveBpsAddr & 0x10) != 0)
    {
        IOCTRLSet(IOCTRL_HIGH, time);
    }
    else
    {
        IOCTRLSet(IOCTRL_LOW, time);
    }

    if ((fiveBpsAddr & 0x20) != 0)
    {
        IOCTRLSet(IOCTRL_HIGH, time);
    }
    else
    {
        IOCTRLSet(IOCTRL_LOW, time);
    }

    if ((fiveBpsAddr & 0x40) != 0)
    {
        IOCTRLSet(IOCTRL_HIGH, time);
    }
    else
    {
        IOCTRLSet(IOCTRL_LOW, time);
    }

    if ((fiveBpsAddr & 0x80) != 0)
    {
        IOCTRLSet(IOCTRL_HIGH, time);
    }
    else
    {
        IOCTRLSet(IOCTRL_LOW, time);
    }

    // 结束位
    IOCTRLSet(IOCTRL_HIGH, 1);
}


//------------------------------------------------------------------------------
// Funtion: ISO14230 发送一个数据包
// Input  : PakSend - 待发送数据
// Output : none
// Return : 发送总线仲裁失败时返回 false
// Info   : 查询方式发送，发送时需要停止当前所有中断
//------------------------------------------------------------------------------
int SC_CML_ISO14230_Send(SC_PAKSEND * PakSend)
{
    unsigned char temp_1;
    int Paklen, i;

    while (TOOL_SENDING == SC_TagKWP2000.m_chState) // 等待链路维持发送完毕
    {
        if (SC_TagKWP2000.m_nMScount >= (SC_TagKWP2000.m_Idletime / 5))
        {
            return FALSE;
        }
    }

    SC_TagKWP2000.m_nMScount = 0;

    // 构造待发送数据包
    if (NULL != PakSend)
    {
        Paklen = PakSend->PakLenth;

        for (i = 0; i < Paklen; i++)
        {
            SC_TagKWP2000.m_chTemp[i] = PakSend->Pakdata[i];
        }
    }
    else
    {
        SC_TagKWP2000.m_chState = CMLISO14230_SENDFLA;
        return true;
    }

    if (SC_TagKWP2000.m_chCount) // 链路维持等待返回时间
    {
        if (SC_TagKWP2000.m_nMScount < (SC_TagKWP2000.m_Idletime / 2))
        {
            delay(SC_TagKWP2000.m_Idletime / 2);
        }

        SC_TagKWP2000.m_chCount = 0;
    }

    SC_TagKWP2000.m_nMScount = 0;

    // 定时器停止
    if (SC_TagKWP2000.m_Idletime)
    {
        set_time0Stop();
    }

    // 仲裁时间
    i = 0;

    while (true == ComByte(&temp_1, SC_TagKWP2000.m_nFrameTime))
    {
        if (i++ > 100)
        {
            SC_TagKWP2000.m_chState = CMLISO14230_SENDFLA;

            // 仲裁失败
            return FALSE;
        }
    }

    // 发送数据包
    ComSendByte(SC_TagKWP2000.m_chTemp[0]);
    i = 1;

    while (i < Paklen)
    {
        delayus(SC_TagKWP2000.m_nBtyetime);
        ComSendByte(SC_TagKWP2000.m_chTemp[i]);
        i++;
    }

    // 定时器启动
    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    SC_TagKWP2000.m_chState = CMLISO14230_SENDFLA;
    return true;
}


//------------------------------------------------------------------------------
// Funtion: ISO14230 接收一个数据包
// Input  :
// Output : PakRecv - 接收到的数据包
// Return : 应答码
// Info   : 添加整帧模式，返回仅为 TRUE 和 FALSE
//------------------------------------------------------------------------------
int SC_CML_ISO14230_Recv(SC_PAKRECV * PakRecv)
{
    unsigned int iCount, i;
    unsigned char PakType;
    unsigned int Time;

    if (0 == PakRecv)
    {
        return FALSE;
    }

    if (SC_TagKWP2000.m_chCount) // 链路维持等待返回时间
    {
        delay(SC_TagKWP2000.m_Idletime / 2);
        SC_TagKWP2000.m_chCount = 0;
    }

    PakRecv->PakLenth = 0;

    if (SC_TagKWP2000.m_chState == CMLISO14230_SENDFLA)
    {
        Time = SC_TagKWP2000.m_Maxwaittime;
    }
    else
    {
        Time = SC_TagKWP2000.m_VehFrametime;
    }

    SC_TagKWP2000.m_chState = CMLISO14230_READING;

    // 定时器停止
    if (SC_TagKWP2000.m_Idletime)
    {
        set_time0Stop();
    }

    // 接收一个包前 3 个字节
    i = 0;

    while (i < 3)
    {
        if (FALSE == ComByte(&SC_TagKWP2000.m_chTemp[i], Time))
        {
            SC_TagKWP2000.m_chState = CMLISO14230_WAITING;

            // 定时器启动
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            return FALSE;
        }

        i++;
    }

    // 计算数据包长度 ------ 协议数据包格式产生改变收发方式不变,需要改变部分 --
    PakType = SC_TagKWP2000.m_chTemp[0] &0xc0; //数据包类型

    if (CMPISO14230_FMT_RESERVE == PakType)
    {
        SC_TagKWP2000.m_chState = CMLISO14230_WAITING;
        return FALSE;
    }
    else if (CMPISO14230_FMT_ADR_LEN == PakType)
    {
        if (SC_TagKWP2000.m_chTemp[0] &0x3f)
        {
            iCount = (SC_TagKWP2000.m_chTemp[0] &0x3f) + 4;
        }
        else
        {
            if (FALSE == ComByte(&SC_TagKWP2000.m_chTemp[i], Time))
            {
                SC_TagKWP2000.m_chState = CMLISO14230_WAITING;
                return FALSE;
            }

            i++;
            iCount = SC_TagKWP2000.m_chTemp[3] +5;
        }
    }
    else if (CMPISO14230_FMT_ADR_LEN_FUN == PakType)
    {
        if (SC_TagKWP2000.m_chTemp[0] &0x3f)
        {
            iCount = (SC_TagKWP2000.m_chTemp[0] &0x3f) + 4;
        }
        else
        {
            if (FALSE == ComByte(&SC_TagKWP2000.m_chTemp[i], Time))
            {
                SC_TagKWP2000.m_chState = CMLISO14230_WAITING;
                return FALSE;
            }

            i++;
            iCount = SC_TagKWP2000.m_chTemp[3] +5;
        }
    }
    else if (CMPISO14230_FMT_LEN == PakType)
    {
        if (SC_TagKWP2000.m_chTemp[0] &0x3f)
        {
            iCount = (SC_TagKWP2000.m_chTemp[0] &0x3f) + 2;
        }
        else
        {
            iCount = SC_TagKWP2000.m_chTemp[3] +3;
        }
    }
    else
    {
    }

    // 计算数据包长度 ------ 协议数据包格式产生改变收发方式不变,需要改变部分 --
    // 接收剩余字节
    while (i < iCount)
    {
        if (FALSE == ComByte(&SC_TagKWP2000.m_chTemp[i], Time))
        {
            SC_TagKWP2000.m_chState = CMLISO14230_WAITING;

            // 定时器启动
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            return FALSE;
        }

        i++;
    }

    PakRecv->PakLenth = i;

    for (i = 0; i < PakRecv->PakLenth; i++)
    {
        PakRecv->Pakdata[i] = SC_TagKWP2000.m_chTemp[i];
    }

    // 根据需求移位
    SC_TagKWP2000.m_chState = CMLISO14230_WAITING;

    // 定时器启动
    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    return 1;
}


//------------------------------------------------------------------------------
// Funtion: ISO15765 接收一个数据包
// Input  :
// Output : PakRecv - 接收的数据
// Return : 数据包属性和格式
// Info   : ( FUN_KLine.c - OneToOne_SendDataToEcuGetAnswer_1 )
//------------------------------------------------------------------------------
unsigned char SC_CML_ISO15765_Recv(unsigned char * ans, unsigned char rcvlen)
{
    unsigned char i, checkXOR = 0;
    unsigned int Time, ansPos = 0;
    CAN_RXMessage CanRcvMsg;

    Time = SC_TagKWP2000.m_Maxwaittime;

    // 定时器停止
    if (Keep_Link.IdleTime)
    {
        set_time0Stop();
    }

    ans[ansPos++] = 0x55;
    ans[ansPos++] = 0xaa;

    if (FALSE == CANRecv(&CanRcvMsg, Time))
    {
        return FALSE;
    }
    else
    {
        if (CanRcvMsg.IDE == CAN_Id_Extended) // 帧格式
        {
            ans[ansPos++] = SC_TagISO15765.m_RXM.DLC + 5;
            ans[ansPos++] = 0x80 | SC_TagISO15765.m_RXM.DLC;
            CanRcvMsg.ExtId = CanRcvMsg.ExtId << 3;
            ans[ansPos++] = (unsigned char) ((CanRcvMsg.ExtId & 0xff000000) >> 24);
            ans[ansPos++] = (unsigned char) ((CanRcvMsg.ExtId & 0x00ff0000) >> 16);
            ans[ansPos++] = (unsigned char) ((CanRcvMsg.ExtId & 0x0000ff00) >> 8);
            ans[ansPos++] = (unsigned char) (CanRcvMsg.ExtId & 0x000000ff);

            for (i = 0; i < CanRcvMsg.DLC; i++)
            {
                ans[ansPos++] = CanRcvMsg.Data[i];
            }
        }
        else
        {
            ans[ansPos++] = CanRcvMsg.DLC + 3;
            ans[ansPos++] = CanRcvMsg.DLC;
            CanRcvMsg.StdId = CanRcvMsg.StdId << 5;
            ans[ansPos++] = (unsigned char) ((CanRcvMsg.StdId & 0x0000ff00) >> 8);
            ans[ansPos++] = (unsigned char) (CanRcvMsg.StdId & 0x000000ff);

            for (i = 0; i < CanRcvMsg.DLC; i++)
            {
                ans[ansPos++] = CanRcvMsg.Data[i];
            }
        }
    }

    for (i = 0; i <= ans[2]; i++)
    {
        checkXOR ^= ans[i + 2];
    }

    ans[ansPos++] = checkXOR;

    // 定时器启动
    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    //    if(rcvlen > ansPos)
    //        return FALSE;
    return ansPos;
}


//------------------------------------------------------------------------------
// Funtion: 设置can通信模式：1、单线can，2、双线can。 设置can通信过滤ID
// Input  : 无
// Output : 无
// Return : void
// Info   :
//------------------------------------------------------------------------------
void SetCANFilter(unsigned char * dataPos)
{
    //unsigned char pMaskArray[8] = {0x01,0x03,0x07,0x0f,0x1f,0x3f,0x7f,0xff};
    unsigned char i;

    //设置can通信模式为单线can或双线can
    switch (SC_RelayState.CommType)
    {
        case 0x0:
        case 0x1: //双线can
            SC_RelayState.CommType = COMM_DOUBLE_CAN_TYPE;
            CAN_SetPara.CanPort = CAN_PORTDOUBLE;
            break;

        case 0x2: //单线can
            SC_RelayState.CommType = COMM_SINGLE_CAN_TYPE;
            CAN_SetPara.CanPort = CAN_PORTSIGNAL;
            break;

        default:
            SC_RelayState.CommType = COMM_DOUBLE_CAN_TYPE;
            CAN_SetPara.CanPort = CAN_PORTDOUBLE;
            break;
    }

    CMPHY_Relay_Set(&SC_RelayState);

    //设置can通信过滤ID
    if ((dataPos[0] != 0x00) && (dataPos[0] != 0xff))
    {
        //Send_ErrAck();
        return;
    }

    switch (dataPos[1])
    {
        //禁止接收
        case 0x00:
            break;

        //全部过滤
        case 0xff:
            break;

        //掩码
        case 0xfd:
            if (dataPos[0] == 0x00)
            {
            }
            else
            {
            }

            break;

        //范围
        case 0xfe:
            if (dataPos[0] == 0x00)
            {
            }
            else
            {
            }

            break;

        //ID
        default:
            //ff,01,c6,d7,88,80,8e
            CAN_SetPara.FilterNum = 0;

            for (i = 0; i < dataPos[1]; i++)
            {
                //标准帧
                if (dataPos[0] == 0x00)
                {
                    //   pCanFilterIDArray[i] = ((dataPos[2+i*2]*0x0100+dataPos[3+i*2])>>5)&0x07ff;
                    CAN_SetPara.ECUID[CAN_SetPara.FilterNum] = ((dataPos[2 + i * 2] *0x0100 + dataPos[3 + i * 2]) >> 5) & 0x07ff;
                    CAN_SetPara.FilterNum++;
                    PHY_CAN_SetPara.Mode = CAN_NORMALFRAME;
                }

                //扩展帧
                else
                {
                    // pCanFilterIDArray[i] = ((dataPos[2+i*4]*0x01000000+dataPos[3+i*4]*0x010000+dataPos[4+i*4]*0x0100+dataPos[5+i*4])>>3)&0x1fffffff;
                    CAN_SetPara.ECUID[CAN_SetPara.FilterNum] =
                         ((dataPos[2 + i * 4] *0x01000000 + dataPos[3 + i * 4] *0x010000 + dataPos[4 + i * 4] *0x0100 + dataPos[5 + i * 4]) >> 3) & 0x1fffffff;
                    CAN_SetPara.FilterNum++;
                    PHY_CAN_SetPara.Mode = CAN_EXTERNFRAME;
                }
            }

            break;
    }

    // 初始化 CAN 管脚
    CANInit();
    PHY_CAN_SetPara.FilterNum = CAN_SetPara.FilterNum;
    PHY_CAN_SetPara.FilterID = CAN_SetPara.ECUID;
    CANOpen(CAN_SetPara.CanPort, &PHY_CAN_SetPara);
}


//------------------------------------------------------------------------------
// Funtion: 设置canbus通信引脚
// Input  : 无
// Output : 无
// Return : void
// Info   :
//------------------------------------------------------------------------------
void SetCanCommunicatioIO(unsigned char * dataPos)
{
    SC_RelayState.CommType = COMM_DOUBLE_CAN_TYPE;

    if (dataPos[0] == 0x00)
    {
        SC_RelayState.CANL = 6;
        SC_RelayState.CANH = 5;
    }
    else if (dataPos[0] == 0xff)
    {
        SC_RelayState.CANL = 10;
        SC_RelayState.CANH = 9;
    }

    CMPHY_Relay_Set(&SC_RelayState);
}


//------------------------------------------------------------------------------
// Funtion: 将整帧的CANBUS链路维持数据解包放入指定的结构体中
// Input  : 无
// Output : 无
// Return : void
// Info   : Keep_Link.dataBuf 55,aa,0f,61,01,88,c6,d0,87,88,02,3e,80,ff,ff,ff,ff,ff,bd,0d,c8,44
//------------------------------------------------------------------------------
void setCanbusNormalDataToken(unsigned char * dataPos)
{
    unsigned char i = 0, checkXOR = 0, j = 0;
    unsigned int dataLenth = 0;
    unsigned char data[50] =
    {
        0
    };
    dataLenth = dataPos[j]; //有效数据长度
    data[i++] = dataPos[j];
    checkXOR = dataPos[j++]; //开始异或校验

    while (dataLenth--)
    {
        data[i++] = dataPos[j];
        checkXOR ^= dataPos[j++];
    }

    checkXOR ^= dataPos[j++];

    if (checkXOR != 0)
    {
        return;
    }

    switch (data[1])
    {
        case 0x60:
            switch (data[2])
            {
                case 0x01:
                    SetCANFilter(&dataPos[3]);
                    break;

                case 0x02:
                    SetCanBaudRate(&dataPos[4]);
                    break;

                case 0x04:
                    //  SetCanWorkMode(&dataPos[3],0);
                    break;

                case 0x05:
                    SetCanCommunicatioIO(&dataPos[3]);
                    break;

                case 0x06:
                    //          SetCanMaskFilterID(&dataPos[3],(&dataPos[0]-2)/8);
                    break;

                case 0x07:
                    //         SetCanDataFrameReceiveMode(&dataPos[0],0);
                    break;

                case 0x0A:
                    //         SetCanCommunicationMode(&dataPos[0],0);
                    break;

                case 0x0B:
                    //           SetCanCommunicationIO(&dataPos[0],0);
                    break;

                case 0x10:
                    //         GetCanConnectorSoftVersion(&dataPos[0],0);
                    break;

                case 0x11:
                    //          GetSoftWareVersion(&dataPos[0],0);
                    break;

                default:
                    break;
            }

            break;

        case 0x61:
            switch (data[2])
            {
                case 0x01:
                    OneToOne_CanbusSendOneFrameOnly(&dataPos[3]);
                    break;

                default:
                    break;
            }

            break;

        default:
            break;
    }
}


//------------------------------------------------------------------------------
// Funtion: 检测总线是否空闲
// Input  : checkCount:检测的次数，  checkTimeMS:每次检测的时间(单位毫秒)
// Output :
// Return : 总线忙:返回true，否则返回FALSE
// Info   :
//------------------------------------------------------------------------------
unsigned char CheckIoBusy(unsigned char checkCount, unsigned char checkTimeMS)
{
    int i = 0;
    unsigned char rcvByte = 0;

    for (i = 0; i < checkCount; i++)
    {
        if (ComByte(&rcvByte, checkTimeMS + 1) == FALSE)
        {
            return FALSE;
        }
    }

    return true;
}


//------------------------------------------------------------------------------
// Funtion: 设置can通信模波特率
// Input  : 无
// Output : 无
// Return : void
// Info   :
//------------------------------------------------------------------------------
void SetCanBaudRate(unsigned char * buff)
{
    unsigned int CANBaudrate[11] =
    {
        0, 0, CAN_500KBPS, CAN_250KBPS, CAN_125KBPS, CAN_625KBPS, CAN_50KBPS, CAN_25KBPS, CAN_100KBPS, 0, CAN_33KBPS
    };

    if ((buff[0] == 0) || (buff[0] > 0xb))
    {
        return;
    }
    else
    {
        PHY_CAN_SetPara.Baudrate = CANBaudrate[buff[0] -1];
    }

    CANInit();
    CANOpen(CAN_SetPara.CanPort, &PHY_CAN_SetPara);
}


//------------------------------------------------------------------------------
// 功能：发送一帧canbus数据到ECU，不接收回复。
//
//------------------------------------------------------------------------------
void OneToOne_CanbusSendOneFrameOnly(unsigned char * buff)
{
    CAN_TXMessage Canmsg;
    unsigned int head = 0;
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int Temp;

    head = buff[i++];

    if (! (head & 0x80)) //说明是标准帧
    {
        Canmsg.IDE = CAN_Id_Standard; //说明是标准帧
        Temp = buff[i++] *0x100;
        Temp += buff[i++];
        Temp >>= 5;
        Canmsg.StdId = Temp;
        Canmsg.RTR = head & 0x40;
        Canmsg.DLC = head & 0x3f;

        for (j = 0; j < Canmsg.DLC; j++)
        {
            Canmsg.Data[j] = buff[i++];
        }
    }
    else //说明是扩展帧
    {
        Canmsg.IDE = CAN_Id_Extended; //说明是标准帧
        Canmsg.ExtId = buff[i++] *0x1000000;
        Canmsg.ExtId += buff[i++] *0x10000;
        Canmsg.ExtId += buff[i++] *0x100;
        Canmsg.ExtId += buff[i++];
        Canmsg.ExtId >>= 3;
        Canmsg.RTR = head & 0x40;
        Canmsg.DLC = head & 0x3f;

        for (j = 0; j < Canmsg.DLC; j++)
        {
            Canmsg.Data[j] = buff[i++];
        }
    }

    CANSend(&Canmsg);
}


//------------------------------------------------------------------------------
// Funtion: ISO15765 协议定时器中断函数
// Input  : none
// Output : none
// Return : none
// Info   : 该函数仅对内使用
//------------------------------------------------------------------------------
void SC_CML_ISO15765_Time(void)
{
    SC_TagISO15765.m_nMScount++;

    if ((SC_TagISO15765.m_nMScount > (int) (Keep_Link.IdleTime / 10)) && Keep_Link.IdleTime) // 握手等待时间满
    {
        SC_TagISO15765.m_nMScount = 0;

        if (false == CANRecv(&SC_TagISO15765.m_RXM, 1))
        {
            receive_buff[260] ++;
        }
        else
        {
            receive_buff[260] = 0;
        }

        if (receive_buff[260] > 9)
        {
            TimerStop(TIMER0);
        }

        //发送
        CANSend(&SC_TagISO15765.m_chHoldDig);
    }
}


#endif

//--------------------------------------------------------- End Of File --------
