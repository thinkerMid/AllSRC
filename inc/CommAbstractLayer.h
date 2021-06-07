

//------------------------------------------------------------------------------
//  Purpose: DPD MYRCAR 模式 通讯抽象层
//  Funtion: MYCAR 方式所有函数
//  Dependent:
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef __Communication_Abstract_Layer__
#define __Communication_Abstract_Layer__
#define CAC_CONTRAL_COMMAND             0x60
#define CAC_IO_CONTRAL                  0x01
#define CAC_ECU_COMMUNICATION_BAUD_RATE 0x02
#define CAC_ECU_COMM_TIME_INTERVAL      0x03
#define CAC_ECU_LINK_KEEP               0x04
#define CAC_ECU_ADDRESS_CODE_WAY        0x05
#define CAC_ECU_BOSCH_ENTER_GET_VERSION 0x06
#define CAC_SET_IO_LINE_VOLTAGE         0x07
#define CA_BOSCH_LINK_KEEP_SET_AND_CLEAR 0x08 //2002.12.12 add
#define BOSCH_ON_LINK_KEEP              0xFF //2002.12.12 add
#define BOSCH_OFF_LINK_KEEP             0x00 //2002.12.12 add
#define CAC_ECU_ADDRESS_GET_KWD_CHECK   0x09
#define CAC_ECU_ADDRESS_CODE_WAY2       0x0A
#define BOSCH_ON_LINK_KEEP              0xFF //2002.12.12 add
#define BOSCH_OFF_LINK_KEEP             0x00 //2002.12.12 add
#define CAC_OPEL_ECU_ADDRESS_CODE_WAY   0x0B
#define CAC_CANBUS_LINK_KEEP            0x0C
#define CAC_CANBUS2_LINK_KEEP           0x0D
#define CAC_CAN2BUS_LINK_KEEP           0x0D//<---0x10 renault
#define CAC_VPW_SET_TIME                0x0E
#define CAC_MABCO_ECU_LINK_KEEP         0x0F
#define CAC_LINK_KEEP_NO_ANSWER         0x0F
#define CAC_ECU_LINK_KEEP_WABCO         0X0F//<---0x14     //2004.04.24  by hejun
#define CAC_LUHU_ENTER                  0x10
#define CAC_LUHUABS_ENTER               0x11
#define CAC_PEUGEOT_SPECIAL_ADDRESS_CODE_WAY 0x12//<---0x0b
#define BOSCH_ON_LINK_KEEP              0xFF //2002.12.12 add
#define BOSCH_OFF_LINK_KEEP             0x00 //2002.12.12 add
#define CA_ECU_ENTER_MABCO              0X13//<---0x0e
#define CAC_ECU_YATAI_BOSCH_ENTER_GET_VERSION 0x14//<----old id==600E for yaitai bosch
#define CAC_ECU_BOSCH_ENTER_GET_VERSION2 0x15//<----old id==6009 for BOSCH_FIAT
#define CAC_IO_38_CONTRAL               0x16//<----old id==6009 for sprinter
#define CAC_LINK_KEEP_NO_ANSWER_MIT     0X17//<----old id==600f for MIT
#define CAC_RETURN_ECU_10_ANSWER_FRAME  0X18// 20080511 add
#define CAC_LFY_BOSCH_MP73_ADDRESS_CODE_WAY 0x19 //2007.10.16 zy add for LFY BOSCH MP7.3
#define CAC_ADJUST_TIME_ADDRESS_CODE_WAY 0x1A//20081127 add time adjust
#define CAC_ECU_LINK_KEEP_BENZ_HMF      0x1B //ziyingzhu add 2009-3-12
#define CAC_ADJUST_TIME_ADDRESS_CODE_WAY_BENZ 0x1C //ziyingzhu add 2009-3-17
#define CAC_ECU_ADDRESS_CODE_WAY_LLINE_ENABLE 0x1D
#define CAC_SET_MULTI_IO_CONTRAL        0x1E  //liyangcheng add 2010-12-27
#define CAC_SEND_MORE_30_CMD_WAY        0x1F  //liyangcheng add 2011-01-14
#define CAC_SETCANFRAMETIME             0x20  //kai.li
#define CAC_DTSTEST                     0x2F //yaoming add 20100715 GX3/DIAGUN 用于IO测试的DTS线测试

#define CAC_DATA_SEND_RECEIVE           0x61
#define CAC_READ_FLASH_CODE             0x01
#define MINIMUM_FLASH_CODE_WAY          0x00
#define MAXIMUM_FLASH_CODE_WAY          0x20
#define CAC_RECEIVE_DATA_FROM_ECU       0x02
#define MINIMUM_MODE_RECEIVE_DATA_FROM_ECU 0x01
#define MAXIMUM_MODE_RECEIVE_DATA_FROM_ECU 0x0E
#define CAC_DELAY_N_SM                  0x03
#define CAC_SEND_N_FRAME_DATA           0x04
#define CAC_SEND_N_FRAME_RECEIVE_N_FRAME 0x05
#define CAC_KNOW_ANSWER_FRAME_NUMBER__SEND_1_FRAME_RECEIVE_N_FRAME 0x06
#define CAC_ISO__SEND_1_FRAME_RECEIVE_N_FRAME 0x07
#define CAC_SEND_1_FRAME_RECEIVE_N_FRAME_END_FLAG 0x08
#define CAC_KWP2000_SEND_N_FRAME_RECEIVE_N_FRAME 0x09
#define CAC_BOSCH_SEND_1_FRAME_RECEIVE_N_FRAME 0x0A
#define CA_BOSCH_RECEIVE_ONE_FRAME      0x01
#define CAN_MULTI_FLAG_30_CONFIG        0x1F //liyangcheng add 2011-01-14
#define CA_BOSCH_RECEIVE_MULTI_FRAME    0xFF
#define CAC_KWP2000__SEND_1_FRAME_RECEIVE_N_FRAME 0x0B
#define CAC_PWM_SEND_N_FRAME_RECEIVE_N_FRAME 0x0C
#define CAC_VPW_SEND_N_FRAME_RECEIVE_N_FRAME 0x0E
#define CAC_BOSCH_SEND_N_FRAME_RECEIVE_N_FRAME 0x0F
#define CAC_SEND_NBYTE_REC_NBYTE        0x0F//<---6113 ssangyong
#define CAC_ISO_SEND_N_FRAME_RECEIVE_N_FRAME 0x10
#define CAC_SEND_1_FRAME_RECEIVE_ANY_FRAME 0x11
#define CAC_PWM__SEND_1_FRAME_RECEIVE_N_FRAME 0x12
#define CAC_SEND_1_FRAME_NO_ANSWER      0x13
#define CAC_CAN_SEND_1_FRAME_QUICK_GET_INFO 0x14
#define CAC_LANDROVER__SEND_1_BTYE_RECEIVE_1_char 0x15//landrover send 1 byte and receive 1btye
#define CAC_FORD_ISO__SEND_1_FRAME_RECEIVE_N_FRAME 0x16
#define CAC_HOLDEN_NORMAL_RING_LINK     0x17
#define CAC_HOLDEN_NORMAL_RING_LINK_CHECK 0x18
#define CAC_SUBARUOLD_SEND_1_FRAME_RECEIVE_1_FRAME 0x19
#define CAC_NEW_CAN_SEND_1_FRAME_QUICK_GET_INFO 0x1A// eobd add no used
#define CAC_HDN_VPW_SEND_1_FRAME_RECEIVE_N_FRAME 0x1B//<---6121 holden by zjz
#define CAC_SEND_ONEFRAME_ANY_FRAME_SETTIME 0x1c// special for opel
#define CAC_HDN_VPW_SEND_1_FRAME_RECEIVE_N_FRAME_1 0x1D//<---6122 holden by zjz
#define CAC_HDN_CAN_SND_1_FRAME_RECV_N_FRAME 0x1E//<---6123 holden can1.0
#define CAC_HDN_CAN20_SND_N_FRAME_RECV_N_FRAME 0x1F//<<---6124 holden can2.0  use6203
#define CAC_PWM__SEND_1_FRAME_RECEIVE_N_FRAME_JG 0x20//<---6112 jaguar PWM IDFILTER
#define CAC_MABCO_ABS                   0x21
#define CAC_CLRAN_ABS                   0x22
#define CAC_MABCO_ABS_CLEAN             0x22
#define CAC_ONLYRECDATA_NISSAN          0x23
#define CAC_SET_IO_LINE_VOLTAGE_NISSAN  0x24
#define QUICK_SEND_TWO_SERIAL_COMMAND   0x25
#define CAC_SNED_LONG_BYTE_RECEIVE_LONG_BYTE 0x26
#define CAC_RECEIVE_DATA_FROM_ECU_VPW   0x27  //(<-6203)
#define CAC_SEND_ONEBYTE_RECEIVE_ONEBYTE_LM 0x28
#define CAC_J1939FUNCTION               0x29
#define CAC_LANDROVER__SEND_1_BTYE_RECEIVE_1_BYTE 0x15//landrover send 1 byte and receive 1btye
#define SYS_SEND_ONE_FRAME_REC_N_BYTES_UNTILL_TIMEOVER 0x22


#define CAC_DATA_SEND_RECEIVE_1         0x62
#define CAC_DATA_SEND_RECEIVE_NEW       0x62
#define CAC_NEW_CAN_SEND_1_FRAME_QUICK_GET_INFO_CONNECTORII 0x01
#define CAC_CAN2_SEND_1_FRAME_QUICK_GET_INFO 0x01//<--2A can20 send tow command and get multinous frame
#define CAC_CAN_SEND_MUL_FRAME_GET_MUL_FRAME 0x02//<---6116 toyota
#define CAC_CAN_SEND_MUL_FRAME_GET_MUL_FRAME_GN 0x03//<---新增所有车通用模块
#define CAC_VW_CAN_SEND_1_FRAME_MULTI_ANSWER2 0x04//<---6116 VW CAN老接头
#define CAC_VW_CAN_SEND_1_FRAME_MULTI_ANSWER1 0x05//<---6117 VW CAN新接头
#define AUDI_READMIMA_CAC_SEND_N_FRAME_RECEIVE_N_FRAME 0x06//<---6118 VW read secret
#define CAC_CAN2_SEND_1_FRAME_QUICK_GET_INFO_FRAME_NUM 0x07//<---sgm
#define CAC_DENGFENG_CAN_SEND_N_REC_N_FRAME 0X08//<---611F DENFFENG  CANBUS
#define CAC_SUZUKI_CAN_SEND_N_REC_N_FRAME 0X08//<---6120 SUZUKI
#define CAC_CCD_SEND_1_FRAME_GET_1_FRAME_ID 0x09//<---6120 chrysler
#define LAND_OLD_RANGE_RANGE_ROVER_MODE 0X0A//<---612A LANDROVER
#define LAND_OLDRANGEROVER_ABS_FAULT    0X0B//<---612b LANDROVER
#define CAC_BENZ_HFM_OLD_PROTOCOL       0X0C// 20080708
#define CAC_GM_CAN2_SEND_MULTIFRAME_DW  0x0D//20080709
#define CAC_CAN2_SEND_2_FRAME_QUICK_GET_INFO_CTN 0x0E//<---6114 CITREON
#define CAC_SEND_Nchar_REC_NBYTE        0x0F//<---6113 ssangyong
#define CAC_WABCO_CAN_SEND_N_REC_N_FRAME 0X10//<---6121 WABCO(DENFENG machine)
#define KWP_SEND_MORE_256               0X11//<---6128 huachen
#define CAC_VOLVO_KWP2000__SEND_1_FRAME_RECEIVE_N_FRAME 0X12//<---6113 vovol
#define CAC_CAN_SEND_A2_MODE_FRAME_GET_MUL_FRAME 0x13//<---6113 toyota
#define CAC_KWP2000__SEND_1_FRAME_RECEIVE_N_FRAME_TY 0x14//<---610b toyota
#define CAC_CHRYSLER_SEND_2_FRAME_QUICK_GET_INFO_FRAME_NUM 0x15// 6202<--- 0x1F
#define CAC_VOVOL_CAN_SEND_N_FRAME_RECEIVE_N_FRAME 0X16
#define CAC_CUMMINS_CAN_SEND_ONE_FRAME_RECEIVE_N_FRAME 0X17//<---6128 KANGMINGSI 发一帧收多帧
#define CAC_PURSCHE_CAN_SEND_N_FRAME_RECEIVE_N_FRAME 0X18//<---6218 PORSCHE 多发帧收多帧
#define CAC_MICAS_1_FRAME_RECEIVE_ANY_FRAME 0X19//<---6219 gaz 多一帧收多帧
#define CAC_FORD_ISO__SEND_1_FRAME_RECEIVE_N_FRAME_ZJZ 0X04
#define CAC_CHERRY_CAN_SEND_N_REC_N_FRAME 0X0B//dengfeng  can special
#define CAC_DF_CTN_CAN_SEND_N_REC_N_FRAME 0x13//<---0X0B//dengfeng CITENON  can special,len<08
#define CAC_GM_SEND_1_FRAME_QUICK_GET_INFO 0X1C//<---0x611F GM
#define CAC_KWP2000_SEND_N_FRAME_RECEIVE_N_FRAME_BENZ 0x20
#define CAC_KWP2000__SEND_1_FRAME_RECEIVE_N_FRAME_BENZ 0x21
#define SYS_SEND_ONE_FRAME_REC_N_charS_UNTILL_TIMEOVER 0x22
#define CUMMINSSAE1939_SEND_MULTIFRAMES_TOECU_GETANSWER 0x23
#define CAC_KWP2000_SEND_N_FRAME_RECEIVE_N_FRAME_BMW 0x24
#define CAC_CAN_SEND_MUL_FRAME_GET_MUL_FRAME_GN_BENZ 0x25//<---0x6225 yaoming add 20100628 for zhanwei
#define CAC_CAN_SEND_MULFRAME_GET_MULFRAME_MOREFRAME 0x27//<---0x6227

//Select line
#define CA_LINE_TDS                     0
#define CA_LINE_IO1                     1
#define CA_LINE_IO2                     2
#define CA_LINE_IO3                     3
#define CA_LINE_IO4                     4
#define CA_LINE_IO5                     5
#define CA_LINE_IO6                     6
#define CA_LINE_IO7                     7
#define CA_LINE_K                       CA_LINE_IO1
#define CA_LINE_IO7_REVERSE             0
#define CA_LINE_IO8                     8
#define CA_LINE_IO9                     9
#define CA_LINE_IO10                    10

//Work Voltage
#define CA_12V                          0xFF
#define CA_5V                           0x00

//Voltage Level
#define CA_HIGH                         0xFF
#define CA_LOW                          0x00

//Logic
//Logic
#ifdef CA_NEGATIVE
#undef CA_NEGATIVE
#endif

#define CA_POSITIVE                     0xFF
#define CA_NEGATIVE                     0x00

//L_LINE
#define CA_L_LINE_ENABLE                0xFF
#define CA_L_LINE_DISABLE               0x00

//DTS 这二个宏为在内部使用, 在诊断程序员在设置DTS电压时应使用CA_LOW和CA_HIGH
//#define CA_DTS_ENABLE                                       0xFF
//#define CA_DTS_DISABLE                                      0x00
//CHECK_BIT
#define CA_ODD                          0x01
#define CA_EVEN                         0x02
#define CA_ZERO //??

#define CA_ONE //??

#define CA_NONE                         0x00

#define CA_2_STOP_BIT                   0x03
#define CA_IDFDATA_COM                  0x04

//RECEIVE PACKAGE
#define CA_BY_LENGTH
#define CA_BY_OVERTIME
#define CA_BY_END_CONDITION

//other
#define CA_AUTO_RECOGNIZE               0
#define CA_ONCE_ONLY                    0xFF
#define CA_REPEAT_UNTIL_NEW_COMMAND_BREAK 0

#define CA_IS_STATUS                    0xFF
#define CA_FOK                          0x00
#define CA_FNG                          0x01
#define CA_ECU_BREAK                    0x02

#define CA_IS_DATA                      0x00
#define CA_RECEIVE_END_FLAG             -1

#define SET_ENTER_FRAME_3000H           0x00
#define SET_ENTER_FRAME_3001H           0x01


typedef struct
{
//select line
uint8_t ucInputLine;
uint8_t ucOutputLine;
uint8_t ucEcuCommunicationVoltage;
uint8_t ucLogic;
uint8_t bEnableLLine;
uint8_t ucDTS_Volage;
uint8_t ucCommType;

//bps
float fBps;
uint8_t ucStartBit;
uint8_t ucDataBit;
uint8_t ucStopBit;
uint8_t ucCheckBitWay;

//flash code way
uint8_t bReadingFlashCode;
uint8_t ucReadFlashCodeWay;
} SMARTBOX_CAL;

//COMMUNICATION CONTRAL & SET PARAMETER
int32_t SetDiagunBPS(int32_t iBaut);
uint8_t OpenStdCommDll(void);
int8_t CanInitialize(uint8_t ucCanBitRate, uint8_t * pucFilterId);
int32_t BeginCombination(void);
int32_t EndCombination(void);
void SetCommunicationMaximumWaitTime(int32_t iCommunicationMaximumWaitTime);
int32_t SelectIoLine(uint8_t ucInputLine, uint8_t ucOutputLine);
int32_t EnableLLine(int8_t bEnableLLine);
int32_t SetEcuCommunicationVoltageLogic(uint8_t ucWorkVoltage, uint8_t ucLogic);
int32_t SetDtsVoltage(uint8_t ucVoltageLevel);
int32_t SetEcuBaudRate(float fBps);

#define SetEctBaudRate(fBps)            SetEcuBaudRate(fBps)
uint8_t Can2CheckXOR(uint8_t * pDataBuffer, int32_t iDataLen);
int32_t SetCanBus2LinkKeep(uint8_t * pLinkCommand,
    int32_t iCommandLength,
    int32_t iLinkCommandReturnLength,
    int32_t iLinkKeepTime);

int32_t SetCan2BusLinkKeep(uint8_t * pLinkCommand,
    int32_t iCommandLength,
    int32_t iLinkCommandReturnLength,
    int32_t iLinkKeepTime);

int32_t SetCommunicationParameter(uint8_t ucStartBit,
    uint8_t ucDataBit,
    uint8_t ucStopBit,
    uint8_t ucCheckWay);

//int32_t SetEcuCommTimeInterval(int32_t wByteToByte, int32_t wPacketToPacket, int32_t iOverTime);
int32_t SetEcuCommTimeInterval(float fByteToByte,
    int32_t wPacketToPacket,
    int32_t iOverTime);
int32_t CanSendNReceiveNFrame(int32_t iSendDataLength, uint8_t * pDataToSend, uint8_t * pRecBuffer, int32_t iRecBufferLength);


int32_t SetLinkKeep(uint8_t * pLinkCommand,
    int32_t iCommandLength,
    int32_t iLinkCommandReturnLength,
    int32_t iLinkKeepTime);
int32_t SetCommunicationLineVoltage(uint8_t ucChangeTimes, ...);

int32_t AddressCodeCommunicationWay(uint8_t ucAddressCode,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    float fBps,
    int8_t bRecognizeBaudRate,
    int8_t bSecondByteReverseReturnToECU,
    int32_t iReceiveFrameNumber,
    int32_t iLengthEachFrame);
int32_t VolvoKwpSendDataToEcuGetMultiFrameAnswer(uint8_t * pToEcuData,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iMaximumWaitTime);
int32_t RangRoverSendDataToEcuGetMultiFrameAnswer(uint8_t * pToEcuData,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iMaximumWaitTime);
int32_t LandOldRangeRoverAbsReadFault(uint8_t * pReceiveBuffer, int32_t iLengthReceiveBuffer);

int32_t AddressCodeWay_luhu(uint8_t ucAddressCode,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    float fBps,
    int8_t bRecognizeBaudRate,
    int8_t bSecondByteReverseReturnToECU,
    int32_t iReceiveFrameNumber, ...);
int32_t LandOldRangeRoverAbsEnter(uint8_t ucAddressCode);

int32_t PeugeotSpecialAddressCodeWay(uint8_t ucAddressCode,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    float fBps,
    int8_t bRecognizeBaudRate,
    int8_t bSecondByteReverseReturnToECU,
    int32_t iReceiveFrameNumber, ...);
int32_t CanSendTwoFrameReciveMulFrame(int32_t iSendDataLength,
    uint8_t * pDataToSend,
    uint8_t pFrameNumber,
    uint8_t * pRecBuffer,
    int32_t iRecBufferLength);
int32_t WabcoSendDataToEcuSiftBusyFrameAnswer(uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iFrameNumber,
    ...);

int32_t KwpSendDataToEcuGetAnswer_more(uint8_t ucSendDataTimes,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iFrameNumber,
    ...);
int32_t VPW_Init_SendMultiDataToEcuGetMulti(uint8_t ucIDFirst,
    uint8_t ucIDSecond,
    uint8_t ucFrameNum,
    uint8_t ucSendDataTimes);
int32_t AddressCodeWay(uint8_t ucAddressCode,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    float fBps,
    int8_t bRecognizeBaudRate,
    int8_t bSecondByteReverseReturnToECU,
    int32_t iReceiveFrameNumber, ...);
int32_t SetAddressWay_Mabco(uint8_t ucAddress,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer);
int32_t SetLinkKeep_Mabco(uint8_t * pLinkCommand,
    int32_t iCommandLength,
    int32_t iLinkCommandReturnLength,
    int32_t iLinkKeepTime);
int32_t BoschEnterReturnVersion(uint8_t ucAddress, uint8_t * pReceiveBuffer, int32_t iLengthReceiveBuffer);
int32_t BoschEnterReturnVersionAudi(uint8_t ucAddress, uint8_t * pReceiveBuffer, int32_t iLengthReceiveBuffer, uint8_t FlagLLine);
int32_t SetFlashCodeWay(int32_t iWay);
int32_t ReadFlashCode(void);
int32_t SendDataToEcu(uint8_t ucSendTimes, int32_t iFrameNumber, ...);
int32_t OpelCanSendNReceiveNFrameCircle(int32_t iSendFrameNumber, int32_t iRecFrameNumber, int32_t iSendDataLength, uint8_t * pDataToSend,
     uint8_t * pRecBuffer, int32_t iRecBufferLength);

//////////////
int32_t ReceiveDataFromEcu(uint8_t ucMode, uint8_t * pReceiveBuffer, int32_t iLengthReceiveBuffer);
int32_t OneToOne_SendDataToEcuGetAnswer(uint8_t ucSendDataTimes,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iFrameNumber,
    ...);

int32_t OneToOne_Init_SendDataToEcuGetAnswer(uint8_t ucSendDataTimes);
int32_t OneToOne_Add_SendDataToEcuGetAnswer(int32_t iSendToEcuDataLength,
    uint8_t * pSendToEcuData,
    int32_t iLengthReceiveFromEcu,
    ...);
int32_t OneToOne_SendAndReceive_SendDataToEcuGetAnswer(uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer);

int32_t KnowReceiveFrameNumber_SendDataToEcuGetAnswer(int32_t iLengthSendToEcuData,
    uint8_t * pToEcuData,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iReceiveFrameNumber,
    ...);

/*int32_t	Can20SendMoreFrameQuickGetInfoCHERRY(int32_t iSendDataFnum,
                    uint8_t *pDataToSend,
                    uint8_t *pRecBuffer,
                    int32_t iRecBufferLength);*/
int32_t Can20SendMoreFrameQuickGetInfoDFLZY(int32_t iSendFrameNum,
    uint8_t * pDataToSend,
    uint8_t * pRecBuffer,
    int32_t iRecBufferLength,
    uint8_t ucReceiveFrameNumber);
int32_t Can20SendMoreFrameQuickGetInfoJAGUAR(int32_t iSendFrameNum,
    uint8_t * pDataToSend,
    uint8_t * pRecBuffer,
    int32_t iRecBufferLength,
    uint8_t ucReceiveFrameNumber);

//ISO protocol begin
int32_t IsoSendDataToEcuGetMultiFrameAnswer(int32_t iLengthSendToEcuData,
    uint8_t * pToEcuData,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iMaximumWaitTime);
int32_t IsoSendDataToEcuGetAnswer(uint8_t ucSendDataTimes,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iFrameNumber,
    ...);
int32_t IsoInitSendDataToEcuGetAnswer(uint8_t ucSendDataTimes);
int32_t IsoAddSendDataToEcuGetAnswer(int32_t iLengthSendToEcuData,
    uint8_t * pSendToEcuData);
int32_t IsoSendAndReceiveSendDataToEcuGetAnswer(uint8_t * pReceiveBuffer, int32_t iLengthReceiveBuffer);

//
int32_t BoschEnterReturnVersion2(uint8_t ucAddress, uint8_t * pReceiveBuffer, int32_t iLengthReceiveBuffer);

//下面是为兼容保留函数,不建议使用
#define ISO_SendDataToEcuGetAnswer(iLengthSendToEcuData, pToEcuData, pReceiveBuffer, iLengthReceiveBuffer, iMaximumWaitTime) IsoSendDataToEcuGetMultiFrameAnswer(iLengthSendToEcuData,pToEcuData,pReceiveBuffer,	iLengthReceiveBuffer,iMaximumWaitTime)

//ISO protocol end
//KWP protocol begin
int32_t KwpSendDataToEcuGetMultiFrameAnswer(uint8_t * pToEcuData,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iMaximumWaitTime);
int32_t KwpSendDataToEcuGetAnswer(uint8_t ucSendDataTimes,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iFrameNumber,
    ...);
int32_t KwpInitSendDataToEcuGetAnswer(uint8_t ucSendDataTimes);
int32_t KwpAddSendDataToEcuGetAnswer(uint8_t * pSendToEcuData);
int32_t KwpSendAndReceiveSendDataToEcuGetAnswer(uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer);

//下面是为兼容保留函数,不建议使用
#define KWP2000_SendDataToEcuGetMultiFrameAnswer(iLengthSendToEcuData, pToEcuData, pReceiveBuffer, iLengthReceiveBuffer, iMaximumWaitTime) KwpSendDataToEcuGetMultiFrameAnswer(pToEcuData,pReceiveBuffer,iLengthReceiveBuffer,iMaximumWaitTime)
#define KWP2000_Init_SendDataToEcuGetAnswer(iSendDataTimes) KwpInitSendDataToEcuGetAnswer(iSendDataTimes)
#define KWP2000_Add_SendDataToEcuGetAnswer(iLengthSendToEcuData, pSendToEcuData) KwpAddSendDataToEcuGetAnswer(pSendToEcuData)
#define KWP2000_SendAndReceive_SendDataToEcuGetAnswer(pReceiveBuffer, iLengthReceiveBuffer) KwpSendAndReceiveSendDataToEcuGetAnswer(pReceiveBuffer,iLengthReceiveBuffer)
int32_t KWP2000_SendDataToEcuGetAnswer(uint8_t ucSendDataTimes,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iFrameNumber,
    ...);

//KWP protocol end
int32_t Can2SendOneFrameReceiveData(int32_t iMode, int32_t iReqLen, uint8_t * pReqBuffer, uint8_t * pRecBuffer, int32_t iRecLen);

//BOSCH protocol begin
int32_t BoschSendDataToEcuGetMultiFrameAnswer(uint8_t * pSendToEcuData,
    uint8_t ucReceiveFrameNumber,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer);
int32_t BoschSendDataToEcuGetAnswer(uint8_t ucSendDataTimes,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iFrameNumber,
    ...);
int32_t BoschInitSendDataToEcuGetAnswer(uint8_t ucSendDataTimes);
int32_t BoschAddSendDataToEcuGetAnswer(uint8_t * pSendToEcuData);
int32_t BoschSendAndReceiveSendDataToEcuGetAnswer(uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer);

//下面是为兼容保留函数,不建议使用
#define Bosch_SendDataToEcuGetAnswer(pSendToEcuData, iReceiveFrameNumber, pReceiveBuffer, iLengthReceiveBuffer) BoschSendDataToEcuGetMultiFrameAnswer(pSendToEcuData,iReceiveFrameNumber,pReceiveBuffer,iLengthReceiveBuffer)

//BOSCH protocol end
int32_t GetMultiFrameData(uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int8_t bCleanReceiveBuffer);

int32_t PWM_SendDataToEcuGetAnswer(uint8_t ucSendDataTimes,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iFrameNumber,
    ...);
int32_t PWM_Init_SendDataToEcuGetAnswer(uint8_t ucSendDataTimes);
int32_t PWM_Add_SendDataToEcuGetAnswer(int32_t iLengthSendToEcuData,
    uint8_t * pSendToEcuData);
int32_t PWM_SendAndReceive_SendDataToEcuGetAnswer(uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer);



/*int32_t VPW_SendDataToEcuGetMultiFrameAnswer(int32_t iLengthSendToEcuData,
                                uint8_t *pToEcuData,
                                uint8_t *pReceiveBuffer,
                                int32_t iLengthReceiveBuffer,
                                int32_t iMaximumWaitTime);*/
int32_t VPW_SendDataToEcuGetMultiFrameAnswer_GM(uint8_t TimeMode, int32_t iLengthSendToEcuData,
    uint8_t * pToEcuData,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iMaximumWaitTime);

int32_t VPW_SendDataToEcuGetAnswer(uint8_t ucSendDataTimes,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iFrameNumber,
    ...);
int32_t EnableBoschLinkKeep(int8_t bKeepLink);

int32_t SendOneFrameDataToEcuGetAnyFrameAnswer(uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iLengthSendData,
    uint8_t * pSendData,
    int32_t iLengthReceiveData,
    ...);
int32_t AddressCodeWay2(uint8_t ucAddressCode,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    float fBps,
    int8_t bRecognizeBaudRate,
    int8_t bSecondByteReverseReturnToECU,
    int32_t iReceiveFrameNumber, ...);


int32_t HoldenNormalRingLinkCheck(uint8_t ucSendDataTimes, //v2700 zjz add
uint8_t Addr_code,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer);

/*#if 0
int32_t HoldenNormalRingLinkSendDataToEcuGetAnswer(
                uint8_t ucSendDataTimes,
                uint8_t Time_Value,
                int32_t iLengthSendToEcuData,
                uint8_t *pToEcuData,
                uint8_t *pReceiveBuffer,
                int32_t iLengthReceiveBuffer
                );
                */
int32_t HoldenNormalRingLinkSendDataToEcuGetAnswer(uint8_t ucSendDataTimes,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iFrameNumber,
    int32_t iFrameReceive,
    uint8_t bFlag,
    ... //uint8_t head,

//uint8_t Time_Value,
//int32_t iLengthSendToEcuData,
//uint8_t *pToEcuData,
);

/* #endif
int32_t HoldenNormalRingLinkSendDataToEcuGetAnswer(
                uint8_t ucSendDataTimes,
                uint8_t *pReceiveBuffer,
                int32_t iLengthReceiveBuffer,
                int32_t iFrameNumber,
                ...
                //uint8_t head,
                //uint8_t Time_Value,
                //int32_t iLengthSendToEcuData,
                //uint8_t *pToEcuData,
                );

*/
int32_t PWM_SendDataToEcuGetMultiFrameAnswer(int32_t iLengthSendToEcuData,
    uint8_t * pToEcuData,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iMaximumWaitTime);

int32_t PWM_SendDataToEcuGetFrames(int32_t iLengthSendToEcuData,
    uint8_t * pToEcuData,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    uint8_t bFrameCnt);

int32_t SubaruOld_OneToOne_SendDataToEcuGetAnswer(uint8_t ucSendDataTimes,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iFrameNumber,
    ...);

/***********************add it to can+*/
#define CAN_HEAD_LEN                    1
#define CAN_SEND_ONETIME                1
#define CAN_MODE_COMM                   4
#define CAN_LEN                         15
uint8_t CheckXOR(uint8_t * pDataBuffer, int32_t iDataLen);
int32_t CANReceiveOneFrameFromEcu(uint8_t * pAnsBuffer, int32_t iBufferLen);

//#define CAC_CANBUS_LINK_KEEPAC_SEND_1_FRAME_NO_ANSWER
//#define CAC_SEND_1_FRAME_NO_ANSWER						0x13  //Added 20040205 CSL. For CAN BUS.
//#define CAC_CAN_SEND_1_FRAME_QUICK_GET_INFO				0x14  // For CAN bus. read dtc and info.
int32_t SendOneFrameNoAnswer(int32_t siDataLength, uint8_t * pDataToSend); //Added 20040205 CSL. For CAN BUS.
int32_t CanSendOneFrameReceiveDatas(int32_t iMode, int32_t iReqLen, uint8_t * pReqBuffer, uint8_t * pRecBuffer, int32_t iRecLen);
int32_t CanSendOneFrameReceiveDatas_AUDI(int32_t iMode, int32_t iReqLen, uint8_t * pReqBuffer, uint8_t * pRecBuffer, int32_t iRecLen, int32_t iAnsLen);

//	iRet=CanSendOneFrameReceiveDatas(0,12,CommandBuff,ReceiveBuffer,256,0);
int32_t CanSendOneFrameReceiveDatasVW(int32_t iMode, int32_t iReqLen, uint8_t * pReqBuffer, uint8_t * pRecBuffer, int32_t iRecLen, int32_t iAnsLen); // //add for VW CAN 2007.11.06
int32_t CanSendTwoFrameQuickGetInfo(int32_t iSendDataLength, uint8_t * pDataToSend, uint8_t * pRecBuffer, int32_t iRecBufferLength);
int32_t SetCanBusLinkKeep(uint8_t * pLinkCommand,
    int32_t iCommandLength,
    int32_t iLinkCommandReturnLength,
    int32_t iLinkKeepTime);
int32_t Mabco_Abs_Clean(int32_t iLengthSendToEcuData,
    uint8_t * pToEcuData,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer);
int32_t Mabco_SendOneData(uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iFrameNumber,
    ...);
int32_t SendOneFrameDataToEcuGetAnyFrameAnswer_Check(uint8_t ucSendDataTimes,
    int32_t iCheckTime,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iLengthSendData,
    uint8_t * pSendData,
    int32_t iLengthReceiveData,
    ...);
int32_t Maboc_SetLinkKeep(uint8_t * pLinkCommand,
    int32_t iCommandLength,
    int32_t iLinkCommandReturnLength,
    int32_t iLinkKeepTime);

int32_t SendAddressCodeGetKeyword(uint8_t ucAddress,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer);


int32_t AddressCodeWay_Opel(uint8_t ucAddressCode,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    float fBps,
    int8_t bRecognizeBaudRate,
    int8_t bSecondByteReverseReturnToECU,
    int32_t iReceiveFrameNumber, ...);
int32_t OneToOne_SendDataToEcuGetAnswer_Nissan(uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iPosition,
    int32_t iAddorDecNumber);
int32_t HoldenCanSendOneFrameReceiveDatas(uint8_t ucSendDataTimes,
    int32_t iReceiveFrameNumber,
    int32_t iLengthSendToEcuData,
    uint8_t * pToEcuData,
    int32_t iLengthReceiveBuffer,
    uint8_t * pReceiveBuffer);
int32_t VPW_HDN_SendDataToEcuGetMultiFrameAnswer(int32_t iLengthSendToEcuData,
    uint8_t * pToEcuData,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iMaximumWaitTime);

int32_t VPW_Init_SendMultiDataToEcuGetAnswer_HDN(uint8_t ucSendDataTimes);

int32_t SetCommunicationLineVoltage_Nissan(uint8_t ucChangeTimes, ...);


/****************/
int32_t FordIsoSendDataToEcuGetMultiFrameAnswer(uint8_t * pToEcuData,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iMaximumWaitTime);

int32_t CanSendTwoFrameQuickGetInfo_Connector2(int32_t iSendDataLength,
    uint8_t * pDataToSend,
    uint8_t * pRecBuffer,
    int32_t iRecBufferLength);

int32_t FordIsoSendDataToEcuGetMultiFrameAnswer_ZJZ(uint8_t ucSendDataTimes, //v1403 zjz
uint8_t * pToEcuData,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iMaximumWaitTime);

int32_t VPW_Init_Set_RecTime(uint8_t time, uint8_t Voltage);
int32_t VWCANSendDataToEcuGetMultiFrameAnswer1(int32_t iLengthSendToEcuData,
    uint8_t * pToEcuData,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    uint8_t AddressHigh,
    uint8_t AddressLow);

int32_t VWCANSendDataToEcuGetMultiFrameAnswer2(int32_t iLengthSendToEcuData,
    uint8_t * pToEcuData,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    uint8_t AddressHigh,
    uint8_t AddressLow);

int32_t AUDI_READMIMA_OneToOneSendData(uint8_t ucSendDataTimes,
    int32_t LowVolgatetime,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iFrameNumber,
    ...);

int32_t CanSendTwoFrameQuickGetInfo_New(int32_t iSendDataLength,
    uint8_t * pDataToSend,
    uint8_t * pRecBuffer,
    int32_t iRecBufferLength);

//int32_t	CanSendMulFrameGetMulFrame(int32_t iSendDataLength, uint8_t *pDataToSend, uint8_t *pRecBuffer,int32_t iRecBufferLength);
int32_t CanSendMulFrameGetMulFrame(int32_t iSendDataLength, uint8_t * pDataToSend, uint8_t * pRecBuffer, int32_t iRecBufferLength);

int32_t CanSendA2_ModeFrameGetMulFrame(int32_t iSendDataLength, uint8_t * pDataToSend, uint8_t * pRecBuffer, int32_t iRecBufferLength);


int32_t Can2SendTwoFrameQuickGetInfo(int32_t iSendDataLength, uint8_t * pDataToSend, uint8_t * pRecBuffer, int32_t iRecBufferLength);

//int32_t	Can2SendTwoFrameQuickGetFrameNumInfo(int32_t iSendDataLength, uint8_t *pDataToSend, uint8_t *pRecBuffer,int32_t iRecBufferLength);
int32_t Can20SendTwoFrameQuickGetInfo(int32_t iSendDataLength, uint8_t * pDataToSend, uint8_t * pRecBuffer, int32_t iRecBufferLength);
int32_t Can20SendAnyFrameQuickGetInfo(int32_t iFrameNumber, int32_t iSendDataLength, uint8_t * pDataToSend, uint8_t * pRecBuffer,
     int32_t iRecBufferLength);

int32_t SelectIoLine_New(uint8_t ucInputLine, uint8_t ucOutputLine);

int8_t EnableCanFunction(int8_t bLinstenOnly);

//int8_t VolvoCanInitialize(uint8_t ucCanBitRate, uint8_t *pucFilterId);
int8_t SetCanFilterId(int8_t bExtendedFrame, uint8_t * pucFilterId);
int32_t Can2SendTwoFrameQuickGetInfoGM(int32_t iSendDataLength, uint8_t * pDataToSend, uint8_t * pRecBuffer, int32_t iRecBufferLength,
     uint8_t bFrameNum);
int8_t EnableListenOnly(int8_t bLinstenOnly);
int8_t SetMaskFilterId(uint8_t * pucFilterId);
int8_t SetMaskFilterDoubleId(uint8_t * pucMaskFilterId);
int8_t SetCanIOLine(int8_t bNormal);
int8_t SetCanBitRate(int8_t bExtendedFrame, uint8_t ucRate);

int32_t Can2SendTwoFrameQuickGetFrameNumInfo(int32_t iSendDataLength, uint8_t * pDataToSend, uint8_t * pRecBuffer, int32_t iRecBufferLength,
     uint8_t bFrameNum);
int32_t JAGUARPWM_SendDataToEcuGetMultiFrameAnswer(uint8_t * pPWMFilterID,
    int32_t iLengthSendToEcuData,
    uint8_t * pToEcuData,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iMaximumWaitTime);

int32_t CanIISendMultiFrameQuickGetInfo(int32_t iFrameNumber, int32_t iSendDataLength, uint8_t * pDataToSend, uint8_t * pRecBuffer,
     int32_t iRecBufferLength);
int32_t CanIISendTwoFrameQuickGetInfo(int32_t iSendDataLength, uint8_t * pDataToSend, uint8_t * pRecBuffer, int32_t iRecBufferLength);
int32_t SelectIoLine_38(uint8_t ucCommLine);
int32_t SetCan20LinkKeep_NoAnswer(uint8_t SwitchValue);

int32_t CanIISendTwoFrameQuickGetInfo_CTN(int32_t iSendDataLength,
    uint8_t * pDataToSend,
    uint8_t * pRecBuffer,
    int32_t iRecBufferLength);

int32_t SetCan20LinkKeep(uint8_t * pLinkCommand,
    int32_t iCommandLength,
    int32_t iLinkCommandReturnLength,
    int32_t iLinkKeepTime);

int32_t SendDataToSmartBoxAndWaitForRightData(uint8_t * pSendBuffer,
    int32_t iSendDataLength,
    uint8_t * pReceiveBuffer,
    int32_t iReceiveBufferLength,
    int32_t iMaxWaitTime);
int32_t ReadByteBytedata(uint8_t ucMode, uint8_t * pReceiveBuffer, int32_t iLengthReceiveBuffer);

//20080505 add
int32_t SetCANLinkKeep20(uint8_t * pLinkCommand,
    int32_t iCommandLength,
    int32_t iLinkCommandReturnLength,
    int32_t iLinkKeepTime);

int32_t ToyotaKwpSendDataToEcuGetMultiFrameAnswer(int32_t wBusyFrameSum,
    uint8_t * pToEcuData,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iMaximumWaitTime);

int32_t yatai_BoschEnterReturnVersion(uint8_t ucAddress,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer);
int32_t SAE_J1708_SendDataToEcuGetAnswer(int32_t iSendDataLength, uint8_t * pDataToSend, uint8_t * pRecBuffer, int32_t iRecBufferLength);
int32_t OneToOne_SendDataToEcuGetAnswer_Can20(uint8_t ucSendDataTimes,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iFrameNumber,
    ...);

int32_t CanSendTwoFrameQuickGetInfo_NewConnector(int32_t iSendDataLength, uint8_t * pDataToSend, uint8_t * pRecBuffer, int32_t iRecBufferLength);
int32_t CumminsDieselSAE1708_SendMultiFramesToEcuGetAnswer(uint8_t ucSendDataTimes,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iFrameNumber,
    uint8_t * pTemp);

int32_t CumminsDieselSAE1708_SendDataToEcuGetAnswer(uint8_t ucSendDataTimes,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iFrameNumber,
    ...);

int32_t SetCanLinkKeep_Mit(int32_t iFrameNumber, uint8_t * pLinkCommand1,
    int32_t iCommandLength1,
    int32_t iLinkCommandReturnLength1,
    uint8_t * pLinkCommand2,
    int32_t iCommandLength2,
    int32_t iLinkCommandReturnLength2,
    int32_t iLinkKeepTime);

int32_t SetCanIIBusLinkKeep(uint8_t * pLinkCommand,
    int32_t iCommandLength,
    int32_t iLinkCommandReturnLength,
    int32_t iLinkKeepTime);
int32_t SetEnterFrame(uint8_t * pLinkCommand,
    int32_t iCommandLength);

//int32_t SetEnterFrameExt(uint8_t ucMode, uint8_t *pEnterFrameCommand,
//				int32_t iCommandLength);
int32_t CanSendMulFrameGetMulFrameGN(int32_t iSendDataLength,
    uint8_t * pDataToSend,
    uint8_t * pRecBuffer,
    int32_t iRecBufferLength);


int32_t GMOPEL_CanSendMulFrameGetMulFrame(int32_t iSendFrameNum,
    int32_t iRecFrameNum,
    int32_t iFrameIdNum,
    int32_t iSendDataLen,
    uint8_t * pDataToSend,
    uint8_t * pRecBuf,
    int32_t iRecBufMaxLen);

int32_t ReadByteBytedataLand(uint8_t ucMode, uint8_t * pReceiveBuffer, int32_t iLengthReceiveBuffer);

int32_t SelectCommMode(uint8_t ucCommType); //20080801

int32_t VPW_SendDataToEcuGetMultiFrameAnswer(uint8_t ucIDFirst,
    uint8_t ucIDSecond,
    uint8_t ucFrameNum,
    int32_t iLengthSendToEcuData,
    uint8_t * pToEcuData,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer);

//int8_t SetCanIOLine(int8_t bNormal);
//int8_t SetCanFilterId(int8_t bExtendedFrame, uint8_t *pucFilterId);
//int8_t SetCanBitRate(int8_t bExtendedFrame, uint8_t ucRate);
//int8_t SetMaskFilterId(uint8_t *pucFilterId);
void ResetCanController(void);

//int8_t EnableCanFunction(int8_t bLinstenOnly);
//int8_t EnableListenOnly(int8_t bLinstenOnly);
//int8_t CanInitialize(uint8_t ucCanBitRate, uint8_t *pucFilterId);
int32_t OneToOne_SendLongDataToEcuGetLongAnswer(int32_t iSendDataLength,
    uint8_t * pDataToSend,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer);
int32_t OneToOne_SendOneByteToEcuGetAnswerLM(int32_t iSendDataLength,
    uint8_t * pDataToSend,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer);

/*
int32_t LFYBOSCHMP73AddressCodeWay(
                                uint8_t ucAddressCode,
                                uint8_t *pReceiveBuffer,
                                int32_t iLengthReceiveBuffer,
                                float fBps,
                                int8_t bRecognizeBaudRate,
                                int8_t bSecondByteReverseReturnToECU,
                                int32_t iReceiveFrameNumber,...
                                );
*/
//int32_t Can2SendTwoFrameQuickGetFrameNumInfo(int32_t iSendDataLength, uint8_t *pDataToSend, uint8_t *pRecBuffer,int32_t iRecBufferLength);
int32_t BenzReadByteFromECU(int32_t iSendDataLength, uint8_t * pDataToSend, uint8_t ucMode, uint8_t * pReceiveBuffer, int32_t iLengthReceiveBuffer);
int32_t GMSendMultiFrameToECUGetAnswer(int32_t iSendDataLength, uint8_t * pDataToSend, uint8_t * pReceiveBuffer, int32_t iLengthReceiveBuffer);
int32_t CHRYSLER_OneToOne_SendDataToEcuGetAnswer(uint8_t ID1, uint8_t ID2, uint8_t ucSendDataTimes,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iFrameNumber,
    ...);
int32_t ExtReceiveData(uint8_t * pReceiveBuffer, int32_t iBufferLength);
int32_t AddressCodeWayAdjustTime(uint8_t ucAddressCode,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    float fBps,
    int8_t bRecognizeBaudRate,
    uint8_t iTimeKW1Before,
    uint8_t iTimeSendKW2ToEcu,
    int8_t iLLineSwitch,
    int8_t bSecondByteReverseReturnToECU,
    int32_t iReceiveFrameNumber, ...);
int32_t EndCombinationBuffer(uint8_t * ucRecBuffer, int32_t iLength);



int32_t HoldenCan20SendNFramesReceiveNDatas(uint8_t ucSendDataTimes,
    int32_t iSendFrameNumber,
    int32_t iReceiveFrameNumber,
    int32_t iLengthReceiveBuffer,
    uint8_t * pReceiveBuffer,
    ... //int32_t iLengthSendToEcuData,

//uint8_t *pToEcuData
);

int32_t KnowFramesCanSendDataToECUGetAnswer(int32_t iSendDataLength,
    int32_t iReqFrames,
    uint8_t * pDataToSend,
    uint8_t * pRecBuffer,
    int32_t iRecBufferLength,
    int32_t iAskFrames);
int32_t PorscheCanSendMulFrameGetMulFrame(int32_t iSendDataLength,
    uint8_t * pDataToSend,
    uint8_t * pRecBuffer,
    int32_t iRecBufferLength);
int32_t AddressCodeWayChoiceMode(uint8_t ucAddressCode,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    uint8_t iMode,
    int8_t bRecognizeBaudRate,
    int8_t bSecondByteReverseReturnToECU,
    int32_t iReceiveFrameNumber, ...);
int32_t Micas_SendDataToEcuGetAnswer(uint8_t ucSendDataTimes,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iFrameNumber,
    ...);
void Super16ResetConnector(void); //09.02.20 by xja

int32_t SetBenzHMFLinkKeep(uint8_t * pLinkCommand,
    int32_t iCommandLength,
    int32_t iLinkCommandReturnLength,
    int32_t iLinkKeepTime);

int32_t KwpSendDataToEcuGetAnswer_benz(uint8_t ucSendDataTimes,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iFrameNumber,
    ...);

int32_t AddressCodeWayAdjustTime_benz(uint8_t ucAddressCode,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    float fBps,
    int8_t bRecognizeBaudRate,
    uint8_t iTimeKW1Before,
    uint8_t iTimeSendKW2ToEcu,
    int8_t iLLineSwitch,
    int8_t bSecondByteReverseReturnToECU,
    int32_t iReceiveFrameNumber, ...);

int32_t AddressCodeCommunicationWay_Lline(uint8_t ucAddressCode,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    float fBps,
    int8_t bRecognizeBaudRate,
    int8_t bSecondByteReverseReturnToECU,
    int32_t iReceiveFrameNumber,
    int32_t iLengthEachFrame);

int32_t KwpSendDataToEcuGetMultiFrameAnswer_benz(uint8_t * pToEcuData,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iMaximumWaitTime);

int32_t TestSmartboxExternMemory_ALL(void);
int32_t GetSmartBoxVersion_ALL(uint8_t * pReceiveBuffer, int32_t iLengthReceiveBuffer);
int32_t BreakCommWithECM(void);

int32_t SendOneFrameToEcuGetMultiFrameAnswer(uint8_t * pToEcuData,
    int32_t iLengthSendToEcuData,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iLengthReceiveWant,
    int32_t iMaximumWaitTime);
int32_t CumminsSAE1939_SendMultiFramesToEcuGetAnswer(uint8_t ucSendDataTimes, int32_t iSendDataLength, uint8_t * pDataToSend, uint8_t * pRecBuffer,
     int32_t iRecBufferLength);
int32_t KwpSendDataToEcuGetAnswer_BMW(uint8_t ucSendDataTimes,
    uint8_t * pReceiveBuffer,
    int32_t iLengthReceiveBuffer,
    int32_t iFrameNumber,
    ...);
int32_t SAE1939Function(uint8_t * pRecBuffer, int32_t iRecBufferLength, uint8_t Mode);

// 0x6225 yaoming add 20100628 for zhanwei
int32_t CanSendMulFrameGetMulFrameGN_BENZ(int32_t iSendDataLength,
    uint8_t * pDataToSend,
    uint8_t * pRecBuffer,
    int32_t iRecBufferLength);
int32_t CanSendMulFrameGetMoreSingleFrame(int32_t iSendDataLength, uint8_t * pDataToSend, uint8_t * pRecBuffer, int32_t iRecBufferLength,
     int32_t iRecFrameNumber);

/****************************************************************
*							*
*	功  能：测试DTS线     				*
*							*
*	参数：DTSPin	选择DTS线引脚                   *
*							*
*	      pReceiveBuffer	接收缓冲区指针          *
*							*
*             iLengthReceiveBuffer接收缓冲区长度        *
*							*
*	返回值：成功 1        			    	*
*		失败	0   				*
*                                                       *
*							*
****************************************************************/
//yaoming add 20100715 GX3/DIAGUN 用于IO测试的DTS线测试
int32_t TestDTS(uint8_t DTSPin, uint8_t * pRecBuffer, int32_t iRecBufferLength);

uint8_t OpenStdCommDll(void);
uint8_t CloseStdCommDll(void);

//------------------------------------------------------------------------------------------------
// Funtion: 设置 CAN 请求帧间距
// Input: Waittime - 请求帧间距
// Output: none
// Return: ID_FOK
// Other: 6020
//------------------------------------------------------------------------------------------------
uint8_t SetCanRequestFrametime(uint8_t Waittime);

//------------------------------------------------------------------------------
// Funtion: IO线及参数控制
// Input  :
// Output : none
// Return : 成功         GD_OK
//          失败         出错代码
// Info   : none
//------------------------------------------------------------------------------
int32_t SetIoParameter(void);

//------------------------------------------------------------------------------
// Funtion:
// Input  :
// Output :
// Return : 成功    实际接收到的总长度
//          失败    出错代码(小于0)
// Info   :
//------------------------------------------------------------------------------
int32_t CalculateKwpCommandLength(uint8_t * pSendToEcuCommand);
int32_t CalculateKwpCommandLength_BMW(uint8_t * pSendToEcuCommand);

int32_t SetEnterFrameExt(uint8_t ucCan30ConfigMode, uint8_t * pEnterFrameCommand,
    int32_t iCommandLength);

#endif



