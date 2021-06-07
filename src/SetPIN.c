
#include "SetPIN.h"
#include <string.h>
#include "gdstd.h"
#include "X431toDPU.h"
#include "CommAbstractLayer.h"
extern int logger(char * String);
extern SMARTBOX_CAL s_SmartboxStruct;

/********************************************************************************************************************************************
* 函数名称: char SelectPIN(char ConnectorType,char FuncType,char IsEnable,char InputPin,char OutputPin)                                     *
* 函数功能: 综合设置CANBUSII,TOPIII,SUPER-16,OBD 16接头引脚功能                                                                             *
* 函数参数: char ConnectorType  接头类型:根据Connector_Identify()函数返回值                                                                 *
*           char FuncType   引脚功能类型,如下九种其一                                                                                       *
*                           FUNC_IO||FUNC_CanDouble||FUNC_CanSingle||FUNC_BUSLINE||FUNC_LLINE||FUNC_TDS||FUNC_VOLVO_K||FUNC_DTS ||FUNC_RESET*
*                           为HEAVYDUTY新增FUNC_HEAVYDUTY_J1708||FUNC_HEAVYDUTY_RS232
            char IsEnable   对应功能(DTS,TDS,L线,VOLVO K线,BUS+/-)是否使能: 值为 FUNC_ENABLE 或 FUNC_DISABLE                                *
*           char InputPin   接头接收数据引脚号: 1-15 (电源16,地4,5除外)                                                                     *
*           char OutputPin  接头接收数据引脚号: 1-15 (电源16,地4,5除外)                                                                     *
* 返 回 值:                                                                                                                                 *
*           RT_OK           设置成功                                                                                                        *
*           CHANGE_PLUG     该功能当前接头无法实现,需要更换接头                                                                             *
*           其他--          设置失败                                                                                                         *
* 修改记录: 1. 20080722     JJH                                                                                                             *
*********************************************************************************************************************************************/
char SelectPIN(char ConnectorType, char FuncType, char IsEnable, char InputPin, char OutputPin)
{
    if (InputPin == 4 || InputPin == 5 || InputPin == 16)
    {
        return 0X01;
    }

    if (OutputPin == 4 || OutputPin == 5 || OutputPin == 16)
    {
        return 0X02;
    }

    return DiaGun_SetIOLine(FuncType, IsEnable, InputPin, OutputPin);
}


////ziyingzhu 修改 注释上面的函数，改为下面的函数 2009-7-22 20:28:40
char DiaGun_SetIOLine(char FuncType, unsigned char IsEnable, char InputPin, char OutputPin)
{
    char InputIO = 0, OutputIO = 0;
    char DtsValue = 0;

    if (FuncType == FUNC_DTS)
    {
        switch (InputPin)
        {
            case OBD16_PIN1:
                DtsValue = 0x01;
                break;

            case OBD16_PIN2:
                DtsValue = 0x02;
                break;

            case OBD16_PIN12:
                DtsValue = 0x00;
                break;

            default:
                return 0X51;
        }

        if (IsEnable == FUNC_DISABLE)
        {
            DtsValue = 0XFF;
        }

        if (SetDtsVoltage(DtsValue))
        {
            return 0X52;
        }

        return RT_OK;
    }
    else if (FuncType == FUNC_LLINE)
    {
        if (IsEnable == FUNC_ENABLE)
        {
            EnableLLine(TRUE);
        }
        else
        {
            EnableLLine(FALSE);
        }

        return RT_OK;
    }

    //ziyingzhu 注释 在进入模块之后才设置bus 2009-6-23 14:12:31
    else if (FuncType == FUNC_BUSLINE)
    {
        return RT_OK;
    }
    else if ((FuncType == FUNC_CanDouble) || (FuncType == FUNC_CanSingle) || (FuncType == FUNC_IO) || (FuncType == FUNC_HEAVYDUTY_J1708) ||
         (FuncType == FUNC_HEAVYDUTY_RS232))
    {
        switch (FuncType)
        {
            case FUNC_CanDouble:
                //SelectCommMode(1);
                s_SmartboxStruct.ucCommType = 1;
                break;

            case FUNC_CanSingle:
                //SelectCommMode(2);
                s_SmartboxStruct.ucCommType = 2;
                break;

            case FUNC_IO:
                //SelectCommMode(0);
                s_SmartboxStruct.ucCommType = 0;
                break;

            //yaoming add 20100722 for heavy_duty
            case FUNC_HEAVYDUTY_J1708:
                //SelectCommMode(0);
                s_SmartboxStruct.ucCommType = 3;
                break;

            //yaoming add 20100722 for heavy_duty
            case FUNC_HEAVYDUTY_RS232:
                //SelectCommMode(0);
                s_SmartboxStruct.ucCommType = 4;
                break;
        }

        switch (InputPin)
        {
            case OBD16_PIN1:
                InputIO = 0x04;
                break;

            case OBD16_PIN2:
                InputIO = 0x0B;
                break;

            case OBD16_PIN3:
                InputIO = 0x05;
                break;

            case OBD16_PIN6:
                InputIO = 0x09;
                break;

            case OBD16_PIN7:
                InputIO = 0x01;
                break;

            case OBD16_PIN8:
                InputIO = 0x02;
                break;

            case OBD16_PIN9:
                InputIO = 0x03;
                break;

            case OBD16_PIN10:
                InputIO = 0x0C;
                break;

            case OBD16_PIN11:
                InputIO = 0x06;
                break;

            case OBD16_PIN12:
                InputIO = 0x08;
                break;

            case OBD16_PIN13:
                InputIO = 0x07;
                break;

            case OBD16_PIN14:
                InputIO = 0x0A;
                break;

            case OBD16_PIN15:
                InputIO = 0x0D;
                break;

            default:
                return 0X56;
        }

        if (InputPin == OutputPin)
        {
            OutputIO = InputIO;
        }
        else
        {
            switch (OutputPin)
            {
                case OBD16_PIN1:
                    OutputIO = 0x04;
                    break;

                case OBD16_PIN2:
                    OutputIO = 0x0B;
                    break;

                case OBD16_PIN3:
                    OutputIO = 0x05;
                    break;

                case OBD16_PIN6:
                    OutputIO = 0x09;
                    break;

                case OBD16_PIN7:
                    OutputIO = 0x01;
                    break;

                case OBD16_PIN8:
                    OutputIO = 0x02;
                    break;

                case OBD16_PIN9:
                    OutputIO = 0x03;
                    break;

                case OBD16_PIN10:
                    OutputIO = 0x0C;
                    break;

                case OBD16_PIN11:
                    OutputIO = 0x06;
                    break;

                case OBD16_PIN12:
                    OutputIO = 0x08;
                    break;

                case OBD16_PIN13:
                    OutputIO = 0x07;
                    break;

                case OBD16_PIN14:
                    OutputIO = 0x0A;
                    break;

                case OBD16_PIN15:
                    OutputIO = 0x0D;
                    break;

                default:
                    return 0X57;
            }
        }

        if (SelectIoLine(InputIO, OutputIO))
        {
            return 0X58;
        }
    }

    return RT_OK;
}


/********************************************************************************************************************************************
* 函数名称: BYTE CanSetParameter(BYTE ConnectorType,BYTE SetType,CAN_COM *CanParam)                                                         *
* 函数功能: 设置CAN通讯参数功能:设置过滤ID(包含设置掩码),设置ECU通讯波特率,设置工作模式
                                                         *
*           CanSetParameterALL()函数区别在于单独设置CAN参数                                                                                 *
* 函数参数: BYTE ConnectorType 接头类型:CON_Super16V30 || CON_TOPV30 || CON_DiaGunV50 ||CON_CANV20                                          *
*           BYTE SetType       设置类型:CANSET_FILID   || CANSET_BITRATE|| CANSET_WORKMODE                                                  *
*           CAN_COM *CanParam  设置参数结构                                                                                                 *
* 返 回 值: RT_OK 设置成功,其他-设置失败                                                                                                    *
* 修改记录: 1. 20080807     JJH                                                                                                             *
*********************************************************************************************************************************************/
uint8_t CanSetParameter(uint8_t ConnectorType, uint8_t SetType, CAN_COM * CanParam)
{
    uint8_t CountNum = 0, Temp = 0, FilterNum = 0, bTime = 0;
    uint8_t FilID[50];
    uint8_t MaskID[20];
    uint8_t TempBuf[20];
    uint8_t pSendCmd[256] =
    {
        0x55, 0xAA, 0x00, 0x60, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    uint8_t MaskArray[128];
    int i, lenth = pSendCmd[2];

    memset(FilID, 0, sizeof(FilID));
    memset(MaskID, 0, sizeof(MaskID));
    memset(TempBuf, 0, sizeof(TempBuf));
    memset(MaskArray, 0, sizeof(MaskArray));

    //hejm接头类型从哪里获取？如果基本类型不设置，那么can的初始化在哪里设置？
    if (ConnectorType == CON_BASIC)
    {
        //return 0X61; //hejm暂时不返回，不然can无法初始化
    }

    pSendCmd[4] = SetType;

    switch (SetType)
    {
        //ziyingzhu 修改.波特率的命令格式和CANII相同
        case CANSET_BITRATE: /*********Set BaudRate***********/

            if (CanParam->IsExFrame)
            {
                pSendCmd[5] = 0xFF;
            }
            else
            {
                pSendCmd[5] = 0x00;
            }

            pSendCmd[6] = CanParam->BaudRate;
            pSendCmd[2] = 4;
            break;

        case CANSET_WORKMODE: /*********Set EnableListen*******/

            if (CanParam->WorkMode)
            {
                pSendCmd[5] = 0XFF;
            }
            else
            {
                pSendCmd[5] = 0X00;
            }

            pSendCmd[2] = 3;
            break;

        case CANSET_FILID:
            Temp = CanParam->FilterId[0];

            //这个地方需要修改，对设置过滤ID的限制
            //----------普通CAN-BUS设置过滤ID
            //字迎柱 modify 2009-6-22 15:10:39 解决opel can断线的问题，此问题和澳大利亚张健中碰到的相似 super16的ID过滤直接用掩码滤波来实现
            //路虎的多帧180帧，这里用掩码过滤是否会有问题总线上一直有数据
            if ((Temp > 0) && (Temp <= 13))
            {
                /*
                if(CanParam->IsExFrame)
                {
                    pSendCmd[5]=0xFF;
                    pSendCmd[2]=CanParam->FilterId[0]*4+4;
                }
                else
                {
                    pSendCmd[5]=0x00;
                    pSendCmd[2]=CanParam->FilterId[0]*2+4;
                }
                for(i=0; i<(pSendCmd[2]-3); i++)
                    pSendCmd[6+i]=CanParam->FilterId[i];
                */
                if (ConnectorType == CON_Super16V30)
                {
                    //ID转化为掩码,先处理标准幁
                    if (CanParam->IsExFrame)
                    {
                        for (i = 0; i < CanParam->FilterId[0]; i++)
                        {
                            MaskArray[0 + 8 * i] = CanParam->FilterId[1 + 4 * i];
                            MaskArray[1 + 8 * i] = CanParam->FilterId[2 + 4 * i];
                            MaskArray[2 + 8 * i] = CanParam->FilterId[3 + 4 * i];
                            MaskArray[3 + 8 * i] = CanParam->FilterId[4 + 4 * i];
                        }
                    }
                    else
                    {
                        for (i = 0; i < CanParam->FilterId[0]; i++)
                        {
                            MaskArray[0 + 8 * i] = CanParam->FilterId[1 + 2 * i];
                            MaskArray[1 + 8 * i] = CanParam->FilterId[2 + 2 * i];
                        }
                    }

                    for (i = 0; i < (CanParam->FilterId[0]) * 8; i++)
                    {
                        pSendCmd[5 + i] = MaskArray[i];
                    }

                    pSendCmd[2] = (CanParam->FilterId[0]) * 8 + 2;
                    pSendCmd[4] = CANSET_MASKFILID;
                }
                else
                {
                    if (CanParam->IsExFrame)
                    {
                        pSendCmd[5] = 0xFF;
                        pSendCmd[2] = CanParam->FilterId[0] *4 + 4;
                    }
                    else
                    {
                        pSendCmd[5] = 0x00;
                        pSendCmd[2] = CanParam->FilterId[0] *2 + 4;
                    }

                    for (i = 0; i < (pSendCmd[2] -3); i++)
                    {
                        pSendCmd[6 + i] = CanParam->FilterId[i];
                    }
                }

                break;
            }

            //----------无过滤或禁止接收
            else if ((Temp == 0X00) || (Temp == 0XFF))
            {
                pSendCmd[2] = 4;

                if (CanParam->IsExFrame)
                {
                    pSendCmd[5] = 0xFF;
                }
                else
                {
                    pSendCmd[5] = 0x00;
                }

                for (i = 0; i < (pSendCmd[2] -3); i++)
                {
                    pSendCmd[6 + i] = CanParam->FilterId[i];
                }

                break;
            }

            //范围
            else if (Temp == 0XFE)
            {
                return 0X62; //－－－－－暂不处理
            }

            //过滤ID + 一组掩码 //----------带掩码和过滤ID同时设置部分: VOLVO为例
            //目前设置掩码的都是扩展帧！！！
            else if ((Temp & 0XF0) == 0XF0)
            {
                FilterNum = CanParam->FilterId[0] &0X0F;

                if (CanParam->IsExFrame)
                {
                    bTime = 4;
                }
                else
                {
                    bTime = 2;
                }

                for (i = 0; i < FilterNum * bTime; i++)
                {
                    FilID[i] = CanParam->FilterId[1 + i];
                }

                //这个地方可能存在问题，标准帧和扩展帧的掩码设置格式应该是一样的，不过目前只有扩展帧才涉及设置掩码
                for (i = 0; i < 2 * bTime; i++)
                {
                    MaskID[i] = CanParam->FilterId[1 + FilterNum * bTime + i];
                }

                if (ConnectorType == CON_CANV20 || ConnectorType == CON_TOPV30) //CANBUS 2.0 和TOP接头需要同时设置过滤ID和掩码
                {
                    pSendCmd[2] = 0X04 + FilterNum * bTime;
                    pSendCmd[4] = 0X01;             //6001

                    if (CanParam->IsExFrame)
                    {
                        pSendCmd[5] = 0xFF;
                    }
                    else
                    {
                        pSendCmd[5] = 0x00;
                    }

                    pSendCmd[6] = FilterNum;

                    for (i = 0; i < FilterNum * bTime; i++)
                    {
                        pSendCmd[7 + i] = FilID[i];
                    }

                    CountNum = 0;
                    pSendCmd[pSendCmd[2] +3] = 0x00;

                    while (1)
                    {
                        memset(TempBuf, 0, sizeof(TempBuf));

                        if ((CountNum++) == 2)
                        {
                            return 0X63;
                        }
                    }
                }

                //下面的设置存在问题
                else if (ConnectorType == CON_Super16V30 || ConnectorType == CON_DiaGunV50) //Super16和DIAGUN接头只需设置掩码,不能同时设置过滤ID
                {
                    /*
                    pSendCmd[2]=0X06 + bTime;
                    pSendCmd[4]=0X01;                       //6001
                    if(CanParam->IsExFrame)
                        pSendCmd[5]=0xFF;
                    else
                        pSendCmd[5]=0x00;
                    pSendCmd[6]=0XFD;
                    pSendCmd[7]=0x18;
                    pSendCmd[8]=0x08;
                    for(i=0;i<bTime;i++)
                        pSendCmd[9+i]=MaskID[i];
                    */
                    memset(pSendCmd, 0, sizeof(pSendCmd));
                    pSendCmd[0] = 0x55;
                    pSendCmd[1] = 0xAA;
                    pSendCmd[2] = 0X02 + (FilterNum + 1) * 8; //0X04 EDIT BY QQ 20081126
                    pSendCmd[3] = 0x60;
                    pSendCmd[4] = 0X06;             //6006

                    for (i = 0; i < FilterNum; i++)
                    {
                        pSendCmd[5 + i * 8] = CanParam->FilterId[1 + i * 4];
                        pSendCmd[6 + i * 8] = CanParam->FilterId[2 + i * 4];
                        pSendCmd[7 + i * 8] = CanParam->FilterId[3 + i * 4];
                        pSendCmd[8 + i * 8] = CanParam->FilterId[4 + i * 4];
                    }

                    for (i = 0; i < 8; i++)
                    {
                        pSendCmd[5 + FilterNum * 8 + i] = CanParam->FilterId[1 + FilterNum * 4 + i];
                    }

                    break;
                }
                else
                {
                    return 0X65;
                }
            }
            else
            {
                return 0X66;
            }

        case CANSET_MASKFILID: /*********Set EnableListen*******/
            //Super16支持32组掩码
            if ((ConnectorType == CON_Super16V30) && ((CanParam->FilterId[0]) > 0) && ((CanParam->FilterId[0]) < 33))
            {
                for (i = 0; i < (CanParam->FilterId[0]) * 8; i++)
                {
                    pSendCmd[5 + i] = CanParam->FilterId[i + 1];
                }
            }

            //CANII支持2组掩码
            else if ((ConnectorType == CON_CANV20) && ((CanParam->FilterId[0]) > 0) && ((CanParam->FilterId[0]) < 3))
            {
                for (i = 0; i < (CanParam->FilterId[0]) * 8; i++)
                {
                    pSendCmd[5 + i] = CanParam->FilterId[i + 1];
                }
            }
            else if ((ConnectorType == CON_TOPV30) && ((CanParam->FilterId[0]) > 0) && ((CanParam->FilterId[0]) < 3))
            {
                for (i = 0; i < (CanParam->FilterId[0]) * 8; i++)
                {
                    pSendCmd[5 + i] = CanParam->FilterId[i + 1];
                }
            }

            //diagun 支持32组掩码-ziyingzhu2009-25修改
            else if ((ConnectorType == CON_DiaGunV50) && ((CanParam->FilterId[0]) > 0) && ((CanParam->FilterId[0]) < 33))
            {
                for (i = 0; i < (CanParam->FilterId[0]) * 8; i++)
                {
                    pSendCmd[5 + i] = CanParam->FilterId[i + 1];
                }
            }
            else
            {
                return 0X67;
            }

            pSendCmd[2] = (CanParam->FilterId[0]) * 8 + 2;
            break;

        default:
            return 0X63;
    }

#if 1 //add by ysa
    NewBuffer();
    AddCharToBuffer(CAC_DATA_SEND_RECEIVE);         //61
    AddCharToBuffer(CAC_SEND_N_FRAME_RECEIVE_N_FRAME); //05
    AddCharToBuffer(0xFF);                          //FF
    AddCharToBuffer(0x01);                          //01帧数
    lenth = pSendCmd[2]; //add by hejm
    AddCharToBuffer(lenth + 4);                     //长度

    //55,aa,04,60,02,00,03,65
    //长度04,校验位0x65
    pSendCmd[2 + lenth + 2] = 0;

    for (i = 0; i <= lenth; i++)
    {
        pSendCmd[2 + pSendCmd[2] +1] ^= pSendCmd[2 + i];
    }

    AddToBuffer(lenth + 4, pSendCmd);
    DisposeSendAndReceive(NULL, 0);
    DelBuffer();
    return RT_OK;
#endif

    CountNum = 0;
    pSendCmd[pSendCmd[2] +3] = 0x00;

    while (1)
    {
        memset(TempBuf, 0, sizeof(TempBuf));

        if ((CountNum++) == 2)
        {
            return RT_ERROR;
        }
    }
}


/********************************************************************************************************************************************
* 函数名称: BYTE CanCmdCheckXor(BYTE *InputCmd)                                                                                             *
* 函数功能: CAN命令异或校验计算函数.                                                                                                        *
* 函数参数: BYTE *InputCmd: 命令缓冲区                                                                                                      *
*           命令格式: 0x55,0xaa,0x0b,0x08,0xfc,0x00,0x02,0x1a,0x97,0x00,0x00,0x00,0x00,0x00,0x70                                            *
* 返 回 值: 命令总长度                                                                                                                      *
* 修改记录: 1. 20080722     JJH                                                                                                             *
*********************************************************************************************************************************************/
uint8_t CanCmdCheckXor(uint8_t * InputCmd)
{
    uint8_t i = 0;
    uint8_t CsLen = 0;
    uint8_t DataLen = 0;

    if (InputCmd[2] == 0)
    {
        return RT_ERROR;
    }

    DataLen = InputCmd[2] +4;
    InputCmd[DataLen - 1] = 0x00;
    CsLen = InputCmd[2] +1;

    for (i = 0; i < CsLen; i++)
    {
        InputCmd[DataLen - 1] ^= InputCmd[i + 2];
    }

    return DataLen;
}


/********************************************************************************************************************************************
* 函数名称: BYTE CanSetParameterNormal(BYTE ConnectorType,CAN_COM *CanParam)                                                                *
* 函数功能: 设置CAN通讯参数:根据CanParam全部设置完过滤ID(包含掩码),波特率,工作模式
                                                              *
* 函数参数: BYTE ConnectorType 接头类型:CON_Super16V30 || CON_TOPV30 || CON_DiaGunV50 ||CON_CANV20                                          *
*           CAN_COM *CanParam  设置参数结构                                                                                                 *
* 返 回 值: RT_OK 设置成功,其他-设置失败                                                                                                    *
* 修改记录: 1. 20080807     JJH                                                                                                             *
*********************************************************************************************************************************************/
//ziyingzhu 修改2009-9-21 22:32 将波特率的设置放在前面，解决super16出现的掩码设置失败的问题
//工程师如果单独使用掩码设置函数，则要求设置掩码前必须先设置波特率，也就是波特率设置必须在掩码之前！！！因为super16的限制而造成的
uint8_t CanSetParameterNormal(uint8_t ConnectorType, CAN_COM * CanParam)
{
    uint8_t Reback = 0;

    Reback = CanSetParameter(ConnectorType, CANSET_BITRATE, CanParam);

    if (Reback != RT_OK)
    {
        return Reback;
    }

    Reback = CanSetParameter(ConnectorType, CANSET_FILID, CanParam);

    if (Reback != RT_OK)
    {
        return Reback;
    }

    //如果是super16的话，设置ID这里需要执行两次(因为super16的过滤ID用掩码来实现，要求设置掩码前必须要设置波特率，用掩码方式来实现过滤ID)

    /*
    if(ConnectorType == CON_Super16V30)
    {
        Reback=CanSetParameter(ConnectorType,CANSET_FILID,CanParam);
        if(Reback!=RT_OK)
            return Reback;
    }*/
    Reback = CanSetParameter(ConnectorType, CANSET_WORKMODE, CanParam);

    if (Reback != RT_OK)
    {
        return Reback;
    }

    return RT_OK;
}


