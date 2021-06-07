

//------------------------------------------------------------------------------
//  Purpose: 指令函数集 - 总线通讯类（J1850 VPW PWM）
//  Funtion: 完成所有物理层总线参数设置函数，主要集中与 指令ID 6100 - 6200
//  Dependent:
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef FUN_J1850_H
#define FUN_J1850_H

//------------------------------------------------------------------------------
// Funtion: VPW协议发多 收多
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6127 ,ff,01,10,6c,f1,05,6c,25,f1,20,c5,6d,
//------------------------------------------------------------------------------
int VpwSendMultiFrameAndReceiveMultiFrameKnownAckFrameNumber_1(int argc, unsigned char * argv, unsigned char * ans);

//------------------------------------------------------------------------------
// Funtion: VPW协议发一收一
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 610E
//------------------------------------------------------------------------------
int VpwSendOneAndReceiveOneFrame_1(int argc, unsigned char * argv, unsigned char * ans);

//------------------------------------------------------------------------------
// Funtion: PWM协议发一收多
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 610C
//------------------------------------------------------------------------------
int PWM_SendDataToEcuGetAnswer_1(int argc, unsigned char * argv, unsigned char * ans);

//------------------------------------------------------------------------------
// Funtion: PWM协议发一收多
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6112
//------------------------------------------------------------------------------
int PwmSendOneAndReceiveMultiFrame_1(int argc, unsigned char * argv, unsigned char * ans);

//------------------------------------------------------------------------------
// Funtion: JAGUAR 专用 PWM 协议 发一帧数据到ECU 并接收应答(直到ECU 不发才结束)
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : recv: 61,20,01,f5,10,05,c4,10,f5,13,c7,50
//------------------------------------------------------------------------------
int JAGUARPWM_SendDataToEcuGetMultiFrameAnswer_1(int argc, unsigned char * argv, unsigned char * ans);

//------------------------------------------------------------------------------
// Funtion: VPW_Holden专用_发多帧收多帧
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : recv: 611D,05,c4,10,f5,13,c7,50
//          send:
//         改函数带有ID过滤功能。由于接收解码程序由中断程序完成，所以过滤处理
//         必须在接收完成后，再将不符合要求的数据过滤掉，接收过程只统计有效帧
//         数，过滤后帧数与统计有效帧数对比，必须相等，否则出错。
//------------------------------------------------------------------------------
int VpwSendMultiFrameAndReceiveMultiFrameForHolden_1(int argc, unsigned char * argv, unsigned char * ans);

//------------------------------------------------------------------------------
// Funtion: VPW_Holden专用_发多帧收多帧
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : recv: 611B,05,c4,10,f5,13,c7,50
//          send:
//         改函数带有ID过滤功能。由于接收解码程序由中断程序完成，所以过滤处理
//         必须在接收完成后，再将不符合要求的数据过滤掉，接收过程只统计有效帧
//         数，过滤后帧数与统计有效帧数对比，必须相等，否则出错。
//------------------------------------------------------------------------------
int VPW_HDN_SendDataToEcuGetMultiFrameAnswer_1(int argc, unsigned char * argv, unsigned char * ans);

//------------------------------------------------------------------------------
// Funtion: VPW_Holden专用_发多帧收多帧
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : recv: 611D,05,c4,10,f5,13,c7,50
//          send:
//------------------------------------------------------------------------------
int VpwSendMultiFrameAndReceiveMultiFrameForHolden_1(int argc, unsigned char * argv, unsigned char * ans);

#endif

//--------------------------------------------------------- End Of File --------
