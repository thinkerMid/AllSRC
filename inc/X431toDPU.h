//------------------------------------------------------------------------------
//  Purpose: X431 通讯抽象层转换至 DPU MYRCAR 模式
//  Funtion: MYCAR 方式所有函数
//  Dependent:
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef X431toDPU_H
#define X431toDPU_H
int DisposeSendAndReceive( unsigned char *pReceiveBuffer,
                           unsigned int iLengthReceiveBuffer );
unsigned char *NewBuffer(void);
int AddCharToBuffer( unsigned char Byte );
int AddToBuffer(unsigned int iLength,unsigned char *pBuffer);
int GetBufferContainLength(void);
int EndCombinationBuffer(unsigned char *ucRecBuffer,int iLength);
unsigned char* GetBuffer(void);
//------------------------------------------------------------------------------
// Funtion: 开始命令组合,从执行本函数起,任何通信接口指令均不被
//          立即发送, 直到执行EndCombination(); 在被组合的函数
//          中,最多只能有一条需要从ECU接收数据的函数.
// Input  :
// Output :
// Return : 成功    GD_OK
//          失败    出错代码
// Info   :
//------------------------------------------------------------------------------
int BeginCombination(void);
//------------------------------------------------------------------------------
// Funtion: 结束命令组合, 并将指令组发送到SMARTBOX, 接收应答数据.
// Input  :
// Output :
// Return : 成功    返回从ECU收到的数据长度
//          失败    出错代码
// Info   :
//------------------------------------------------------------------------------
int EndCombination(void);
void DelBuffer(void);
#endif
