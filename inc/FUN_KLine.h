//------------------------------------------------------------------------------
//  Purpose: 指令函数集 - 总线通讯类（K Line）
//  Funtion: 完成所有物理层总线参数设置函数，主要集中与 指令ID 6100 - 6200
//  Dependent:
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef FUN_KLINE_H
#define FUN_KLINE_H
//------------------------------------------------------------------------------
// Funtion: BOSCH地址码触发函数
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6006
//------------------------------------------------------------------------------
int SetBosch_1( int argc, unsigned char *argv, unsigned char *ans );
//----------------------------------------------------------------------------
// 600B WABCO linkeep
// send linkeep to ecu,ecu no answer
//----------------------------------------------------------------------------
int SetLinkNoAns_1( int argc, unsigned char *argv, unsigned char *ans );
//-------------------------------------------------------------------------------
//wabco ABS send one frame to ecu and receive one frame,but each times read or send byte
//command word;6121
//ziyingzhu 对此函数进行了较大修改，尚未验证
//------------------------------------------------------------------------------
int WabcoAbsSendOneAndReicveOneFrame_1( int argc, unsigned char *argv,
                                        unsigned char *ans );
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
int WabcoAbsSendOneAndReicveMultiFrame_1( int argc, unsigned char *argv, unsigned char *ans );
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
int QuickSendTwoFrameReceiveMultiFrame_1( int argc, unsigned char *argv, unsigned char *ans );
//*****************************************************************************
//  功  能：发一帧数据到ECU并接收一帧ECU的应答，一帧数据长度大于
//          256字节.
//  参  数：argc 发送数据长度
//          argv     发送缓冲区指针,格式为:数据长度高位
//                      （1BYTE）数据长度低位（1BYTE）, 数据内容,
// Output : ans - 回复到上位机的数据
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、ans的格式为：总帧数 帧长度 帧数据
// Return : 返回的ans缓冲区的有效数据长度
//  0x6126  用于长安铃木车型
//*****************************************************************************
int OneToOne_SendLongDataToEcuGetLongAnswer_1( int argc, unsigned char *argv, unsigned char *ans );
//-------------------------------------------------------------------
// SendAddressCodeTimeAdjustable :
// 0x601a
// ex1: 55 AA 00 0D FF F2 60 1a 01 35 00 ff 00 00 ff 00 00 CS
// 说明：601a|模式|地址码|已知或未知波特率|
//-------------------------------------------------------------------
int SendAddressCodeTimeAdjustable_1( int argc, unsigned char *argv,
                                     unsigned char *ans );
//------------------------------------------------------------------------------
//FIAT bosch system enter,only different to VW bosch is read  five keyword
// 接受五个关键字，对第二个关键字取反发回
//info:6015
//------------------------------------------------------------------------------
int FiatBoschSystem_1( int argc, unsigned char *argv, unsigned char *ans );
//------------------------------------------------------------------------------
// SetBosch : 0x6008
// bosch链路维持。bosch协议链路维持固定发 03 XX  09  03 ，  XX为递增命令序列号
//
//------------------------------------------------------------------------------
int SetBoschLink_1( int argc, unsigned char *argv, unsigned char *ans );
//------------------------------------------------------------------------------
// BoschSendDataToEcuGetAnswer: 0x610f
//------------------------------------------------------------------------------
int BoschSendDataToEcuGetAnswer_1( int argc, unsigned char *argv,
                                   unsigned char *ans );
//------------------------------------------------------------------------------
// 0x610a
//------------------------------------------------------------------------------
int BoschSendDataToEcuGetMultiFrameAnswer_1( int argc, unsigned char *argv,
        unsigned char *ans );
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
int SendOneFrameDataToEcuGetAnyFrameAnswer_1( int argc, unsigned char *argv, unsigned char *ans );
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
int Set5BpsParameter_1( int argc, unsigned char *argv, unsigned char *ans );
//*****************************************************************************
// AddressCodeWayAdjustTime_benz
// 0x<input type="button" 601c>
//601c
//*****************************************************************************
//已经和x431校对过
int  AddressCodeWayAdjustTime_benz_1( int argc, unsigned char *argv, unsigned char *ans );
//------------------------------------------------------------------------------
//Only send data to ecu not receive anyone data from ecu
//command word:6104
//------------------------------------------------------------------------------
//已经校对 ziyingzhu 2009-9-10 10:34
int OnlySendToEcu_1( int argc, unsigned char *argv, unsigned char *ans );
//=============================================================================
// AddressCodeCommunicationWay_Lline :
// 0x601D
//=============================================================================
//已经和431校对过了
int AddressCodeCommunicationWay_Lline_1( int argc, unsigned char *argv,
        unsigned char *ans );
//------------------------------------------------------------------------------
//shanghuan bosch system enter,only delay time different (delay 2ms)
//bosch协议
//command word:6014
//------------------------------------------------------------------------------
int ShangHuanBoschSystem_1( int argc, unsigned char *argv, unsigned char *ans );
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
int LandroverABSEnter_1( int argc, unsigned char *argv, unsigned char *ans );
//------------------------------------------------------------------------------
// SetAddressFiat :发送地址码到ECU，返回六个字节，对第二字节取返发回。
// 0x600a   0k 20080531
//------------------------------------------------------------------------------
int SetAddressFiat_1( int argc, unsigned char *argv, unsigned char *ans );
//---------------------- ------------------------------------------------------
// 类似于Set5BpsParameter   对ECU发来的关键字不取反发回
//info  6009
//-----------------------------------------------------------------------------
int BoschFiat_1( int argc, unsigned char *argv, unsigned char *ans );
//-----------------------------------------------------------------------------
// SetVpwLogicLevel
// 0x600E
//-----------------------------------------------------------------------------
int SetVpwLogicLevel_1( int argc, unsigned char *argv, unsigned char *ans );
//------------------------------------------------------------------------------
//函数名称:ReceiveOneKwpFrameFromECU
//函数功能:从ECU获取一帧信息
//函数的参数:buff接受缓冲区
//          :size接受的总的长度
//        :overtime接受时帧与帧之间的间隔，字节与字节之间的间隔
//        :speci_data.0x7f,0x78所对应的字节内容
//函数的返回值:true  成功
//            false 失败
//与LPC2368相比，这里的函数直接把0x7f,0x78直接
//在这里实现的是接受一帧kwp指令，在指令中没有关闭和开启链路保持。
//------------------------------------------------------------------------------
unsigned int  ReceiveOneKwpFrameFromECU(unsigned char *buff1,unsigned int  *size,
                                        unsigned int  overtime,
                                        unsigned char* speci_data);
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
int KWPSendDataToEcuGetAnswer_1( int argc, unsigned char *argv, unsigned char *ans );
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
//------------------------------------------------------------------------------
int KWPSendOneAndReceiveMultiFrame_1( int argc, unsigned char *argv, unsigned char *ans );
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
int KWPSendDataToEcuGetAnswer_more_1( int argc, unsigned char *argv, unsigned char *ans );
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
int KWPSendOneFrameAndReceiveMultiFrame_Volvo_1 ( int argc, unsigned char *argv,
        unsigned char *ans );
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
int KWPSendOneAndReceiveMultiFrame_Toyota_1( int argc, unsigned char *argv,
        unsigned char *ans );
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
//                                         0x07,0x80,0x10,0xF1,0x02,0x22,0x07,0xA8,
//                                         0x05,0x00,0x02,0x22,0x07,0x64,
//                                         0x04,0x02,0x22,0x07,0x24};
//   KWP_SendDataToEcuGetAnswer_Benz(0,command_data7,ans7);
//------------------------------------------------------------------------------
int KWP_SendDataToEcuGetAnswer_Benz_1( int argc, unsigned char *argv,
                                       unsigned char *ans );
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
int KWP_SendDataToEcuGetMultiFrameAnswer_Benz_1( int argc, unsigned char *argv,
        unsigned char *ans );
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
int KWPSendDataToEcuGetAnswer_BMW_1( int argc, unsigned char *argv, unsigned char *ans );
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
int ISO_SendDataToEcuGetMultiFrameAnswer_1( int argc, unsigned char *argv,
        unsigned char *ans );
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
int ISO_SendDataToEcuGetAnswer_1( int argc, unsigned char *argv, unsigned char *ans );
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
int FordIsoSendOneAndReceiveMultiFrame_1( int argc, unsigned char *argv,
        unsigned char *ans );
//------------------------------------------------------------------------------
// Funtion: send one frame to ecu and receive multiframe know nothing of frame
// Input  : argc - 参数长度
//          argv - 指令参数缓冲区
//          argv[0]:数据流标志
//          argv[1]:发送帧数
//          argv[2]:接收帧数
//          argv[3]:检测 08 55 a3开关, 如果为0，则检测
//     根据帧数后面字节循环依次为：headid : 检验从ECU所收数据的帧头是否匹配
//                 busidletime: 检测总线空闲最大等待时间
// Output : ans - 回复到上位机的数据
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、ans的格式为：总帧数 帧长度 帧数据
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 6117
//------------------------------------------------------------------------------
int HoldenNormalRingLinkSendOneAndOneFrame_1( int argc, unsigned char *argv, unsigned char *ans );
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
int HoldenOnlyKnowHeadOfFrame_1( int argc, unsigned char *argv, unsigned char *ans );
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
int SubruOldProtocol_1( int argc, unsigned char *argv, unsigned char *ans );
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
int SendOneFrameDataToEcuGetAnyFrameAnswer_Check_1( int argc, unsigned char *argv, unsigned char *ans );
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
int CanbusOnlySendDataToEcu_1( int argc, unsigned char *argv, unsigned char *ans );
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
int OneToOne_SendOneByteToEcuGetAnswerLM_1( int argc, unsigned char *argv,
        unsigned char *ans );
//------------------------------------------------------------------------------
// Funtion: KWP通讯时，可过滤指定ID
// Input  : argc - 参数长度
//          argv - 指令参数缓冲区
//          argv[0]:recmode
//          argv[1]:发送一帧数数据的长度len
//          argv[2]~argv[len+1]:发送往ECU的一帧有效数据
//          argv[3]:目标地址
//          argv[4]:源地址
// Output : ans - 回复到上位机的数据
//                1、指令格式异常返回 FF 01
//                2、下位机通信异常返回 FF 02
//                3、ans的格式为：总帧数 帧长度 帧数据
// Return : 返回的ans缓冲区的有效数据长度
// Info   : 6129 用于下线检测的指令，诊断中未使用
//------------------------------------------------------------------------------
int KWPSendOneAndReceiveMultiFrameHasFilter_1( int argc, unsigned char *argv, unsigned char *ans );
//------------------------------------------------------------------------------
// Funtion: lanrdrover send one frame and receive multiframe until timeout
// Input  : argc - 参数长度
//          argv - 数据格式
// 实例   : 55,aa,00,0a,ff,f5,62,0b,11,80,0c,f4,
// Output : ans - ff,00
// Return : 0xff
// Info   : 620b
//------------------------------------------------------------------------------
int LandroverSendAndReceiveFault_1( int argc, unsigned char *argv,
                                    unsigned char *ans );
//------------------------------------------------------------------------------
// Funtion: lanrdrover send one frame and receive multiframe until timeout
// Input  : argc - 参数长度
//          argv - 数据格式
// 实例   : 55,aa,00,0a,ff,f5,62,0b,11,80,0c,f4,
// Output : ans - ff,00
// Return : 0xff
// Info   : 6219
//------------------------------------------------------------------------------
int SysGazSendBankSingleFrame_1( int argc, unsigned char *argv, unsigned char *ans );
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
int SendOneFrameToEcuGetMultiFrameAnswer_1( int argc, unsigned char *argv,
        unsigned char *ans );
#endif
//--------------------------------------------------------- End Of File --------
