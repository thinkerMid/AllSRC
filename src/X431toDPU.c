

//------------------------------------------------------------------------------
//  Purpose: X431 通讯抽象层转换至 DPU MYRCAR 模式
//  Funtion: MYCAR 方式所有函数
//  Dependent:
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef X431toDPU_C
#define X431toDPU_C
#include "gdstd.h"
#include "CommAbstractLayer.h"
#include "SC_cmdline.h"
#include "X431toDPU.h"
int iLengthContain = 0;
unsigned char cBuffer[512];
int AddCharToBuffer(unsigned char Byte);
int AddToBuffer(unsigned int iLength, unsigned char * pBuffer);

#define EC_PARAMETER_ERROR      -50
#define EC_SMART_RETURN_DATA_ERROR -60
#define EC_ECU_BREAK            -70
#define GD_OK                   0
#define EC_OVER_MEMORY_SIZE     -1
#define EC_OVER_MAXIMUM_NUMBER  -2
#define EC_ALLOC_MEMORY_ERROR   -3
#define EC_SEND_DATA_LENGTH     -10
#define EC_DATA_ERROR           -11
#define EC_DATA_PACKET_ERROR    -12
#define EC_DATA_CHECKSUM_ERROR  -13
#define EC_TIMEOVER             -14
#define EC_OVER_FLOW            -15
#define EC_HANDSHAKE_FAILURE    -16


int DisposeSendAndReceive(unsigned char * pReceiveBuffer, unsigned int iLengthReceiveBuffer)
{
    int i;
    int iRet;

    iRet = CmdLineProcess(&iLengthContain, cBuffer, pReceiveBuffer);

    if (0 == pReceiveBuffer[0])
    {
        iLengthContain--;

        for (i = 0; i < iLengthContain; i++)
        {
            pReceiveBuffer[i] = pReceiveBuffer[i + 1];
        }

        if ((0xff == pReceiveBuffer[0]) && (0x02 == pReceiveBuffer[1]))
        {
            return 0;
        }
        else
        {
            return (iLengthContain);
        }
    }
    else if (0xff == pReceiveBuffer[0])
    {
        switch (pReceiveBuffer[1])
        {
            case CA_FOK: //0x00
                iRet = GD_OK;
                break;

            case CA_FNG: //0x01
                iRet = EC_TIMEOVER;
                break;

            case CA_ECU_BREAK: //0x02
                iRet = EC_ECU_BREAK;
                break;

            case 0x80:
                iRet = 0x80;
                break;

            case 0x81:
                iRet = 0x81;
                break;
        }

        return iRet;
    }

    return EC_SMART_RETURN_DATA_ERROR;
}


unsigned char * NewBuffer(void)
{
    iLengthContain = 0;
    return cBuffer;
}


int AddCharToBuffer(unsigned char Byte)
{
    cBuffer[iLengthContain] = Byte;
    iLengthContain++;
    return iLengthContain;
}


int AddToBuffer(unsigned int iLength, unsigned char * pBuffer)
{
    unsigned int i;

    for (i = 0; i < iLength; i++)
    {
        cBuffer[iLengthContain++] = pBuffer[i];
    }

    return iLengthContain;
}


int GetBufferContainLength(void)
{
    return iLengthContain;
}


unsigned char * GetBuffer(void)
{
    return cBuffer;
}


void DelBuffer(void)
{
}


int EndCombinationBuffer(unsigned char * ucRecBuffer, int iLength)
{
    int iRet;

    iRet = DisposeSendAndReceive(ucRecBuffer, iLength);
    return iRet;
}


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
int BeginCombination()
{
    return 1;
}


//------------------------------------------------------------------------------
// Funtion: 结束命令组合, 并将指令组发送到SMARTBOX, 接收应答数据.
// Input  :
// Output :
// Return : 成功    返回从ECU收到的数据长度
//          失败    出错代码
// Info   :
//------------------------------------------------------------------------------
int EndCombination()
{
    int iRet;

    iRet = DisposeSendAndReceive(cBuffer, iLengthContain);
    return iRet;
}


#endif

