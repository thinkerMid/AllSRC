

//------------------------------------------------------------------------------
//  Purpose: 指令函数集通用函数 （中断函数和特殊函数，对上层提供支持）
//  Funtion: 对 FUN_CAN FUN_KLine FUN_SetLinePara 提供支持
//  Dependent:
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef FUN_Base_H
#define FUN_Base_H
#include "CMPHY_Common.h"
#include "CMPHY_Com.h"
#include "CMPHY_IOCtrl.h"
#include "CMPHY_Timer.h"
#include "CMPHY_CAN.h"
#include "CMPHY_Com.h"
#include "CMPHY_IOCtrl.h"
#include "CMPHY_PWM.h"
#include "CMPHY_VPW.h"
#include "CMPHY_Timer.h"
#include "CMPHY_Relay.h"

//#include "CMP_ISO14230.h"
#define INPUTCHANNEL            1
#define OUTPUTCHANNEL           2
#define CAN_H                   3
#define CAN_L                   4
#define COMMTYPE                5
#define LOGICTYPE               6
#define LEVELTYPE               7
#define LLINE                   8
#define DTS                     9
#define BAUDRATE                1
#define WORDLENGTH              2
#define STOPBITS                3
#define PARITY                  4
#define ID_FOK                  0x02
#define ID_FNG                  0x02
#define ID_FBREAK               0x02
#define KEEPLINK_CAN_GENERAL    1
#define KEEPLINK_MITSUBISHI     2
#define KEEPLINK_BENZ           3
#define KEEPLINK_ACCESS         4
#define KEEPLINK_BOSCH          5

//Define Of KW2000.State
#define CMLISO14230_WAITING     0          //等待状态
#define CMLISO14230_SENDING     1          //发送状态
#define CMLISO14230_READING     2          //接收状态
#define CMLISO14230_SENDFLA     3          //发送完毕

//Define Of KWP1281.State
#define TOOL_WAITING            5      //等待状态
#define TOOL_SENDING            1      //发送状态
#define TOOL_READING            2      //接收状态
#define TOOL_RECEREVERSE        3      //接收取反
#define TOOL_SENDREVERSE        4      //发送取反
#define UART_ODD_PARITY         0x01
#define UART_EVEN_PARITY        0x02
#define UART_2STOPBITSFIX       0x03
#define UART_2STOPBITSVAR       0x04
#define LINK_OPEN               1
#define LINK_OFF                0
#define TRUE                    1
#define FALSE                   0

//#define NULL                  0
typedef struct //模块数据结构
{
volatile unsigned char m_chState;                                                                                                                           //当前通讯状态
volatile int    m_nMScount;                                                                                                                                 //MS记数
int             m_nFrameTime;                                                                                                                               //帧发送前的间隔时间
int             m_nBtyetime;                                                                                                                                //字节间距
int             m_Idletime;                                                                                                                                 //链路发生时间
int             m_ReserveTime;                                                                                                                              //取反时间
int             m_Maxwaittime;                                                                                                                              //最大等待时间
unsigned char   m_chTemp[50];                                                                                                                               //目标数据包
unsigned char * m_Senddata;                                                                                                                                 //待发送数据包
unsigned char   m_chHoldDig[20];                                                                                                                            //链路保持数据包
unsigned char   m_chHoldDataLen;                                                                                                                            //链路保持数据长度
unsigned char   m_LinkFG;                                                                                                                                   //链路标记
unsigned char   m_Framecount;                                                                                                                               //帧计数
unsigned char   m_Reverse;                                                                                                                                  //取反回复字节
volatile unsigned char m_chCount;                                                                                                                           //当前字节计数
unsigned char   m_Lenth;                                                                                                                                    //长度标识
unsigned char   m_SendFinishFlag;                                                                                                                           //用户区起始移位
unsigned char   m_PakMode;                                                                                                                                  //回复格式
} SC_CML_KWP1281;


typedef struct //模块数据结构
{
volatile unsigned char m_chState;                                                                                                                           //当前通讯状态
unsigned char   m_PakType;                                                                                                                                  //数据包格式
volatile int    m_nMScount;                                                                                                                                 //MS记数
int             m_nFrameTime;                                                                                                                               //帧发送前的间隔时间
int             m_nBtyetime;                                                                                                                                //字节间距
int             m_Idletime;                                                                                                                                 //链路发生时间
int             m_Maxwaittime;                                                                                                                              //最大等待时间
int             m_VehFrametime;                                                                                                                             //多包超时时间
int             m_nHoldLen;                                                                                                                                 //链路保持数据包长度
unsigned char   m_chHoldDig[50];                                                                                                                            //链路保持数据包
unsigned char   m_chTemp[264];                                                                                                                              //目标数据包
volatile int    m_chCount;                                                                                                                                  //当前字节计数
unsigned char   m_Lenth;                                                                                                                                    //长度标识
unsigned char   m_PakMode;                                                                                                                                  //回复格式
} SC_CML_ISO14230;


typedef struct //模块数据结构
{
volatile unsigned char m_chState;                                                                                                                           //通讯状态
volatile int    m_nMScount;                                                                                                                                 //MS记数

//int           m_nFrameTime;         //帧发送前的间隔时间
//int           m_Idletime;           //链路发生时间
//int           m_Maxwaittime;        //最大等待时间
//int           m_VehFrametime;       //多包超时时间
CanTxMsg   m_chHoldDig;                                                                                                                                //链路保持发送数据包

CanTxMsg m_TXM;                //发送数据包
CanRxMsg   m_RXM;                                                                                                                                      //接收数据包

//unsigned char m_Lenth;              //长度标识
//unsigned char m_PakType;            //包属性
//CAN_CONFIGPARA CAN_Init;            //CAN初始化数据
unsigned char   m_PakMode;                                                                                                                                  //回复格式
} SC_CML_ISO15765;


typedef struct //数据包传输参数
{
CAN_PORT        CanPort;
unsigned char   PakType;                                                                                                                                    //数据包类型 注意数据包类型中允许出现

//CMPISO11898_BASISFRAME | CMPISO11898_RFBFRAME
//即远程标准帧
unsigned long   Address;                                                                                                                                    //数据包ECU地址,需要移位，右对齐方式
unsigned char   FilterNum;                                                                                                                                  //过滤 ID 个数
unsigned long   ECUID[8];                                                                                                                                   //数据包过滤ID 无过滤时请用 0
unsigned char   PakMode;                                                                                                                                    //回复格式
} SC_CMLISO15765para;


typedef struct
{
unsigned char   linkType;//29F8
unsigned int    IdleTime;//29FC
unsigned char   rightState;//2A00
unsigned int    timeCount;//2A04
unsigned char   idleSendLen;//2A08
unsigned char   idleRecvLen;//2A09
unsigned char   dataBuf[0x30];//2A0B+30 2A29
unsigned char   chCount;//2A3D
unsigned char   linkState;//2A3E
} SET_LINK_KEEP;


typedef struct
{
unsigned int    PakLenth;                                                                                                                                   // 接收数据包长度
unsigned char * Pakdata;                                                                                                                                    // 接收数据包数据
} SC_PAKRECV;


typedef struct //流控制帧
{
unsigned char   dataLenth;                                                                                                                                  //流控制帧数据包长度
unsigned char   data[30];                                                                                                                                   //流控制帧数据包
} SC_CMLISO15765ENTERFRAME;


// 接收数据包用户区结构
#define SC_PAKSEND              SC_PAKRECV
extern unsigned char szFOK[4];
extern unsigned char szFNG[4];
extern unsigned char szFBREAK[4];
extern unsigned char szFNOWAIT[4];
extern unsigned char szErrorFrame[7];
extern unsigned char timer_open_flag;
extern COM_PORTMODE SC_com_portMode;
extern CMPHY_Relay SC_RelayState; //继电器相关数据参数
extern SC_CML_ISO14230 SC_TagKWP2000;
extern SC_CML_ISO15765 SC_TagISO15765;
extern SC_CML_KWP1281 SC_TagKWP1281;
extern SC_CMLISO15765para CAN_SetPara;
extern CAN_CONFIGPARA PHY_CAN_SetPara;
extern CAN_RXMessage Can_RXM;
extern SC_CMLISO15765ENTERFRAME CanEnterFrame;
extern SET_LINK_KEEP Keep_Link;
extern unsigned char receive_buff[261];

//------------------------------------------------------------------------------
//函数名称:CANUnPacket
//函数的功能:实现的是把接收到得canbus数据包,以数组的形式进行存储
//函数的参数:buff转换后的存储区
//           candata从can总线接受的数据
//函数的返回值：接受的数据长度.
//------------------------------------------------------------------------------
unsigned char CANUnPacket(unsigned char * buff, CAN_RXMessage rx_data);

//------------------------------------------------------------------------------
//函数名称:ReadOneFrameFromCANBuf_Ext
//函数的参数:*ch接受的数据
//           IDNum接受的ID数
//           specialdata特殊数字//包括0x30,0x7f,0x78
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
    unsigned int timeout);

//-------------------------------------------------------------------------------
//函数的功能是:发送一帧数据到ECU
//函数参数: uint8_t *BUFF  发送缓冲区 0x08,0xid1,0xid2,0x.......
//          uint8_t  IDNum 发送的ID个数//在实际的处理中只有扩展帧和标准帧两种.
//          uint8_t *specialdata 用来存储标志位
//函数的返回值：true/false
//--------------------------------------------------------------------------------
unsigned int CANSendOneFrame_Ext(unsigned char * buff, unsigned char IDNum, unsigned char * specialdata);

//------------------------------------------------------------------------------
// Funtion: 关闭定时器，清空计数器
// Input  : 无
// Output : 无
// Return : void
// Info   :
//------------------------------------------------------------------------------
void set_time0Stop(void);

//recmode = 0 : normal --- 80, 8x, 0x
//          1 : C0
//          2 : 00 xx(>80)
unsigned char ReceiveOneKwpFrameFromECUHasMode(unsigned char recmode, unsigned char * buff, unsigned int * size, unsigned int overtime);

//------------------------------------------------------------------------------
// Funtion: 检测总线是否空闲
// Input  : checkCount:检测的次数，  checkTimeMS:每次检测的时间(单位毫秒)
// Output :
// Return : 总线忙:返回TRUE，否则返回FALSE
// Info   :
//------------------------------------------------------------------------------
unsigned char CheckIoBusy(unsigned char checkCount, unsigned char checkTimeMS);
unsigned char GetKwpFrameBusyFlag(unsigned char * buff, unsigned char * busyid);

//------------------------------------------------------------------------------
// Funtion: 设置canbus通信引脚
// Input  : 无
// Output : 无
// Return : void
// Info   :
//------------------------------------------------------------------------------
void SetCanCommunicatioIO(unsigned char * dataPos);

//------------------------------------------------------------------------------
// Funtion: 设置回复给上位机的数据包
// Input  : data - 回复给上位机的数据
//          Ans - 回复给上位机的数据包
//          dataLenth - 数据包的长度
// Output : ans - 回复到上位机的数据
// Return : 无
// Info   :
//------------------------------------------------------------------------------
void PutDataToAns(unsigned char * data, unsigned char * Ans, int dataLenth);

//------------------------------------------------------------------------------
// Funtion: 设置继电器参数
// Input  : relayVal - 要修改的参数的值
//          relayType - 要修改的参数的类型
// Return : 回复到上位机的数据长度
// Info   :
//------------------------------------------------------------------------------
int SetComPara(unsigned char comVal, unsigned char comType);

//------------------------------------------------------------------------------
// Funtion: 设置继电器参数
// Input  : relayVal - 要修改的参数的值
//          relayType - 要修改的参数的类型
// Return : 回复到上位机的数据长度
// Info   :
//------------------------------------------------------------------------------
int SetRelayPara(unsigned char relayVal, unsigned char relayType);

//------------------------------------------------------------------------------
// Funtion: 进行异或校验检查
// Input  : datalenth - 进行校验的数据长度
//          dataPos - 进行校验的数据头指针
// Output :
// Return : 校验正确返回TRUE,否则返回FALSE
// Info   :
//------------------------------------------------------------------------------
int checkXOR_Parity(int datalenth, unsigned char * dataPos);

//------------------------------------------------------------------------------
// Funtion: 将整帧的CANBUS链路维持数据解包放入指定的结构体中
// Input  : 无
// Output : 无
// Return : void
// Info   : Keep_Link.dataBuf 55,aa,0f,61,01,88,c6,d0,87,88,02,3e,80,ff,ff,ff,ff,ff,bd,0d,c8,44
//------------------------------------------------------------------------------
void setCanbusLinkDataToken(void);

//------------------------------------------------------------------------------
// Funtion: 普通协议链路保持中断处理函数
// Input  : 无
// Output : 无
// Return : void
// Info   :
//------------------------------------------------------------------------------
void SC_setlinkkeep(void);

//------------------------------------------------------------------------------
// Funtion: 使能L线
// Return : 无
// Info   :
//------------------------------------------------------------------------------
void EnableLLine_1(void);

//------------------------------------------------------------------------------
// Funtion: 禁能L线
// Return : 无
// Info   :
//------------------------------------------------------------------------------
void DisableLLine_1(void);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Funtion: 检查L线是否打开
// Return : 无
// Info   :
//------------------------------------------------------------------------------
unsigned char CheckLLine(void);

// Funtion: 设置CANBUS链路保持
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 600d
//------------------------------------------------------------------------------
int SetCANBus2LinkData_1(int argc, unsigned char * argv, unsigned char * ans);

//------------------------------------------------------------------------------
// Funtion: KWP1281 定时器中断
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 600d
//------------------------------------------------------------------------------
void SC_CML_KWP1281_Time(void);

//------------------------------------------------------------------------------
// Funtion: KWP1281 协议串口发送中断函数,串口发送完一个字节后产生的中断
// Input  : none
// Output : none
// Return : none
// Info   : 该函数仅对内使用
//------------------------------------------------------------------------------
void SC_CML_KWP1281_Comsend(void);

//------------------------------------------------------------------------------
// Funtion: KWP1281 协议串口接收中断函数
// Input  : none
// Output : none
// Return : none
// Info   : 该函数仅对内使用
//------------------------------------------------------------------------------
void SC_CML_KWP1281_Comread(void);

//------------------------------------------------------------------------------
// Funtion: KWP1281 发送一个数据包
// Input  : PakSend - 待发送数据
// Output : none
// Return : 发送总线仲裁失败时返回 false
// Info   : none
//------------------------------------------------------------------------------
int SC_CML_KWP1281_Send(SC_PAKSEND * PakSend);

//------------------------------------------------------------------------------
// Funtion: KWP1281 接收一个数据包
// Input  :
// Output : Lenth - 数据长度
//          Data - 数据
// Return : 应答码
// Info   : 当返回 Lenth 为
//------------------------------------------------------------------------------
int SC_CML_KWP1281_Recv(SC_PAKRECV * Pakrecv);

//------------------------------------------------------------------------------
// Funtion: 释放动态申请的缓冲区
// Input  : 参数1：PAKRECV结构体指针，参数2：num结构体数量
// Output : none
// Return : 无
// Info   :
//------------------------------------------------------------------------------
void freeBuffer(SC_PAKRECV * PakRecv, unsigned int num);

//------------------------------------------------------------------------------
// Funtion: 计算ISO91941里面的延时
// Input  : 参数1：A B 延时参数
// Output : none
// Return : 无
// Info   :
//------------------------------------------------------------------------------
unsigned int CalculateTimeOut(unsigned int A, unsigned int B);

//------------------------------------------------------------------------------
// Funtion: 采用模拟方式发送5bps地址
// Input  : 参数1：PAKRECV结构体指针，参数2：num结构体数量
// Output : none
// Return : 无
// Info   :
//------------------------------------------------------------------------------
void sendAddressToEcu(unsigned char fiveBpsAddr);

//-----------------------------------------------------------------------------
//  自动计算波特率
//  bosch协议中 发送5 bps地址码后根据ECU回复的第一个字节计算通信波特率
//------------------------------------------------------------------------------
int32_t autoCheckBaudRate(void);

//------------------------------------------------------------------------------
// Funtion: 采用模拟方式发送任意波特率的地址码
// Input  : 参数1：地址码   参数2：波特率
// Output : none
// Return : 无
// Info   :
//------------------------------------------------------------------------------
void sendAddressCodeToEcuOnAnyBaudrate(unsigned char fiveBpsAddr,
    unsigned int baudrate);

//------------------------------------------------------------------------------
// Funtion: ISO14230 发送一个数据包
// Input  : PakSend - 待发送数据
// Output : none
// Return : 发送总线仲裁失败时返回 false
// Info   : 查询方式发送，发送时需要停止当前所有中断
//------------------------------------------------------------------------------
int SC_CML_ISO14230_Send(SC_PAKSEND * PakSend);

//------------------------------------------------------------------------------
// Funtion: ISO14230 接收一个数据包
// Input  :
// Output : PakRecv - 接收到的数据包
// Return : 应答码
// Info   : 添加整帧模式，返回仅为 TRUE 和 FALSE
//------------------------------------------------------------------------------
int SC_CML_ISO14230_Recv(SC_PAKRECV * PakRecv);

//------------------------------------------------------------------------------
// 功能：发送一帧canbus数据到ECU，不接收回复。
//
//------------------------------------------------------------------------------
void OneToOne_CanbusSendOneFrameOnly(unsigned char * buff);

//------------------------------------------------------------------------------
// Funtion: 设置can通信模波特率
// Input  : 无
// Output : 无
// Return : void
// Info   :
//------------------------------------------------------------------------------
void SetCanBaudRate(unsigned char * buff);

//------------------------------------------------------------------------------
// Funtion: 将整帧的CANBUS链路维持数据解包放入指定的结构体中
// Input  : 无
// Output : 无
// Return : void
// Info   : Keep_Link.dataBuf 55,aa,0f,61,01,88,c6,d0,87,88,02,3e,80,ff,ff,ff,ff,ff,bd,0d,c8,44
//------------------------------------------------------------------------------
void setCanbusNormalDataToken(unsigned char * dataPos);

//------------------------------------------------------------------------------
// Funtion: 设置can通信模式：1、单线can，2、双线can。 设置can通信过滤ID
// Input  : 无
// Output : 无
// Return : void
// Info   :
//------------------------------------------------------------------------------
void SetCANFilter(unsigned char * dataPos);

//------------------------------------------------------------------------------
// Funtion: ISO15765 接收一个数据包
// Input  :
// Output : PakRecv - 接收的数据
// Return : 数据包属性和格式
// Info   : ( FUN_KLine.c - OneToOne_SendDataToEcuGetAnswer_1 )
//------------------------------------------------------------------------------
unsigned char SC_CML_ISO15765_Recv(unsigned char * ans, unsigned char rcvlen);

//------------------------------------------------------------------------------
// Funtion: ISO15765 协议定时器中断函数
// Input  : none
// Output : none
// Return : none
// Info   : 该函数仅对内使用
//------------------------------------------------------------------------------
void SC_CML_ISO15765_Time(void);

#endif

//--------------------------------------------------------- End Of File --------
