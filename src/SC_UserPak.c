//------------------------------------------------------------------------------
//  Purpose: Smartbox 下位机数据包用户区解包模块
//  Funtion: 对输入的下位机用户区数据进行解包输出
//           数据包格式: 2字节指令ID 多字节指令参数
//           Example = { 60 02 00 00 00 00 00 00 }
//  Dependent:
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef SC_UserPak_C
#define SC_UserPak_C
#include "SC_UserPak.h"
#include <gdstd.h>
//------------------------------------------------------------------------------
// Funtion: 解包一个请求数据包
// Input  : Temp - 输出的原始数据
// Output : UserPak - 用户数据包
// Return : none
// Info   : Temp = { 60 02 00 00 00 00 00 00 }
//          PakCmdID = 6002 PakLenth = 6 Pakdata = { 00 00 00 00 00 00 }
//------------------------------------------------------------------------------
void SC_UserPak_SolveREQ( unsigned char *Temp, SC_UserPak *UserPak )
{
    unsigned int i, framenum;
    struct buff_info
    {
        unsigned int length;
        unsigned int pos;
    }   Temp_info;
    unsigned int frame_num;
    unsigned int length = 0;
    unsigned char framelen = 0, postion = 0;
    UserPak->PakCmdID = Temp[ 0 ] * 256 + Temp[ 1 ];
    switch( UserPak->PakCmdID )
    {
    case 0x6001:    // SetIoParameter_1
        UserPak->PakLenth = 7;
        break;
    case 0x6002:    // SetEcuBaudRate_1
        if(Temp[2] == 0)
        {
            UserPak->PakLenth = 5;
        }
        else
        {
            UserPak->PakLenth = 3;
        }
        break;
    case 0x6003:    // SetEcuCommTimeInterval_1
        UserPak->PakLenth = 5;
        break;
    case 0x6004:    // SetLinkData_1
        if( Temp[2] != 0 )
        {
            UserPak->PakLenth = Temp[2] + 3;
        }
        else
        {
            UserPak->PakLenth = 3;
        }
        break;
    case 0x6005:    // Set5BpsParameter_1
        if(2 == Temp[2])
        {
            if( Temp[7] )
            {
                UserPak->PakLenth = Temp[7] + 6;
            }
            else
            {
                UserPak->PakLenth = 7;
            }
        }
        else
        {
            if( Temp[6] )
            {
                UserPak->PakLenth = Temp[6] + 5;
            }
            else
            {
                UserPak->PakLenth = 6;
            }
        }
        break;
    case 0x6006:    // SetBosch_1
        UserPak->PakLenth = 2;
        break;
    case 0x6007:    // SetCommunicationLineVoltage_1
        UserPak->PakLenth = 3;
        break;
    case 0x6008:    // SetBoschLink_1
        UserPak->PakLenth = 1;
        break;
    case 0x6009:    // BoschFiat_1
        UserPak->PakLenth = 1;
        break;
    case 0x600a:    // SetAddressFiat_1
        UserPak->PakLenth = 6;
        break;
    case 0x600c:    // SetCANBusLinkData_1
        if(Temp[2] != 0)
        {
            UserPak->PakLenth = Temp[2] + 3;
        }
        else
        {
            UserPak->PakLenth = 3;
        }
        break;
    case 0x600d:    // SetCANBus2LinkData_1
    case 0x601B:    // SetBenzHMFLinkKeep_1
        if(Temp[2] != 0)
        {
            UserPak->PakLenth = Temp[2] + 3;
        }
        else
        {
            UserPak->PakLenth = 3;
        }
        break;
    case 0x600e:    // SetVpwLogicLevel_1
        UserPak->PakLenth = 2;
        break;
    case 0x6011:    // LandroverABSEnter_1
        UserPak->PakLenth = 4;
        break;
    case 0x6014:    // ShangHuanBoschSystem_1
        UserPak->PakLenth = 1;
        break;
    case 0x6015:    // FiatBoschSystem_1
        UserPak->PakLenth = 1;
        break;
    case 0x6018:    // SetEnterFrameData_1
        UserPak->PakLenth = Temp[2] + 1;
        break;
    case 0x6104:    // OnlySendToEcu_1
        UserPak->PakLenth =  3;
        framenum = Temp[3];
        for(i = 0; i < framenum; i++)
        {
            UserPak->PakLenth += Temp[UserPak->PakLenth + 1] + 1;
        }
        break;
    case 0x6019:    // SetConnectorBaudRate_1
    case 0x601A:    // SendAddressCodeTimeAdjustable_1
        if(Temp[2] == 1)
        {
            UserPak->PakLenth = 8 + Temp[9];
        }
        if(Temp[2] == 2)
        {
            UserPak->PakLenth = 9 + Temp[10];
        }
        break;
    case 0x601C:    // AddressCodeWayAdjustTime_benz_1
        if(Temp[2] == 1)
        {
            UserPak->PakLenth = 9;
        }
        if(Temp[2] == 2)
        {
            UserPak->PakLenth = 11;
        }
        break;
    case 0x601D:    // AddressCodeCommunicationWay_Lline_1
        switch(Temp[2])
        {
        case 1:
            UserPak->PakLenth = 5 + Temp[6];
            break;
        case 2:
            UserPak->PakLenth = 7 + Temp[8];
            break;
        }
        break;
    case 0x601E:    // SelectMultiIoLine
        UserPak->PakLenth = Temp[ 0 ] + 1;
        break;
    case 0x601f:    // SetEnterFrameDataExt_1
        UserPak->PakLenth = Temp[3] + 2;
        break;
    case 0x6020:    // GetBoxInfo
        UserPak->PakLenth = 2;
        break;
    case 0x6121:
        UserPak->PakLenth = 3;
        for(i = 0; i < Temp[2]; i++)
        {
            UserPak->PakLenth += Temp[UserPak->PakLenth] + 2;
        }
        break;
    case 0x6122:
        UserPak->PakLenth = Temp[2] +3;
        break;
    case 0x6125:
        UserPak->PakLenth = 2;
        for(i = 0; i < 2; i++)
        {
            UserPak->PakLenth += Temp[UserPak->PakLenth] + 1;
        }
        break;
    case 0x6126:
        UserPak->PakLenth = Temp[2] * 0x100 + Temp[3] + 2;
        break;
    case 0x6129:
        UserPak->PakLenth = Temp[3] + 5;
        break;
    case 0x6105:    // GetDownloadType
        UserPak->PakLenth = 5;
        framelen = Temp[3];
        postion = 4;
        while(framelen--)
        {
            UserPak->PakLenth += Temp[postion];
            postion += Temp[postion] + 1;
            if(Temp[postion] != 0)
            {
                UserPak->PakLenth += 1;
            }
            else
            {
                UserPak->PakLenth += 3;
            }
        }
        break;
    case 0x610a:    // BoschSendDataToEcuGetMultiFrameAnswer_1
        UserPak->PakLenth = Temp[2] + 2;
        break;
    case 0x610f:    // BoschSendDataToEcuGetAnswer_1
        framenum = Temp[3];
        UserPak->PakLenth = 2;
        for(i = 0; i < framenum; i++)
        {
            UserPak->PakLenth += Temp[UserPak->PakLenth + 2] + 1;
        }
        break;
    case 0x6111:
        UserPak->PakLenth = Temp[3] + 4;
        break;
    case 0x6117:
        UserPak->PakLenth = 5;
        for(i = 0; i < Temp[3]; i++)
        {
            UserPak->PakLenth += Temp[UserPak->PakLenth + 3] + 2;
        }
        break;
    case 0x6118:
        UserPak->PakLenth = 4;
        break;
    case 0x6119:
        UserPak->PakLenth = Temp[4] + 8;
        break;
    case 0x611c:
        UserPak->PakLenth = Temp[4] + 8;
        break;
    case 0x6109:    // KWPSendDataToEcuGetAnswer_1
    case 0x6107:    // ISO_SendDataToEcuGetMultiFrameAnswer  (9)
    case 0x6110:    // ISO_SendDataToEcuGetAnswer            (10)
        //UserPak->PakCmdID = 0x6109;
        frame_num = Temp[3];
        Temp_info.length = 0;
        Temp_info.pos = 4;
        for(i = 0; i<frame_num; i++)
        {
            length += Temp[Temp_info.pos];//+1表示的是加上长度字节本身
            Temp_info.pos += Temp[Temp_info.pos]+1;
        }
        UserPak->PakLenth = length + 2 + frame_num;//+2表示的是加上发送的次数,发送的帧数.
        break;
    case 0x610b:    //KWPSendOneAndReceiveMultiFrame             (2)//2010-12-14
    case 0x6116:    //FordIsoSendOneAndReceiveMultiFrame();      (11)ok
    case 0x6113:    //CanbusOnlySendDataToEcu                    (12)OK
        UserPak->PakLenth = Temp[4] + 4;
        UserPak->Pakdata  = &Temp[2];
        break;
    case 0x611f:    //HoldenCan20SendOneFrameReceiveDatas         (17)OK
        UserPak->PakLenth = Temp[3] * Temp[4] + 2 + Temp[4];
        break;
    default:
        UserPak->PakLenth = 0;
        break;
    }
    UserPak->Pakdata = &Temp[2];
}
#endif
//--------------------------------------------------------- End Of File --------
