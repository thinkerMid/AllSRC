//------------------------------------------------------------------------------
//  Purpose: 下位机指令函数指针列表
//  Funtion:
//  Dependent:
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef SC_cmdfunc_H
#define SC_cmdfunc_H
#include "FUN_SetLinePara.h"
#include "FUN_KLine.h"
#include "FUN_J1850.h"
#include "FUN_CAN.h"
#include "FUN_OneToOne.h"
//*****************************************************************************
//
// Command line function callback type.
//
//*****************************************************************************
typedef int (*pfnCmdLine)(int argc, unsigned char *argv, unsigned char *ans);
//*****************************************************************************
//
//! This is the command table that must be provided by the application.
//
//*****************************************************************************
pfnCmdLine g_sCmdTable[ 3 ][ 0x30 ] =
{
    {
        /*6000*/    0,
        /*6001*/    SetIoParameter_1,                                           // JV700 通讯IO 设置
        /*6002*/    SetEcuBaudRate_1,                                           // K Line 通讯波特率
        /*6003*/    SetEcuCommTimeInterval_1,                                   // 整体通讯时序
        /*6004*/    SetLinkData_1,                                              // 设置链路维持
        /*6005*/    Set5BpsParameter_1,                                         // 普通 5 Bps 地址码进入
        /*6006*/    SetBosch_1,                                                 // bosch 5 Bps 地址码进入
        /*6007*/    SetCommunicationLineVoltage_1,                              // K LINE 电平拉高或拉低
        /*6008*/    SetBoschLink_1,                                             // bosch 协议链路维持
        /*6009*/    BoschFiat_1,                                                // Fiat车型 bosch 5 Bps 地址码进入
        /*600A*/    SetAddressFiat_1,                                           // Fiat车型 5 Bps 地址码进入
        /*600B*/    KWPSendOneAndReceiveMultiFrame_1,                           // KWP2000 发送一帧接收 多帧
        /*600C*/    SetCANBusLinkData_1,                                        // 设置 CANBUS 链路维持
        /*600D*/    SetCANBus2LinkData_1,                                       // 设置 CANBUS 链路维持
        /*600E*/    SetVpwLogicLevel_1,                                         // 设置 VPW 通讯接收电平识别脉宽
        /*600F*/    SetLinkNoAnswer_1,                                          // 设置链路位置且不等待应答
        /*6010*/    Set5BpsParameter_1,                                         // bosch 5 Bps 地址码进入
        /*6011*/    LandroverABSEnter_1,                                        // Landrover 地址码进入
        /*6012*/    0,                                                          // 无效函数
        /*6013*/    0,                                                          // 无效函数
        /*6014*/    ShangHuanBoschSystem_1,                                     // ShangHuan bosch 5 Bps 地址码进入
        /*6015*/    FiatBoschSystem_1,                                          // Fiat车型 bosch 5 Bps 地址码进入
        /*6016*/    Benz38SelectLine_1,                                         // BENZ 38PIN 接头选线
        /*6017*/    0,                                                          // 待完成 SetMitLinkKeep CAN 中断模式还没有做
        /*6018*/    SetEnterFrameData_1,                                        // 设置 CAN 流控制帧
        /*6019*/    SetConnectorBaudRate_1,                                     // 设置上下位机通讯波特率，此函数无效
        /*601A*/    SendAddressCodeTimeAdjustable_1,                            // 普通 5 Bps 地址码进入
        /*601B*/    SetBenzHMFLinkKeep_1,                                       // SetBenzHMFLinkKeep
        /*601C*/    AddressCodeWayAdjustTime_benz_1,                            // 普通 5 Bps 地址码进入 FOR BENZ
        /*601D*/    AddressCodeCommunicationWay_Lline_1,                        // 普通 5 Bps 地址码进入
        /*601E*/    SelectMultiIoLine,                                          // JV700 无效果，设置多个 IO 口
        /*601F*/    SetEnterFrameDataExt_1,                                     // 设置 CAN 流控制帧（扩展帧）
        /*6020*/    GetBoxInfo,                                                 // 获取 BOX 信息
        /*6021*/    GetDownloadType,                                            // 获取升级类型
        /*6022*/    0,                                                          // 待完成 SetTimeParameter
        /*6023*/    0,                                                          // None
        /*6024*/    0,                                                          // None
        /*6025*/    0,                                                          // None
        /*6026*/    0,                                                          // None
        /*6027*/    0,                                                          // None
        /*6028*/    0,                                                          // None
        /*6029*/    0,                                                          // None
        /*602A*/    0,                                                          // None
        /*602B*/    0,                                                          // None
        /*602C*/    0,                                                          // None
        /*602D*/    0,                                                          // None
        /*602E*/    0,                                                          // None
        /*602F*/    0,                                                          // None
    },
    {
        /*6100*/    0,                                                          // None
        /*6101*/    0,                                                          // 待完成 FlashCodeFunction
        /*6102*/    0,                                                          // 待完成 OnlyReceiveFromEcu
        /*6103*/    0,                                                          // None
        /*6104*/    OnlySendToEcu_1,                                            // 向 ECU 发送一帧数据
        /*6105*/    OneToOne_SendDataToEcuGetAnswer_1,                          // 向 ECU 发送一帧数据并接收一帧数据
        /*6106*/    0,                                                          // 待完成 SendOneAndReceiveMultiFrameKnownAckFrameNumber
        /*6107*/    ISO_SendDataToEcuGetMultiFrameAnswer_1,                     // ISO 发送一帧接收多帧
        /*6108*/    0,                                                          // None
        /*6109*/    KWPSendDataToEcuGetAnswer_1,                                // KWP 发送一帧接收一帧
        /*610A*/    BoschSendDataToEcuGetMultiFrameAnswer_1,                    // BOSCH 发送一帧接收多帧
        /*610B*/    KWPSendOneAndReceiveMultiFrame_1,                           // KWP 发送一帧接收多帧
        /*610C*/    PWM_SendDataToEcuGetAnswer_1,                               // PWM 发送一帧接收一帧
        /*610D*/    0,                                                          // None
        /*610E*/    VpwSendOneAndReceiveOneFrame_1,                             // VPW 发送一帧接收一帧
        /*610F*/    BoschSendDataToEcuGetAnswer_1,                              // BOSCH 发送一帧接收多帧
        /*6110*/    ISO_SendDataToEcuGetAnswer_1,                               // ISO 发送一帧接收一帧
        /*6111*/    SendOneFrameDataToEcuGetAnyFrameAnswer_1,                   // 发一帧收任意帧 hejm k线发送，格式有点奇怪
        /*6112*/    PwmSendOneAndReceiveMultiFrame_1,                           // PWM 发送一帧接收多帧
        /*6113*/    CanbusOnlySendDataToEcu_1,                                  // CAN 仅发不收，hejm 函数实际上是k线发送，怎么回事？
        /*6114*/    CanbusSendTwoFrameToEcuGetAnswerOldCon10_1,                 // None
        /*6115*/    0,                                                          // ReadByteBytedataLand
        /*6116*/    FordIsoSendOneAndReceiveMultiFrame_1,                       // Ford ISO 发送一帧接收多帧
        /*6117*/    HoldenNormalRingLinkSendOneAndOneFrame_1,                   // HoldenNormalRingLinkSendOneAndOneFrame
        /*6118*/    HoldenOnlyKnowHeadOfFrame_1,                                // HoldenOnlyKnowHeadOfFrame
        /*6119*/    SubruOldProtocol_1,                                         // SubruOldProtocol
        /*611A*/    0,                                                          // SYS_SENDDATA_VOLVE
        /*611B*/    VPW_HDN_SendDataToEcuGetMultiFrameAnswer_1,                 // VPW_Holden专用_发多帧收多帧
        /*611C*/    SendOneFrameDataToEcuGetAnyFrameAnswer_Check_1,             //  SendOneFrameDataToEcuGetAnyFrameAnswer_Check
        /*611D*/    VpwSendMultiFrameAndReceiveMultiFrameForHolden_1,           // VPW发多收多Holden专用
        /*611E*/    0,                                                          // none
        /*611F*/    HoldenCan20SendOneFrameReceiveDatas_1,                      // Holden Can 发一帧 接收多帧
        /*6120*/    JAGUARPWM_SendDataToEcuGetMultiFrameAnswer_1,               // 发一帧 PWM 接收指定 PWM 数据
        /*6121*/    WabcoAbsSendOneAndReicveOneFrame_1,                         // WabcoAbsSendOneAndReicveOneFrame
        /*6122*/    WabcoAbsSendOneAndReicveMultiFrame_1,                       //  WabcoAbsSendOneAndReicveMultiFrame
        /*6123*/    0,                                                          // none
        /*6124*/    0,                                                          // none
        /*6125*/    QuickSendTwoFrameReceiveMultiFrame_1,                       //  QuickSendTwoFrameReceiveMultiFrame
        /*6126*/    OneToOne_SendLongDataToEcuGetLongAnswer_1,                  //  OneToOne_SendLongDataToEcuGetLongAnswer
        /*6127*/    VpwSendMultiFrameAndReceiveMultiFrameKnownAckFrameNumber_1, // VPW 发多收多
        /*6128*/    OneToOne_SendOneByteToEcuGetAnswerLM_1,                     // OneToOne_SendOneByteToEcuGetAnswerLM
        /*6129*/    KWPSendOneAndReceiveMultiFrameHasFilter_1,                  // KWPSendOneAndReceiveMultiFrameHasFilter
        /*612A*/    0,                                                          //
        /*612B*/    0,                                                          //
        /*612C*/    0,                                                          //
        /*612D*/    0,                                                          //
        /*612E*/    0,                                                          //
        /*612F*/    0,                                                          //
    },
    {
        /*6200*/    0,                                                          // none
        /*6201*/    Can20SendTwoFrameQuickGetInfo_1,                            // CANBUS 发两帧收一块
        /*6202*/    CANSendAndReceiveMultiFrame_1,                              // CANBUS 发一帧收指定多帧
        /*6203*/    CANSendAndReceiveUnknownFrame_1,                            // CANBUS 发一帧收未知多帧
        /*6204*/    0,                                                          // none
        /*6205*/    VWCANSendDataToEcuGetMultiFrameAnswer1_1,                   // VW 发送一块接收一块
        /*6206*/    0,                                                          // VwSpecialFunctionReadPassword
        /*6207*/    GMOPEL_CanSendMulFrameGetMulFrame_1,                        // OPEL CAN 发多帧收多帧
        /*6208*/    DongFengSendMultiFrameAndReceiveOneFrame_1,                 // 东风专用发多帧收一帧
        /*6209*/    0,                                                          // ChryslerCcdOneToOne
        /*620A*/    0,                                                          // LandroverSendAndReceiveFilter
        /*620B*/    LandroverSendAndReceiveFault_1,                             // lanrdrover send one frame and receive multiframe until timeout
        /*620C*/    0,                                                          // BenzReadByteFromECU
        /*620D*/    GMSendMultiFrameToECUGetAnswer_1,                           // GMSendMultiFrameToECUGetAnswer
        /*620E*/    0,                                                          // none
        /*620F*/    0,                                                          // 待完成 SsangyongSendOneByteAndReceiveByte
        /*6210*/    0,                                                          // 待完成 WabcoEcasSendOneByteAndReceiveByte
        /*6211*/    KWPSendDataToEcuGetAnswer_more_1,                           // KWP 接收一帧发送一帧
        /*6212*/    KWPSendOneFrameAndReceiveMultiFrame_Volvo_1,                // KWP 接收一帧发送一帧 VOVLO
        /*6213*/    ToyotaA2ModeCanMultiframe_1,                                // Toyota CAN 发送一帧接收一帧
        /*6214*/    KWPSendOneAndReceiveMultiFrame_Toyota_1,                    // Toyota KWP 发送一帧接收一帧
        /*6215*/    0,                                                          // none
        /*6216*/    0,                                                          // SAE_J1708_SendDataToEcuGetAnswer
        /*6217*/    0,                                                          // CumminsDieselSAE1708_SendMultiFramesToEcuGetAnswer
        /*6218*/    CAN_RecieveMultiFrame_Porsche_1,                            // Porsche CAN 接收多帧
        /*6219*/    SysGazSendBankSingleFrame_1,                                // SysGazSendBankSingleFrame
        /*621A*/    0,                                                          // ResetComm
        /*621B*/    0,                                                          // none
        /*621C*/    0,                                                          // none
        /*621D*/    0,                                                          // none
        /*621E*/    0,                                                          // none
        /*621F*/    0,                                                          // none
        /*6220*/    KWP_SendDataToEcuGetAnswer_Benz_1,                          // BENZ KWP 发一帧收一帧
        /*6221*/    KWP_SendDataToEcuGetMultiFrameAnswer_Benz_1,                // BENZ KWP 发一帧收多帧
        /*6222*/    SendOneFrameToEcuGetMultiFrameAnswer_1,                     // SendOneFrameToEcuGetMultiFrameAnswer
        /*6223*/    0,                                                          // 待完成 CumminsSAE1939_SendMultiFramesToEcuGetAnswer
        /*6224*/    KWPSendDataToEcuGetAnswer_BMW_1,                            // KWP BMW  发一帧收一帧
        /*6225*/    CANSendAndReceiveUnknownFrame_benz_1,                       // CAN 发一帧收多帧
        /*6226*/    0,                                                          // 待完成 OneToOne_SendLongDataToEcuGetLongAnswer
        /*6227*/    CanSendMulFrameGetMoreSingleFrame_1,                        // CAN 发多帧收一帧
        /*6228*/    0,                                                          // 待完成 OneToOne_SendOneByteToEcuGetAnswer
        /*6229*/    0,                                                          // none
        /*622A*/    0,                                                          // none
        /*622B*/    0,                                                          // none
        /*622C*/    0,                                                          // none
        /*622D*/    0,                                                          // none
        /*622E*/    0,                                                          // none
        /*622F*/    0,                                                          // none
    }
};
#endif
//--------------------------------------------------------- End Of File --------
