//------------------------------------------------------------------------------
//  Purpose: DPD MYRCAR 模式 通讯抽象层
//  Funtion: MYCAR 方式所有函数
//  Dependent:
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef CommAbstractLayer_C
#define CommAbstractLayer_C

//#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "SetPIN.h"
#include "X431toDPU.h"
#include "gdstd.h"
#include "CommAbstractLayer.h"


//错误代码 EC_ Error Code
#define GD_OK                                    0
#define EC_OVER_MEMORY_SIZE                      -1
#define EC_OVER_MAXIMUM_NUMBER                   -2
#define EC_ALLOC_MEMORY_ERROR                    -3

//-10~-20 Serial port Commnunication
#define EC_SEND_DATA_LENGTH                      -10
#define EC_DATA_ERROR                            -11
#define EC_DATA_PACKET_ERROR                     -12
#define EC_DATA_CHECKSUM_ERROR                   -13
#define EC_TIMEOVER                              -14
#define EC_OVER_FLOW                             -15
#define EC_HANDSHAKE_FAILURE                     -16

#define EC_OPENFILE_FAILURE                      -17

#define EC_NOT_GAG_FILE                          -20
#define EC_NOT_EXIST_OR_LICENSE_FILE_ERROR       -21
#define EC_VEHICLES_SYSTEM_LICENSE               -22
#define EC_SERIAL_NUMBER_LICENSE                 -23
#define EC_DIAGNOSE_SOFT_VERSION_ERROR           -24
#define EC_RUN_PLATFORM_ERROR                    -25

#define EC_SET_SIGNAL_CALL_BACK_FUNCTION         -30

#define EC_NOT_EXIST_DIAGNOSE_FILE               -40
#define EC_EXEC_DIAGNOSE_FAULURE                 -41

#define EC_PARAMETER_ERROR                       -50
#define EC_SMART_RETURN_DATA_ERROR               -60
#define EC_ECU_BREAK                             -70
#define X431 			0
#define X431C 			1
#define X431AFRICA 		2
#define X431INFINITE		4
#define X431INFINITEAFRICA		6
#define X431NOPRE				8
#define X431PRINTEXTEXT		16
#define X431TOOL				68
#define X431TOOLAFRICA			70
#define DIAGUN_PRODUCT      80
#define GX3_PRODUCT         85
#define TOP_PRODUCT         90


SMARTBOX_CAL s_SmartboxStruct =
{
    //select line
    CA_LINE_K,            //int32_t ucInputLine;
    CA_LINE_K,            //int32_t ucOutputLine;
    CA_12V,                //int32_t ucEcuCommunicationVoltage;
    CA_POSITIVE,        //int32_t ucLogic;
    FALSE,                //int8_t bEnableLLine;
    CA_HIGH,            //ucDTS_Volage;
    0,
    //bps
    9600,                //int32_t iBps;
    1,                    //int32_t ucStartBit;
    8,                    //int32_t ucDataBit;
    1,                    //int32_t ucStopBit;
    CA_NONE,            //int32_t ucCheckBitWay;

/*
    0,                    //int32_t iByteToByteTime;
    0,                    //int32_t iPacketToPacketTime;
    30,                    //int32_t iOverTime;
    30,                    //int32_t iReceivePacketEndTime;
*/
    //flash code way
    FALSE,                //int8_t bReadingFlashCode;
    1,                  //int32_t ucReadFlashCodeWay;

};

int32_t s_iWaitAnswerMaximumTime = 5 * 1000;

#define ALLOC_SIZE_ONCE                   8192
struct COMBINATION
{
    int8_t bCombinating;
    uint8_t *pSendBuffer;
    int32_t iContainLength;
    int32_t iAllocLength;
    uint8_t *pReceiveBuffer;
    int32_t iLengthReceiveBuffer;
    int32_t iLastPacketType;
};

struct COMBINATION s_Combination =
{
    FALSE,
    NULL,
    0,
    0,
    NULL,
    0,
    0
};

//------------------------------------------------------------------------------
// Funtion: 设置SMARTBOX通信输入输出线
// Input  : ucInputLine   输入线
//          ucOutputLine  输出线
// Output : none
// Return : 成功         GD_OK
//          失败         出错代码
// Info   : none
//------------------------------------------------------------------------------
int32_t SelectIoLine( uint8_t ucInputLine, uint8_t ucOutputLine )
{
    if( ucInputLine > 16 )
    {
        return EC_PARAMETER_ERROR;
    }
    if( ucOutputLine > 16 )
    {
        return EC_PARAMETER_ERROR;
    }
    s_SmartboxStruct.ucInputLine = ucInputLine;
    s_SmartboxStruct.ucOutputLine = ucOutputLine;
    return SetIoParameter();
}

//------------------------------------------------------------------------------
// Funtion: IO线及参数控制
// Input  :
// Output : none
// Return : 成功         GD_OK
//          失败         出错代码
// Info   : none
//------------------------------------------------------------------------------
int32_t SetIoParameter()
{
    int32_t iRet = GD_OK;
    NewBuffer();
    AddCharToBuffer( CAC_CONTRAL_COMMAND );
    AddCharToBuffer( CAC_IO_CONTRAL );
    AddCharToBuffer( (uint8_t)s_SmartboxStruct.ucLogic );
    AddCharToBuffer( (uint8_t)s_SmartboxStruct.ucEcuCommunicationVoltage );
    AddCharToBuffer( (uint8_t)s_SmartboxStruct.ucOutputLine );
    AddCharToBuffer( (uint8_t)s_SmartboxStruct.ucInputLine );
    AddCharToBuffer( (uint8_t)( s_SmartboxStruct.bEnableLLine
                     ?CA_L_LINE_ENABLE:CA_L_LINE_DISABLE ) );
    AddCharToBuffer( (uint8_t)s_SmartboxStruct.ucDTS_Volage );
    AddCharToBuffer( (uint8_t)s_SmartboxStruct.ucCommType );
    iRet = DisposeSendAndReceive( NULL, 0 );
    DelBuffer();
    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 设置与ECU通信的时间常数
// Input  : fByteToByte 字节与字节间隔时间,单位毫秒,可识别精度
//                       为0.1毫秒, 小数点后第二位四舍五入.
//            wPacketToPacket    数据包之间时间间隔,单位毫秒
//            iOverTime    接收ECU数据包最大等待时间,单位毫秒
// Output : none
// Return : 成功         GD_OK
//          失败         出错代码
// Info   : none
//------------------------------------------------------------------------------
int32_t SetEcuCommTimeInterval( float fByteToByte, int32_t wPacketToPacket,
                               int32_t iOverTime )
{
    int32_t iRet = GD_OK;
    int32_t iTemp;

    NewBuffer();
    AddCharToBuffer( CAC_CONTRAL_COMMAND );
    AddCharToBuffer( CAC_ECU_COMM_TIME_INTERVAL );

    iTemp = ( int32_t )( fByteToByte*10 + 0.5 );
    AddCharToBuffer(( uint8_t )( ( iTemp >> 8 ) & 0xFF ) );
    AddCharToBuffer(( uint8_t )( iTemp & 0xFF ) );

    AddCharToBuffer((uint8_t)((wPacketToPacket+5)/10));
    iOverTime = ( iOverTime + 5 ) / 10;
    AddCharToBuffer((uint8_t)((iOverTime>>8)&0xFF));
    AddCharToBuffer((uint8_t)(iOverTime&0xFF));
    iRet = DisposeSendAndReceive( NULL, 0 );
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 地址码通信方式
// Input  : ucAddressCode       地址码
//          pReceiveBuffer      接收缓冲区
//          iLengthReceiveBuffer接收缓冲区长度
//          iBps                与ECU通信波特率
//          bRecognizeBaudRate  需要识别并自动设置波特率
//          bSecondByteReverseReturnToECU 第二个字节是否要求 取反发回
//          iReceiveFrameNumber 要接收的帧数,取值范围:0~255 (不包括取反发回的字节)
//          iLengthEachFrame    接收每帧的长度
// Output : none
// Return : 成功         GD_OK
//          失败         出错代码
// Info   : none
//------------------------------------------------------------------------------
int32_t AddressCodeCommunicationWay( uint8_t ucAddressCode, uint8_t *pReceiveBuffer,
                                    int32_t iLengthReceiveBuffer,
                                    float fBps, int8_t bRecognizeBaudRate,
                                    int8_t bSecondByteReverseReturnToECU,
                                    int32_t iReceiveFrameNumber,
                                    int32_t iLengthEachFrame )
{
    int32_t iRet=GD_OK;

    if(iReceiveFrameNumber < 0 || iReceiveFrameNumber > 255 )
    {
        return EC_PARAMETER_ERROR;
    }
    if( iLengthEachFrame <= 0 || iLengthEachFrame > 255 )
    {
        return EC_PARAMETER_ERROR;
    }

    NewBuffer();
    AddCharToBuffer( CAC_CONTRAL_COMMAND );
    AddCharToBuffer( CAC_ECU_ADDRESS_CODE_WAY );
    if( fabs( fBps - 5 ) < 0.0001 )
    {
        AddCharToBuffer( 1 ); //way 1
        AddCharToBuffer( ucAddressCode );
        AddCharToBuffer((uint8_t)(bRecognizeBaudRate?0xFF:0x00));
        AddCharToBuffer((uint8_t)(bSecondByteReverseReturnToECU?0xFF:0x00));
        AddCharToBuffer((uint8_t)(iReceiveFrameNumber&0xff));
        AddCharToBuffer((uint8_t)(iLengthEachFrame&0xff));
    }
    else
    {
        AddCharToBuffer( 2 ); //way 2
        AddCharToBuffer( ucAddressCode );
        if( fBps > 1000.0 )
        {
            AddCharToBuffer((uint8_t)(+(((int32_t)(65536-18432000/32/fBps))>>8)&0xFF));
            AddCharToBuffer((uint8_t)(((int32_t)(65536-18432000/32/fBps))&0xFF));
        }
        else
        {      //2003.3.26 add
            int32_t iTemp = (int32_t)( fBps + 0.5 );
            AddCharToBuffer(( uint8_t )( (iTemp>>8)&0xFF) );
            AddCharToBuffer(( uint8_t )(iTemp&0xFF));
        }

        AddCharToBuffer((uint8_t)(bSecondByteReverseReturnToECU?0xFF:0x00));
        AddCharToBuffer((uint8_t)(iReceiveFrameNumber&0xFF));
        AddCharToBuffer((uint8_t)(iLengthEachFrame&0xff));
    }

    iRet = DisposeSendAndReceive( pReceiveBuffer, iLengthReceiveBuffer );
    DelBuffer();

    //ziyingzhu 修改.与原来的协议保持兼容2009-6-5 12:01:40
    if( iRet >= 0 )
    {
        if( iReceiveFrameNumber == 0 )
        {
            return GD_OK;
        }
        else
        {
            return iRet;
        }
    }
    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: BOSCH通信方式
// Input  : ucAddress             地址码
//           pReceiveBuffer       接收缓冲区指针
//           wLengthReceiveBuffer 接收缓冲区长度
// Output : none
// Return : 成功         版本信息长度
//          失败         出错代码
// Info   : none
//------------------------------------------------------------------------------
int32_t BoschEnterReturnVersion( uint8_t ucAddress,
                                uint8_t *pReceiveBuffer,
                                int32_t iLengthReceiveBuffer )
{
    int32_t iRet = GD_OK;

    NewBuffer();
    AddCharToBuffer( CAC_CONTRAL_COMMAND );
    AddCharToBuffer( CAC_ECU_BOSCH_ENTER_GET_VERSION );
    AddCharToBuffer( 0x00 );
    AddCharToBuffer( ucAddress );

    iRet = DisposeSendAndReceive( pReceiveBuffer, iLengthReceiveBuffer );
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 控制IO线输出电平及电平保持时间
// Input  : ucChangeTimes    为电平改变次数,后面依次为低电平时间
//                           高电平时间, 低电平时间, 高电平时间
//                           ... ... , 如果电平先拉高,再拉低则第
//                           一个低电平时间为0.
//          电平保持时间单位:毫秒, 数据类型:int32_t
// Output : none
// Return : 成功         GD_OK
//          失败         出错代码
// Info   : none
//------------------------------------------------------------------------------
int32_t SetCommunicationLineVoltage( uint8_t ucChangeTimes, ... )
{
    va_list args;
    int32_t iTemp;
    int32_t i;
    int32_t iRet = GD_OK;

    if( ucChangeTimes == 0 )
    {
        return EC_PARAMETER_ERROR;
    }

    NewBuffer();
    va_start( args, ucChangeTimes );
    for ( i = 0; i < (int32_t)ucChangeTimes; i++ )
    {
        iTemp = va_arg( args, int32_t );
        if( ( iTemp < 0 ) || ( iTemp > 65535 ) )
        {
            iRet = EC_OVER_MEMORY_SIZE;
            break;
        }
        if( iTemp > 0 )
        {
            AddCharToBuffer( CAC_CONTRAL_COMMAND );
            AddCharToBuffer( CAC_SET_IO_LINE_VOLTAGE );

            if( s_SmartboxStruct.ucLogic == CA_POSITIVE )
            {
                AddCharToBuffer( (uint8_t)( i % 2 ? CA_HIGH : CA_LOW ) );
            }
            else
            {
                AddCharToBuffer( (uint8_t)( i % 2 ? CA_LOW : CA_HIGH ) );
            }

            AddCharToBuffer((uint8_t)((iTemp>>8)&0xFF));
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
        }
    }
    va_end( args );

    if( ( iRet == GD_OK ) && ( GetBufferContainLength() > 0 ) )
    {
        iRet = DisposeSendAndReceive( NULL, 0 );
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: BOSCH设置链路保持设定函数, 在以BOSCH方式进入系统后
//          自动进行链路保持; 本函数可以中断链路或恢复保持
// Input  : bKeepLink   TRUE  进行链路保持                      *
//                      FALSE 暂停链路保持
// Output : none
// Return : 成功         GD_OK
//          失败         出错代码
// Info   : none
//------------------------------------------------------------------------------
int32_t EnableBoschLinkKeep( int8_t bKeepLink )
{
    int32_t iRet = GD_OK;

    NewBuffer();
    AddCharToBuffer( CAC_CONTRAL_COMMAND );
    AddCharToBuffer( CA_BOSCH_LINK_KEEP_SET_AND_CLEAR );
    if( bKeepLink )
    {
        AddCharToBuffer( BOSCH_ON_LINK_KEEP );
    }
    else
    {
        AddCharToBuffer( BOSCH_OFF_LINK_KEEP );
    }

    iRet = DisposeSendAndReceive( NULL, 0 );
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 发送地址码得到keyword
// Input  : ucAddress               地址码
//          pReceiveBuffer          接收缓冲区指针
//          wLengthReceiveBuffer    接收缓冲区长度
// Output : none
// Return : 成功         版本信息长度
//          失败         出错代码
// Info   : none
//------------------------------------------------------------------------------
int32_t SendAddressCodeGetKeyword( uint8_t ucAddress, uint8_t *pReceiveBuffer,
                                  int32_t iLengthReceiveBuffer )
{
    int32_t iRet = GD_OK;

    NewBuffer();
    AddCharToBuffer( CAC_CONTRAL_COMMAND );
    AddCharToBuffer( CAC_ECU_ADDRESS_GET_KWD_CHECK );
    AddCharToBuffer( ucAddress );

    iRet = DisposeSendAndReceive( pReceiveBuffer, iLengthReceiveBuffer );
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 地址码通信方式
// Input  : ucAddressCode        地址码
//          iLengthReceiveBuffer接收缓冲区长度
//          fBps                与ECU通信波特率
//          bRecognizeBaudRate  需要识别并自动设置波特率
//          bSecondByteReverseReturnToECU 第二个字节是否要求取反发回
//          iReceiveFrameNumber 要接收的帧数,取值范围:0~255(不包括取反发回的字节)            *
// Output : pReceiveBuffer      接收缓冲区
//          后面参数依次为:
//          接收的第1帧数据长度,类型:int32_t
//          接收的第2帧数据长度,类型:int32_t
//                  ...   ...
//          接收的第iReceiveFrameNumber帧数据长度
// Return : 成功         返回从ECU接收的版本信息长度,如无版本信息, 返回GD_OK
//          失败         出错代码
// Info   : none
//------------------------------------------------------------------------------
int32_t AddressCodeWay2( uint8_t ucAddressCode, uint8_t *pReceiveBuffer,
                        int32_t iLengthReceiveBuffer,
                        float fBps, int8_t bRecognizeBaudRate,
                        int8_t bSecondByteReverseReturnToECU,
                        int32_t iReceiveFrameNumber, ... )
{
    int32_t iRet = GD_OK;

    if( iReceiveFrameNumber < 0 || iReceiveFrameNumber > 255 )
    {
        return EC_PARAMETER_ERROR;
    }

    NewBuffer();
    AddCharToBuffer( CAC_CONTRAL_COMMAND );
    AddCharToBuffer( CAC_ECU_ADDRESS_CODE_WAY2 );
    if( fabs( fBps - 5 ) < 0.0001 )
    {
        AddCharToBuffer( 1 ); //way 1
        AddCharToBuffer( ucAddressCode );
        AddCharToBuffer( (uint8_t)( bRecognizeBaudRate ? 0xFF : 0x00 ) );
    }
    else
    {
        AddCharToBuffer( 2 ); //way 2
        AddCharToBuffer( ucAddressCode );

        if( fBps > 1000.0 )
        {
            AddCharToBuffer((uint8_t)((((int32_t)(65536-18432000/32/fBps))>>8)&0xFF));
            AddCharToBuffer((uint8_t)(((int32_t)(65536-18432000/32/fBps))&0xFF));
        }
        else
        {      //2003.3.26 add
            int32_t iTemp=(int32_t)(fBps+0.5);
            AddCharToBuffer((uint8_t)((iTemp>>8)&0xFF));
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
        }

    }

    AddCharToBuffer((uint8_t)(bSecondByteReverseReturnToECU?0xFF:0x00));
    AddCharToBuffer((uint8_t)(iReceiveFrameNumber&0xFF));

    if( iReceiveFrameNumber <= 0 )
    {
        AddCharToBuffer((uint8_t)(0));
    }
    else
    {
        int32_t i,iTemp;
        va_list args;
        va_start( args, iReceiveFrameNumber );
        for( i = 0; i < iReceiveFrameNumber; i++ )
        {
            iTemp = va_arg(args,int32_t);
            if(( iTemp > 255 ) || (iTemp <= 0 ) )
            {
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
        }
        va_end(args);
    }

    if( iRet == GD_OK )
    {
        iRet = DisposeSendAndReceive( pReceiveBuffer, iLengthReceiveBuffer );
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 设置VPW读取命令等待时间，使用于SGM
// Input  : TIME      脉宽时间
//          VOLTAGE   通迅电平
// Output :
// Return : 成功         返回GD_OK
//          失败         出错代码
// Info   : none
//------------------------------------------------------------------------------
int32_t VPW_Init_Set_RecTime( uint8_t time, uint8_t Voltage )     //2006-5-10
{
    int32_t iRet = GD_OK;

    NewBuffer();
    AddCharToBuffer( CAC_CONTRAL_COMMAND );
    AddCharToBuffer( (uint8_t)(CAC_VPW_SET_TIME&0xff) );
    AddCharToBuffer( (uint8_t)(time&0xff) );
    AddCharToBuffer( (uint8_t)(Voltage&0xff) );
    iRet = DisposeSendAndReceive( NULL, 0 );
    DelBuffer();
    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: luhu enter send 5bps addresscode and receive code 0xaa
// Input  : ucAddressCode        地址码
//          iLengthReceiveBuffer接收缓冲区长度
//          fBps                与ECU通信波特率
//          bRecognizeBaudRate    需要识别并自动设置波特率
//          bSecondByteReverseReturnToECU    第二个字节是否要求取反发回
//          iReceiveFrameNumber    要接收的帧数,取值范围:0~255(不包括取反发回的字节)            *
// Output : 后面参数依次为:
//          接收的第1帧数据长度,类型:int32_t
//          接收的第2帧数据长度,类型:int32_t
//                  ...   ...
//          接收的第iReceiveFrameNumber帧数据长度
//          pReceiveBuffer        接收缓冲区
// Return : 成功         返回GD_OK
//          失败         出错代码
// Info   : none
//------------------------------------------------------------------------------
int32_t AddressCodeWay_luhu( uint8_t ucAddressCode, uint8_t *pReceiveBuffer,
                            int32_t iLengthReceiveBuffer,
                            float fBps, int8_t bRecognizeBaudRate,
                            int8_t bSecondByteReverseReturnToECU,
                            int32_t iReceiveFrameNumber, ... )
{
    int32_t iRet=GD_OK;

    if( iReceiveFrameNumber < 0 || iReceiveFrameNumber > 255 )
    {
        return EC_PARAMETER_ERROR;
    }

    NewBuffer();
    AddCharToBuffer( CAC_CONTRAL_COMMAND );
    AddCharToBuffer( CAC_LUHU_ENTER );
    if( fabs( fBps - 5 ) < 0.0001 )
    {
        AddCharToBuffer( 1 ); //way 1
        AddCharToBuffer( ucAddressCode );
        AddCharToBuffer( (uint8_t)( bRecognizeBaudRate ? 0xFF : 0x00 ) );
    }
    else
    {
        AddCharToBuffer( 2 ); //way 2
        AddCharToBuffer( ucAddressCode );

        if( fBps > 1000.0 )
        {
                 AddCharToBuffer((uint8_t)((((int32_t)(65536-18432000/32/fBps))>>8)&0xFF));
                 AddCharToBuffer((uint8_t)(((int32_t)(65536-18432000/32/fBps))&0xFF));
        }
        else
        {  //2003.3.26 add
                 int32_t iTemp=(int32_t)(fBps+0.5);
                 AddCharToBuffer((uint8_t)((iTemp>>8)&0xFF));
                 AddCharToBuffer((uint8_t)(iTemp&0xFF));
          }

    }

    AddCharToBuffer((uint8_t)(bSecondByteReverseReturnToECU?0xFF:0x00));
    AddCharToBuffer((uint8_t)(iReceiveFrameNumber&0xFF));

    if( iReceiveFrameNumber <= 0 )
    {
        AddCharToBuffer( (uint8_t)( 0 ) );
    }
    else
    {
        int32_t i,iTemp;
        va_list args;
        va_start(args, iReceiveFrameNumber);
        for( i = 0; i < iReceiveFrameNumber; i++ )
        {
            iTemp = va_arg( args, int32_t );
            if( ( iTemp > 255 ) || ( iTemp <= 0 ) )
            {
                iRet = EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer( ( uint8_t )( iTemp & 0xFF ) );
        }
        va_end( args );
    }

    if(iRet==GD_OK)
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: PEUGEOT SPECIAL     地址码通信方式
// Input  : ucAddressCode       地址码
//          iLengthReceiveBuffer接收缓冲区长度
//          fBps                与ECU通信波特率
//          bRecognizeBaudRate  需要识别并自动设置波特率
//          bSecondByteReverseReturnToECU    第二个字节是否要求取反发回
//          iReceiveFrameNumber  要接收的帧数,取值范围:0~255(不包括取反发回的字节)            *
// Output : 后面参数依次为:
//          接收的第1帧数据长度,类型:int32_t
//          接收的第2帧数据长度,类型:int32_t
//                  ...   ...
//          接收的第iReceiveFrameNumber帧数据长度
//          pReceiveBuffer        接收缓冲区
// Return : 成功         返回GD_OK
//          失败         出错代码
// Info   : none
//------------------------------------------------------------------------------
//此函数不再使用
int32_t PeugeotSpecialAddressCodeWay( uint8_t ucAddressCode,
                                     uint8_t *pReceiveBuffer,
                                     int32_t iLengthReceiveBuffer,
                                     float fBps,
                                     int8_t bRecognizeBaudRate,
                                     int8_t bSecondByteReverseReturnToECU,
                                     int32_t iReceiveFrameNumber, ... )
{
    int32_t iRet=GD_OK;

    if( iReceiveFrameNumber < 0 || iReceiveFrameNumber > 255 )
    {
        return EC_PARAMETER_ERROR;
    }

    NewBuffer();
    AddCharToBuffer( CAC_CONTRAL_COMMAND );
    AddCharToBuffer( CAC_PEUGEOT_SPECIAL_ADDRESS_CODE_WAY );
    if( fabs( fBps - 5 ) < 0.0001 )
    {
        AddCharToBuffer( 1 ); //way 1
        AddCharToBuffer( ucAddressCode );
        AddCharToBuffer( (uint8_t)(bRecognizeBaudRate?0xFF:0x00) );
    }
    else
    {
        AddCharToBuffer( 2 ); //way 2
        AddCharToBuffer( ucAddressCode );

       if(fBps>1000.0)
       {
             AddCharToBuffer((uint8_t)((((int32_t)(65536-18432000/32/fBps))>>8)&0xFF));
             AddCharToBuffer((uint8_t)(((int32_t)(65536-18432000/32/fBps))&0xFF));
        }
        else
        {   //2003.3.26 add
            int32_t iTemp=(int32_t)(fBps+0.5);
            AddCharToBuffer((uint8_t)((iTemp>>8)&0xFF));
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
        }

    }

    AddCharToBuffer((uint8_t)(bSecondByteReverseReturnToECU?0xFF:0x00));
    AddCharToBuffer((uint8_t)(iReceiveFrameNumber&0xFF));

    if(iReceiveFrameNumber<=0)
    {
        AddCharToBuffer((uint8_t)(0));
    }
    else
    {
        int32_t i,iTemp;
        va_list args;
        va_start( args, iReceiveFrameNumber );
        for( i = 0; i < iReceiveFrameNumber; i++ )
        {
            iTemp = va_arg(args,int32_t);
            if((iTemp>255) || (iTemp<=0) )
            {
                iRet = EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
        }
        va_end( args );
    }

    if( iRet == GD_OK )
    {
        iRet = DisposeSendAndReceive( pReceiveBuffer, iLengthReceiveBuffer );
    }

    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: mabco通信方式
// Input  : ucAddress               地址码
// Output : pReceiveBuffer          接收缓冲区指针
//          wLengthReceiveBuffer    接收缓冲区长度
// Return : 成功         版本信息长度
//          失败         出错代码
// Info   : none
//------------------------------------------------------------------------------
int32_t SetAddressWay_Mabco( uint8_t ucAddress, uint8_t *pReceiveBuffer,
                            int32_t iLengthReceiveBuffer )
{
    int32_t iRet = GD_OK;

    NewBuffer();
    AddCharToBuffer( CAC_CONTRAL_COMMAND );
    AddCharToBuffer( CA_ECU_ENTER_MABCO );
    AddCharToBuffer( ucAddress );

    iRet = DisposeSendAndReceive( pReceiveBuffer, iLengthReceiveBuffer );
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 亚太BOSCH通信方式
// Input  : ucAddress               地址码
//          wLengthReceiveBuffer    接收缓冲区长度
// Output : pReceiveBuffer          接收缓冲区指针
// Return : 成功         版本信息长度
//          失败         出错代码
// Info   : none
//------------------------------------------------------------------------------
int32_t yatai_BoschEnterReturnVersion( uint8_t ucAddress, uint8_t *pReceiveBuffer,
                                      int32_t iLengthReceiveBuffer )
{
    int32_t iRet = GD_OK;

    NewBuffer();
    AddCharToBuffer( CAC_CONTRAL_COMMAND );
    AddCharToBuffer( CAC_ECU_YATAI_BOSCH_ENTER_GET_VERSION );
    AddCharToBuffer( ucAddress );

    iRet = DisposeSendAndReceive( pReceiveBuffer, iLengthReceiveBuffer );
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: BOSCH通信方式2
// Input  : ucAddress               地址码
//          wLengthReceiveBuffer    接收缓冲区长度
// Output : pReceiveBuffer          接收缓冲区指针
// Return : 成功         版本信息长度
//          失败         出错代码
// Info   : none
//------------------------------------------------------------------------------
int32_t BoschEnterReturnVersion2( uint8_t ucAddress, uint8_t *pReceiveBuffer,
                                 int32_t iLengthReceiveBuffer )
{
    int32_t iRet = GD_OK;

    NewBuffer();
    AddCharToBuffer( CAC_CONTRAL_COMMAND );
    AddCharToBuffer( CAC_ECU_BOSCH_ENTER_GET_VERSION2 );
    AddCharToBuffer( ucAddress );

    iRet = DisposeSendAndReceive( pReceiveBuffer, iLengthReceiveBuffer );
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 设置链路保持命令
// Input  : pEnterFrameCommand  发送给ECU（10H）30H确认帧，
//                              带55+AA+LEN+DATA1+DATA2+...+DATAn+CS
//          ucCommandLength     发送确认帧的长度，长度不包括长度本身，即要发送数的实际长度
// Output :
// Return : 成功         GD_OK
//          失败         出错代码
// Info   : none
//------------------------------------------------------------------------------
int32_t SetEnterFrame( uint8_t *pEnterFrameCommand, int32_t iCommandLength )
{
    int32_t iRet = GD_OK;

    if( ( iCommandLength > 86 - 3 ) || (iCommandLength < 0 ) )
    {
        return EC_PARAMETER_ERROR;
    }

    NewBuffer();
    AddCharToBuffer( CAC_CONTRAL_COMMAND );
    AddCharToBuffer( CAC_RETURN_ECU_10_ANSWER_FRAME );
    AddCharToBuffer( (uint8_t)( iCommandLength & 0xFF ) );
    if( iCommandLength != 0 )
    {
        AddToBuffer( iCommandLength, pEnterFrameCommand );
    }
    iRet = DisposeSendAndReceive( NULL, 0 );
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 空函数
// Input  :
// Output :
// Return : 成功         1
// Info   : none
//------------------------------------------------------------------------------
uint8_t OpenStdCommDll(void)
{
    return 1;
}

//------------------------------------------------------------------------------
// Funtion: CAN 初始化
// Input  : ucCanBitRate  波特率
//          pucFilterId   过滤ID
// Output :
// Return : 成功         GD_OK
//          失败         出错代码
// Info   : none
//------------------------------------------------------------------------------
int8_t CanInitialize( uint8_t ucCanBitRate, uint8_t * pucFilterId )
{
    uint8_t ucTemp = 0x00;
    BeginCombination();
    SetCommunicationMaximumWaitTime( 18000 );
    EnableLLine( CA_L_LINE_DISABLE );
    SetEcuCommunicationVoltageLogic( CA_12V,CA_POSITIVE );
    SetEctBaudRate( 57600 );
    SetCommunicationParameter( 1, 8, 1, 0 );
    SetEcuCommTimeInterval( 3, 6, 250 );
    EndCombination();

    do
    {
        if( ucCanBitRate == 0x05 )
        {
            if( !SetCanIOLine( FALSE ) )
            {
                break;
            }
        }
        else
        {
            if( !SetCanIOLine( TRUE ) )
            {
                break ;
            }
        }
        if(!SetCanBitRate(FALSE,ucCanBitRate))
        {
            break;
        }
        ucTemp = pucFilterId[ 0 ];
        pucFilterId[ 0 ] &= 0x0F;
        if( !SetCanFilterId(FALSE,pucFilterId) )
        {
            break;
        }
        if( ( ucTemp & 0xF0 ) == 0xF0 )
        {
            SetMaskFilterId( pucFilterId+pucFilterId[ 0 ] * 4 + 1 );
        }
        if( !EnableListenOnly( FALSE ) )
        {
            break;
        }

        return TRUE;
    }
    while( 0 );
    return FALSE;
}

//------------------------------------------------------------------------------
// Funtion: CAN 初始化
// Input  : ucCanBitRate  波特率
//          pucFilterId   过滤ID
// Output :
// Return : 成功         GD_OK
//          失败         出错代码
// Info   : 从本版本起增加了掩码滤波功能
//          20070831 设置两组掩码滤波ID 闫朝国
//------------------------------------------------------------------------------
int8_t SetMaskFilterDoubleId( uint8_t *pucMaskFilterId )
{
    int32_t i,iRet;
    uint8_t pucBuffer[80];
    //    uint8_t pBuf[100];
    uint8_t pucSendCommand[ 80 ] = { 0x55,0xAA,0x0A,0x60,0x06,0x00,0x00,0x00 };
    if( pucMaskFilterId == NULL )
    {
        return FALSE;
    }
    if( pucMaskFilterId[ 0 ] > 2 )
    {
        return FALSE;
    }

    for(i=0; i<8*pucMaskFilterId[0]; i++)
    {
        pucSendCommand[5+i]=pucMaskFilterId[i+1];
    }
    pucSendCommand[2] = pucMaskFilterId[0]*8+2;
    pucSendCommand[pucSendCommand[2]+3] = 0x00;

    for( i=2; i<pucSendCommand[2]+3; i++ )
    {
        pucSendCommand[pucSendCommand[2]+3] ^= pucSendCommand[i];
    }

    iRet = OneToOne_SendDataToEcuGetAnswer(CA_ONCE_ONLY,pucBuffer,80,1,pucSendCommand[2]+4,pucSendCommand,CA_AUTO_RECOGNIZE,2,4);
    if( iRet > 0 )
    {
        return TRUE;
    }

    return FALSE;
}

//------------------------------------------------------------------------------
// Funtion: Toyota专用CAN BUS 通讯函数1：发多帧收多帧
//          发送帧数由pDataToSend第1个字节决定接收帧数从接收到的数据判断,
//          直到接收到最后一帧不是多帧开始帧为止。
//          遇到最后一帧是多帧开始帧时会自动发送0x30流控制帧激发连续帧
// Input  : iSendDataLength 所要发送数据的长度.
//          pDataToSend  要发送的数据.(第一个字节表示发送的帧数，第二个字节为接
//                       收有效字节数 第三个字节表示ID个数)
//          iRecBufferLength   接收缓冲区长度
// Output : pRecBuffer   接收数据缓冲.前两个字节表示接收到的帧数
// Return : 成功         GD_OK
//          失败         出错代码
// Info   : 6202h
//------------------------------------------------------------------------------
int32_t CanSendMulFrameGetMulFrame( int32_t iSendDataLength,
                                   uint8_t *pDataToSend, uint8_t *pRecBuffer,
                                   int32_t iRecBufferLength )
{

    int32_t iRet=GD_OK;

    NewBuffer();
    do
    {
        if( iSendDataLength <= 0 ) break;
        if( iRecBufferLength < 0 )
        {
            iRet = EC_PARAMETER_ERROR;
            break;
        }
        if( iRecBufferLength == 0 )
        {
            pRecBuffer = NULL;
        }

        AddCharToBuffer( CAC_DATA_SEND_RECEIVE_1 );
        AddCharToBuffer( CAC_CAN_SEND_MUL_FRAME_GET_MUL_FRAME );
        //    AddCharToBuffer((uint8_t)(iSendDataLength&0xFF));
        AddToBuffer( (int32_t)iSendDataLength, pDataToSend );
        //    AddCharToBuffer((uint8_t)(((iMaxWaitTime+5)/10)&0xFF));
    }
    while( 0 );

    if( iRet == GD_OK )
    {
        iRet = DisposeSendAndReceive( pRecBuffer, (int32_t)iRecBufferLength );
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: CAN BUS 通讯函数1：发多帧收多帧
//          发送帧数由pDataToSend第1个字节决定接收帧数从接收到的数据判断,
//          直到接收到最后一帧不是多帧开始帧为止。
//          遇到最后一帧是多帧开始帧时会自动发送0x30流控制帧激发连续帧
// Input  : iSendDataLength 所要发送数据的长度.
//          pDataToSend  要发送的数据.(第一个字节表示发送的帧数，第二个字节为接收有效字节数
//                       第三个字节表示ID个数)
//          iRecBufferLength   接收缓冲区长度
// Output : pRecBuffer   接收数据缓冲.前两个字节表示接收到的帧数
// Return : 成功         GD_OK
//          失败         出错代码
// Info   : 6202h
//------------------------------------------------------------------------------
int32_t CanSendMulFrameGetMulFrameGN( int32_t iSendDataLength,
                                     uint8_t *pDataToSend, uint8_t *pRecBuffer,
                                     int32_t iRecBufferLength )
{

    int32_t iRet = GD_OK;

    NewBuffer();
    do
    {
        if( iSendDataLength <= 0 )
        {
            break;
        }
        if(iRecBufferLength < 0 )
        {
            iRet = EC_PARAMETER_ERROR;
            break;
        }
        if( iRecBufferLength == 0 )
            pRecBuffer=NULL;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_CAN_SEND_MUL_FRAME_GET_MUL_FRAME_GN);
        //AddCharToBuffer((uint8_t)(iSendDataLength&0xFF));
        AddToBuffer((int32_t)iSendDataLength,pDataToSend);
        //AddCharToBuffer((uint8_t)(((iMaxWaitTime+5)/10)&0xFF));
    }
    while( 0 );

    if( iRet == GD_OK )
    {
        iRet = DisposeSendAndReceive( pRecBuffer, (int32_t)iRecBufferLength );
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: BENZ CAN BUS 通讯函数1：发多帧收多帧
//          发送帧数由pDataToSend第1个字节决定接收帧数从接收到的数据判断,
//          直到接收到最后一帧不是多帧开始帧为止。
//          遇到最后一帧是多帧开始帧时会自动发送0x30流控制帧激发连续帧
// Input  : iSendDataLength 所要发送数据的长度.
//          pDataToSend  要发送的数据.(第一个字节表示发送的帧数，第二个字节为接收有效字节数
//                       第三个字节表示ID个数)
//          iRecBufferLength   接收缓冲区长度
// Output : pRecBuffer   接收数据缓冲.前两个字节表示接收到的帧数
// Return : 成功         GD_OK
//          失败         出错代码
// Info   : 0x6225
//------------------------------------------------------------------------------
int32_t CanSendMulFrameGetMulFrameGN_BENZ( int32_t iSendDataLength,
                                          uint8_t *pDataToSend, uint8_t *pRecBuffer,
                                          int32_t iRecBufferLength )
{

    int32_t iRet=GD_OK;

    NewBuffer();
    do
    {
        if( iSendDataLength <= 0 )
        {
            break;
        }
        if( iRecBufferLength < 0 )
        {
            iRet = EC_PARAMETER_ERROR;
            break;
        }
        if( iRecBufferLength == 0 )
        {
            pRecBuffer=NULL;
        }
        AddCharToBuffer( CAC_DATA_SEND_RECEIVE_1 );
        AddCharToBuffer( CAC_CAN_SEND_MUL_FRAME_GET_MUL_FRAME_GN_BENZ );
        //    AddCharToBuffer((uint8_t)(iSendDataLength&0xFF));
        AddToBuffer((int32_t)iSendDataLength,pDataToSend);
        //    AddCharToBuffer((uint8_t)(((iMaxWaitTime+5)/10)&0xFF));
    }
    while( 0 );

    if( iRet == GD_OK )
    {
        iRet = DisposeSendAndReceive(pRecBuffer,(int32_t)iRecBufferLength);
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 通用型发多帧收多帧: 适用GM OPEL数据流模式
//          具体可以参见OPEL和GMCAN BUS协议数据流部分多帧格式
// Input  : iSendFrameNum 发送帧数.
//          iRecFrameNum 接收帧数.
//          iFrameIdNum 帧中ID字节个数:标准帧02,扩张帧04,其他视实际车型填写
//          iSendDataLen 发送命令总长度,即pDataToSend中命令字节个数
//          *pDataToSend 发送命令缓冲区 内部格式为:   [0D 61 01 08 FC 00 ---第一帧数据---],
//           [0D 61 01 08 FC 00 ---第二帧数据---] 即完整帧中去掉55 AA 和 CS
//           注:多帧命令一帧一帧逐一往后排列,每帧第一个字节为整帧数据长度,帧长度不固定
// Output : uint8_t *pRecBuf 接收缓冲区
// Return : 成功         接收的总帧数
//          失败         通讯出错  <= 0
// Info   : 6207h
//------------------------------------------------------------------------------
int32_t GMOPEL_CanSendMulFrameGetMulFrame( int32_t iSendFrameNum, int32_t iRecFrameNum,
                                          int32_t iFrameIdNum, int32_t iSendDataLen,
                                          uint8_t *pDataToSend, uint8_t *pRecBuf,
                                          int32_t iRecBufMaxLen )
{
    int32_t iRet = GD_OK;

    NewBuffer();
    do
    {
        if( iSendDataLen<=0 || iSendDataLen>255 )
        {
            iRet = EC_PARAMETER_ERROR;
            break;
        }
        if( iRecBufMaxLen < 0 )
        {
            iRet = EC_PARAMETER_ERROR;
            break;
        }
        if( iRecBufMaxLen == 0 )
        {
            pRecBuf = NULL;
        }
        AddCharToBuffer( CAC_DATA_SEND_RECEIVE_1 );                                //62
        AddCharToBuffer( CAC_CAN2_SEND_1_FRAME_QUICK_GET_INFO_FRAME_NUM );        //07
        AddCharToBuffer( (uint8_t)(iSendFrameNum&0xFF) );                            //发送总帧数
        AddCharToBuffer( (uint8_t)(iRecFrameNum&0xFF) );                                //接收总帧数
        AddCharToBuffer( (uint8_t)(iFrameIdNum&0xFF) );                                //ID字节个数
        AddToBuffer( (int32_t)iSendDataLen,pDataToSend );                            //发送命令缓冲区
    }
    while( 0 );
    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pRecBuf,(int32_t)iRecBufMaxLen);

        //ziyingzhu modify 2009-6-22 17:16:23 这个问题也可以在下位机处理
        //pRecBuf[0]为帧数的高字节，pRecBuf[1]为帧数的低字节
        if((iRet>0)&&(pRecBuf[0]==0)&&(pRecBuf[1]==0))
            //return EC_TIMEOVER;
            iRet = EC_TIMEOVER;

    }
    DelBuffer();
    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: Toyota专用CAN BUS 通讯函数2：发A2模式专用帧接受多帧（读数据流值）。
//          和CanSendMulFrameGetMulFrame相比区别在于：
//          pDataToSend第1个字节不是发送帧数，而是要接收到的总有效字节数，
//         来结束无限重复循环的多帧遇到最后一帧是多帧开始帧时会自动发送0x30流控制帧激发连续帧
//          具体见Toyota CAN BUS协议中多帧格式
// Input  : iSendDataLength  所要发送数据的长度.
//          pDataToSend  要发送的数据.
//          pRecBuffer  接收数据缓冲.前两个字节表示接收到的帧数
//          iRecBufferLength 接收缓冲区长度.
// Output : uint8_t *pRecBuf 接收缓冲区
// Return : 成功         接收的总帧数
//          失败         通讯出错  <= 0
// Info   : 6213h
//------------------------------------------------------------------------------
int32_t CanSendA2_ModeFrameGetMulFrame( int32_t iSendDataLength,
                                       uint8_t *pDataToSend, uint8_t *pRecBuffer,
                                       int32_t iRecBufferLength )
{

    int32_t iRet=GD_OK;

    NewBuffer();
    do
    {
        if(iSendDataLength<=0 || iSendDataLength>255)
        {
            iRet = EC_PARAMETER_ERROR;
            break;
        }
        if( iRecBufferLength < 0 )
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if( iRecBufferLength == 0 )
        {
            pRecBuffer = NULL;
        }

        AddCharToBuffer( CAC_DATA_SEND_RECEIVE_1 );
        AddCharToBuffer( CAC_CAN_SEND_A2_MODE_FRAME_GET_MUL_FRAME );
        //    AddCharToBuffer((uint8_t)(iSendDataLength&0xFF));
        AddToBuffer( (int32_t)iSendDataLength,pDataToSend );
        //    AddCharToBuffer((uint8_t)(((iMaxWaitTime+5)/10)&0xFF));
    }
    while( 0 );

    if( iRet == GD_OK )
    {
        iRet = DisposeSendAndReceive( pRecBuffer, (int32_t)iRecBufferLength );
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 设置上位机与SMARTBOX通信应答的最大等待时间
// Input  :
// Output :
// Return : 成功    返回从ECU收到的数据长度
//          失败    出错代码
// Info   :
//------------------------------------------------------------------------------
void SetCommunicationMaximumWaitTime( int32_t iCommunicationMaximumWaitTime )
{

}

//------------------------------------------------------------------------------
// Funtion: 通迅类型设置
// Input  : ucCommType
//                      0x00 普通通迅模式
//                      0x01 双线CANBUS
//                      0X02 单线CANBUS
//                      0X03 PWM/VPW 模式
// Output :
// Return : 成功    GD_OK
//          失败    出错代码
// Info   :
//------------------------------------------------------------------------------
int32_t SelectCommMode( uint8_t ucCommType )
{
    s_SmartboxStruct.ucCommType=ucCommType ;
    return SetIoParameter();
}

//------------------------------------------------------------------------------
// Funtion: 控制L线是否输出有效
// Input  : bEnableLLine 为TRUE则L线输出有效,为FALSE禁止L线输出
// Output :
// Return : 成功    GD_OK
//          失败    出错代码
// Info   :
//------------------------------------------------------------------------------
int32_t EnableLLine( int8_t bEnableLLine )
{
    s_SmartboxStruct.bEnableLLine = bEnableLLine;
    return SetIoParameter();
}

//------------------------------------------------------------------------------
// Funtion: 设置与ECU通信的工作电压
// Input  : ucWorkVoltage ECU通信的工作电压 其取值范围是:CA_12V或CA_5V
//          ucLogic       ECU通信的电压逻辑
// Output :
// Return : 成功    GD_OK
//          失败    出错代码
// Info   :
//------------------------------------------------------------------------------
int32_t SetEcuCommunicationVoltageLogic( uint8_t ucWorkVoltage, uint8_t ucLogic )
{
    if( ucWorkVoltage != CA_12V && ucWorkVoltage != CA_5V )
    {
        return EC_PARAMETER_ERROR;
    }
    if( ucLogic != CA_POSITIVE && ucLogic != CA_NEGATIVE )
    {
        return EC_PARAMETER_ERROR;
    }

    s_SmartboxStruct.ucEcuCommunicationVoltage = ucWorkVoltage;
    s_SmartboxStruct.ucLogic = ucLogic;

    return SetIoParameter();
}

//------------------------------------------------------------------------------
// Funtion: 设置DTS电压
// Input  : ucVoltageLevel DTS电压 取值范围是: 低电平为CA_LOW  高电平为CA_HIGH
// Output :
// Return : 成功    GD_OK
//          失败    出错代码
// Info   :
//------------------------------------------------------------------------------
int32_t SetDtsVoltage( uint8_t ucVoltageLevel )
{
    s_SmartboxStruct.ucDTS_Volage = ucVoltageLevel;
    return SetIoParameter();
}

//------------------------------------------------------------------------------
// Funtion: 设置 对ECU通讯的 串口波特率
// Input  : fBps - 通讯波特率
// Output :
// Return : 成功    GD_OK
//          失败    出错代码
// Info   :
//------------------------------------------------------------------------------
int32_t SetEcuBaudRate( float fBps )
{
    int32_t iTemp = 0;
    int32_t iRet = GD_OK;

    s_SmartboxStruct.fBps = fBps;

    iTemp = (int32_t)fBps;
    NewBuffer();
    AddCharToBuffer( CAC_CONTRAL_COMMAND );
    AddCharToBuffer( CAC_ECU_COMMUNICATION_BAUD_RATE );
    AddCharToBuffer( (uint8_t)((iTemp>>24)&0xFF) );
    AddCharToBuffer( (uint8_t)((iTemp>>16)&0xFF) );
    AddCharToBuffer( (uint8_t)((iTemp>>8)&0xFF) );
    AddCharToBuffer( (uint8_t)(iTemp&0xFF) );

    if( s_SmartboxStruct.ucStopBit == 2 )
    {
        AddCharToBuffer( CA_2_STOP_BIT );
    }
    else
    {
        AddCharToBuffer( (uint8_t)s_SmartboxStruct.ucCheckBitWay );
    }

    iRet = DisposeSendAndReceive( NULL, 0 );
    DelBuffer();
    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 设置与ECU通信参数
// Input  : ucStartBit 起始位长度
//          ucDataBit  数据位长度
//          ucStopBit  停止位长度
//          ucCheckWay  校验方式
// Output :
// Return : 成功    GD_OK
//          失败    出错代码
// Info   :
//------------------------------------------------------------------------------
int32_t SetCommunicationParameter( uint8_t ucStartBit, uint8_t ucDataBit,
                                  uint8_t ucStopBit, uint8_t ucCheckWay )
{
    if( ucStartBit != 1 )
    {
        return EC_PARAMETER_ERROR;
    }
    if( ucDataBit != 8 )
    {
        return EC_PARAMETER_ERROR;
    }
    if(ucStopBit==1)
    {
        if( ucCheckWay==CA_ODD || ucCheckWay==CA_EVEN ||
            ucCheckWay==CA_NONE||ucCheckWay==CA_IDFDATA_COM )
        {
            s_SmartboxStruct.ucStartBit=ucStartBit;
            s_SmartboxStruct.ucDataBit=ucDataBit;
            s_SmartboxStruct.ucStopBit=ucStopBit;
            s_SmartboxStruct.ucCheckBitWay=ucCheckWay;
        }
        else
        {
            return EC_PARAMETER_ERROR;
        }
    }
    else if(ucStopBit==2)
    {
        if(ucCheckWay==CA_NONE)
        {
            s_SmartboxStruct.ucStartBit=ucStartBit;
            s_SmartboxStruct.ucDataBit=ucDataBit;
            s_SmartboxStruct.ucStopBit=ucStopBit;
            s_SmartboxStruct.ucCheckBitWay=ucCheckWay;
        }
        else
        {
            return EC_PARAMETER_ERROR;
        }
    }
    else
    {
        return EC_PARAMETER_ERROR;
    }

    return SetEctBaudRate( s_SmartboxStruct.fBps );
}

//------------------------------------------------------------------------------
// Funtion: 设置与ECU通信参数
// Input  : pLinkCommand    链路保持命令缓冲区,如为NULL则停止链路保持
//          ucCommandLength 链路保持命令长度, 如为0则停止链路保持
//          ucLinkCommandReturnLength    链路保持ECU返回字节数
//          wLinkKeepTime   链路保持间隔时间,单位毫秒; 取值范
//                          围: 大于0, 小于等于2550毫秒
// Output :
// Return : 成功    GD_OK
//          失败    出错代码
// Info   :
//------------------------------------------------------------------------------
int32_t SetLinkKeep(uint8_t *pLinkCommand, int32_t iCommandLength,
                    int32_t iLinkCommandReturnLength,
                    int32_t iLinkKeepTime )
{
    int32_t iRet=GD_OK;

    if((iCommandLength>255-3)||(iCommandLength<0))
    {
        return EC_PARAMETER_ERROR;
    }
    if((iLinkCommandReturnLength>255)|| (iLinkCommandReturnLength<0))
    {
        return EC_PARAMETER_ERROR;
    }
    if((iLinkKeepTime>2550) || (iLinkKeepTime<0))
    {
        return EC_PARAMETER_ERROR;
    }

    if( (pLinkCommand==NULL) || (iCommandLength==0)
        || (iLinkKeepTime==0) )
    {
        pLinkCommand=NULL;
        iCommandLength=0;
        iLinkCommandReturnLength=0;
        iLinkKeepTime=0;
    }

    NewBuffer();
    AddCharToBuffer( CAC_CONTRAL_COMMAND );
    AddCharToBuffer( CAC_ECU_LINK_KEEP );
    AddCharToBuffer( (uint8_t)(iCommandLength&0xFF) );
    if( iCommandLength != 0 )
    {
        AddToBuffer( iCommandLength,pLinkCommand );
    }
    AddCharToBuffer( (uint8_t)(iLinkCommandReturnLength&0xFF) );
    AddCharToBuffer( (uint8_t)(((iLinkKeepTime+5)/10)&0xFF) );

    iRet = DisposeSendAndReceive(NULL,0);
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 地址码通信方式
// Input  : ucAddressCode       地址码
//          pReceiveBuffer      接收缓冲区
//          iLengthReceiveBuffer接收缓冲区长度
//          fBps                与ECU通信波特率
//          bRecognizeBaudRate  需要识别并自动设置波特率
//          bSecondByteReverseReturnToECU 第二个字节是否要求取反发回
//          iReceiveFrameNumber 要接收的帧数,取值范围:0~255(不包括取反发回的字节)
// Output : 后面参数依次为:
//          接收的第1帧数据长度,类型:int32_t
//          接收的第2帧数据长度,类型:int32_t
//                  ...   ...
//          接收的第iReceiveFrameNumber帧数据长度
// Return : 成功    返回从ECU接收的版本信息长度,如无版本信息
//          失败    出错代码
// Info   :
//------------------------------------------------------------------------------
int32_t AddressCodeWay( uint8_t ucAddressCode, uint8_t *pReceiveBuffer,
                       int32_t iLengthReceiveBuffer, float fBps,
                       int8_t bRecognizeBaudRate,
                       int8_t bSecondByteReverseReturnToECU,
                       int32_t iReceiveFrameNumber, ... )
{
    int32_t iRet=GD_OK;

    if( iReceiveFrameNumber < 0 || iReceiveFrameNumber > 255 )
    {
        return EC_PARAMETER_ERROR;
    }

    NewBuffer();
    AddCharToBuffer( CAC_CONTRAL_COMMAND );
    AddCharToBuffer( CAC_ECU_ADDRESS_CODE_WAY );
    if( fabs( fBps - 5 ) < 0.0001 )
    {
        AddCharToBuffer( 1 ); //way 1              5bps
        AddCharToBuffer( ucAddressCode );
        AddCharToBuffer( (uint8_t)(bRecognizeBaudRate?0xFF:0x00) );
    }
    else if(fabs(fBps-0xc8)<0.0001)
    {
        AddCharToBuffer( 3 ); //way 3                200bps
        AddCharToBuffer( ucAddressCode );
        AddCharToBuffer( (uint8_t)(bRecognizeBaudRate?0xFF:0x00) );
    }
    else
    {
        AddCharToBuffer( 2 ); //way 2                其他
        AddCharToBuffer( ucAddressCode );
        if( fBps > 1000.0 )
        {
            AddCharToBuffer((uint8_t)((((int32_t)(65536-18432000/32/fBps))>>8)&0xFF));
            AddCharToBuffer((uint8_t)(((int32_t)(65536-18432000/32/fBps))&0xFF));
        }
        else
        {      //2003.3.26 add
            //200 bps
            int32_t iTemp=(int32_t)(fBps+0.5);
            AddCharToBuffer((uint8_t)((iTemp>>8)&0xFF));
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
        }
    }

    AddCharToBuffer((uint8_t)(bSecondByteReverseReturnToECU?0xFF:0x00));
    AddCharToBuffer((uint8_t)(iReceiveFrameNumber&0xFF));

    if(iReceiveFrameNumber<=0)
    {
        AddCharToBuffer((uint8_t)(0));
    }
    else
    {
        int32_t i,iTemp;
        va_list args;
        va_start( args, iReceiveFrameNumber );
        for( i = 0; i < iReceiveFrameNumber; i++ )
        {
            iTemp = va_arg( args, int32_t );
            if((iTemp>255) || (iTemp<=0) )
            {
                iRet = EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
        }
        va_end( args );
    }

    if( iRet == GD_OK )
    {
        iRet = DisposeSendAndReceive( pReceiveBuffer, iLengthReceiveBuffer );
    }
    DelBuffer();

    if( iRet >= 0 )
    {
        if( iReceiveFrameNumber == 0 )
        {
            return GD_OK;
        }
        else
        {
            return iRet;
        }
    }

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: BOSCH通信方式
// Input  : ucAddress      地址码
//          wLengthReceiveBuffer 接收缓冲区长度
//          FlagLLine      LLINE 开关标志
// Output : pReceiveBuffer 接收缓冲区指针
// Return : 成功    版本信息长度
//          失败    出错代码
// Info   :
//------------------------------------------------------------------------------
int32_t BoschEnterReturnVersionAudi( uint8_t ucAddress, uint8_t *pReceiveBuffer,
                                    int32_t iLengthReceiveBuffer,
                                    uint8_t FlagLLine )
{
    int32_t iRet=GD_OK;

    NewBuffer();
    AddCharToBuffer(CAC_CONTRAL_COMMAND);
    AddCharToBuffer(CAC_ECU_BOSCH_ENTER_GET_VERSION);
    AddCharToBuffer(FlagLLine);
    AddCharToBuffer(ucAddress);
    iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 设置过滤 ID
// Input  : pucFilterId 过滤 ID
// Output :
// Return : 成功    TRUE
//          失败    FALSE
// Info   :
//------------------------------------------------------------------------------
int8_t SetMaskFilterId( uint8_t *pucFilterId )
{
    int32_t i,iRet;
    uint8_t pucBuffer[80];
    //    uint8_t pBuf[100];
    uint8_t pucSendCommand[80]={0x55,0xAA,0x0A,0x60,0x06,0x00,0x00,0x00};
    if(pucFilterId==NULL) return FALSE;

    for(i=0; i<8; i++) pucSendCommand[5+i]=pucFilterId[i];

    pucSendCommand[pucSendCommand[2]+3]=0x00;
    for(i=2; i<pucSendCommand[2]+3; i++) pucSendCommand[pucSendCommand[2]+3]^=pucSendCommand[i];

    iRet=OneToOne_SendDataToEcuGetAnswer(CA_ONCE_ONLY,pucBuffer,80,1,pucSendCommand[2]+4,pucSendCommand,CA_AUTO_RECOGNIZE,2,4);
    if(iRet>0) return TRUE;

    return FALSE;
}

//------------------------------------------------------------------------------
// Funtion: 向ECU发送数据,不需要ECU应答
// Input  : ucSendTimes        发送次数
//          iFrameNumber    向ECU发送的帧数
//           后面参数依次为:
//           发送到ECU的第1帧长度, 数据类型 int32_t
//           发送到ECU的第1帧缓冲区指针
//           发送到ECU的第2帧长度,数据类型 int32_t
//           发送到ECU的第2帧缓冲区指针
//               ... ...
//           发送到ECU的第n帧长度 数据类型 int32_t
//           发送到ECU的第n帧缓冲区指针
// Output :
// Return : 成功    GD_OK
//          失败    出错代码
// Info   :
//------------------------------------------------------------------------------
int32_t SendDataToEcu( uint8_t ucSendTimes, int32_t iFrameNumber, ... )
{
    va_list args;
    int32_t iTemp,i;
    uint8_t *pTemp;

    int32_t iRet=GD_OK;

    NewBuffer();
    do
    {
        if( iFrameNumber > 255 || iFrameNumber <= 0 )
        {
            iRet = EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer( CAC_DATA_SEND_RECEIVE );
        AddCharToBuffer( CAC_SEND_N_FRAME_DATA );
        AddCharToBuffer( (uint8_t)(ucSendTimes&0xFF) );
        AddCharToBuffer( (uint8_t)(iFrameNumber&0xFF) );

        va_start( args, iFrameNumber );
        for ( i = 0; i < iFrameNumber; i++ )
        {
            iTemp = va_arg(args,int32_t);
            if(iTemp>255 || iTemp<=0)
            {
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            pTemp = va_arg( args, uint8_t* );
            AddToBuffer( iTemp, pTemp );
        }
        va_end( args );
        break;
    }
    while( 0 );
    if( iRet == GD_OK )
    {
        iRet = DisposeSendAndReceive( NULL, 0 );
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 从ECU接收数据
// Input  : ucMode 从ECU接收数据的模式
//                 模式一: 现在TOYOTA-17F,TOYOTA-17使用,在125
//                         bps下,每帧间隔不小于16bit(128ms),起
//                         始标志:1bit低电平,8bit高电平,4bit低
//                         电平,1bit高电平. 每个字节间隔1bit即
//                         8ms.-----接收到的一帧为12字节
//                 模式二: 现在用于悦达,bps:10416, 起始标志:FF
//                         00,2A,46,00; 数据一帧(包括起始标志)
//                         :46字节,间隙:无定义.
//                 模式三: 现在用于庆铃4缸，波特率:9600, 起始
//                  MODE A: 只收模式长度+有效命令+校验
//                  MDDE B: 只收模式00+长度+有效命令+校验
//                  MODE C: 只收模式(80+长度)+有效命令+校验
//                  MODE D: 只收模式80+d+r+长度+有效命令+校验
//                          标志:0x14,0xFC,0x0C; 数据一帧(包括
//                          起始标志)共20字节, 间隙:无定义.
//                  iLengthReceiveBuffer 接收缓冲区长度
// Output : pReceiveBuffer 接收缓冲区指针,内容为ECU返回的一帧数据.
// Return : 成功    从ECU接收到的一帧数据的长度
//          失败    出错代码
// Info   :
//------------------------------------------------------------------------------
int32_t ReceiveDataFromEcu( uint8_t ucMode, uint8_t *pReceiveBuffer,
                           int32_t iLengthReceiveBuffer )
{
    int32_t iRet;
    if( (ucMode<MINIMUM_MODE_RECEIVE_DATA_FROM_ECU)
            || (ucMode>MAXIMUM_MODE_RECEIVE_DATA_FROM_ECU))
    {
        return EC_PARAMETER_ERROR;
    }

    NewBuffer();
    AddCharToBuffer( CAC_DATA_SEND_RECEIVE );
    AddCharToBuffer( CAC_RECEIVE_DATA_FROM_ECU );
    AddCharToBuffer( ucMode );

    iRet = DisposeSendAndReceive( pReceiveBuffer, iLengthReceiveBuffer );
    DelBuffer();
    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 发一帧数据到ECU并接收无数帧ECU的应答, 直到新命令发出就有停止.
// Input  : iLengthReceiveBuffer 接收缓冲区长度
//           iLengthSendData    发送的数据长度
//           pSendData        发送到ECU的数据
//           iLengthReceiveData    从ECU接收的数据长度
// Output : pReceiveBuffer 接收缓冲区指针,格式为:接收到的总帧数(1byte),
//                         第1帧数据长度(1byte),第1帧数据内容
//           (如果其值为CA_AUTO_RECOGNIZE, 后面的参数为表示
//           长度的字节在该帧中的位置(0为起始值, 类型：W_INT2),
//          再下一个参数为取得该帧帧长度而在该表示长度的字节加
//          的数值, 类型：W_INT2)(如果帧数据长度为CA_AUTO_RECOGNIZE, 后面的参数
//          为CA_RECEIVE_END_FLAG,则再下一个参数表示接收帧结束标志字节, 类型：BYTE)
// Return : 成功    实际接收到的总长度
//          失败    出错代码
// Info   :
//------------------------------------------------------------------------------
int32_t SendOneFrameDataToEcuGetAnyFrameAnswer( uint8_t *pReceiveBuffer,
                                               int32_t iLengthReceiveBuffer,
                                               int32_t iLengthSendData,
                                               uint8_t *pSendData,
                                               int32_t iLengthReceiveData,
                                               ... )
{
    va_list args;
    int32_t iTemp;
//    uint8_t *pTemp;

    int32_t iRet = GD_OK;

    NewBuffer();
    do
    {
        if(iLengthSendData>255 || iLengthSendData<=0)
        {
            iRet = EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_SEND_1_FRAME_RECEIVE_ANY_FRAME);
        AddCharToBuffer((uint8_t)(iLengthSendData&0xFF));
        AddToBuffer(iLengthSendData,pSendData);
        AddCharToBuffer((uint8_t)(iLengthReceiveData&0xFF));

        if(iLengthReceiveData==CA_AUTO_RECOGNIZE)
        {
            va_start(args, iLengthReceiveData);

            iTemp = va_arg(args,int32_t);
            if(iTemp>255 || iTemp<CA_RECEIVE_END_FLAG)
            {
                iRet=EC_PARAMETER_ERROR;
                va_end(args);
                break;
            }
            AddCharToBuffer((uint8_t)((iTemp+1)&0xFF));
            if(iTemp==CA_RECEIVE_END_FLAG)
            {
                uint8_t ucTemp=va_arg(args,uint8_t);      //end flag char
                AddCharToBuffer(ucTemp);
            }
            else
            {
                iTemp=va_arg(args,int32_t);
                iTemp*=-1;
                AddCharToBuffer((uint8_t)(iTemp&0xFF));
            }
            va_end(args);
        }
        break;
    }
    while(0);

    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();    //2003.1.7
    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: VolvoKwpSendDataToEcuGetMultiFrameAnswer
// Input  : iLengthReceiveBuffer 接收缓冲区长度
//           iLengthSendData    发送的数据长度
//           pToEcuData        发送到ECU的数据
// Output : pReceiveBuffer 接收缓冲区指针,格式为:接收到的总帧数(1byte),
//                         第1帧数据长度(1byte),第1帧数据内容
// Return : 同于KwpSendDataToEcuGetMultiFrameAnswer函数
// Info   :
//------------------------------------------------------------------------------
int32_t VolvoKwpSendDataToEcuGetMultiFrameAnswer(
                                uint8_t *pToEcuData,
                                uint8_t *pReceiveBuffer,
                                int32_t iLengthReceiveBuffer,
                                int32_t iMaximumWaitTime)
{
    int32_t iRet=GD_OK;
    int32_t iLengthSendToEcuData=CalculateKwpCommandLength(pToEcuData)-1;

    NewBuffer();
    do
    {
        if(iLengthSendToEcuData<=0 || iLengthSendToEcuData>255)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer<0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer==0)
            pReceiveBuffer=NULL;
        if(iMaximumWaitTime>255*10 && iMaximumWaitTime<0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_VOLVO_KWP2000__SEND_1_FRAME_RECEIVE_N_FRAME);
        //AddCharToBuffer(CAC_SEND_1_FRAME_RECEIVE_ANY_FRAME);
        AddCharToBuffer((uint8_t)(iLengthSendToEcuData&0xFF));
        AddToBuffer(iLengthSendToEcuData,pToEcuData);
        AddCharToBuffer((uint8_t)(((iMaximumWaitTime+5)/10)&0xFF));
    }
    while(0);

    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 发多帧数据到ECU并接收多帧ECU的多帧应答(每发送一帧就有对应的一帧应答)
// Input  : iSendDataTimes    发送次数,可以是CA_ONCE_ONLY 或
//                           CA_REPEAT_UNTIL_NEW_COMMAND_BREAK
//           iLengthReceiveBuffer接收缓冲区长度
//           iFrameNumber        发送的帧数
// Output : pReceiveBuffer    接收缓冲区指针,格式为:接收到的总
//                           帧数(1byte),第1帧数据长度(1byte),
//                           第1帧数据内容, ... 第n帧数据长度,
//                           第n帧数据内容
//          后面参数依次为:
//           发送的第1帧数据长度,类型:int32_t
//           发送到ECU的第1帧数据,类型:uint8_t*
//           从ECU接收的第1帧数据长度,类型:int32_t
//               (如果其值为CA_AUTO_RECOGNIZE, 后面的参数为表示
//          长度的字节在该帧中的位置(0为起始值,类型：W_INT2),
//           再下一个参数为取得该帧帧长度而在该表示长度的字节加
//         的数值, 类型：W_INT2)
//              (如果帧数据长度为CA_AUTO_RECOGNIZE, 后面的参数
//           为CA_RECEIVE_END_FLAG,则再下一个参数表示接收帧结束
//           标志字节, 类型：BYTE)
//           发送的第2帧数据长度,类型:int32_t
//            发送到ECU的第2帧数据,类型:uint8_t*
//            从ECU接收的第2帧数据长度,类型:int32_t
//                (如果其值为CA_AUTO_RECOGNIZE, 后面的参数为表示
//            长度的字节在该帧中的位置(0为起始值,类型：W_INT2),
//           再下一个参数为取得该帧帧长度而在该表示长度的字节加
//           的数值, 类型：W_INT2)
//              (如果帧数据长度为CA_AUTO_RECOGNIZE, 后面的参数
//          为CA_RECEIVE_END_FLAG,则再下一个参数表示接收帧结束
//          标志字节, 类型：BYTE)
//               ... ...
//            发送的第iFrameNumber帧数据长度,类型:int32_t
//           发送到ECU的第iFrameNumber帧数据,类型:uint8_t*
//            从ECU接收的第iFrameNumber帧数据长度,类型:int32_t
//                (如果其值为CA_AUTO_RECOGNIZE, 后面的参数为表示
//           长度的字节在该帧中的位置(0为起始值,类型：W_INT2),
//          再下一个参数为取得该帧帧长度而在该表示长度的字节加
//          的数值, 类型：W_INT2)
//              (如果帧数据长度为CA_AUTO_RECOGNIZE, 后面的参数
//          为CA_RECEIVE_END_FLAG,则再下一个参数表示接收帧结束
//          标志字节, 类型：BYTE)
// Return : 同于KwpSendDataToEcuGetMultiFrameAnswer函数
// Info   :
//------------------------------------------------------------------------------
int32_t OneToOne_SendDataToEcuGetAnswer(uint8_t ucSendDataTimes,
                uint8_t *pReceiveBuffer,
                int32_t iLengthReceiveBuffer,
                int32_t iFrameNumber,
                ...)
{
    va_list args;
    int32_t iTemp,i;
    uint8_t *pTemp;

    int32_t iRet=GD_OK;

    NewBuffer();
    do
    {
        if(iLengthReceiveBuffer<=0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        if(iFrameNumber>255 || iFrameNumber<=0)
        {
            iRet = EC_PARAMETER_ERROR;
            break;
        }
        if( (ucSendDataTimes!=CA_ONCE_ONLY)
                &&(ucSendDataTimes!=CA_REPEAT_UNTIL_NEW_COMMAND_BREAK))
        {
            iRet = EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer( CAC_DATA_SEND_RECEIVE );
        AddCharToBuffer( CAC_SEND_N_FRAME_RECEIVE_N_FRAME );
        AddCharToBuffer( ucSendDataTimes );
        AddCharToBuffer( (uint8_t)(iFrameNumber&0xFF) );
        va_start(args, iFrameNumber);
        for( i=0; i<iFrameNumber; i++ )
        {
            iTemp = va_arg(args,int32_t);
            if(iTemp>255 || iTemp<=0)
            {
                iRet = EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            pTemp=va_arg(args,uint8_t*);             //SendCommand
            AddToBuffer(iTemp,pTemp);
            iTemp = va_arg(args,int32_t);
            if(iTemp>255 || iTemp<CA_AUTO_RECOGNIZE)
            {
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            if(iTemp==CA_AUTO_RECOGNIZE)
            {
                iTemp=va_arg(args,int32_t);
                if(iTemp>255 || iTemp< CA_RECEIVE_END_FLAG)
                {
                    iRet=EC_PARAMETER_ERROR;
                    break;
                }

                AddCharToBuffer((uint8_t)((iTemp+1)&0xFF));
                if(iTemp==CA_RECEIVE_END_FLAG)
                {
                    uint8_t ucTemp = va_arg(args,uint8_t);      //end flag char
                    AddCharToBuffer(ucTemp);
                }
                else
                {
                    iTemp = va_arg( args, int32_t );
                    iTemp *= -1;
                    AddCharToBuffer((uint8_t)(iTemp&0xFF));
                }
            }
        }
        va_end( args );
        break;
    }
    while(0);
    
    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();    //2003.1.7
    return iRet;
}


//------------------------------------------------------------------------------
// Funtion: 发一帧数据到ECU并接收ECU应答(已知接收的帧数)
// Input  : iLengthSendToEcuData发送的数据长度                    *
//          pToEcuData            发送到ECU数据                    *
//          iLengthReceiveBuffer接收缓冲区长度                  *
//          iReceiveFrameNumber    从ECU接收的帧数
// Output : pReceiveBuffer    接收缓冲区指针
//          后面参数依次为:
//           发送的第1帧数据长度,类型:int32_t
//           发送到ECU的第1帧数据,类型:uint8_t*
//           从ECU接收的第1帧数据长度,类型:int32_t
//               (如果其值为CA_AUTO_RECOGNIZE, 后面的参数为表示
//          长度的字节在该帧中的位置(0为起始值,类型：W_INT2),
//           再下一个参数为取得该帧帧长度而在该表示长度的字节加
//         的数值, 类型：W_INT2)
//              (如果帧数据长度为CA_AUTO_RECOGNIZE, 后面的参数
//           为CA_RECEIVE_END_FLAG,则再下一个参数表示接收帧结束
//           标志字节, 类型：BYTE)
//           发送的第2帧数据长度,类型:int32_t
//            发送到ECU的第2帧数据,类型:uint8_t*
//            从ECU接收的第2帧数据长度,类型:int32_t
//                (如果其值为CA_AUTO_RECOGNIZE, 后面的参数为表示
//            长度的字节在该帧中的位置(0为起始值,类型：W_INT2),
//           再下一个参数为取得该帧帧长度而在该表示长度的字节加
//           的数值, 类型：W_INT2)
//              (如果帧数据长度为CA_AUTO_RECOGNIZE, 后面的参数
//          为CA_RECEIVE_END_FLAG,则再下一个参数表示接收帧结束
//          标志字节, 类型：BYTE)
//               ... ...
//            发送的第iFrameNumber帧数据长度,类型:int32_t
//           发送到ECU的第iFrameNumber帧数据,类型:uint8_t*
//            从ECU接收的第iFrameNumber帧数据长度,类型:int32_t
//                (如果其值为CA_AUTO_RECOGNIZE, 后面的参数为表示
//           长度的字节在该帧中的位置(0为起始值,类型：W_INT2),
//          再下一个参数为取得该帧帧长度而在该表示长度的字节加
//          的数值, 类型：W_INT2)
//              (如果帧数据长度为CA_AUTO_RECOGNIZE, 后面的参数
//          为CA_RECEIVE_END_FLAG,则再下一个参数表示接收帧结束
//          标志字节, 类型：BYTE)
// Return : 同于KwpSendDataToEcuGetMultiFrameAnswer函数
// Info   :
//------------------------------------------------------------------------------
int32_t KnowReceiveFrameNumber_SendDataToEcuGetAnswer(
                int32_t iLengthSendToEcuData, uint8_t *pToEcuData,
                uint8_t *pReceiveBuffer, int32_t iLengthReceiveBuffer,
                int32_t iReceiveFrameNumber, ... )
{
    va_list args;
    int32_t iTemp,i;

    int32_t iRet=GD_OK;

    NewBuffer();
    do
    {
        if(iLengthSendToEcuData<=0 || iLengthSendToEcuData>255)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer<=0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_KNOW_ANSWER_FRAME_NUMBER__SEND_1_FRAME_RECEIVE_N_FRAME);
        AddCharToBuffer((uint8_t)(iLengthSendToEcuData&0xFF));
        AddToBuffer(iLengthSendToEcuData,pToEcuData);
        AddCharToBuffer((uint8_t)(iReceiveFrameNumber&0xFF));

        va_start(args, iReceiveFrameNumber);
        for (i=0;i<iReceiveFrameNumber;i++)
        {
            iTemp = va_arg(args,int32_t);
            if(iTemp>255 || iTemp<0)
            {
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));

            if(iTemp==CA_AUTO_RECOGNIZE)
            {
                iTemp=va_arg(args,int32_t);
                if(iTemp>255 || iTemp< CA_RECEIVE_END_FLAG)
                {
                    iRet=EC_PARAMETER_ERROR;
                    break;
                }

                AddCharToBuffer((uint8_t)((iTemp+1)&0xFF));
                if(iTemp==CA_RECEIVE_END_FLAG)
                {
                    uint8_t ucTemp=va_arg(args,uint8_t);      //end flag char
                    AddCharToBuffer(ucTemp);
                }
                else
                {
                    iTemp=va_arg(args,int32_t);
                    iTemp*=-1;
                    AddCharToBuffer((uint8_t)(iTemp&0xFF));
                }
            }
        }
        va_end(args);
        break;
    }
    while( 0 );

    if( iRet == GD_OK )
    {
        iRet = DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: ISO协议 发一帧数据到ECU并接收应答(直到ECU不发才结束)
// Input  : iLengthSendToEcuData 发送的数据长度
//          pToEcuData           发送到ECU数据
//          iLengthReceiveBuffer 接收缓冲区长度
//          iMaximumWaitTime     最大等待时间(单位:毫秒)
// Output : pReceiveBuffer 接收缓冲区指针
// Return : 成功    实际接收到的总长度
//           失败    出错代码(小于0)
// Info   :
//------------------------------------------------------------------------------
int32_t IsoSendDataToEcuGetMultiFrameAnswer( int32_t iLengthSendToEcuData,
                                            uint8_t *pToEcuData, uint8_t *pReceiveBuffer,
                                            int32_t iLengthReceiveBuffer,
                                            int32_t iMaximumWaitTime )
{
    int32_t iRet=GD_OK;

    NewBuffer();
    do
    {
        if(iLengthSendToEcuData<=0 || iLengthSendToEcuData>255)
        {
            iRet = EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer<0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer==0)
        {
            pReceiveBuffer=NULL;
        }
        if(iMaximumWaitTime>255*10 && iMaximumWaitTime<0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iMaximumWaitTime<5)
        {
            iMaximumWaitTime=5;
        }

        AddCharToBuffer( CAC_DATA_SEND_RECEIVE );
        AddCharToBuffer( CAC_ISO__SEND_1_FRAME_RECEIVE_N_FRAME );
        AddCharToBuffer( (uint8_t)(iLengthSendToEcuData&0xFF) );
        AddToBuffer( iLengthSendToEcuData,pToEcuData );
        AddCharToBuffer( (uint8_t)(((iMaximumWaitTime+5)/10)&0xFF) );
    }
    while( 0 );

    if(iRet==GD_OK)
    {
        iRet = DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: ISO协议    发多帧数据到ECU并接收多帧应答
// Input  : ucSendDataTimes  发送次数,可以是CA_ONCE_ONLY 或
//                           CA_REPEAT_UNTIL_NEW_COMMAND_BREAK
//          iLengthReceiveBuffer接收缓冲区长度
//          iFrameNumber    发送的帧数
//          后面参数依次为:
//          发送的第1帧数据长度,类型:int32_t
//          发送到ECU的第1帧数据,类型:uint8_t*
//          发送的第2帧数据长度,类型:int32_t
//          发送到ECU的第2帧数据,类型:uint8_t*
//               ... ...
//          发送的第n帧数据长度,类型:int32_t
//          发送到ECU的第n帧数据,类型:uint8_t*
// Output : pReceiveBuffer   接收缓冲区指针,格式为:接收到的总
//                           帧数(1byte),第1帧数据长度(1byte),
//                           第1帧数据内容, ... 第n帧数据长度,
//                           第n帧数据内容.
// Return : 成功    实际接收到的总长度
//          失败    出错代码(小于0)
// Info   :
//------------------------------------------------------------------------------
int32_t IsoSendDataToEcuGetAnswer( uint8_t ucSendDataTimes, uint8_t *pReceiveBuffer,
                                  int32_t iLengthReceiveBuffer, int32_t iFrameNumber,
                                  ... )
{
    va_list args;
    int32_t iTemp,i;
    uint8_t *pTemp;

    int32_t iRet=GD_OK;

    NewBuffer();
    do
    {
        if(ucSendDataTimes!=CA_ONCE_ONLY
            && ucSendDataTimes!=CA_REPEAT_UNTIL_NEW_COMMAND_BREAK)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iFrameNumber>255 || iFrameNumber<=0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_ISO_SEND_N_FRAME_RECEIVE_N_FRAME);
        AddCharToBuffer((uint8_t)(ucSendDataTimes&0xFF));
        AddCharToBuffer((uint8_t)(iFrameNumber&0xFF));

        va_start(args, iFrameNumber);
        for (i=0;i<iFrameNumber;i++)
        {
            iTemp=va_arg(args,int32_t);
            if(iTemp>255 || iTemp<=0)
            {
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            pTemp=va_arg(args,uint8_t*);             //SendCommand
            AddToBuffer(iTemp,pTemp);
        }
        va_end(args);
        break;
    }
    while( 0 );

    if( iRet == GD_OK )
    {
        iRet = DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: KWP2000协议    发多帧数据到ECU并接收多帧应答
// Input  : ucSendDataTimes  发送次数,可以是CA_ONCE_ONLY 或
//                           CA_REPEAT_UNTIL_NEW_COMMAND_BREAK
//          iLengthReceiveBuffer 接收缓冲区长度
//          iFrameNumber    发送的帧数
// Output : pReceiveBuffer  接收缓冲区指针,格式为:接收到的总
//                          帧数(1byte),第1帧数据长度(1byte),
//                          第1帧数据内容, ... 第n帧数据长度,
//                          第n帧数据内容
//          后面参数依次为:                                     *
//           发送到ECU的第1帧数据,类型:uint8_t*
//           发送到ECU的第2帧数据,类型:uint8_t*
//               ... ...
//           发送到ECU的第n帧数据,类型:uint8_t*
// Return : 成功    实际接收到的总长度
//          失败    出错代码(小于0)
// Info   :
//------------------------------------------------------------------------------
int32_t KwpSendDataToEcuGetAnswer( uint8_t ucSendDataTimes, uint8_t *pReceiveBuffer,
                                  int32_t iLengthReceiveBuffer,
                                  int32_t iFrameNumber, ... )
{
    va_list args;
    int32_t iTemp,i;
    uint8_t *pTemp;

    int32_t iRet=GD_OK;

    NewBuffer();
    do
    {
        if(ucSendDataTimes!=CA_ONCE_ONLY
            && ucSendDataTimes!=CA_REPEAT_UNTIL_NEW_COMMAND_BREAK)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iFrameNumber>255 || iFrameNumber<=0)
        {
            iRet = EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer( CAC_DATA_SEND_RECEIVE );
        AddCharToBuffer( CAC_KWP2000_SEND_N_FRAME_RECEIVE_N_FRAME );
        AddCharToBuffer( (uint8_t)(ucSendDataTimes&0xFF) );
        AddCharToBuffer( (uint8_t)(iFrameNumber&0xFF) );

        va_start( args, iFrameNumber );
        for ( i = 0 ; i < iFrameNumber; i++ )
        {
            pTemp = va_arg(args,uint8_t*);                 //SendCommand
            iTemp = CalculateKwpCommandLength(pTemp); //SendData Length
            if(iTemp>255 || iTemp<=0)
            {
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            AddToBuffer(iTemp,pTemp);
        }
        va_end(args);
        break;
    }
    while(0);

    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: KWP2000协议 发一帧数据到ECU并接收应答
//           (直到ECU不发才结束)
// Input  : pToEcuData            发送到ECU数据
//          iLengthReceiveBuffer接收缓冲区长度
//          iMaximumWaitTime    最大等待时间(单位:毫秒)
// Output : pReceiveBuffer        接收缓冲区指针
// Return : 成功    实际接收到的总长度
//          失败    出错代码(小于0)
// Info   :
//------------------------------------------------------------------------------
int32_t KwpSendDataToEcuGetMultiFrameAnswer( uint8_t *pToEcuData, uint8_t *pReceiveBuffer,
                                            int32_t iLengthReceiveBuffer,
                                            int32_t iMaximumWaitTime )
{
    int32_t iRet=GD_OK;
    int32_t iLengthSendToEcuData=CalculateKwpCommandLength(pToEcuData);

    NewBuffer();
    do
    {
        if(iLengthSendToEcuData<=0 || iLengthSendToEcuData>255)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer<0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer==0)
            pReceiveBuffer=NULL;
        if(iMaximumWaitTime>255*10 && iMaximumWaitTime<0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_KWP2000__SEND_1_FRAME_RECEIVE_N_FRAME);
        AddCharToBuffer((uint8_t)(iLengthSendToEcuData&0xFF));
        AddToBuffer(iLengthSendToEcuData,pToEcuData);
        AddCharToBuffer((uint8_t)(((iMaximumWaitTime+5)/10)&0xFF));
    }
    while(0);

    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: KWP2000协议    发多帧数据到ECU并接收多帧应答
// Input  : ucSendDataTimes    发送次数,可以是CA_ONCE_ONLY 或
//                           CA_REPEAT_UNTIL_NEW_COMMAND_BREAK
//           iLengthReceiveBuffer 接收缓冲区长度
//           iFrameNumber        发送的帧数
// Output : pReceiveBuffer    接收缓冲区指针,格式为:接收到的总
//                           帧数(1byte),第1帧数据长度(1byte),
//                           第1帧数据内容, ... 第n帧数据长度,
//                           第n帧数据内容.
//          后面参数依次为:
//          发送的第1帧数据长度,类型:int32_t
//          发送到ECU的第1帧数据,类型:uint8_t*
//          发送的第2帧数据长度,类型:int32_t
//          发送到ECU的第2帧数据,类型:uint8_t*
//               ... ...
//           发送的第n帧数据长度,类型:int32_t
//           发送到ECU的第n帧数据,类型:uint8_t*
// Return : 成功    实际接收到的总长度
//          失败    出错代码(小于0)
// Info   :
//------------------------------------------------------------------------------
int32_t KWP2000_SendDataToEcuGetAnswer( uint8_t ucSendDataTimes, uint8_t *pReceiveBuffer,
                                       int32_t iLengthReceiveBuffer,
                                       int32_t iFrameNumber, ... )
{
    va_list args;
    int32_t iTemp,i;
    uint8_t *pTemp;

    int32_t iRet=GD_OK;

    NewBuffer();
    do
    {
        if(ucSendDataTimes!=CA_ONCE_ONLY
            && ucSendDataTimes!=CA_REPEAT_UNTIL_NEW_COMMAND_BREAK)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iFrameNumber>255 || iFrameNumber<=0)
        {
            iRet = EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_KWP2000_SEND_N_FRAME_RECEIVE_N_FRAME);
        AddCharToBuffer((uint8_t)(ucSendDataTimes&0xFF));
        AddCharToBuffer((uint8_t)(iFrameNumber&0xFF));

        va_start(args, iFrameNumber);
        for (i=0;i<iFrameNumber;i++)
        {
            iTemp=va_arg(args,int32_t);

            if(iTemp>255 || iTemp<=0)
            {
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            pTemp=va_arg(args,uint8_t*);             //SendCommand
            AddToBuffer((int32_t)iTemp,pTemp);
        }
        va_end(args);
        break;
    }
    while(0);

    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: KWP2000协议    发多帧数据到ECU并接收多帧应答
// Input  : ucSendDataTimes    发送次数,可以是CA_ONCE_ONLY 或
//                           CA_REPEAT_UNTIL_NEW_COMMAND_BREAK
//           iLengthReceiveBuffer 接收缓冲区长度
//           iFrameNumber        发送的帧数
// Output : pReceiveBuffer    接收缓冲区指针,格式为:接收到的总
//                           帧数(1byte),第1帧数据长度(1byte),
//                           第1帧数据内容, ... 第n帧数据长度,
//                           第n帧数据内容.
//          后面参数依次为:
//          发送的第1帧数据长度,类型:int32_t
//          发送到ECU的第1帧数据,类型:uint8_t*
//          发送的第2帧数据长度,类型:int32_t
//          发送到ECU的第2帧数据,类型:uint8_t*
//               ... ...
//           发送的第n帧数据长度,类型:int32_t
//           发送到ECU的第n帧数据,类型:uint8_t*
// Return : 成功    实际接收到的总长度
//          失败    出错代码(小于0)
// Info   : 本模块增加了接受是以0X00开头的KWP的模式,但在发送给上位机的时候将0x00给省略
//------------------------------------------------------------------------------
int32_t KwpSendDataToEcuGetAnswer_new( uint8_t ucSendDataTimes, uint8_t *pReceiveBuffer,
                                      int32_t iLengthReceiveBuffer,
                                      int32_t iFrameNumber, ... )
{
    va_list args;
    int32_t iTemp,i;
    uint8_t *pTemp;

    int32_t iRet=GD_OK;

    NewBuffer();
    do
    {
        if(ucSendDataTimes!=CA_ONCE_ONLY
            && ucSendDataTimes!=CA_REPEAT_UNTIL_NEW_COMMAND_BREAK)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iFrameNumber>255 || iFrameNumber<=0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_KWP2000_SEND_N_FRAME_RECEIVE_N_FRAME);
        AddCharToBuffer((uint8_t)(ucSendDataTimes&0xFF));
        AddCharToBuffer((uint8_t)(iFrameNumber&0xFF));

        va_start(args, iFrameNumber);
        for (i=0;i<iFrameNumber;i++)
        {
            pTemp=va_arg(args,uint8_t*);                 //SendCommand
            iTemp=CalculateKwpCommandLength(pTemp); //SendData Length
            if(iTemp>255 || iTemp<=0)
            {
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            AddToBuffer(iTemp,pTemp);
        }
        va_end(args);
        break;
    }
    while(0);

    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: PWM协议    发多帧数据到ECU并接收多帧应答
// Input  : ucSendDataTimes    发送次数,可以是CA_ONCE_ONLY 或
//                           CA_REPEAT_UNTIL_NEW_COMMAND_BREAK
//          iLengthReceiveBuffer接收缓冲区长度
//          iFrameNumber        发送的帧数
// Output : pReceiveBuffer    接收缓冲区指针,格式为:接收到的总
//                           帧数(1byte),第1帧数据长度(1byte),
//                           第1帧数据内容, ... 第n帧数据长度,
//                           第n帧数据内容.
//          后面参数依次为:
//          发送的第1帧数据长度,类型:int32_t
//          发送到ECU的第1帧数据,类型:uint8_t*
//          发送的第2帧数据长度,类型:int32_t
//          发送到ECU的第2帧数据,类型:uint8_t*
//               ... ...
//           发送的第n帧数据长度,类型:int32_t
//           发送到ECU的第n帧数据,类型:uint8_t*
// Return : 成功    实际接收到的总长度
//          失败    出错代码(小于0)
// Info   :
//------------------------------------------------------------------------------
int32_t PWM_SendDataToEcuGetAnswer( uint8_t ucSendDataTimes, uint8_t *pReceiveBuffer,
                                   int32_t iLengthReceiveBuffer,
                                   int32_t iFrameNumber, ... )
{
    va_list args;
    int32_t iTemp,i;
    uint8_t *pTemp;

    int32_t iRet=GD_OK;

    NewBuffer();
    do
    {
        if(ucSendDataTimes!=CA_ONCE_ONLY
            && ucSendDataTimes!=CA_REPEAT_UNTIL_NEW_COMMAND_BREAK)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iFrameNumber>255 || iFrameNumber<=0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_PWM_SEND_N_FRAME_RECEIVE_N_FRAME);
        AddCharToBuffer((uint8_t)(ucSendDataTimes&0xFF));
        AddCharToBuffer((uint8_t)(iFrameNumber&0xFF));

        va_start(args, iFrameNumber);
        for (i=0;i<iFrameNumber;i++)
        {
            iTemp=va_arg(args,int32_t);
            if(iTemp>255 || iTemp<=0)
            {
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            pTemp=va_arg(args,uint8_t*);             //SendCommand
            AddToBuffer(iTemp,pTemp);
        }
        va_end(args);
        break;
    }
    while(0);

    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: VPW协议    发多帧数据到ECU并接收多帧应答
// Input  : ucSendDataTimes    发送次数,可以是CA_ONCE_ONLY 或
//                           CA_REPEAT_UNTIL_NEW_COMMAND_BREAK
//          iLengthReceiveBuffer接收缓冲区长度
//          iFrameNumber        发送的帧数
// Output : pReceiveBuffer    接收缓冲区指针,格式为:接收到的总
//                           帧数(1byte),第1帧数据长度(1byte),
//                           第1帧数据内容, ... 第n帧数据长度,
//                           第n帧数据内容.
//          后面参数依次为:
//          发送的第1帧数据长度,类型:int32_t
//          发送到ECU的第1帧数据,类型:uint8_t*
//          发送的第2帧数据长度,类型:int32_t
//          发送到ECU的第2帧数据,类型:uint8_t*
//               ... ...
//           发送的第n帧数据长度,类型:int32_t
//           发送到ECU的第n帧数据,类型:uint8_t*
// Return : 成功    实际接收到的总长度
//          失败    出错代码(小于0)
// Info   :
//------------------------------------------------------------------------------
int32_t VPW_SendDataToEcuGetAnswer( uint8_t ucSendDataTimes, uint8_t *pReceiveBuffer,
                                   int32_t iLengthReceiveBuffer,
                                   int32_t iFrameNumber, ... )
{
    va_list args;
    int32_t iTemp,i;
    uint8_t *pTemp;

    int32_t iRet=GD_OK;

    NewBuffer();
    do
    {
        if(ucSendDataTimes!=CA_ONCE_ONLY
            && ucSendDataTimes!=CA_REPEAT_UNTIL_NEW_COMMAND_BREAK)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iFrameNumber>255 || iFrameNumber<=0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_VPW_SEND_N_FRAME_RECEIVE_N_FRAME);
        AddCharToBuffer((uint8_t)(ucSendDataTimes&0xFF));
        AddCharToBuffer((uint8_t)(iFrameNumber&0xFF));

        va_start(args, iFrameNumber);
        for (i=0;i<iFrameNumber;i++)
        {
            iTemp=va_arg(args,int32_t);
            if(iTemp>255 || iTemp<=0)
            {
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            pTemp=va_arg(args,uint8_t*);             //SendCommand
            AddToBuffer(iTemp,pTemp);
        }
        va_end(args);
        break;
    }
    while(0);

    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: BOSCH发一帧回一帧或回多帧
// Input  : pSendToEcuData    发送到ECU的数据
//          iReceiveFrameNumber    接收的帧数,可以是以下参数之一
//                   CA_BOSCH_RECEIVE_ONE_FRAME 或
//                   CA_BOSCH_RECEIVE_MULTI_FRAME
//
//          iLengthReceiveBuffer    接收缓冲区长度
// Output : pReceiveBuffer    接收缓冲区指针
// Return : 成功    实际接收到的总长度
//          失败    出错代码(小于0)
// Info   :
//------------------------------------------------------------------------------
int32_t BoschSendDataToEcuGetMultiFrameAnswer( uint8_t *pSendToEcuData,
                                              uint8_t ucReceiveFrameNumber,
                                              uint8_t *pReceiveBuffer,
                                              int32_t iLengthReceiveBuffer )
{
    int32_t iRet=GD_OK;

    if((ucReceiveFrameNumber!=CA_BOSCH_RECEIVE_ONE_FRAME)
        && (ucReceiveFrameNumber!=CA_BOSCH_RECEIVE_MULTI_FRAME))
    {
        return EC_PARAMETER_ERROR;
    }

    NewBuffer();
    AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
    AddCharToBuffer(CAC_BOSCH_SEND_1_FRAME_RECEIVE_N_FRAME);
    AddToBuffer((int32_t)((pSendToEcuData[0]&0xFF)+1),pSendToEcuData);
    AddCharToBuffer((uint8_t)(ucReceiveFrameNumber));

    iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: BOSCH协议    发多帧数据到ECU并接收多帧应答
// Input  : ucSendDataTimes    发送次数,可以是CA_ONCE_ONLY 或
//          iLengthReceiveBuffer接收缓冲区长度
//          iFrameNumber        发送的帧数
// Output : pReceiveBuffer    接收缓冲区指针,格式为:接收到的总
//                           帧数(1byte),第1帧数据长度(1byte),
//                           第1帧数据内容, ... 第n帧数据长度,
//                           第n帧数据内容.
//          后面参数依次为:
//          发送到ECU的第1帧数据,类型:uint8_t*
//           发送到ECU的第2帧数据,类型:uint8_t*
//               ... ...
//           发送到ECU的第iFrameNumber帧数据,类型:uint8_t*
// Return : 成功    实际接收到的总长度
//          失败    出错代码(小于0)
// Info   :
//------------------------------------------------------------------------------
int32_t BoschSendDataToEcuGetAnswer( uint8_t ucSendDataTimes, uint8_t *pReceiveBuffer,
                                    int32_t iLengthReceiveBuffer,
                                    int32_t iFrameNumber, ... )
{
    va_list args;
    int32_t i;
    uint8_t *pTemp;

    int32_t iRet=GD_OK;

    NewBuffer();
    do
    {
        if(ucSendDataTimes!=CA_ONCE_ONLY
            && ucSendDataTimes!=CA_REPEAT_UNTIL_NEW_COMMAND_BREAK)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iFrameNumber>255 || iFrameNumber<=0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_BOSCH_SEND_N_FRAME_RECEIVE_N_FRAME);
        AddCharToBuffer((uint8_t)(ucSendDataTimes&0xFF));
        AddCharToBuffer((uint8_t)(iFrameNumber&0xFF));

        va_start(args, iFrameNumber);
        for (i=0;i<iFrameNumber;i++)
        {
            pTemp=va_arg(args,uint8_t*);             //SendCommand
            AddToBuffer((int32_t)((pTemp[0]&0xFF)+1),pTemp);
        }
        va_end(args);
        break;
    }
    while( 0 );

    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion:
// Input  :
// Output :
// Return : 成功    实际接收到的总长度
//          失败    出错代码(小于0)
// Info   :
//------------------------------------------------------------------------------
int32_t CalculateKwpCommandLength(uint8_t *pSendToEcuCommand)
{
    uint8_t ucKwpHeadHigh2Bit=((pSendToEcuCommand[0]>>6)&0x03);
    uint8_t ucKwpHeadLow6Bit=pSendToEcuCommand[0]&0x3F;

    int32_t iCommandLength=EC_PARAMETER_ERROR;

    if(ucKwpHeadHigh2Bit==0x00)
    {
        if(ucKwpHeadLow6Bit==0)
            iCommandLength=(pSendToEcuCommand[1]&0xFF)+3;
        else
            iCommandLength=ucKwpHeadLow6Bit+2;
    }
    else if(ucKwpHeadHigh2Bit==0x01)
        ;
    else
    {
        //80 c0
        if(ucKwpHeadLow6Bit==0)
            iCommandLength=(pSendToEcuCommand[3]&0xFF)+5;
        //8x cx
        else
            iCommandLength=ucKwpHeadLow6Bit+4;
    }

    return iCommandLength;
}

//------------------------------------------------------------------------------
// Funtion:
// Input  :
// Output :
// Return : 成功    实际接收到的总长度
//          失败    出错代码(小于0)
// Info   :
//------------------------------------------------------------------------------
int32_t CalculateKwpCommandLength_BMW(uint8_t *pSendToEcuCommand)
{
    int32_t iCommandLength=EC_PARAMETER_ERROR;


    iCommandLength=(pSendToEcuCommand[3]&0xFF)+5;

    return iCommandLength;
}

//------------------------------------------------------------------------------
// Funtion: For Holden Normal Ring Link system check in.
//          Receive frames and seek for the frame that lead by Addr_code *
// Input  :
// Output :
// Return : 成功    从ECU接收到的一帧数据的长度
//          失败    出错代码
// Info   :
//------------------------------------------------------------------------------
int32_t HoldenNormalRingLinkCheck( uint8_t ucSendDataTimes, uint8_t Addr_code,
                                  uint8_t *pReceiveBuffer, int32_t iLengthReceiveBuffer )//v2700 zjz add uint8_t ucSendDataTimes,
{
    int32_t iRet;

    NewBuffer();
    AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
    AddCharToBuffer(CAC_HOLDEN_NORMAL_RING_LINK_CHECK);
    AddCharToBuffer(ucSendDataTimes);
    AddCharToBuffer(Addr_code);

    iRet = DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    DelBuffer();
    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 发一帧数据到ECU并接收一帧ECU的应答，一帧数据长度大于256字节.
// Input  : iSendDataLength 发送数据长度
//          pDataToSend    发送缓冲区指针,格式为:数据长度高位
//                        （1BYTE）数据长度低位（1BYTE）, 数据内容,
//          pReceiveBuffer 接收缓冲区指针,格式为:数据长度高位
//                        （1BYTE）数据长度低位（1BYTE）, 数据内容,
//          iLengthReceiveBuffer 接收缓冲区长度
// Output :
// Return : 成功    实际接收到的总长度
//          失败     出错代码(小于0)
// Info   :
//------------------------------------------------------------------------------
int32_t OneToOne_SendLongDataToEcuGetLongAnswer( int32_t iSendDataLength,
                                                uint8_t *pDataToSend,
                                                uint8_t *pReceiveBuffer,
                                                int32_t iLengthReceiveBuffer )
{
    int32_t iRet=GD_OK;
    NewBuffer();
    AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
    AddCharToBuffer(CAC_SNED_LONG_BYTE_RECEIVE_LONG_BYTE);
    AddToBuffer(iSendDataLength,pDataToSend);
    iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    DelBuffer();    //2003.1.7
    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 发一帧数据到ECU并接收一帧ECU的应答，一帧数据长度大于256字节.
// Input  : iSendDataLength 发送数据长度
//          pDataToSend    发送缓冲区指针,格式为:数据长度高位
//                        （1BYTE）数据长度低位（1BYTE）, 数据内容,
//          pReceiveBuffer 接收缓冲区指针,格式为:数据长度高位
//                        （1BYTE）数据长度低位（1BYTE）, 数据内容,
//          iLengthReceiveBuffer 接收缓冲区长度
// Output :
// Return : 成功    实际接收到的总长度
//          失败     出错代码(小于0)
// Info   :
//------------------------------------------------------------------------------
int32_t OneToOne_SendOneByteToEcuGetAnswerLM( int32_t iSendDataLength,
                                             uint8_t *pDataToSend,
                                             uint8_t *pReceiveBuffer,
                                             int32_t iLengthReceiveBuffer )
{

    int32_t iRet=GD_OK;
    NewBuffer();
    AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
    AddCharToBuffer(CAC_SEND_ONEBYTE_RECEIVE_ONEBYTE_LM);
    AddToBuffer(iSendDataLength,pDataToSend);
    iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    DelBuffer();    //2003.1.7
    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: CanSendTwoFrameQuickGetInfo_NewConnector
//          发送一帧数据给ECU, 并接收返回的一帧数据. 收到后在较短时
//          间内迅速发送第二帧命令, 这样才能接收到ECU的某些状态信息,
//          接收的帧数不定. 用于CAN 通讯方式.
// Input  : iSendDataLength  所要发送数据的长度.
//          pDataToSend   要发送的数据.
//          iRecBufferLength  接收缓冲区长度.
// Output : pRecBuffer   接收数据缓冲.
// Return : >  0   接收的总帧数.
//          <  0   通讯出错.
// Info   : 611ah
//------------------------------------------------------------------------------
int32_t CanSendTwoFrameQuickGetInfo_NewConnector( int32_t iSendDataLength,
                                                 uint8_t *pDataToSend,
                                                 uint8_t *pRecBuffer,
                                                 int32_t iRecBufferLength )
{

    int32_t iRet = GD_OK;

    NewBuffer();
    do
    {
        if(iSendDataLength<=0 || iSendDataLength>255)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength<0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength==0)
            pRecBuffer=NULL;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_CAN2_SEND_1_FRAME_QUICK_GET_INFO);//old id==6116 for bejjing bus
        //    AddCharToBuffer((uint8_t)(iSendDataLength&0xFF));
        AddToBuffer(iSendDataLength,pDataToSend);
        //    AddCharToBuffer((uint8_t)(((iMaxWaitTime+5)/10)&0xFF));
    }
    while(0);

    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pRecBuffer,iRecBufferLength);
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: VPW协议 发一帧数据到ECU并接收应答(直到ECU不发才结束)
// Input  : wLengthSendToEcuData  发送的数据长度
//          pToEcuData            发送到ECU数据
//          iLengthReceiveBuffer  接收缓冲区长度
//          iMaximumWaitTime      最大等待时间(单位:毫秒)
// Output : pReceiveBuffer        接收缓冲区指针
// Return : >  0   接收的总帧数.
//          <  0   通讯出错.
// Info   : 611ah
//------------------------------------------------------------------------------
int32_t JAGUARPWM_SendDataToEcuGetMultiFrameAnswer( uint8_t *pPWMFilterID,
                                                   int32_t iLengthSendToEcuData,
                                                   uint8_t *pToEcuData,
                                                   uint8_t *pReceiveBuffer,
                                                   int32_t iLengthReceiveBuffer,
                                                   int32_t iMaximumWaitTime )
{
    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(iLengthSendToEcuData<=0 || iLengthSendToEcuData>255)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer<0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer==0)
            pReceiveBuffer=NULL;
        if(iMaximumWaitTime>255*10 && iMaximumWaitTime<0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iMaximumWaitTime<5)
        {
            iMaximumWaitTime=5;
        }

        AddCharToBuffer( CAC_DATA_SEND_RECEIVE );
        AddCharToBuffer( CAC_PWM__SEND_1_FRAME_RECEIVE_N_FRAME_JG );
        AddCharToBuffer( pPWMFilterID[0] );
        AddCharToBuffer( pPWMFilterID[1] );
        AddCharToBuffer( pPWMFilterID[2] );
        AddCharToBuffer( (uint8_t)(iLengthSendToEcuData&0xFF) );
        AddToBuffer(iLengthSendToEcuData,pToEcuData );
        AddCharToBuffer( (uint8_t)(((iMaximumWaitTime+5)/10)&0xFF) );
    }
    while(0);

    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 针对路虎车型
// Input  : wLengthSendToEcuData  发送的数据长度
//          pToEcuData            发送到ECU数据
//          iLengthReceiveBuffer  接收缓冲区长度
//          iMaximumWaitTime      最大等待时间(单位:毫秒)
// Output : pReceiveBuffer        接收缓冲区指针
// Return : >  0   接收的总帧数.
//          <  0   通讯出错.
// Info   : 611ah
//------------------------------------------------------------------------------
int32_t RangRoverSendDataToEcuGetMultiFrameAnswer( uint8_t *pToEcuData,
                                                  uint8_t *pReceiveBuffer,
                                                  int32_t iLengthReceiveBuffer,
                                                  int32_t iMaximumWaitTime )
{
    int32_t iRet=GD_OK;
    int32_t iLengthSendToEcuData=CalculateKwpCommandLength(pToEcuData);

    NewBuffer();
    do
    {
        if(iLengthSendToEcuData<=0 || iLengthSendToEcuData>255)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer<0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer==0)
            pReceiveBuffer=NULL;
        if(iMaximumWaitTime>255*10 && iMaximumWaitTime<0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(LAND_OLD_RANGE_RANGE_ROVER_MODE);
        AddCharToBuffer((uint8_t)(iLengthSendToEcuData&0xFF));
        AddToBuffer(iLengthSendToEcuData,pToEcuData);
        AddCharToBuffer((uint8_t)(((iMaximumWaitTime+5)/10)&0xFF));
    }
    while(0);

    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 东风商用车VECU中扩展CAN，连续发多帧给ECU收一帧,一次最多可以发255帧，
//          发的字节总数无限制
// Input  : iSendFrameNum         发送帧数
//          pDataToSend           发送到ECU数据
//          iRecBufferLength      接收缓冲区长度
//          ucReceiveFrameNumber  接收帧数
// Output : pRecBuffer        接收缓冲区指针
// Return : >  0   接收的总帧数.
//          <  0   通讯出错.
// Info   : 611fh
//------------------------------------------------------------------------------
int32_t Can20SendMoreFrameQuickGetInfoDFLZY( int32_t iSendFrameNum,
                                            uint8_t *pDataToSend, uint8_t *pRecBuffer,
                                            int32_t iRecBufferLength,
                                            uint8_t ucReceiveFrameNumber )
{

    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(iSendFrameNum<=0 || iSendFrameNum>255)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength<0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iSendFrameNum==0)
        {
            pRecBuffer=NULL;
        }

        AddCharToBuffer( CAC_DATA_SEND_RECEIVE_1 );//0x611f
        AddCharToBuffer( CAC_DENGFENG_CAN_SEND_N_REC_N_FRAME );//东风商用车VECU发多帧收一帧
        AddCharToBuffer( (uint8_t)(ucReceiveFrameNumber) );//接收帧数
        AddToBuffer( iSendFrameNum*19+1,pDataToSend );//扩展CAN，每帧19个字节+一个表示几帧的字节
    }
    while(0);

    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pRecBuffer,iRecBufferLength);
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: JAGUAR CAN，连续发多帧给ECU收一帧,一次最多可以发255帧，
//          发的字节总数无限制,接收帧数知道给出具体数,不知道设成FF
// Input  : iSendFrameNum         发送帧数
//          pDataToSend           发送到ECU数据
//          iRecBufferLength      接收缓冲区长度
//          ucReceiveFrameNumber  接收帧数
// Output : pRecBuffer        接收缓冲区指针
// Return : >  0   接收的总帧数.
//          <  0   通讯出错.
// Info   : 611fh
//------------------------------------------------------------------------------
int32_t  Can20SendMoreFrameQuickGetInfoJAGUAR( int32_t iSendFrameNum,
                                              uint8_t *pDataToSend, uint8_t *pRecBuffer,
                                              int32_t iRecBufferLength,
                                              uint8_t ucReceiveFrameNumber )
{

    int32_t iRet=GD_OK;

    NewBuffer();
    do
    {
        if(iSendFrameNum<=0 || iSendFrameNum>255)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength<0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iSendFrameNum==0)
            pRecBuffer=NULL;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);//0x611f
        AddCharToBuffer(CAC_DENGFENG_CAN_SEND_N_REC_N_FRAME);//东风商用车VECU发多帧收一帧
        AddCharToBuffer((uint8_t)(ucReceiveFrameNumber));//接收帧数
        AddToBuffer(iSendFrameNum*17+1,pDataToSend);//标准CAN，每帧1许个字节+一个表示几帧的字节
    }
    while(0);

    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pRecBuffer,iRecBufferLength);
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 发送一帧数据给ECU, 并接收返回的一帧数据. 收到后在较短时
//          间内迅速发送第二帧命令, 再收多帧, 接收的帧数pFrameNumber. 用于CAN 通讯方式.
// Input  : iSendFrameNum     发送帧数
//          pDataToSend       发送到ECU数据
//          iRecBufferLength  接收缓冲区长度
//          pFrameNumber      接收帧数
// Output : pRecBuffer        接收缓冲区指针
// Return : >  0   接收的总帧数.
//          <  0   通讯出错.
// Info   : 6208h
//------------------------------------------------------------------------------
int32_t  CanSendTwoFrameReciveMulFrame( int32_t iSendDataLength, uint8_t *pDataToSend,
                                       uint8_t pFrameNumber, uint8_t *pRecBuffer,
                                       int32_t iRecBufferLength )
{

    int32_t iRet=GD_OK;

    NewBuffer();
    do
    {
        if(iSendDataLength<=0 || iSendDataLength>255)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength<0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength==0)
            pRecBuffer=NULL;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        //AddCharToBuffer(CAC_SEND_TWO_REC_MULFRAME_COMMAND);// lxl 2007-4-16 10:16
        AddCharToBuffer(CAC_SUZUKI_CAN_SEND_N_REC_N_FRAME);
        //    AddCharToBuffer((uint8_t)(iSendDataLength&0xFF));
        AddCharToBuffer((uint8_t)pFrameNumber);

        //ziyingzhu 增加2009-9-10 16:17 固定发送两帧
        AddCharToBuffer(2);

        AddToBuffer((int32_t)iSendDataLength,pDataToSend);

    }
    while(0);

    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pRecBuffer,iRecBufferLength);
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: 发多帧收多帧
//  WABCO ECAS系统中有部分功能ECU多帧应答.如下:
//  Req:27  27  80
//  Ans:01  01  80  //ECU繁忙,请等待肯定应答
//  Ans:01  01  80  //此函数会把ECU繁忙应答帧过滤掉,一直到肯定应答,并只把肯定应答送到上位机
//  Ans:01  01  80
//  Ans:04  04  80  //ECU肯定应答
// Input  : pToEcuData:     发到ECU命令;
//          iLengthSendToEcuData:   发送长度;
//          iLengthReceiveBuffer:   接收缓冲区长度;
// Output : pReceiveBuffer        接收缓冲区
// Return : >  0   接收的总帧数.
//          <  0   通讯出错.
// Info   : 6208h
//------------------------------------------------------------------------------
int32_t WabcoSendDataToEcuSiftBusyFrameAnswer( uint8_t *pReceiveBuffer,
                                              int32_t iLengthReceiveBuffer,
                                              int32_t iFrameNumber, ... )
{
    va_list args;
    int32_t iTemp,i;
    uint8_t *pTemp;

    int32_t iRet=GD_OK;

    NewBuffer();
    do
    {
        if(iLengthReceiveBuffer<=0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        if(iFrameNumber>255 || iFrameNumber<=0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_WABCO_CAN_SEND_N_REC_N_FRAME);
        AddCharToBuffer((uint8_t)(iFrameNumber&0xFF));
        va_start(args, iFrameNumber);
        for (i=0;i<iFrameNumber;i++)
        {

            iTemp=va_arg(args,int32_t);
            if(iTemp>255 || iTemp<=0)
            {
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            pTemp=va_arg(args,uint8_t*);             //SendCommand
            AddToBuffer(iTemp,pTemp);

        }
        va_end(args);
        break;
    }
    while(0);

    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();    //2003.1.7
    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: Holden normal ring link 发多帧收多帧
// Input  : ucSendDataTimes - CA_ONCE_ONLY （发一次）或 CA_REPEAT_UNTIL_NEW_COMMAND_BREAK （不停发直到新命令打断）
//   *pReceiveBuffer - 收数缓存指针
//   iLengthReceiveBuffer - 收数缓存的长度
//   iFrameNumber - 发送的帧数
//   iFrameReceive -接受的帧数
//   bFlag         -检查(08 55 a3)的标志位,CA_CHECK_DELEMETER检查,CA_NOCHECK_DELEMETER不检查
//    head - 要接收帧的头
//    Time_Value - 第一帧等待的静默时间长
//    iLengthSendToEcuData - 第一帧发送命令的长度
//    pToEcuData - 第一帧发送命令的指针
//    head - 要接收帧的头
//     Time_Value - 第二帧等待的静默时间长
//     iLengthSendToEcuData - 第二帧发送命令的长度
//     pToEcuData - 第二帧发送命令的指针
// Output :
// Return : >  0   收到数的长度.
//          <  0   通讯出错.
// Info   : 6208h
//------------------------------------------------------------------------------
int32_t HoldenNormalRingLinkSendDataToEcuGetAnswer( uint8_t ucSendDataTimes,
                                                   uint8_t *pReceiveBuffer,
                                                   int32_t iLengthReceiveBuffer,
                                                   int32_t iFrameNumber,
                                                   int32_t iFrameReceive,
                                                   uint8_t bFlag, ... )
{
    va_list args;
    int32_t iTemp,i;
    uint8_t *pTemp;
    uint8_t bTemp,bTemp1;

    int32_t iRet=GD_OK;

    NewBuffer();
    do
    {
        if(ucSendDataTimes!=CA_ONCE_ONLY
            && ucSendDataTimes!=CA_REPEAT_UNTIL_NEW_COMMAND_BREAK)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iFrameNumber>255 || iFrameNumber<=0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_HOLDEN_NORMAL_RING_LINK);
        AddCharToBuffer((uint8_t)(ucSendDataTimes&0xFF));
        AddCharToBuffer((uint8_t)(iFrameNumber&0xFF));
        AddCharToBuffer((uint8_t)(iFrameReceive&0xFF));//v3200 zjz
        AddCharToBuffer((uint8_t)(bFlag&0xFF));//v3200 zjz

        va_start(args, bFlag);
        for (i=0;i<iFrameNumber;i++)
        {            //Command
            bTemp1 = va_arg(args,uint8_t);  //head id
            if(bTemp1<=0)
            {
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            bTemp = va_arg(args,uint8_t);  //idle time
            iTemp = va_arg(args,int32_t); //length
            if(iTemp > 255 || iTemp <= 0)
            {
                iRet = EC_PARAMETER_ERROR;
                break;
            }
            pTemp = va_arg(args,uint8_t*);

            AddCharToBuffer((uint8_t)(bTemp1&0xFF));
            AddCharToBuffer((uint8_t)(bTemp&0xFF));
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            AddToBuffer(iTemp,pTemp);
        }
        va_end(args);
        break;
    }
    while(0);

    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }

    DelBuffer();
    return iRet;

}

//------------------------------------------------------------------------------
// Funtion: PWM 协议 发一帧数据到ECU并接收应答(直到ECU不发才结束)
// Input  : wLengthSendToEcuData发送的数据长度
//          pToEcuData            发送到ECU数据
//          iLengthReceiveBuffer接收缓冲区长度
//          iMaximumWaitTime    最大等待时间(单位:毫秒)
// Output : pReceiveBuffer        接收缓冲区指针
// Return : 成功   实际接收到的总长度.
//          失败  出错代码(小于0).
// Info   : 6208h
//------------------------------------------------------------------------------
int32_t PWM_SendDataToEcuGetMultiFrameAnswer( int32_t iLengthSendToEcuData,
                                             uint8_t *pToEcuData,
                                             uint8_t *pReceiveBuffer,
                                             int32_t iLengthReceiveBuffer,
                                             int32_t iMaximumWaitTime )
{
    int32_t iRet=GD_OK;

    NewBuffer();
    do
    {
        if(iLengthSendToEcuData<=0 || iLengthSendToEcuData>255)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer<0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer==0)
            pReceiveBuffer=NULL;
        if(iMaximumWaitTime>255*10 && iMaximumWaitTime<0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iMaximumWaitTime<5)
            iMaximumWaitTime=5;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_PWM__SEND_1_FRAME_RECEIVE_N_FRAME);
        AddCharToBuffer((uint8_t)(iLengthSendToEcuData&0xFF));
        AddToBuffer(iLengthSendToEcuData,pToEcuData);
        AddCharToBuffer((uint8_t)(((iMaximumWaitTime+5)/10)&0xFF));
    }
    while(0);

    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------
// Funtion: PWM协议 发一帧数据到ECU并接收指定帧数
// Input  : wLengthSendToEcuData发送的数据长度
//          pToEcuData            发送到ECU数据
//          iLengthReceiveBuffer接收缓冲区长度
//          iMaximumWaitTime    最大等待时间(单位:毫秒)
// Output : pReceiveBuffer        接收缓冲区指针
// Return : 成功   实际接收到的总长度.
//          失败  出错代码(小于0).
// Info   : 6208h
//------------------------------------------------------------------------------
int32_t PWM_SendDataToEcuGetFrames( int32_t iLengthSendToEcuData,
                                   uint8_t *pToEcuData,
                                   uint8_t *pReceiveBuffer,
                                   int32_t iLengthReceiveBuffer,
                                   uint8_t bFrameCnt )
{
    int32_t iRet=GD_OK;

    NewBuffer();
    do
    {
        if(iLengthSendToEcuData<=0 || iLengthSendToEcuData>255)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer<0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer==0)
            pReceiveBuffer=NULL;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_PWM__SEND_1_FRAME_RECEIVE_N_FRAME);
        AddCharToBuffer((uint8_t)(iLengthSendToEcuData&0xFF));
        AddToBuffer(iLengthSendToEcuData,pToEcuData);
        AddCharToBuffer(bFrameCnt);
    }
    while(0);

    if(iRet==GD_OK)
    {
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}


/****************************************************************
*                                                                *
*    功  能：FORD ABS协议 发一帧数据到ECU并接收应答                *
*            (直到ECU不发才结束)                                    *
*                                                                *
*    参  数：pToEcuData            发送到ECU数据                    *
*            pReceiveBuffer        接收缓冲区指针                    *
*            iLengthReceiveBuffer接收缓冲区长度                    *
*            iMaximumWaitTime    最大等待时间(单位:毫秒)            *
*                                                                *
*    返回值：成功    实际接收到的总长度                            *
*            失败    出错代码(小于0)                                 *
*                                                                *
****************************************************************/
int32_t FordIsoSendDataToEcuGetMultiFrameAnswer(
                                uint8_t *pToEcuData,
                                uint8_t *pReceiveBuffer,
                                int32_t iLengthReceiveBuffer,
                                int32_t iMaximumWaitTime)
{
    int32_t iRet=GD_OK;
    uint8_t CS,i;
    //int32_t iLengthSendToEcuData=CalculateKwpCommandLength(pToEcuData);
    uint8_t iLengthSendToEcuData = ((pToEcuData[0] & 0xF0)>>4) + 1;

    CS = 0;
    for (i=0;i<iLengthSendToEcuData-1;i++)
        CS += pToEcuData[i];
    pToEcuData[iLengthSendToEcuData -1] = CS;
    NewBuffer();
    do{
        if(iLengthReceiveBuffer<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer==0)
            pReceiveBuffer=NULL;
        if(iMaximumWaitTime>255*10 && iMaximumWaitTime<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_FORD_ISO__SEND_1_FRAME_RECEIVE_N_FRAME);
        AddCharToBuffer((uint8_t)(iLengthSendToEcuData&0xFF));
        AddToBuffer(iLengthSendToEcuData,pToEcuData);
        AddCharToBuffer((uint8_t)(((iMaximumWaitTime+5)/10)&0xFF));
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

/*v2200 zjz
*/
int32_t  HoldenCanSendOneFrameReceiveDatas(uint8_t ucSendDataTimes,
                                int32_t iReceiveFrameNumber,
                                int32_t iLengthSendToEcuData,
                                uint8_t *pToEcuData,
                                int32_t iLengthReceiveBuffer,
                                uint8_t *pReceiveBuffer
                                )
{
    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(ucSendDataTimes!=CA_ONCE_ONLY
            && ucSendDataTimes!=CA_REPEAT_UNTIL_NEW_COMMAND_BREAK)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthSendToEcuData<=0 || iLengthSendToEcuData>255){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer==0)
            pReceiveBuffer=NULL;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_HDN_CAN_SND_1_FRAME_RECV_N_FRAME);
        AddCharToBuffer((uint8_t)(ucSendDataTimes&0xFF));
        AddCharToBuffer((uint8_t)(iReceiveFrameNumber&0xff));
        AddToBuffer(iLengthSendToEcuData,pToEcuData);
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();


    return iRet;
}

/****************************************************************
*                                                                *
*    功  能：holden gen3 VPW协议 发一帧数据到ECU并接收应答(直到ECU不发才        *
*            结束)                                                *
*                                                                *
*    参  数：wLengthSendToEcuData发送的数据长度                    *
*            pToEcuData            发送到ECU数据                    *
*            pReceiveBuffer        接收缓冲区指针                    *
*            iLengthReceiveBuffer接收缓冲区长度                    *
*            iMaximumWaitTime    最大等待时间(单位:毫秒)            *
*                                                                *
*    返回值：成功    实际接收到的总长度                            *
*            失败    出错代码(小于0)                                 *
*                                                                *
****************************************************************/
int32_t VPW_HDN_SendDataToEcuGetMultiFrameAnswer(int32_t iLengthSendToEcuData,
                                uint8_t *pToEcuData,
                                uint8_t *pReceiveBuffer,
                                int32_t iLengthReceiveBuffer,
                                int32_t iMaximumWaitTime)
{
    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(iLengthSendToEcuData<=0 || iLengthSendToEcuData>255){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer==0)
            pReceiveBuffer=NULL;
        if(iMaximumWaitTime>255*10 && iMaximumWaitTime<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iMaximumWaitTime<5)
            iMaximumWaitTime=5;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_HDN_VPW_SEND_1_FRAME_RECEIVE_N_FRAME);
        AddCharToBuffer((uint8_t)(iLengthSendToEcuData&0xFF));
        AddToBuffer(iLengthSendToEcuData,pToEcuData);
        AddCharToBuffer((uint8_t)(((iMaximumWaitTime+5)/10)&0xFF));
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

/*v2200 zjz
holden VPW发1帧收9帧0x6C开头的数据流数据，
*/
int32_t VPW_Init_SendMultiDataToEcuGetAnswer_HDN(uint8_t ucSendDataTimes)
{
    int32_t iRet=GD_OK;
    if( ((ucSendDataTimes&0xff)==CA_ONCE_ONLY)
            || ((ucSendDataTimes&0xff)==CA_REPEAT_UNTIL_NEW_COMMAND_BREAK))
    {
        NewBuffer();
        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_HDN_VPW_SEND_1_FRAME_RECEIVE_N_FRAME_1);
        AddCharToBuffer((uint8_t)(ucSendDataTimes&0xFF));
        AddCharToBuffer(0);
    }

    return iRet;
}

////////////////////////////////////////////////////////////////////////////

int32_t CHRYSLER_OneToOne_SendDataToEcuGetAnswer(uint8_t ID1 , uint8_t ID2 , uint8_t ucSendDataTimes,
                uint8_t *pReceiveBuffer,
                int32_t iLengthReceiveBuffer,
                int32_t iFrameNumber,
                ...)
{
    va_list args;
    int32_t iTemp,i;
    uint8_t *pTemp;

    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(iLengthReceiveBuffer<=0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        if(iFrameNumber>255 || iFrameNumber<=0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if( (ucSendDataTimes!=CA_ONCE_ONLY)
                &&(ucSendDataTimes!=CA_REPEAT_UNTIL_NEW_COMMAND_BREAK))
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
    //    AddCharToBuffer(CAC_SEND_N_FRAME_RECEIVE_N_FRAME);
        //AddCharToBuffer(0X20);
        AddCharToBuffer(CAC_CCD_SEND_1_FRAME_GET_1_FRAME_ID);
        AddCharToBuffer(ucSendDataTimes);

        AddCharToBuffer(ID1);
        AddCharToBuffer(ID2);

        AddCharToBuffer((uint8_t)(iFrameNumber&0xFF));
        va_start(args, iFrameNumber);
        for (i=0;i<iFrameNumber;i++){
            iTemp=va_arg(args,int32_t);
            if(iTemp>255 || iTemp<=0){
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            pTemp=va_arg(args,uint8_t*);             //SendCommand
            AddToBuffer(iTemp,pTemp);

            iTemp=va_arg(args,int32_t);

            if(iTemp>255 || iTemp<CA_AUTO_RECOGNIZE){
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            if(iTemp==CA_AUTO_RECOGNIZE){

                iTemp=va_arg(args,int32_t);

                if(iTemp>255 || iTemp< CA_RECEIVE_END_FLAG){
                    iRet=EC_PARAMETER_ERROR;
                    break;
                }

                AddCharToBuffer((uint8_t)((iTemp+1)&0xFF));
                if(iTemp==CA_RECEIVE_END_FLAG){

                    uint8_t ucTemp=va_arg(args,uint8_t);      //end flag char

                    AddCharToBuffer(ucTemp);
                }
                else{

                    iTemp=va_arg(args,int32_t);

                    iTemp*=-1;
                    AddCharToBuffer((uint8_t)(iTemp&0xFF));
                }
            }
        }
        va_end(args);
        break;
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();    //2003.1.7
    return iRet;
}


/****************************************************************
*                                                                *
*    功  能：VW_CAN协议 发一帧数据到ECU并接收应答                *
*                                                *
*                                                                *
*    参  数：iLenthToEcuData
*            pToEcuData            发送到ECU数据                    *
*            pReceiveBuffer        接收缓冲区指针                    *
*            iLengthReceiveBuffer    接收缓冲区长度                    *
*            iMaximumWaitTime    最大等待时间(单位:毫秒)            *
*                                                                *
*    返回值：成功    实际接收到的总长度                            *
*            失败    出错代码(小于0)                                 *
*                                                                *
****************************************************************/
int32_t VWCANSendDataToEcuGetMultiFrameAnswer2(
                                int32_t iLengthSendToEcuData,
                                uint8_t *pToEcuData,
                                uint8_t *pReceiveBuffer,
                                int32_t iLengthReceiveBuffer,
                                uint8_t AddressHigh,
                                uint8_t AddressLow)
{
    int32_t iRet=GD_OK;
//    int32_t iLengthSendToEcuData=CalculateKwpCommandLength(pToEcuData);

    NewBuffer();
    do{
        if(iLengthSendToEcuData<0 || iLengthSendToEcuData>255){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer==0)
            pReceiveBuffer=NULL;
        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_VW_CAN_SEND_1_FRAME_MULTI_ANSWER2);
        AddCharToBuffer((uint8_t)(iLengthSendToEcuData&0xFF));
        AddToBuffer(iLengthSendToEcuData,pToEcuData);
        AddCharToBuffer(AddressHigh);
        AddCharToBuffer(AddressLow);
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

/****************************************************************
*                                                                *
*    功  能：VW_CAN协议 发一帧数据到ECU并接收应答                *
*                                                *
*                                                                *
*    参  数：iLenthToEcuData
*            pToEcuData            发送到ECU数据                    *
*            pReceiveBuffer        接收缓冲区指针                    *
*            iLengthReceiveBuffer    接收缓冲区长度                    *
*            iMaximumWaitTime    最大等待时间(单位:毫秒)            *
*                                                                *
*    返回值：成功    实际接收到的总长度                            *
*            失败    出错代码(小于0)                                 *
*                                                                *
****************************************************************/
int32_t VWCANSendDataToEcuGetMultiFrameAnswer1(
                                int32_t iLengthSendToEcuData,
                                uint8_t *pToEcuData,
                                uint8_t *pReceiveBuffer,
                                int32_t iLengthReceiveBuffer,
                                uint8_t AddressHigh,
                                uint8_t AddressLow)
{
    int32_t iRet=GD_OK;
//    int32_t iLengthSendToEcuData=CalculateKwpCommandLength(pToEcuData);

    NewBuffer();
    do{
        if(iLengthSendToEcuData<=0 || iLengthSendToEcuData>255){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer==0)
            pReceiveBuffer=NULL;
        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_VW_CAN_SEND_1_FRAME_MULTI_ANSWER1);
        AddCharToBuffer((uint8_t)(iLengthSendToEcuData&0xFF));
        AddToBuffer(iLengthSendToEcuData,pToEcuData);
        AddCharToBuffer(AddressHigh);
        AddCharToBuffer(AddressLow);
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

/****************************************************************
*                                                                *
*    功  能：先拉低，再拉高，再发送多帧数据到ECU并接收多帧ECU的多帧应答(每发送一帧    *
*            就有对应的一帧应答)                                    *
*                                                                *
*    参  数：
        LowVolgatetime  拉低时间
        iSendDataTimes    发送次数,可以是CA_ONCE_ONLY 或        *
*                            CA_REPEAT_UNTIL_NEW_COMMAND_BREAK    *
*            pReceiveBuffer    接收缓冲区指针,格式为:接收到的总    *
*                            帧数(1byte),第1帧数据长度(1byte),    *
*                            第1帧数据内容, ... 第n帧数据长度,    *
*                            第n帧数据内容.                        *
*                                                                *
*            iLengthReceiveBuffer接收缓冲区长度                    *
*            iFrameNumber        发送的帧数                        *
*                                                                *
*           后面参数依次为:                                     *
*                                                                *
*            发送的第1帧数据长度,类型:int32_t                        *
*            发送到ECU的第1帧数据,类型:uint8_t*                        *
*            从ECU接收的第1帧数据长度,类型:int32_t                *
*                (如果其值为CA_AUTO_RECOGNIZE, 后面的参数为表示    *
*            长度的字节在该帧中的位置(0为起始值,类型：W_INT2),   *
*           再下一个参数为取得该帧帧长度而在该表示长度的字节加  *
*           的数值, 类型：W_INT2)                               *
*               (如果帧数据长度为CA_AUTO_RECOGNIZE, 后面的参数  *
*           为CA_RECEIVE_END_FLAG,则再下一个参数表示接收帧结束  *
*           标志字节, 类型：BYTE)                               *
*                                                                *
*            发送的第2帧数据长度,类型:int32_t                        *
*            发送到ECU的第2帧数据,类型:uint8_t*                        *
*            从ECU接收的第2帧数据长度,类型:int32_t                *
*                (如果其值为CA_AUTO_RECOGNIZE, 后面的参数为表示    *
*            长度的字节在该帧中的位置(0为起始值,类型：W_INT2),   *
*           再下一个参数为取得该帧帧长度而在该表示长度的字节加  *
*           的数值, 类型：W_INT2)                               *
*               (如果帧数据长度为CA_AUTO_RECOGNIZE, 后面的参数  *
*           为CA_RECEIVE_END_FLAG,则再下一个参数表示接收帧结束  *
*           标志字节, 类型：BYTE)                               *
*                ... ...                                            *
*            发送的第iFrameNumber帧数据长度,类型:int32_t          *
*            发送到ECU的第iFrameNumber帧数据,类型:uint8_t*          *
*            从ECU接收的第iFrameNumber帧数据长度,类型:int32_t     *
*                (如果其值为CA_AUTO_RECOGNIZE, 后面的参数为表示    *
*            长度的字节在该帧中的位置(0为起始值,类型：W_INT2),   *
*           再下一个参数为取得该帧帧长度而在该表示长度的字节加  *
*           的数值, 类型：W_INT2)                               *
*               (如果帧数据长度为CA_AUTO_RECOGNIZE, 后面的参数  *
*           为CA_RECEIVE_END_FLAG,则再下一个参数表示接收帧结束  *
*           标志字节, 类型：BYTE)                               *
*                                                                *
*                                                                *
*    返回值：成功    实际接收到的总长度                            *
*            失败    出错代码(小于0)
                         *
*                                                            *
****************************************************************/
int32_t AUDI_READMIMA_OneToOneSendData( uint8_t ucSendDataTimes,
                int32_t LowVolgatetime,
                uint8_t *pReceiveBuffer,
                int32_t iLengthReceiveBuffer,
                int32_t iFrameNumber,
                ...)
{
    va_list args;
    int32_t iTemp,i;
    uint8_t *pTemp;

    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(iLengthReceiveBuffer<=0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        if(iFrameNumber>255 || iFrameNumber<=0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if( (ucSendDataTimes!=CA_ONCE_ONLY)
                &&(ucSendDataTimes!=CA_REPEAT_UNTIL_NEW_COMMAND_BREAK))
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(AUDI_READMIMA_CAC_SEND_N_FRAME_RECEIVE_N_FRAME);
        AddCharToBuffer(ucSendDataTimes);
        AddCharToBuffer(LowVolgatetime>>8&0xff);
        AddCharToBuffer(LowVolgatetime&0xFF);
        AddCharToBuffer((uint8_t)(iFrameNumber&0xFF));
        va_start(args, iFrameNumber);
        for (i=0;i<iFrameNumber;i++){

            iTemp=va_arg(args,int32_t);

            if(iTemp>255 || iTemp<=0){
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            pTemp=va_arg(args,uint8_t*);             //SendCommand
            AddToBuffer(iTemp,pTemp);

            iTemp=va_arg(args,int32_t);

            if(iTemp>255 || iTemp<CA_AUTO_RECOGNIZE){
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            if(iTemp==CA_AUTO_RECOGNIZE){

                iTemp=va_arg(args,int32_t);

                if(iTemp>255 || iTemp< CA_RECEIVE_END_FLAG){
                    iRet=EC_PARAMETER_ERROR;
                    break;
                }

                AddCharToBuffer((uint8_t)((iTemp+1)&0xFF));
                if(iTemp==CA_RECEIVE_END_FLAG){

                    uint8_t ucTemp=va_arg(args,uint8_t);      //end flag char
                    AddCharToBuffer(ucTemp);
                }
                else{

                    iTemp=va_arg(args,int32_t);
                    iTemp*=-1;
                    AddCharToBuffer((uint8_t)(iTemp&0xFF));
                }
            }
        }
        va_end(args);
        break;
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();    //2003.1.7
    return iRet;
}
/****************************************************************
*                                                                *
*    功  能：SUBARU发多帧数据到ECU并接收多帧ECU的多帧应答(每发送一帧    *
*            就有对应的一帧应答)                                    *
*                                                                *
*    参  数：iSendDataTimes    发送次数,可以是CA_ONCE_ONLY 或        *
*                            CA_REPEAT_UNTIL_NEW_COMMAND_BREAK    *
*            pReceiveBuffer    接收缓冲区指针,格式为:接收到的总    *
*                            帧数(1byte),第1帧数据长度(1byte),    *
*                            第1帧数据内容, ... 第n帧数据长度,    *
*                            第n帧数据内容.                        *
*                                                                *
*            iLengthReceiveBuffer接收缓冲区长度                    *
*            iFrameNumber        发送的帧数                        *
*                                                                *
*           后面参数依次为:                                     *
*                                                                *
*            发送的第1帧数据长度,类型:int32_t                        *
*            发送到ECU的第1帧数据,类型:uint8_t*                        *
*            从ECU接收的第1帧数据长度,类型:int32_t                *
*                (如果其值为CA_AUTO_RECOGNIZE, 后面的参数为表示    *
*            长度的字节在该帧中的位置(0为起始值,类型：W_INT2),   *
*           再下一个参数为取得该帧帧长度而在该表示长度的字节加  *
*           的数值, 类型：W_INT2)                               *
*               (如果帧数据长度为CA_AUTO_RECOGNIZE, 后面的参数  *
*           为CA_RECEIVE_END_FLAG,则再下一个参数表示接收帧结束  *
*           标志字节, 类型：BYTE)                               *
*                                                                *
*            发送的第2帧数据长度,类型:int32_t                        *
*            发送到ECU的第2帧数据,类型:uint8_t*                        *
*            从ECU接收的第2帧数据长度,类型:int32_t                *
*                (如果其值为CA_AUTO_RECOGNIZE, 后面的参数为表示    *
*            长度的字节在该帧中的位置(0为起始值,类型：W_INT2),   *
*           再下一个参数为取得该帧帧长度而在该表示长度的字节加  *
*           的数值, 类型：W_INT2)                               *
*               (如果帧数据长度为CA_AUTO_RECOGNIZE, 后面的参数  *
*           为CA_RECEIVE_END_FLAG,则再下一个参数表示接收帧结束  *
*           标志字节, 类型：BYTE)                               *
*                ... ...                                            *
*            发送的第iFrameNumber帧数据长度,类型:int32_t          *
*            发送到ECU的第iFrameNumber帧数据,类型:uint8_t*          *
*            从ECU接收的第iFrameNumber帧数据长度,类型:int32_t     *
*                (如果其值为CA_AUTO_RECOGNIZE, 后面的参数为表示    *
*            长度的字节在该帧中的位置(0为起始值,类型：W_INT2),   *
*           再下一个参数为取得该帧帧长度而在该表示长度的字节加  *
*           的数值, 类型：W_INT2)                               *
*               (如果帧数据长度为CA_AUTO_RECOGNIZE, 后面的参数  *
*           为CA_RECEIVE_END_FLAG,则再下一个参数表示接收帧结束  *
*           标志字节, 类型：BYTE)                               *
*                                                                *
*                                                                *
*    返回值：成功    实际接收到的总长度                            *
*            失败    出错代码(小于0)                                 *
*                                                                *
****************************************************************/


int32_t SubaruOld_OneToOne_SendDataToEcuGetAnswer(uint8_t ucSendDataTimes,
                uint8_t *pReceiveBuffer,
                int32_t iLengthReceiveBuffer,
                int32_t iFrameNumber,
                ...)
{
    va_list args;
    int32_t iTemp,i;
    uint8_t *pTemp;

    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(iLengthReceiveBuffer<=0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        if(iFrameNumber>255 || iFrameNumber<=0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if( (ucSendDataTimes!=CA_ONCE_ONLY)
                &&(ucSendDataTimes!=CA_REPEAT_UNTIL_NEW_COMMAND_BREAK))
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_SUBARUOLD_SEND_1_FRAME_RECEIVE_1_FRAME);
        AddCharToBuffer(ucSendDataTimes);
        AddCharToBuffer((uint8_t)(iFrameNumber&0xFF));
        va_start(args, iFrameNumber);
        for (i=0;i<iFrameNumber;i++){

            iTemp=va_arg(args,int32_t);
            if(iTemp>255 || iTemp<=0){
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            pTemp=va_arg(args,uint8_t*);             //SendCommand
            AddToBuffer(iTemp,pTemp);
            iTemp=va_arg(args,int32_t);
            if(iTemp>255 || iTemp<CA_AUTO_RECOGNIZE){
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            if(iTemp==CA_AUTO_RECOGNIZE){
                iTemp=va_arg(args,int32_t);
                if(iTemp>255 || iTemp< CA_RECEIVE_END_FLAG){
                    iRet=EC_PARAMETER_ERROR;
                    break;
                }

                AddCharToBuffer((uint8_t)((iTemp+1)&0xFF));
                if(iTemp==CA_RECEIVE_END_FLAG){
                    uint8_t ucTemp=va_arg(args,uint8_t);      //end flag char
                    AddCharToBuffer(ucTemp);
                }
                else{

                    iTemp=va_arg(args,int32_t);
                    iTemp*=-1;
                    AddCharToBuffer((uint8_t)(iTemp&0xFF));
                }
            }
        }
        va_end(args);
        break;
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();    //2003.1.7
    return iRet;
}
/****************************************************************
*                                                                *
*    功  能：发一帧数据到ECU并接收无数帧ECU的应答, 直到新命令    *
*            发出就有停止.                                       *
*                                                                *
*    参  数：
            iSendDataTimes    下位机给上位机的回复次数,可以是CA_ONCE_ONLY 或        *
*                            CA_REPEAT_UNTIL_NEW_COMMAND_BREAK    *
            int32_t iCheckTime 插入时间
            pReceiveBuffer    接收缓冲区指针,格式为:接收到的总    *
*                            帧数(1byte),第1帧数据长度(1byte),    *
*                            第1帧数据内容                       *
*            iLengthReceiveBuffer接收缓冲区长度                    *
*            iLengthSendData    发送的数据长度,                         *
*            pSendData        发送到ECU的数据                        *
*            iLengthReceiveData    从ECU接收的数据长度                *
*                (如果其值为CA_AUTO_RECOGNIZE, 后面的参数为表示    *
*            长度的字节在该帧中的位置(0为起始值, 类型：W_INT2),  *
*           再下一个参数为取得该帧帧长度而在该表示长度的字节加  *
*           的数值, 类型：W_INT2)                               *
*                (如果帧数据长度为CA_AUTO_RECOGNIZE, 后面的参数  *
*           为CA_RECEIVE_END_FLAG,则再下一个参数表示接收帧结束  *
*           标志字节, 类型：BYTE)                               *
*                                                                *
*    返回值：成功    实际接收到的总长度                            *
*            失败    出错代码(小于0)                                 *
*                                                                *
****************************************************************/
//ziyingzhu 修改2009-7-3 9:41:18，增加ucSendDataTimes 参数
int32_t SendOneFrameDataToEcuGetAnyFrameAnswer_Check(
                uint8_t ucSendDataTimes,
                int32_t iCheckTime,
                uint8_t *pReceiveBuffer,
                int32_t iLengthReceiveBuffer,
                int32_t iLengthSendData,
                uint8_t *pSendData,
                int32_t     iLengthReceiveData,
                ...)
{
    va_list args;
    int32_t iTemp;
    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(iLengthSendData>255 || iLengthSendData<=0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_SEND_ONEFRAME_ANY_FRAME_SETTIME);
        AddCharToBuffer(ucSendDataTimes);
        AddCharToBuffer((uint8_t)(iCheckTime&0xFF));
        AddCharToBuffer((uint8_t)(iLengthSendData&0xFF));
        AddToBuffer(iLengthSendData,pSendData);
        AddCharToBuffer((uint8_t)(iLengthReceiveData&0xFF));

        if(iLengthReceiveData==CA_AUTO_RECOGNIZE){
            va_start(args, iLengthReceiveData);
            iTemp=va_arg(args,int32_t);
            if(iTemp>255 || iTemp<CA_RECEIVE_END_FLAG){
                iRet=EC_PARAMETER_ERROR;
                va_end(args);
                break;
            }
            AddCharToBuffer((uint8_t)((iTemp+1)&0xFF));
            if(iTemp==CA_RECEIVE_END_FLAG){
                uint8_t ucTemp=va_arg(args,uint8_t);      //end flag char
                AddCharToBuffer(ucTemp);
            }
            else{
                iTemp=va_arg(args,int32_t);
                iTemp*=-1;
                AddCharToBuffer((uint8_t)(iTemp&0xFF));
            }
            va_end(args);
        }
        break;
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();    //2003.1.7
    return iRet;
}
/***********************add to can+ bus***********************/
/*********************************************
 * 函数名:    CheckXOR()
 * 作用:    对某个长度的数据进行异或运算.
 * 返回值:    异或运算结果.
 *********************************************/
 unsigned char CheckXOR(uint8_t * pDataBuffer, int32_t iDataLen)
{
    int32_t    i;
    unsigned char cRetVal=pDataBuffer[0];
    for(i=1;i<iDataLen;i++)
        cRetVal^=pDataBuffer[i];
    return cRetVal;
}

/**************************************************************
 * 函数名:    CANReceiveDataFromEcu()
 * 作用:    等待接收一ECU数据信息帧.
 * 返回值:    -1---通讯失败.
 *                其他值---返回一帧数据的长度.
 * 注意:    接收数据帧时将对接收到的数据进行异或校验.
 *        如果检验正确,发送确认帧并返回该帧长度,
 *        否则重新接收.
 **************************************************************/
int32_t    CANReceiveOneFrameFromEcu(uint8_t * pAnsBuffer, int32_t iBufferLen)
{

    int32_t    siSendRel=0,siRecRel=0;
    uint8_t    checkRel;

    do{
        memset(pAnsBuffer,0,sizeof(pAnsBuffer));

        siRecRel=ReceiveDataFromEcu(CAN_MODE_COMM,pAnsBuffer,iBufferLen);
        if( siRecRel<=0 )
        {
            return -1;
        }
        else
        {     /* 收到数据, 进行校验. */
            checkRel=CheckXOR(pAnsBuffer+4,12);
            {
               //printf("\n");
            }
            if( checkRel==pAnsBuffer[16] )
            {
                siSendRel=SendOneFrameNoAnswer(3,"\xaa\x55\x75");
                if(siSendRel==GD_OK) /* OK. */
                {
                       //printf("\n校验成功后, 接收确认发送成功!\n");
                        break;
                }
                else
                {
                   //printf("\n校验成功后, 接收确认发送失败!\n");
                    return -1;
                }
            }
            else /* 校验失败, 重新接受.  */
            {
                siSendRel=SendOneFrameNoAnswer(3,"\xaa\x55\x78");
                if(siSendRel==GD_OK)
                    continue;   // Continue waiting for receiving.
                else
                    return -1;
            }
        }
    }while(0);

    return siRecRel;
}

/******************************************************************************
 * [ 函数名 ]       CanSendOneFrameReceiveDatas()
 * [ 功能 ]         通过smartbox, 发送一帧数据, 并根据相应的参数决定如何等待返回的
 *                  数据帧.收到的数据帧格式为:
 *                  [1字节的总帧数][15字节的第一帧收据][15字节的第2帧收据]......
 * [ 参数描述 ]
 * < iMode >-------------发送模式, 具体含义为:
 *        0----------------只发不收.
 *        1----------------发送1帧, 接收1帧.
 *        2----------------发送1帧, 接收2帧.
 *         others----------------暂无定义.
 * < iReqLen >-----------发送帧的长度.
 * < pReqBuffer >--------发送数据的存储区.
 * < pRecBuffer >--------接收收据的缓冲区.
 * < iRecLen >-----------接收缓冲区的长度.
 * [ 返回值 ]          返回收到的数据帧个数.
 *            <0-------出错.
 * [ 说明 ]            本函数为CAN BUS协议所专用,接收2帧模拟可行,在实测中需要调试, ?
 *                    不建议使用. 接收1帧没有任何问题.
 ******************************************************************************/
int  CanSendOneFrameReceiveDatas(int iMode,int iReqLen, uint8_t *pReqBuffer, uint8_t *pRecBuffer, int iRecLen)
{
    int32_t    iRes;
    int     iRecCount=0;
    uint8_t    checkSum;
    uint8_t    TmpBuffer[128];
    pReqBuffer[iReqLen]=CheckXOR(pReqBuffer+1,iReqLen-1);/* add checksum. */

    memset(pRecBuffer,0,sizeof(pRecBuffer));

    do{
        int time_count=0;
        /* send 0x55, get 0x01. */
        do{
            time_count++;
            memset(TmpBuffer,0,sizeof(TmpBuffer));
            if( (iRes=KnowReceiveFrameNumber_SendDataToEcuGetAnswer(1,"\x55",TmpBuffer,sizeof(TmpBuffer),1,1)) < 0 )
            {
                if(time_count <= 6)
                    continue;
                /* 超过握手尝试限数, 通讯出错. */
                else
                    return -1;
            }
            /* 收到应答, 退出握手. */
            if(TmpBuffer[2]==0x01)
                break;
        }while(1);
        /* send one can frame, and get ECU answer(one frame or more). */
        memset( TmpBuffer,0,sizeof(TmpBuffer) );

        if(iMode==0) /* Only send one CAN frame, not receiving any ECU frame. */
        {
            if( (iRes=KnowReceiveFrameNumber_SendDataToEcuGetAnswer(iReqLen+1,pReqBuffer,TmpBuffer,sizeof(TmpBuffer),1,3)) <0 )
                return -1;
            if( iRes>0 && TmpBuffer[2]==0xaa && TmpBuffer[3]==0x55 && TmpBuffer[4]==0x78 )
            /* if CANBOX received ok. */
                return 0;
            else /* CANBOX received error, resend the data. */
                continue;
        }

        if( (iRes=KnowReceiveFrameNumber_SendDataToEcuGetAnswer(iReqLen+1,pReqBuffer,TmpBuffer,sizeof(TmpBuffer),2,3,CAN_LEN)) <0 )
            return  -1;

        if(iRes>0 && TmpBuffer[2]==0xaa && TmpBuffer[3]==0x55 && TmpBuffer[4]==0x78)
            break;  /* succeed. */
        else
            continue;  /* CANBOX receive data error, should resend the data. */
    }while(1);
    /* now we received one frame, check the checksum and decide what to do. */
    checkSum=CheckXOR(TmpBuffer+8,12);
    if(checkSum==TmpBuffer[20])
    /* first Checksum is ok. send the acknowledge frame. */
    {
        memmove(pRecBuffer+1+CAN_LEN*iRecCount,TmpBuffer+6,CAN_LEN);
        iRecCount++;
        if(iMode==1)  /* Only send one CAN frame, receiving one ECU back frame. */
        {    /* Send acknowledge frame to CAN box. */
            if( (iRes=SendOneFrameNoAnswer(3, "\xaa\x55\x75")) <0 )
                return -1;
            return ( pRecBuffer[0]=iRecCount );
        }
        /* mode2: a.发送第一帧数据帧的确认 b.接收第二数据帧并发送第二帧的确认. c.存储收到的第二帧数据.*/
        if(iMode==2)
        {
                // 测试:接收的第一包数据信息.
                {
               //printf("\n模式2:收到的第一个数据包信息:\n");
               //printf("\n");
                }
            iRes=SendOneFrameNoAnswer(3, "\xaa\x55\x75");
            if(iRes!=GD_OK)
                return -1;
            //测试:发送确认是否成功.
            else
            {
               //printf("\n模式2:第一数据包接收成功, 并且确认发送成功!\n");
            }
            iRes=CANReceiveOneFrameFromEcu(TmpBuffer, sizeof(TmpBuffer));
            if(iRes<0)
                return -1;
            memmove(pRecBuffer+1+CAN_LEN*iRecCount,TmpBuffer+2,CAN_LEN);
            return (pRecBuffer[0]=++iRecCount);
        }
        else  /* Other mode is not defined. */
            return -1;

    }
    else  /* first checksum error. tell CANBOX to resend the data. */
    {
        do{
            memset(TmpBuffer,0,sizeof(TmpBuffer));
            if( (iRes=KnowReceiveFrameNumber_SendDataToEcuGetAnswer(3,"\xaa\x55\x78",TmpBuffer,sizeof(TmpBuffer),1,CAN_LEN)) <0)
                return -1;
            if( (checkSum=CheckXOR(TmpBuffer+4,12))==TmpBuffer[16] )
            /* if receive ok */
            {
                memmove(pRecBuffer+1+CAN_LEN*iRecCount,TmpBuffer+2,CAN_LEN);
                iRecCount++;
                if(iMode==1) /* Only receive 1 ECU frame. */
                {
                    iRes=SendOneFrameNoAnswer(3,"\xaa\x55\x75");
                    if(iRes<0)
                        return -1;
                    return ( pRecBuffer[0]=iRecCount );
                }
                /* mode2: a.发送第一帧数据帧的确认 b.接收第二数据帧并发送第二帧的确认. c.存储收到的第二帧数据.*/
                if(iMode==2)
                {
                    iRes=SendOneFrameNoAnswer(3, "\xaa\x55\x75");
                    if(iRes!=GD_OK)
                        return -1;
                    iRes=CANReceiveOneFrameFromEcu(TmpBuffer, sizeof(TmpBuffer));
                    if(iRes<0)
                        return -1;
                    memmove(pRecBuffer+1+CAN_LEN*iRecCount,TmpBuffer+2,CAN_LEN);

                    return ( pRecBuffer[0]=++iRecCount );
                }
                else  /* Other mode not defined. */
                    return -1;
            }
            else /* If receive error again. */
            {
                continue;
            }
        }while(1);
    }
}



/************************************************************************************
 *  Function Name:    SendOneFrameNoAnswer()
 *
 *    Description:    Send one frame to ECU immediately without
 *                    receiving any data from ECU. Here it is used
 *                    for sending acknowledge frame to CAN box. There
 *                    is no delay time after sending a frame. It can
 *                    also be used in other cases if it is suitable.
 *    Parameters:
 *        int32_t    siDataLength-----The length of the frame to send.
 *        uint8_t    pDataToSend------The buffer points to the datas to send.
 *
 *    Return Value:
 *            GD_OK-----------------------Success
 *            ErrorCode(<0)---------------Fail                    CSL.20040205
 ***********************************************************************************/
int32_t    SendOneFrameNoAnswer(int32_t siDataLength, uint8_t * pDataToSend)
{
    int32_t    siRet=GD_OK;

    NewBuffer();

    do{
        if( siDataLength<=0 || siDataLength >255 || pDataToSend==NULL )
        {
            siRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_SEND_1_FRAME_NO_ANSWER);
        AddCharToBuffer((uint8_t)0x01);  /* Send times. */
        AddCharToBuffer((uint8_t)0x01);  /* Frame number. */

        AddCharToBuffer( (uint8_t)(siDataLength & 0xff) );
        AddToBuffer(siDataLength,pDataToSend);

        AddCharToBuffer( (uint8_t)0x00 );
        break;
    }while(0);
    if( siRet==GD_OK)
        siRet=DisposeSendAndReceive(NULL,0);

    DelBuffer();
    return siRet;
}
//============================================================
//commabstractlayer.c  added
/****************************************************************
*
*   功  能：设置 CAN BUS 链路保持命令                                    *
*
*    参  数：pLinkCommand    链路保持命令缓冲区,如为NULL则停止
*                               链路保持
*               ucCommandLength 链路保持命令长度, 如为0则停止链路
*                              保持
*               ucLinkCommandReturnLength    链路保持ECU返回字节数
*               wLinkKeepTime   链路保持间隔时间,单位毫秒; 取值范
*                           围: 大于0, 小于等于2550毫秒
*
*   返回值：成功    GD_OK
*           失败    出错代码
*   说明:该函数与SetLinkKeep()除了命令字不同, 其余一样.
*
****************************************************************/
int32_t SetCanBusLinkKeep(uint8_t *pLinkCommand,
                int32_t iCommandLength,
                int32_t iLinkCommandReturnLength,
                int32_t iLinkKeepTime)
{
    int32_t iRet=GD_OK;

    if((iCommandLength>255-3)||(iCommandLength<0))
        return EC_PARAMETER_ERROR;
    if((iLinkCommandReturnLength>255)|| (iLinkCommandReturnLength<0))
        return EC_PARAMETER_ERROR;
    if((iLinkKeepTime>2550) || (iLinkKeepTime<0))
        return EC_PARAMETER_ERROR;

    if( (pLinkCommand==NULL) || (iCommandLength==0) ||
        (iLinkCommandReturnLength==0) || (iLinkKeepTime==0) )
    {
        pLinkCommand=NULL;
        iCommandLength=0;
        iLinkCommandReturnLength=0;
        iLinkKeepTime=0;
    }

    NewBuffer();
    AddCharToBuffer(CAC_CONTRAL_COMMAND);
    AddCharToBuffer(CAC_CANBUS_LINK_KEEP);  //600CH  FOR CAN BUS
    AddCharToBuffer((uint8_t)(iCommandLength&0xFF));
    if(iCommandLength!=0)
        AddToBuffer(iCommandLength,pLinkCommand);
    AddCharToBuffer((uint8_t)(iLinkCommandReturnLength&0xFF));
    AddCharToBuffer((uint8_t)(((iLinkKeepTime+5)/10)&0xFF));

    iRet=DisposeSendAndReceive(NULL,0);
    DelBuffer();

    return iRet;
}

/****************************************************************
*
*   功  能：设置 CAN BUS 链路保持命令                                    *
*
*    参  数：pLinkCommand    链路保持命令缓冲区,如为NULL则停止
*                               链路保持
*               ucCommandLength 链路保持命令长度, 如为0则停止链路
*                              保持
*               ucLinkCommandReturnLength    链路保持ECU返回字节数
*               wLinkKeepTime   链路保持间隔时间,单位毫秒; 取值范
*                           围: 大于0, 小于等于2550毫秒
*
*   返回值：成功    GD_OK
*           失败    出错代码
*   说明:该函数与SetLinkKeep()除了命令字不同, 其余一样.
*
****************************************************************/
int32_t SetCan2BusLinkKeep( uint8_t *pLinkCommand, int32_t iCommandLength,
                           int32_t iLinkCommandReturnLength, int32_t iLinkKeepTime )
{
    int32_t iRet=GD_OK;

    if((iCommandLength>255-3)||(iCommandLength<0))
        return EC_PARAMETER_ERROR;
    if((iLinkCommandReturnLength>255)|| (iLinkCommandReturnLength<0))
        return EC_PARAMETER_ERROR;
    if((iLinkKeepTime>2550) || (iLinkKeepTime<0))
        return EC_PARAMETER_ERROR;

    if( (pLinkCommand==NULL) || (iCommandLength==0) ||
        (iLinkCommandReturnLength==0) || (iLinkKeepTime==0) )
    {
        pLinkCommand=NULL;
        iCommandLength=0;
        iLinkCommandReturnLength=0;
        iLinkKeepTime=0;
    }

    NewBuffer();
    AddCharToBuffer(CAC_CONTRAL_COMMAND);
    AddCharToBuffer(CAC_CAN2BUS_LINK_KEEP);  //600CH  FOR CAN BUS
    AddCharToBuffer((uint8_t)(iCommandLength&0xFF));
    if(iCommandLength!=0)
        AddToBuffer(iCommandLength,pLinkCommand);
    AddCharToBuffer((uint8_t)(iLinkCommandReturnLength&0xFF));
    AddCharToBuffer((uint8_t)(((iLinkKeepTime+5)/10)&0xFF));

    iRet=DisposeSendAndReceive(NULL,0);
    DelBuffer();

    return iRet;
}

/****************************************************************
*
*   功  能：设置 CAN BUS 链路保持命令                                    *
*
*    参  数：pLinkCommand    链路保持命令缓冲区,如为NULL则停止
*                               链路保持
*               ucCommandLength 链路保持命令长度, 如为0则停止链路
*                              保持
*               ucLinkCommandReturnLength    链路保持ECU返回字节数
*               wLinkKeepTime   链路保持间隔时间,单位毫秒; 取值范
*                           围: 大于0, 小于等于2550毫秒
*
*   返回值：成功    GD_OK
*           失败    出错代码
*   说明:该函数与SetCanBusLinkKeep()除了命令字不同, 其余一样.
*
****************************************************************/
int32_t SetCanBus2LinkKeep(uint8_t *pLinkCommand,
                int32_t iCommandLength,
                int32_t iLinkCommandReturnLength,
                int32_t iLinkKeepTime)
{
    int32_t iRet=GD_OK;

    if((iCommandLength>255-3)||(iCommandLength<0))
        return EC_PARAMETER_ERROR;
    if((iLinkCommandReturnLength>255)|| (iLinkCommandReturnLength<0))
        return EC_PARAMETER_ERROR;
    if((iLinkKeepTime>2550) || (iLinkKeepTime<0))
        return EC_PARAMETER_ERROR;

    if( (pLinkCommand==NULL) || (iCommandLength==0) ||
        (iLinkCommandReturnLength==0) || (iLinkKeepTime==0) )
    {
        pLinkCommand=NULL;
        iCommandLength=0;
        iLinkCommandReturnLength=0;
        iLinkKeepTime=0;
    }

    NewBuffer();
    AddCharToBuffer(CAC_CONTRAL_COMMAND);
    AddCharToBuffer(CAC_CANBUS2_LINK_KEEP);  //600eH  FOR CAN BUS2
    AddCharToBuffer((uint8_t)(iCommandLength&0xFF));
    if(iCommandLength!=0)
        AddToBuffer(iCommandLength,pLinkCommand);
    AddCharToBuffer((uint8_t)(iLinkCommandReturnLength&0xFF));
    AddCharToBuffer((uint8_t)(((iLinkKeepTime+5)/10)&0xFF));

    iRet=DisposeSendAndReceive(NULL,0);
    DelBuffer();

    return iRet;
}

//=====================================================================================


/*****************************************************************/
/****************************************************************
*                                                                *
*    功  能：发多帧数据到MABCO_ECU并接收一帧ECU的多帧应答(每发送一帧    *
*            就有对应的一帧应答)                                    *
*                                                                *
*    参  数：
*            pReceiveBuffer    接收缓冲区指针,格式为:接收到的总    *
*                            帧数(1byte),第1帧数据长度(1byte),    *
*                            第1帧数据内容, ... 第n帧数据长度,    *
*                            第n帧数据内容.                        *
*                                                                *
*            iLengthReceiveBuffer接收缓冲区长度                    *
*            iFrameNumber        发送的帧数                        *
*                                                                *
*           后面参数依次为:                                     *
*                                                                *
*            发送的第1帧数据长度,类型:int32_t
            从ECU接收的第1帧数据长度,类型:int32_t,该长度是程序员指定                        *
*            发送到ECU的第1帧数据,类型:uint8_t*                        *
*                                                                *
*            发送的第2帧数据长度,类型:int32_t
            从ECU接收的第1帧数据长度,类型:int32_t,该长度是程序员指定                        *
*            发送到ECU的第2帧数据,类型:uint8_t*                        *
*                                   *
*                ... ...                                            *
*            发送的第iFrameNumber帧数据长度,类型:int32_t
            从ECU接收的第iFrameNumber帧数据长度,类型:int32_t     *         *
*            发送到ECU的第iFrameNumber帧数据,类型:uint8_t*          *
***                                                    *
*    返回值：成功    实际接收到的总长度                            *
*            失败    出错代码(小于0)                                 *
*
*        ;例如发送命令
*         req: 00 80
*         Ans: 85 08 08 01 00 00 80
*         Mabco_SendOneData(pReceiveBuffer,16,1,7,"/0x85/0x08/0x08/0x01/0x00/0x00/0x80");
*        z                    *
****************************************************************/
int32_t Mabco_SendOneData(
                uint8_t *pReceiveBuffer,
                int32_t iLengthReceiveBuffer,
                int32_t iFrameNumber,
                ...)
{
    va_list args;
    int32_t iTemp,i;
    int32_t iTemp1;
    uint8_t *pTemp;

    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(iLengthReceiveBuffer<=0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        if(iFrameNumber>255 || iFrameNumber<=0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_MABCO_ABS);
        AddCharToBuffer((uint8_t)(iFrameNumber&0xFF));
        va_start(args, iFrameNumber);
        for (i=0;i<iFrameNumber;i++){
            iTemp=va_arg(args,int32_t);
            if(iTemp>255 || iTemp<=0){
                iRet=EC_PARAMETER_ERROR;
                break;
            }

            AddCharToBuffer((uint8_t)(iTemp&0xFF));    ;
                                //;receive lenght
            iTemp1=va_arg(args,int32_t);
            if(iTemp1>255 || iTemp1<CA_AUTO_RECOGNIZE){
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp1&0xFF));

            pTemp=va_arg(args,uint8_t*);             //SendCommand
            AddToBuffer(iTemp,pTemp);
            iTemp=va_arg(args,int32_t);
            if(iTemp1==CA_AUTO_RECOGNIZE){
                iTemp=va_arg(args,int32_t);
                if(iTemp>255 || iTemp< CA_RECEIVE_END_FLAG){
                    iRet=EC_PARAMETER_ERROR;
                    break;
                }

                AddCharToBuffer((uint8_t)((iTemp+1)&0xFF));
                if(iTemp==CA_RECEIVE_END_FLAG){
                    uint8_t ucTemp=va_arg(args,uint8_t);      //end flag char
                    AddCharToBuffer(ucTemp);
                }
                else{
                    iTemp=va_arg(args,int32_t);
                    iTemp*=-1;
                    AddCharToBuffer((uint8_t)(iTemp&0xFF));
                }
            }
        }
        va_end(args);
        break;
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();    //2003.1.7
    return iRet;
}

/****************************************************************
*                                                                *
*    功  能：发一帧数据到ECU并接收ECU应答(已知接收的帧数)        *
*                                                                *
*    参  数：    iLengthSendToEcuData发送的数据长度                    *
*            pToEcuData            发送到ECU数据                    *
*            pReceiveBuffer        接收缓冲区指针
*            iLengthReceiveBuffer接收缓冲区长度                  *
*
*                                                                *
*    返回值：成功    实际接收到的总长度                            *
*            失败    出错代码(小于0)                                 *
*                                                                *
****************************************************************/
int32_t Mabco_Abs_Clean(
                int32_t iLengthSendToEcuData,
                uint8_t *pToEcuData,
                uint8_t *pReceiveBuffer,
                int32_t iLengthReceiveBuffer
            )
{
    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(iLengthSendToEcuData<=0 || iLengthSendToEcuData>255){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer==0)
            pReceiveBuffer=NULL;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_MABCO_ABS_CLEAN);
        AddCharToBuffer((uint8_t)(iLengthSendToEcuData&0xFF));
        AddToBuffer(iLengthSendToEcuData,pToEcuData);
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}


/****************************************************************
*                                                               *
*   功  能：设置链路保持命令                                    *
*                                                               *
*    参  数：pLinkCommand    链路保持命令缓冲区,如为NULL则停止   *
*                           链路保持                            *
*           ucCommandLength  链路保持命令长度, 如为0则停止链路   *
*                           保持                                *
*           ucLinkCommandReturnLength    链路保持ECU返回字节数   *
*           wLinkKeepTime   链路保持间隔时间,单位毫秒; 取值范   *
*                           围: 大于0, 小于等于2550毫秒         *
*                                                               *
*   返回值：成功    GD_OK                                       *
*           失败    出错代码                                    *
*                                                               *
****************************************************************/
int32_t Maboc_SetLinkKeep(uint8_t *pLinkCommand,
                int32_t iCommandLength,
                int32_t iLinkCommandReturnLength,
                int32_t iLinkKeepTime)
{
    int32_t iRet=GD_OK;

    if((iCommandLength>255-3)||(iCommandLength<0))
        return EC_PARAMETER_ERROR;
    if((iLinkCommandReturnLength>255)|| (iLinkCommandReturnLength<0))
        return EC_PARAMETER_ERROR;
    if((iLinkKeepTime>2550) || (iLinkKeepTime<0))
        return EC_PARAMETER_ERROR;

    if( (pLinkCommand==NULL) || (iCommandLength==0) ||
        (iLinkCommandReturnLength==0) || (iLinkKeepTime==0) )
    {
        pLinkCommand=NULL;
        iCommandLength=0;
        iLinkCommandReturnLength=0;
        iLinkKeepTime=0;
    }

    NewBuffer();
    AddCharToBuffer(CAC_CONTRAL_COMMAND);
    AddCharToBuffer(CAC_MABCO_ECU_LINK_KEEP);
    AddCharToBuffer((uint8_t)(iCommandLength&0xFF));
    if(iCommandLength!=0)
        AddToBuffer(iCommandLength,pLinkCommand);
    AddCharToBuffer((uint8_t)(iLinkCommandReturnLength&0xFF));
    AddCharToBuffer((uint8_t)(((iLinkKeepTime+5)/10)&0xFF));

    iRet=DisposeSendAndReceive(NULL,0);
    DelBuffer();

    return iRet;
}

/****************************************************************
*                                                                *
*    功  能：地址码通信方式                                           *
*                                                                *
*    参  数：ucAddressCode        地址码                            *
*            pReceiveBuffer        接收缓冲区                        *
*            iLengthReceiveBuffer接收缓冲区长度                    *
*            fBps                与ECU通信波特率                    *
*            bRecognizeBaudRate    需要识别并自动设置波特率        *
*            bSecondByteReverseReturnToECU    第二个字节是否要求    *
*                                            取反发回            *
*            iReceiveFrameNumber    要接收的帧数,取值范围:0~255     *
*                                (不包括取反发回的字节)            *
*                                                               *
*           后面参数依次为:                                     *
*                                                               *
*           接收的第1帧数据长度,类型:int32_t                     *
*           接收的第2帧数据长度,类型:int32_t                     *
*                   ...   ...                                   *
*           接收的第iReceiveFrameNumber帧数据长度             *
*                                                                *
*    返回值：成功    返回从ECU接收的版本信息长度,如无版本信息,    *
*                    返回GD_OK                                    *
*            失败    出错代码                                        *
*                                                                *
****************************************************************/
int32_t AddressCodeWay_Opel(
                uint8_t ucAddressCode,
                uint8_t *pReceiveBuffer,
                int32_t iLengthReceiveBuffer,
                float fBps,
                int8_t bRecognizeBaudRate,
                int8_t bSecondByteReverseReturnToECU,
                int32_t iReceiveFrameNumber,...
                )
{
    int32_t iRet=GD_OK;

    if(iReceiveFrameNumber<0 || iReceiveFrameNumber>255)
        return EC_PARAMETER_ERROR;

    NewBuffer();
    AddCharToBuffer(CAC_CONTRAL_COMMAND);
    AddCharToBuffer(CAC_ECU_ADDRESS_CODE_WAY);
    if(fabs(fBps-5)<0.0001)
    {
        AddCharToBuffer(1); //way 1
        AddCharToBuffer(ucAddressCode);
        AddCharToBuffer((uint8_t)(bRecognizeBaudRate?0xFF:0x00));
    }
    else
    {
        AddCharToBuffer(2); //way 2
        AddCharToBuffer(ucAddressCode);

           if(fBps>1000.0)
        {
                 AddCharToBuffer((uint8_t)((((int32_t)(65536-18432000/32/fBps))>>8)&0xFF));
                 AddCharToBuffer((uint8_t)(((int32_t)(65536-18432000/32/fBps))&0xFF));
           }
             else
        {  //2003.3.26 add
                 int32_t iTemp=(int32_t)(fBps+0.5);
                 AddCharToBuffer((uint8_t)((iTemp>>8)&0xFF));
                 AddCharToBuffer((uint8_t)(iTemp&0xFF));
          }

    }

    AddCharToBuffer((uint8_t)(bSecondByteReverseReturnToECU?0xFF:0x00));
    AddCharToBuffer((uint8_t)(iReceiveFrameNumber&0xFF));

    if(iReceiveFrameNumber<=0)
    {
        AddCharToBuffer((uint8_t)(0));
    }
    else
    {
        int32_t i,iTemp;
        va_list args;
        va_start(args, iReceiveFrameNumber);
        for(i=0;i<iReceiveFrameNumber;i++){

            iTemp=va_arg(args,int32_t);
            if((iTemp>255) || (iTemp<=0) ){
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
        }
        va_end(args);
    }

    if(iRet==GD_OK)
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    DelBuffer();

    //ziyingzhu 修改.与原来的协议保持兼容2009-6-5 12:01:40
    if(iRet >= 0)
    {
        if(iReceiveFrameNumber==0)
            return GD_OK;
        else
            return iRet;
    }

    return iRet;
}
/****************************************************
    接收的缓冲区        uint8_t *pReceiveBuffer,
    接收的缓冲区长度    int32_t iLengthReceiveBuffer,
    长度的字节在该帧中的位置(0为起始值,类型：W_INT2)int32_t iPosition,
    取得该帧帧长度而在该表示长度的字节加的数值, 类型：W_INT2)                                       int32_t iAddorDecNumber

*****************************************************/
int32_t OneToOne_SendDataToEcuGetAnswer_Nissan(
                uint8_t *pReceiveBuffer,
                int32_t iLengthReceiveBuffer,
                int32_t iPosition,
                int32_t iAddorDecNumber
                )
{
    int32_t iRet=GD_OK;
    NewBuffer();
        if(iLengthReceiveBuffer<=0){
            iRet=EC_PARAMETER_ERROR;
        }
        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_ONLYRECDATA_NISSAN);
        AddCharToBuffer((uint8_t)((iPosition+1)&0xFF));
        iAddorDecNumber*=-1;
        AddCharToBuffer((uint8_t)(iAddorDecNumber&0xFF));
        if(iRet==GD_OK)
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    DelBuffer();
    return iRet;
}

/****************************************************************
*                                                                *
*    功  能：控制IO线输出电平及电平保持时间,在下位机中加了一个5MS的高电平判断时间
*        用法和 SetCommunicationLineVoltage函数相同                        *
*                                                                *
*    参  数：ucChangeTimes    为电平改变次数,后面依次为低电平时间 *
*                            高电平时间, 低电平时间, 高电平时间    *
*                            ... ... , 如果电平先拉高,再拉低则第    *
*                            一个低电平时间为0.                    *
*           电平保持时间单位:毫秒, 数据类型:int32_t              *
*                                                                *
*    返回值：成功    GD_OK                                        *
*            失败    出错代码                                        *
*                                                               *
*   说  明: 本函数执行完毕后, 通信线将保持最后设定的电平值      *
*                                                                *
****************************************************************/
int32_t SetCommunicationLineVoltage_Nissan(uint8_t ucChangeTimes, ...)
{
    va_list args;
    int32_t iTemp;
    int32_t i;
    int32_t iRet=GD_OK;

    if(ucChangeTimes==0)
        return EC_PARAMETER_ERROR;

    NewBuffer();
    va_start(args, ucChangeTimes);
    for (i=0;i<(int32_t)ucChangeTimes;i++){
        iTemp=va_arg(args,int32_t);
        if((iTemp<0)||(iTemp>65535)){
            iRet=EC_OVER_MEMORY_SIZE;
            break;
        }
        if(iTemp>0){
            AddCharToBuffer(CAC_CONTRAL_COMMAND);
            AddCharToBuffer(CAC_SET_IO_LINE_VOLTAGE_NISSAN);

            if(s_SmartboxStruct.ucLogic==CA_POSITIVE)
                AddCharToBuffer((uint8_t)(i%2?CA_HIGH:CA_LOW));
            else
                AddCharToBuffer((uint8_t)(i%2?CA_LOW:CA_HIGH));

            AddCharToBuffer((uint8_t)((iTemp>>8)&0xFF));
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
        }
    }
    va_end(args);

    if((iRet==GD_OK) && (GetBufferContainLength()>0))
        iRet=DisposeSendAndReceive(NULL,0);
    DelBuffer();

    return iRet;
}

/***********************************************************************
 *    函数名:        CanSendTwoFrameQuickGetInfo()
 *    功能:        发送一帧数据给ECU, 并接收返回的一帧数据. 收到后在较短时
 *                间内迅速发送第二帧命令, 这样才能接收到ECU的某些状态信息,
 *                接收的帧数不定. 用于CAN 通讯方式.
 *    参数:        iSendDataLength      ---     所要发送数据的长度.
 *                pDataToSend            ---        要发送的数据.
 *                pRecBuffer            ---        接收数据缓冲.
 *                iRecBufferLength    ---        接收缓冲区长度.
 *
 *
 *    返回值:        >  0         ---        接收的总帧数.
 *                <  0        ---        通讯出错.
 ***********************************************************************/
/* Command Word: 6114h*/
int32_t    CanSendTwoFrameQuickGetInfo(int32_t iSendDataLength, uint8_t *pDataToSend, uint8_t *pRecBuffer,int32_t iRecBufferLength)
{

    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(iSendDataLength<=0 || iSendDataLength>255){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength==0)
            pRecBuffer=NULL;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_CAN_SEND_1_FRAME_QUICK_GET_INFO);
    //    AddCharToBuffer((uint8_t)(iSendDataLength&0xFF));
        AddToBuffer(iSendDataLength,pDataToSend);
    //    AddCharToBuffer((uint8_t)(((iMaxWaitTime+5)/10)&0xFF));
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pRecBuffer,iRecBufferLength);
    }
    DelBuffer();

    return iRet;
}


int32_t    CanSendTwoFrameQuickGetInfo_Connector2(int32_t iSendDataLength, uint8_t *pDataToSend, uint8_t *pRecBuffer,int32_t iRecBufferLength)
{

    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(iSendDataLength<=0 || iSendDataLength>255){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength==0)
            pRecBuffer=NULL;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_NEW_CAN_SEND_1_FRAME_QUICK_GET_INFO_CONNECTORII);
    //    AddCharToBuffer((uint8_t)(iSendDataLength&0xFF));
        AddToBuffer(iSendDataLength,pDataToSend);
    //    AddCharToBuffer((uint8_t)(((iMaxWaitTime+5)/10)&0xFF));
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pRecBuffer,iRecBufferLength);
    }
    DelBuffer();

    return iRet;
}


int32_t    CanSendTwoFrameQuickGetInfo_New(int32_t iSendDataLength, uint8_t *pDataToSend, uint8_t *pRecBuffer,int32_t iRecBufferLength)
{

    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(iSendDataLength<=0 || iSendDataLength>255){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength==0)
            pRecBuffer=NULL;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(0x2A);
    //    AddCharToBuffer((uint8_t)(iSendDataLength&0xFF));
        AddToBuffer(iSendDataLength,pDataToSend);
    //    AddCharToBuffer((uint8_t)(((iMaxWaitTime+5)/10)&0xFF));
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pRecBuffer,iRecBufferLength);
    }
    DelBuffer();

    return iRet;
}

/****************************************************************
*                                                                *
*    功  能：KWP2000协议    发多帧数据到ECU并接收多帧应答,发送的
                字节可以大于256            *
*                                                                *
*    参  数：ucSendDataTimes    发送次数,可以是CA_ONCE_ONLY 或        *
*                            CA_REPEAT_UNTIL_NEW_COMMAND_BREAK

*            pReceiveBuffer    接收缓冲区指针,格式为:接收到的总    *
*                            帧数(1byte),第1帧数据长度(1byte),    *
*                            第1帧数据内容, ... 第n帧数据长度,    *
*                            第n帧数据内容.                        *
*                                                                *
*            iLengthReceiveBuffer接收缓冲区长度                    *
*            iFrameNumber        发送的帧数                        *
*                                                                *
*           后面参数依次为:(分两个字节来告诉下位机,你要发送命令的长度,用16进制)
            发送到ECU的命令的高字节  int32_t
            发送到ECU的命令的低字节  int32_t
                                             *
*            发送到ECU的第1帧数据,类型:uint8_t*
            ;
            发送到ECU的命令的高字节  int32_t
            发送到ECU的命令的低字节  int32_t                      *
*            发送到ECU的第2帧数据,类型:uint8_t*                        *
*                ... ...
            发送到ECU的命令的高字节  int32_t
            发送到ECU的命令的低字节  int32_t                                          *
*            发送到ECU的第n帧数据,类型:uint8_t*
                                *
*                                                                *
*    返回值：成功    实际接收到的总长度                            *
*            失败    出错代码(小于0)                                 *
*                                                                *
****************************************************************/
int32_t KwpSendDataToEcuGetAnswer_more(
                uint8_t ucSendDataTimes,

                uint8_t *pReceiveBuffer,
                int32_t iLengthReceiveBuffer,
                int32_t iFrameNumber,
                ...)
{
    va_list args;
    int32_t iTemp=0,i,iTemp1=0/*,iTemp2*/;
    uint8_t *pTemp;

    int32_t iRet=GD_OK;
    pTemp = 0;
    NewBuffer();
    do{
        if(ucSendDataTimes!=CA_ONCE_ONLY
            && ucSendDataTimes!=CA_REPEAT_UNTIL_NEW_COMMAND_BREAK)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iFrameNumber>255 || iFrameNumber<=0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(KWP_SEND_MORE_256);
        AddCharToBuffer((uint8_t)(ucSendDataTimes&0xFF));
        AddCharToBuffer((uint8_t)(iFrameNumber&0xFF));

        va_start(args, iFrameNumber);
        for (i=0;i<iFrameNumber;i++){
            iTemp1=va_arg(args,int32_t);
            //printf("iTemp1=%d\n",iTemp1);

            AddCharToBuffer((uint8_t)(iTemp1&0xFF));    //higth
            iTemp=va_arg(args,int32_t);

            AddCharToBuffer((uint8_t)(iTemp&0xFF));    //low

            pTemp=va_arg(args,uint8_t*);
            //printf("\n???????????????????\n");
            if(iTemp1==0){
                AddToBuffer(iTemp,pTemp);
            }
            else{
                AddToBuffer(iTemp1*256+iTemp,pTemp);
            }
        }
        va_end(args);
        break;
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();
    return iRet;
}

/****************************************************************
*                                                                *
*    功  能：VPW协议从ECU接收数据                   *
*                                                                *
*    参  数：ucIDFirst,ucIDSecond为过滤的ID，如果不用就设为0x7F *
*        ucFrameNum    从ECU接收数据的帧数           *
*        ucSendDataTimes     指令发送次数                      *
*       格  式：6127+00/FF+TNUM+RFUN+ID1+ID2+DATA1+DATA2+....      *
*                                                                *
*    返回值：成功    从ECU接收到数据的总长度               *
*            失败    出错代码                           *
*******************************************************************/
int32_t VPW_Init_SendMultiDataToEcuGetMulti(
                uint8_t ucIDFirst,
                uint8_t ucIDSecond,
                uint8_t ucFrameNum,
                uint8_t ucSendDataTimes)
{
    int32_t iRet=GD_OK;
    if( ((ucSendDataTimes&0xff)==CA_ONCE_ONLY)
            || ((ucSendDataTimes&0xff)==CA_REPEAT_UNTIL_NEW_COMMAND_BREAK))
    {
        NewBuffer();
        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_RECEIVE_DATA_FROM_ECU_VPW);
        AddCharToBuffer((uint8_t)(ucSendDataTimes&0xFF));
        AddCharToBuffer(0);
        AddCharToBuffer(ucFrameNum);
        //AddCharToBuffer(ucTime);
        AddCharToBuffer(ucIDFirst);
        AddCharToBuffer(ucIDSecond);
    }

    return iRet;
}
/****************************************************************
*                                                                *
*    功  能：KWP2000协议 发一帧数据到ECU并接收应答                *
*            (直到ECU不发才结束)                                    *
*                                                                *
*    参  数：pToEcuData            发送到ECU数据                    *
*            pReceiveBuffer        接收缓冲区指针                    *
*            iLengthReceiveBuffer接收缓冲区长度                    *
*            iMaximumWaitTime    最大等待时间(单位:毫秒)            *
*            ucSendDataTimes - CA_ONCE_ONLY （发一次）或 CA_REPEAT_UNTIL_NEW_COMMAND_BREAK （不停发直到新命令打断）                                                    *
*    返回值：成功    实际接收到的总长度                            *
*            失败    出错代码(小于0)                                 *
*                                                                *
****************************************************************/
int32_t FordIsoSendDataToEcuGetMultiFrameAnswer_ZJZ(
                                uint8_t ucSendDataTimes,//v1403 zjz
                                uint8_t *pToEcuData,
                                uint8_t *pReceiveBuffer,
                                int32_t iLengthReceiveBuffer,
                                int32_t iMaximumWaitTime)
{
    int32_t iRet=GD_OK;
    uint8_t CS,i;
    //int32_t iLengthSendToEcuData=CalculateKwpCommandLength(pToEcuData);
    uint8_t iLengthSendToEcuData = ((pToEcuData[0] & 0xF0)>>4) + 1;

    CS = 0;
    for (i=0;i<iLengthSendToEcuData-1;i++)
        CS += pToEcuData[i];
    pToEcuData[iLengthSendToEcuData -1] = CS;
    NewBuffer();
    do{
        if(ucSendDataTimes!=CA_ONCE_ONLY
            && ucSendDataTimes!=CA_REPEAT_UNTIL_NEW_COMMAND_BREAK)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer==0)
            pReceiveBuffer=NULL;
        if(iMaximumWaitTime>255*10 && iMaximumWaitTime<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_FORD_ISO__SEND_1_FRAME_RECEIVE_N_FRAME_ZJZ);
        AddCharToBuffer((uint8_t)(ucSendDataTimes&0xFF));
        AddCharToBuffer((uint8_t)(iLengthSendToEcuData&0xFF));
        AddToBuffer(iLengthSendToEcuData,pToEcuData);
        AddCharToBuffer((uint8_t)(((iMaximumWaitTime+5)/10)&0xFF));
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}
////////////////////////////////////
//适用于第二代CAN发多帧的情况
//0x55,0xaa,0x0d,0x61,0x01,0x08,0xfc,0x20,0x04,0x18,0x00,0xff,0x00,0x00,0x00,0x00,0xbb,
//0x55,0xaa,0x0d,0x61,0x01,0x08,0xfc,0x20,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xbb
int32_t    Can2SendTwoFrameQuickGetInfo(int32_t iSendDataLength, uint8_t *pDataToSend, uint8_t *pRecBuffer,int32_t iRecBufferLength)
{

    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(iSendDataLength<=0 || iSendDataLength>255){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength==0)
            pRecBuffer=NULL;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_CAN2_SEND_1_FRAME_QUICK_GET_INFO);
    //    AddCharToBuffer((uint8_t)(iSendDataLength&0xFF));
        AddToBuffer(iSendDataLength,pDataToSend);
    //    AddCharToBuffer((uint8_t)(((iMaxWaitTime+5)/10)&0xFF));
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pRecBuffer,iRecBufferLength);
    }
    DelBuffer();

    return iRet;
}



/*********************************************
 * 函数名:    Can2CheckXOR()
 * 作用:    对某个长度的数据进行异或运算.
 * 返回值:    异或运算结果.
 *********************************************/
 unsigned char Can2CheckXOR(uint8_t * pDataBuffer, int32_t iDataLen)
{
    int32_t    i;
    unsigned char cRetVal=pDataBuffer[2];
    for(i=3;i<iDataLen;i++)
        cRetVal^=pDataBuffer[i];
    return cRetVal;
}
/***********************************************************************
 * 函数名:   ()
 * 功能:  发送一帧数据给ECU, 并接收返回的一帧数据. 收到后在较短时
 *    间内迅速发送第二帧命令, 这样才能接收到ECU的某些状态信息,
 *    接收的帧数不定. 用于CAN 通讯方式.
 * 参数:  iSendDataLength   ---  所要发送数据的长度.
 *    pDataToSend   ---  要发送的数据.
 *    pRecBuffer   ---  接收数据缓冲.
 *    iRecBufferLength ---  接收缓冲区长度.
 *              bFrameNum           ---     接收的总帧数,（如果是0x7F，帧数无效，表示由下位机判断帧数）
 *
 * 返回值:  >  0   ---  接收的总帧数.
 *    <  0  ---  通讯出错.
 ***********************************************************************
Command Word: 6207h*/
int32_t Can2SendTwoFrameQuickGetFrameNumInfo(int32_t iSendDataLength, uint8_t *pDataToSend, uint8_t *pRecBuffer,int32_t iRecBufferLength,uint8_t bFrameNum)
{

 int32_t iRet=GD_OK;

 NewBuffer();
 do{
  if(iSendDataLength<=0 || iSendDataLength>255){
   iRet=EC_PARAMETER_ERROR;
   break;
  }
  if(iRecBufferLength<0){
   iRet=EC_PARAMETER_ERROR;
   break;
  }
  if(iRecBufferLength==0)
   pRecBuffer=NULL;
  if((bFrameNum == 0) || (bFrameNum >= 32))
   bFrameNum = 15;

  AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
  AddCharToBuffer(CAC_CAN2_SEND_1_FRAME_QUICK_GET_INFO_FRAME_NUM);
 // AddCharToBuffer((uint8_t)(iSendDataLength&0xFF));
  AddCharToBuffer((bFrameNum)&0xFF);//2007-05-12 yyz modify,因为下位机多减了1
  AddToBuffer(iSendDataLength,pDataToSend);
 // AddCharToBuffer((uint8_t)(((iMaxWaitTime+5)/10)&0xFF));
 }while(0);
 if(iRet==GD_OK){
  iRet=DisposeSendAndReceive(pRecBuffer,iRecBufferLength);
 }
 DelBuffer();
 return iRet;
}


/***********************************************************************
 *    函数名:         ()
 *    功能:        发送一帧数据给ECU, 并接收返回的一帧数据. 收到后在较短时
 *                间内迅速发送第二帧命令, 这样才能接收到ECU的某些状态信息,
 *                接收的帧数不定. 用于CAN 通讯方式.
 *    参数:        iSendDataLength      ---     所要发送数据的长度.
 *                pDataToSend            ---        要发送的数据.
 *                pRecBuffer            ---        接收数据缓冲.
 *                iRecBufferLength    ---        接收缓冲区长度.
 *
 *
 *    返回值:        >  0         ---        接收的总帧数.
 *                <  0        ---        通讯出错.
 ***********************************************************************/
/* Command Word: 6201 old id==6123h special for smart*/
int32_t    CanIISendTwoFrameQuickGetInfo(int32_t iSendDataLength, uint8_t *pDataToSend, uint8_t *pRecBuffer,int32_t iRecBufferLength)
{

    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(iSendDataLength<=0 || iSendDataLength>255){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength==0)
            pRecBuffer=NULL;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);//0x62
        AddCharToBuffer(CAC_CAN2_SEND_1_FRAME_QUICK_GET_INFO);//01
    //    AddCharToBuffer((uint8_t)(iSendDataLength&0xFF));
        AddToBuffer(iSendDataLength,pDataToSend);
    //    AddCharToBuffer((uint8_t)(((iMaxWaitTime+5)/10)&0xFF));
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pRecBuffer,iRecBufferLength);
    }
    DelBuffer();

    return iRet;
}
/****************************************************************
*                                                                *
*    功  能：读取字节型数据,发送一个字节接收一个字节,直到结束条件止.*
*                                                                *
*    参  数：ucMode从ECU接收数据的模式                             *
*            模式一: BENZ HFM进入。                              *
*            模式二: 双字节通迅。                                    *
*            模式三: 单字节通迅。                                *
*           iSendDataLength      ---     所要发送数据的长度.
*            pDataToSend            ---        要发送的数据.
*
*            pReceiveBuffer    接收缓冲区指针,内容为ECU返回的一帧    *
*                            数据.                                *
*                                                                *
*            iLengthReceiveBuffer接收缓冲区长度                    *
*                                                                *
*    返回值：成功    从ECU接收到的一帧数据的长度                    *
*            失败    出错代码                                        *
*   COMMAND WORD：620C                                                        *
****************************************************************/
int32_t BenzReadByteFromECU(int32_t iSendDataLength, uint8_t *pDataToSend,uint8_t ucMode,
                            uint8_t *pReceiveBuffer, int32_t iLengthReceiveBuffer)
{
    int32_t iRet;
    if( (ucMode<MINIMUM_MODE_RECEIVE_DATA_FROM_ECU)
            || (ucMode>MAXIMUM_MODE_RECEIVE_DATA_FROM_ECU))
    {
        return EC_PARAMETER_ERROR;
    }
        NewBuffer();
        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_BENZ_HFM_OLD_PROTOCOL);
        AddCharToBuffer((uint8_t)(ucMode));

        AddCharToBuffer((uint8_t)(iSendDataLength));

        AddToBuffer(iSendDataLength,pDataToSend);
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
        DelBuffer();
        return iRet;
}

/****************************************************************
*                                                                *
*    功  能：读取字节型数据,发送一个字节接收一个字节,直到结束条件止.*
*                                                                *
*    参  数：                                                    *
*           iSendDataLength      ---     所要发送数据的长度.     *
*            pDataToSend            ---        要发送的数据.           *
*                            第一个字节，发送帧数高位              *
*                            第二个字节，发送帧数低位           *
*                            第三个字节,CANBUS ID1                 *
*                            第四个字节,CANBUS ID2              *
*                            第五个字节，发送第一帧数据长度       *
*                            后面第一帧数据，+第N帧数据长度+数据*
*                                                               *
*            pReceiveBuffer    接收缓冲区指针,内容为ECU返回的一帧    *
*                            数据.                                *
*                                                                *
*            iLengthReceiveBuffer接收缓冲区长度                    *
*                                                                *
*    返回值：成功    从ECU接收到的一帧数据的长度                    *
*            失败    出错代码                                        *
*   COMMAND WORD：620D                                            *
****************************************************************/
int32_t GMSendMultiFrameToECUGetAnswer(int32_t iSendDataLength, uint8_t *pDataToSend,
                            uint8_t *pReceiveBuffer, int32_t iLengthReceiveBuffer)
{
           int32_t iRet;
        NewBuffer();
        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_GM_CAN2_SEND_MULTIFRAME_DW);
        AddToBuffer(iSendDataLength,pDataToSend);
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
        DelBuffer();
        return iRet;
}

/************************************************************************
*函  数  int32_t SelectIoLine_New(uint8_t ucInputLine, uint8_t ucOutputLine)   *
*功  能：设置SMARTBOX通信输入输出线                                     *
*                                                                       *
*参  数：ucInputLine   输入线                                           *
*        ucOutputLine  输出线                                           *
*                                                                       *
*返回值：成功          GD_OK                                            *
*        失败          出错代码                                         *
*                                                                          *
*说  明: 该函数只针对使用TOP第IV代接头情况下调用  2007.1.16             *
*************************************************************************/
int32_t SelectIoLine_New(uint8_t ucInputLine, uint8_t ucOutputLine)
{
    if(ucInputLine>15)        return EC_PARAMETER_ERROR;
    if(ucOutputLine>15)        return EC_PARAMETER_ERROR;
    s_SmartboxStruct.ucInputLine=ucInputLine;
    s_SmartboxStruct.ucOutputLine=ucOutputLine;
    return SetIoParameter();
}

/***********************************************************************
 *    函数名:         ()
 *    功能:        发送一帧数据给ECU, 并接收返回的一帧数据. 收到后在较短时
 *                间内迅速发送第二帧命令, 这样才能接收到ECU的某些状态信息,
 *                接收的帧数不定. 用于CAN 通讯方式.
 *    参数:        iSendDataLength      ---     所要发送数据的长度.
 *                pDataToSend            ---        要发送的数据.
 *                pRecBuffer            ---        接收数据缓冲.
 *                iRecBufferLength    ---        接收缓冲区长度.
 *
 *
 *    返回值:        >  0         ---        接收的总帧数.
 *                <  0        ---        通讯出错.
 ***********************************************************************/
/* Command Word: 6126h--->6201*/
int32_t    Can20SendTwoFrameQuickGetInfo(int32_t iSendDataLength, uint8_t *pDataToSend, uint8_t *pRecBuffer,int32_t iRecBufferLength)
{

    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(iSendDataLength<=0 || iSendDataLength>255){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength==0)
            pRecBuffer=NULL;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_CAN2_SEND_1_FRAME_QUICK_GET_INFO);//pqy2060926  0x26
    //    AddCharToBuffer((uint8_t)(iSendDataLength&0xFF));
        AddToBuffer(iSendDataLength,pDataToSend);
    //    AddCharToBuffer((uint8_t)(((iMaxWaitTime+5)/10)&0xFF));
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pRecBuffer,iRecBufferLength);
    }
    DelBuffer();

    return iRet;
}

/****************************************************************
*                                                               *
*   功  能：MABCO设置链路保持命令                                    *
*                                                               *
*    参  数：pLinkCommand    链路保持命令缓冲区,如为NULL则停止   *
*                           链路保持                            *
*           ucCommandLength  链路保持命令长度, 如为0则停止链路   *
*                           保持                                *
*           ucLinkCommandReturnLength    链路保持ECU返回字节数   *
*           wLinkKeepTime   链路保持间隔时间,单位毫秒; 取值范   *
*                           围: 大于0, 小于等于2550毫秒         *
*                                                               *
*   返回值：成功    GD_OK                                       *
*           失败    出错代码                                    *
*                                                               *
****************************************************************/
int32_t SetLinkKeep_Mabco(uint8_t *pLinkCommand,
                int32_t iCommandLength,
                int32_t iLinkCommandReturnLength,
                int32_t iLinkKeepTime)
{
    int32_t iRet=GD_OK;

    if((iCommandLength>255-3)||(iCommandLength<0))
        return EC_PARAMETER_ERROR;
    if((iLinkCommandReturnLength>255)|| (iLinkCommandReturnLength<0))
        return EC_PARAMETER_ERROR;
    if((iLinkKeepTime>2550) || (iLinkKeepTime<0))
        return EC_PARAMETER_ERROR;

    if( (pLinkCommand==NULL) || (iCommandLength==0) || (iLinkKeepTime==0) )
    {
        pLinkCommand=NULL;
        iCommandLength=0;
        iLinkCommandReturnLength=0;
        iLinkKeepTime=0;
    }

    NewBuffer();
    AddCharToBuffer(CAC_CONTRAL_COMMAND);//0x60
    //AddCharToBuffer(CAC_ECU_LINK_KEEP);
    AddCharToBuffer(CAC_ECU_LINK_KEEP_WABCO);//0x0F
    AddCharToBuffer((uint8_t)(iCommandLength&0xFF));
    if(iCommandLength!=0)
        AddToBuffer(iCommandLength,pLinkCommand);
    AddCharToBuffer((uint8_t)(iLinkCommandReturnLength&0xFF));
    AddCharToBuffer((uint8_t)(((iLinkKeepTime+5)/10)&0xFF));

    iRet=DisposeSendAndReceive(NULL,0);
    DelBuffer();

    return iRet;
}


int8_t EnableCanFunction(int8_t bLinstenOnly)
{
    int32_t i;
    uint8_t pucBuffer[80];

    uint8_t pucSendCommand[7]={0x55,0xAA,0x03,0x60,0x07,0x00,0x00};

    if(bLinstenOnly)    pucSendCommand[5]=0xFF;
    else    pucSendCommand[5]=0x00;

    for(i=2; i<6; i++) pucSendCommand[6]^=pucSendCommand[i];

    i=OneToOne_SendDataToEcuGetAnswer(CA_ONCE_ONLY,pucBuffer,80,1,7,pucSendCommand,CA_AUTO_RECOGNIZE,2,4);
    if(i>0)
    {
        return TRUE;
    }

    return FALSE;
}

/*=============================================================
*    FUNCTION NAME:    int8_t SetCanFilterId(int8_t bExtendedFrame, uint8_t *pucFilterId)
*    PARAMETER:        int8_t bExtendedFrame-----> is extended frame CAN or not?
*                    uint8_t *pucFilterId-------> the CAN filter id buffer
*                                            FORMAT: num,id ...
*    RETURN VALUE:    if succeed return TRUE
*                    else ruturn FALSE
===============================================================*/
int8_t SetCanFilterId(int8_t bExtendedFrame, uint8_t *pucFilterId)
{
    int32_t i,iRet;
    uint8_t pucBuffer[80];
//    uint8_t pBuf[100];
    uint8_t pucSendCommand[80]={0x55,0xAA,0x00,0x60,0x01,0x00,0x00,0x00};
    if(pucFilterId==NULL||pucFilterId[0]>4) return FALSE;
    if(bExtendedFrame)
    {
        pucSendCommand[5]=0xFF;
        pucSendCommand[2]=pucFilterId[0]*4+4;
    }
    else
    {
        pucSendCommand[5]=0x00;
        pucSendCommand[2]=pucFilterId[0]*2+4;

    }

    for(i=6; i<pucSendCommand[2]+3; i++) pucSendCommand[i]=pucFilterId[i-6];

    pucSendCommand[pucSendCommand[2]+3]=0x00;
    for(i=2; i<pucSendCommand[2]+3; i++) pucSendCommand[pucSendCommand[2]+3]^=pucSendCommand[i];

    iRet=OneToOne_SendDataToEcuGetAnswer(CA_ONCE_ONLY,pucBuffer,80,1,pucSendCommand[2]+4,pucSendCommand,CA_AUTO_RECOGNIZE,2,4);
    if(iRet>0) return TRUE;

    return FALSE;
}

/***********************************************************************
 *    函数名:         ()
 *    功能:        发送一帧数据给ECU, 并接收返回的一帧数据. 收到后在较短时
 *                间内迅速发送第二帧命令, 这样才能接收到ECU的某些状态信息,
 *                接收的帧数不定. 用于CAN 通讯方式.
 *    参数:        iSendDataLength      ---     所要发送数据的长度.
 *                pDataToSend            ---        要发送的数据.
 *                pRecBuffer            ---        接收数据缓冲.
 *                iRecBufferLength    ---        接收缓冲区长度.
 *              bFrameNum           ---     需要接收的总帧数
 *
 *
 *    返回值:        >  0         ---        接收的总帧数.
 *                <  0        ---        通讯出错.
 ***********************************************************************/
/* Command Word: 6129h*/

int32_t    Can2SendTwoFrameQuickGetInfoGM(int32_t iSendDataLength, uint8_t *pDataToSend, uint8_t *pRecBuffer,int32_t iRecBufferLength,uint8_t bFrameNum)
{

    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(iSendDataLength<=0 || iSendDataLength>255){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength==0)
            pRecBuffer=NULL;
        if((bFrameNum == 0) || (bFrameNum >= 32))
            bFrameNum = 15;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_GM_SEND_1_FRAME_QUICK_GET_INFO);
    //    AddCharToBuffer((uint8_t)(iSendDataLength&0xFF));
        AddCharToBuffer(bFrameNum&0xFF);//2006-08-12
        AddToBuffer(iSendDataLength,pDataToSend);
    //    AddCharToBuffer((uint8_t)(((iMaxWaitTime+5)/10)&0xFF));
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pRecBuffer,iRecBufferLength);
    }
    DelBuffer();

    return iRet;
}

int8_t EnableListenOnly(int8_t bLinstenOnly)
{
    int32_t i;
    uint8_t pucBuffer[80];

    uint8_t pucSendCommand[7]={0x55,0xAA,0x03,0x60,0x04,0x00,0x00};

    if(bLinstenOnly)    pucSendCommand[5]=0xFF;
    else    pucSendCommand[5]=0x00;

    for(i=2; i<6; i++) pucSendCommand[6]^=pucSendCommand[i];

    i=OneToOne_SendDataToEcuGetAnswer(CA_ONCE_ONLY,pucBuffer,80,1,7,pucSendCommand,CA_AUTO_RECOGNIZE,2,4);
    if(i>0) return TRUE;

    return FALSE;
}

/*=================================================================
*    FUNCTION NAME:    int8_t SetCanIOLine(int8_t bNormal)
*    PARAMETER:        int8_t bNormal
*    RETURN VALUE:    if succedd return TRUE
*                        return FALSE
*===================================================================*/
int8_t SetCanIOLine(int8_t bNormal)
{
    int32_t i;
    uint8_t pucBuffer[512]={0};

    uint8_t pucSendCommand[7]={0x55,0xAA,0x03,0x60,0x05,0x00,0x00};


    if(bNormal)    pucSendCommand[5]=0xFF;
    else    pucSendCommand[5]=0x00;

    for(i=2; i<6; i++) pucSendCommand[6]^=pucSendCommand[i];

    i=OneToOne_SendDataToEcuGetAnswer(CA_ONCE_ONLY,pucBuffer,512,1,7,pucSendCommand,CA_AUTO_RECOGNIZE,2,4);

    //if(i>0) return TRUE;
    if((i>0)&&(pucBuffer[2]==0x55)&&(pucBuffer[3]==0xaa)&&(pucBuffer[4]==0x01)&&(pucBuffer[5]==0x00)&&(pucBuffer[6]==0x01))
        return TRUE;
    return FALSE;
}

/*=================================================================
*    FUNCTION NAME:    int8_t SetCanBitRate(int8_t bExtendedFrame, uint8_t ucRate)
*    PARAMETER:        int8_t bExtendedFrame---->is extendedframe CAN or not?
*                    uint8_t ucRate------> CAN bux bitrate
*                    (0x01:1M bps 0x02:800K bps 0x03: 500K bps 0x04:250 bps 0x05:125K bps 0x06:50K bps
*                    0x07:20K bps 0x08:10K bps)
*    RETURN VALUE:    if succeed return TRUE
*                    else        return FALSE
===================================================================*/
int8_t SetCanBitRate(int8_t bExtendedFrame, uint8_t ucRate)
{
    int32_t i,iRet;
    uint8_t pucBuffer[80];
//    uint8_t pBuf[100];
    uint8_t pucSendCommand[8]={0x55,0xAA,0x04,0x60,0x02,0x00,0x00,0x00};

    if(bExtendedFrame)
        pucSendCommand[5]=0xFF;
    else
        pucSendCommand[5]=0x00;
    if(ucRate<1 || ucRate>12)
        return FALSE;
    pucSendCommand[6]=ucRate;
    for(i=2; i<7; i++)
        pucSendCommand[7]^=pucSendCommand[i];

    iRet=OneToOne_SendDataToEcuGetAnswer(CA_ONCE_ONLY,pucBuffer,80,1,8,pucSendCommand,CA_AUTO_RECOGNIZE,2,4);
    if(iRet>0) return TRUE;

    return FALSE;
}

/*=====================================================================
*    FUNCTION NAME: void ResetCanController()
*    PARAMETER:        void
*    RETURN VALUE:    void
=======================================================================*/
void ResetCanController()
{
    uint8_t pucResetCmd[12]={0x55,0xAA,0x08,'R','E','S','E','T','C','P','U',0x00};
    int i;

    BeginCombination();
    SetCommunicationMaximumWaitTime(18000);
    SelectIoLine(CA_LINE_IO5,CA_LINE_IO7);
    EnableLLine(CA_L_LINE_DISABLE);
    //if(bConnectorType!=CON_DiaGunV50)
    SetEcuCommunicationVoltageLogic(CA_12V,CA_POSITIVE);
    SetEctBaudRate(57600);
    SetCommunicationParameter(1,8,1,0);
    SetEcuCommTimeInterval(3,6,250);
    EndCombination();

    //chech the checksum
    for(i=0; i<=8; i++)
        pucResetCmd[11]^=pucResetCmd[i+2];
    SendDataToEcu(1,1,12,pucResetCmd);
//#ifndef _WINDOWS
//#else
//    Sleep(1);
//#endif
}

/******************************************************************************
* [ 函数名 ]       Can2SendOneFrameReceiveData()
* [ 功能 ]         通过smartbox, 发送一帧数据, 并根据相应的参数决定如何等待返回的
*                  数据帧.收到的数据帧格式为:
*                  [1字节的总帧数][15字节的第一帧收据][15字节的第2帧收据]......
* [ 参数描述 ]
* < iMode >-------------发送模式, 未用:
* < iReqLen >-----------发送帧的长度.
* < pReqBuffer >--------发送数据的存储区.
* < pRecBuffer >--------接收收据的缓冲区.
* < iRecLen >-----------接收缓冲区的长度.
* [ 返回值 ]          返回收到的数据帧个数.
*            <0-------出错.
* [ 说明 ]        本函数为CAN BUS协议CAN第二代接头所专用的发一帧收一帧函数

******************************************************************************/
int32_t Can2SendOneFrameReceiveData(int iMode,int iReqLen, uint8_t *pReqBuffer, uint8_t *pRecBuffer, int iRecLen)
{
    uint8_t    CommandBuff[256];
    uint8_t    AnswerBuff[256];
    int32_t    i,iRet;
    memset(CommandBuff,0,sizeof(CommandBuff));
    memset(AnswerBuff,0,sizeof(AnswerBuff));
    memcpy(CommandBuff,"\x55\xaa\x0d\x61\x01\x08\xfc\x20\x02\x10\xc0\xff\xff\xff\xff\xff",16);
    for(i=0;i<10;i++)
        CommandBuff[6+i] = pReqBuffer[4+i];
    CommandBuff[CommandBuff[2]+3] = CheckXOR(CommandBuff+2,CommandBuff[2]+1);
    iRet=OneToOne_SendDataToEcuGetAnswer(CA_ONCE_ONLY,AnswerBuff,256,1,CommandBuff[2]+4,CommandBuff,CA_AUTO_RECOGNIZE,2,4);
    if(iRet<0)
        return -2;
    for(i=0;i<20;i++)
        pRecBuffer[i] = AnswerBuff[i+1];
    return iRet;
}

/******************************************************************************
 * [ 函数名 ]       CanSendOneFrameReceiveDatasVW()
 * [ 功能 ]         通过smartbox, 发送一帧数据, 并根据相应的参数决定如何等待返回的
 *                  数据帧.收到的数据帧格式为:
 *                  [1字节的总帧数][15字节的第一帧收据][15字节的第2帧收据]......
 * [ 参数描述 ]
 * < iMode >-------------发送模式, 具体含义为:
 *        0----------------只发不收.
 *        1----------------发送1帧, 接收1帧.
 *        2----------------发送1帧, 接收2帧.
 *         others----------------暂无定义.
 * < iReqLen >-----------发送帧的长度.
 * < pReqBuffer >--------发送数据的存储区.
 * < pRecBuffer >--------接收收据的缓冲区.
 * < int32_t iAnsLen>--------接收第一帧的长度
 * < iRecLen >-----------接收缓冲区的长度.
 * [ 返回值 ]          返回收到的数据帧个数.
 *            <0-------出错.
 * [ 说明 ]            本函数为CAN BUS协议所专用,接收2帧模拟可行,在实测中需要调试, ?
 *                    不建议使用. 接收1帧没有任何问题.
 ******************************************************************************/
 //add for VW CAN 2007.11.06
int32_t  CanSendOneFrameReceiveDatasVW(int32_t iMode,int32_t iReqLen, uint8_t *pReqBuffer, uint8_t *pRecBuffer, int32_t iRecLen,int32_t iAnsLen)
{
    int32_t    iRes;
    uint8_t    TmpBuffer[128];
    pReqBuffer[iReqLen]=CheckXOR(pReqBuffer+1,iReqLen-1);/* add checksum. */

    memset(pRecBuffer,0,sizeof(pRecBuffer));

    do{
        int32_t time_count=0;
        /* send 0x55, get 0x01. */
        do{
            time_count++;
            memset(TmpBuffer,0,sizeof(TmpBuffer));
            if( (iRes=KnowReceiveFrameNumber_SendDataToEcuGetAnswer(1,"\x55",TmpBuffer,sizeof(TmpBuffer),1,1)) < 0 )
            {
                if(time_count <= 6)
                    continue;
                /* 超过握手尝试限数, 通讯出错. */
                else
                    return -1;
            }
            /* 收到应答, 退出握手. */
            if(TmpBuffer[2]==0x01)
                break;
        }while(1);
        /* send one can frame, and get ECU answer(one frame or more). */
        memset( TmpBuffer,0,sizeof(TmpBuffer) );

        if(iMode==0) /* Only send one CAN frame, not receiving any ECU frame. */
        {
            if( (iRes=KnowReceiveFrameNumber_SendDataToEcuGetAnswer((int32_t)(iReqLen+1),pReqBuffer,TmpBuffer,sizeof(TmpBuffer),1,3)) <0 )
                return -1;
            if( iRes>0 && TmpBuffer[2]==0xaa && TmpBuffer[3]==0x55 && TmpBuffer[4]==0x78 )
            /* if CANBOX received ok. */
                return 0;
            else /* CANBOX received error, resend the data. */
                continue;
        }

        if( (iRes=KnowReceiveFrameNumber_SendDataToEcuGetAnswer((int32_t)(iReqLen+1),pReqBuffer,TmpBuffer,sizeof(TmpBuffer),2,3,iAnsLen)) <0 )
            return  -1;

        if(iRes>0 && TmpBuffer[2]==0xaa && TmpBuffer[3]==0x55 && TmpBuffer[4]==0x78)
            break;  /* succeed. */
        else
            continue;  /* CANBOX receive data error, should resend the data. */
    }while(1);
    memmove(pRecBuffer+1,TmpBuffer+6,iAnsLen);
//    iRecCount++;
    if( (iRes=SendOneFrameNoAnswer(3, "\xaa\x55\x75")) <0 )
    return -1;
    return ( pRecBuffer[0]=1 );
    /* now we received one frame, check the checksum and decide what to do. */
}


//VW 修改
/******************************************************************************
 * [ 函数名 ]       CanSendOneFrameReceiveDatas()
 * [ 功能 ]         通过smartbox, 发送一帧数据, 并根据相应的参数决定如何等待返回的
 *                  数据帧.收到的数据帧格式为:
 *                  [1字节的总帧数][15字节的第一帧收据][15字节的第2帧收据]......
 * [ 参数描述 ]
 * < iMode >-------------发送模式, 具体含义为:
 *        0----------------只发不收.
 *        1----------------发送1帧, 接收1帧.
 *        2----------------发送1帧, 接收2帧.
 *         others----------------暂无定义.
 * < iReqLen >-----------发送帧的长度.
 * < pReqBuffer >--------发送数据的存储区.
 * < pRecBuffer >--------接收收据的缓冲区.
 * < iRecLen >-----------接收缓冲区的长度.
 * [ 返回值 ]          返回收到的数据帧个数.
 *            <0-------出错.
 * [ 说明 ]            本函数为CAN BUS协议所专用,接收2帧模拟可行,在实测中需要调试, ?
 *                    不建议使用. 接收1帧没有任何问题.
 ******************************************************************************/
int  CanSendOneFrameReceiveDatas_AUDI(int32_t iMode,int32_t iReqLen,uint8_t *pReqBuffer,uint8_t *pRecBuffer,int32_t iRecLen,int32_t iAnsLen)
{
    int32_t    iRes;
    uint8_t    TmpBuffer[128];
    pReqBuffer[iReqLen]=CheckXOR(pReqBuffer+1,iReqLen-1);/* add checksum. */

    memset(pRecBuffer,0,sizeof(pRecBuffer));

    do{
        int32_t time_count=0;
        /* send 0x55, get 0x01. */
        do{
            time_count++;
            memset(TmpBuffer,0,sizeof(TmpBuffer));
            if( (iRes=KnowReceiveFrameNumber_SendDataToEcuGetAnswer(1,"\x55",TmpBuffer,sizeof(TmpBuffer),1,1)) < 0 )
            {
                if(time_count <= 6)
                    continue;
                /* 超过握手尝试限数, 通讯出错. */
                else
                    return -1;
            }
            /* 收到应答, 退出握手. */
            if(TmpBuffer[2]==0x01)
                break;
        }while(1);
        /* send one can frame, and get ECU answer(one frame or more). */
        memset( TmpBuffer,0,sizeof(TmpBuffer) );

        if(iMode==0) /* Only send one CAN frame, not receiving any ECU frame. */
        {
            if( (iRes=KnowReceiveFrameNumber_SendDataToEcuGetAnswer((int32_t)(iReqLen+1),pReqBuffer,TmpBuffer,sizeof(TmpBuffer),1,3)) <0 )
                return -1;
            if( iRes>0 && TmpBuffer[2]==0xaa && TmpBuffer[3]==0x55 && TmpBuffer[4]==0x78 )
            /* if CANBOX received ok. */
                return 0;
            else /* CANBOX received error, resend the data. */
                continue;
        }

        if( (iRes=KnowReceiveFrameNumber_SendDataToEcuGetAnswer((int32_t)(iReqLen+1),pReqBuffer,TmpBuffer,sizeof(TmpBuffer),2,3,iAnsLen)) <0 )
            return  -1;

        if(iRes>0 && TmpBuffer[2]==0xaa && TmpBuffer[3]==0x55 && TmpBuffer[4]==0x78)
            break;  /* succeed. */
        else
            continue;  /* CANBOX receive data error, should resend the data. */
    }while(1);
    memmove(pRecBuffer+1,TmpBuffer+6,iAnsLen);
//    iRecCount++;
    if( (iRes=SendOneFrameNoAnswer(3, "\xaa\x55\x75")) <0 )
    return -1;
    return ( pRecBuffer[0]=1 );
    /* now we received one frame, check the checksum and decide what to do. */
}

/****************************************************************
*                                                               *
*       功  能：BENZ-38通讯引脚及参数控制                       *
*                                                               *
*          参  数：无                                              *
*                                                               *
*       返回值：成功    GD_OK                                   *
*               失败    出错代码                                *
* 说  明: 该函数只针对使用BENZ-38第接头情况下调用  2007-3-8 zw  *
****************************************************************/
int32_t SelectIoLine_38(uint8_t ucCommLine)
{
    int32_t iRet=GD_OK;
    NewBuffer();
    AddCharToBuffer(CAC_CONTRAL_COMMAND);
    AddCharToBuffer(CAC_IO_38_CONTRAL);
  AddCharToBuffer(ucCommLine);
  iRet=DisposeSendAndReceive(NULL,0);

    DelBuffer();

    return iRet;
}
/***************************************************************************************************
*      功能：三菱CANBUS链路保持设置，发两帧链路保命令到ECU中，ECU不回复任字节
*      参数：iFrameNumber  发送ECU的链路帧数
*            pLinkCommand1 pLinkCommand2链路命令1 2
*            iLinkCommandReturnLength1 iLinkCommandReturnLength2 ECU链路返回长度
*            iLinkKeepTime  链路保持时间 单位 10MS
*            返回值：成功    GD_OK
*            失败：  出错代码
*      说明：此函数用于三菱车型路保持
*****************************************************************************************************/
int32_t SetCanLinkKeep_Mit(int32_t iFrameNumber,uint8_t *pLinkCommand1,
                int32_t iCommandLength1,
                int32_t iLinkCommandReturnLength1,
                uint8_t *pLinkCommand2,
                int32_t iCommandLength2,
                int32_t iLinkCommandReturnLength2,
                int32_t iLinkKeepTime)
{
    int32_t iRet=GD_OK;


    //if( (pLinkCommand1==NULL) || (iCommandLength1==0) ||
        //(iLinkCommandReturnLength1==0) ||(pLinkCommand2==NULL) || (iCommandLength2==0) ||
        //(iLinkCommandReturnLength2==0) || (iLinkKeepTime==0) )

    if( (pLinkCommand1==NULL) || (iCommandLength1==0) ||(pLinkCommand2==NULL) || (iCommandLength2==0) ||
        (iLinkKeepTime==0) )
    {
        pLinkCommand1=NULL;
        iCommandLength1=0;
        iLinkCommandReturnLength1=0;
        pLinkCommand2=NULL;
        iCommandLength2=0;
        iLinkCommandReturnLength2=0;
        iLinkKeepTime=0;
    }

    NewBuffer();
    AddCharToBuffer(CAC_CONTRAL_COMMAND);
    AddCharToBuffer(CAC_LINK_KEEP_NO_ANSWER_MIT);
    AddCharToBuffer((uint8_t)iFrameNumber);
    AddCharToBuffer((uint8_t)(iCommandLength1&0xFF));
    AddToBuffer(iCommandLength1,pLinkCommand1);
    AddCharToBuffer((uint8_t)(iLinkCommandReturnLength1&0xFF));
    AddCharToBuffer((uint8_t)(iCommandLength2&0xFF));
    AddToBuffer(iCommandLength2,pLinkCommand2);
    AddCharToBuffer((uint8_t)(iLinkCommandReturnLength2&0xFF));
    AddCharToBuffer((uint8_t)(((iLinkKeepTime+5)/10)&0xFF));

    iRet=DisposeSendAndReceive(NULL,0);
    DelBuffer();

    return iRet;
}
/*************************************************************************************************
*
*
***************************************************************************************************/
int32_t SetCan20LinkKeep_NoAnswer(uint8_t SwitchValue)
{
    int32_t iRet=GD_OK;
    NewBuffer();

    AddCharToBuffer(CAC_CONTRAL_COMMAND);
    AddCharToBuffer(CAC_LINK_KEEP_NO_ANSWER);

    iRet=DisposeSendAndReceive(NULL,0);
    DelBuffer();

    return iRet;
}
/****************************************************************************************************
*CITREON CANBUS 发两帧收多帧，不加7F和78过滤，帧长度不固定
*
*COMMAND WORD：620E
*****************************************************************************************************/

int32_t    CanIISendTwoFrameQuickGetInfo_CTN(int32_t iSendDataLength,
                                uint8_t *pDataToSend,
                                uint8_t *pRecBuffer,
                                int32_t iRecBufferLength)
{
    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(iSendDataLength<=0 || iSendDataLength>255){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength==0)
            pRecBuffer=NULL;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_CAN2_SEND_2_FRAME_QUICK_GET_INFO_CTN);//CITREON id==6114
            AddToBuffer(iSendDataLength,pDataToSend);
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pRecBuffer,iRecBufferLength);
    }
    DelBuffer();

    return iRet;
}

/****************************************************************
*                                                               *
*   功  能：设置CAN20链路保持命令                                    *
*                                                               *
*    参  数：pLinkCommand    链路保持命令缓冲区,如为NULL则停止   *
*                           链路保持                            *
*           ucCommandLength  链路保持命令长度, 如为0则停止链路   *
*                           保持                                *
*           ucLinkCommandReturnLength    链路保持ECU返回字节数   *
*           wLinkKeepTime   链路保持间隔时间,单位毫秒; 取值范   *
*                           围: 大于0, 小于等于2550毫秒         *
*                                                               *
*   返回值：成功    GD_OK                                       *
*           失败    出错代码                                    *
*                                                               *
****************************************************************/
int32_t SetCan20LinkKeep(uint8_t *pLinkCommand,
                int32_t iCommandLength,
                int32_t iLinkCommandReturnLength,
                int32_t iLinkKeepTime)
{
    int32_t iRet=GD_OK;

    if((iCommandLength>255-3)||(iCommandLength<0))
        return EC_PARAMETER_ERROR;
    if((iLinkCommandReturnLength>255)|| (iLinkCommandReturnLength<0))
        return EC_PARAMETER_ERROR;
    if((iLinkKeepTime>2550) || (iLinkKeepTime<0))
        return EC_PARAMETER_ERROR;

    if( (pLinkCommand==NULL) || (iCommandLength==0) ||
        (iLinkCommandReturnLength==0) || (iLinkKeepTime==0) )
    {
        pLinkCommand=NULL;
        iCommandLength=0;
        iLinkCommandReturnLength=0;
        iLinkKeepTime=0;
    }

    NewBuffer();
    AddCharToBuffer(CAC_CONTRAL_COMMAND);
    AddCharToBuffer(CAC_CAN2BUS_LINK_KEEP);//nissa  old id 601e
    AddCharToBuffer((uint8_t)(iCommandLength&0xFF));
    if(iCommandLength!=0)
        AddToBuffer(iCommandLength,pLinkCommand);
    AddCharToBuffer((uint8_t)(iLinkCommandReturnLength&0xFF));
    AddCharToBuffer((uint8_t)(((iLinkKeepTime+5)/10)&0xFF));

    iRet=DisposeSendAndReceive(NULL,0);
    DelBuffer();

    return iRet;
}


/****************************************************************
*                                                                *
*    功  能：读取字节型数据,发送一个字节接收一个字节,直到结束条件止.        *
*                                                                *
*    参  数：ucMode    从ECU接收数据的模式                             *
*                模式一: 读取SSANGYONG的版本信息                         *
*            模式二: 读取当前故障码                            *
*            模式三: 读取历史故障码                                  *                                                  .                *
*                       模式四: 清除故障码                                      *
*            模式五: 读取数据流                                      *
*
*            pReceiveBuffer    接收缓冲区指针,内容为ECU返回的一帧    *
*                            数据.            *
*                                                                *
*            iLengthReceiveBuffer接收缓冲区长度                *
*                                                                *
*    返回值：        成功    从ECU接收到的一帧数据的长度            *
*            失败    出错代码                        *
*                                                                *
****************************************************************/
int32_t ReadByteBytedata(uint8_t ucMode, uint8_t *pReceiveBuffer, int32_t iLengthReceiveBuffer)
{
    int32_t iRet;
    if( (ucMode<MINIMUM_MODE_RECEIVE_DATA_FROM_ECU)
            || (ucMode>MAXIMUM_MODE_RECEIVE_DATA_FROM_ECU))
    {
        return EC_PARAMETER_ERROR;
    }
        NewBuffer();
        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_SEND_NBYTE_REC_NBYTE);
        AddCharToBuffer((uint8_t)(ucMode));

        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
        DelBuffer();
        return iRet;
}
//2006.8.1 linye 新接头链路保持设置命令
/****************************************************************
*                                                               *
*   功  能：设置链路保持命令                                    *
*                                                               *
*    参  数：pLinkCommand    链路保持命令缓冲区,如为NULL则停止   *
*                           链路保持                            *
*           ucCommandLength  链路保持命令长度, 如为0则停止链路   *
*                           保持                                *
*           ucLinkCommandReturnLength    链路保持ECU返回字节数   *
*           wLinkKeepTime   链路保持间隔时间,单位毫秒; 取值范   *
*                           围: 大于0, 小于等于2550毫秒         *
*                                                               *
*   返回值：成功    GD_OK                                       *
*           失败    出错代码                                    *
*                                                               *
****************************************************************/
int32_t SetCANLinkKeep20(uint8_t *pLinkCommand,
                int32_t iCommandLength,
                int32_t iLinkCommandReturnLength,
                int32_t iLinkKeepTime)
{
    int32_t iRet=GD_OK;

    if((iCommandLength>255-3)||(iCommandLength<0))
        return EC_PARAMETER_ERROR;
    if((iLinkCommandReturnLength>255)|| (iLinkCommandReturnLength<0))
        return EC_PARAMETER_ERROR;
    if((iLinkKeepTime>2550) || (iLinkKeepTime<0))
        return EC_PARAMETER_ERROR;

    if( (pLinkCommand==NULL) || (iCommandLength==0) ||
        (iLinkCommandReturnLength==0) || (iLinkKeepTime==0) )
    {
        pLinkCommand=NULL;
        iCommandLength=0;
        iLinkCommandReturnLength=0;
        iLinkKeepTime=0;
    }

    NewBuffer();
    AddCharToBuffer(CAC_CONTRAL_COMMAND);
    AddCharToBuffer(CAC_CANBUS2_LINK_KEEP);//<---fiat 0B
    AddCharToBuffer((uint8_t)(iCommandLength&0xFF));
    if(iCommandLength!=0)
        AddToBuffer(iCommandLength,pLinkCommand);
    AddCharToBuffer((uint8_t)(iLinkCommandReturnLength&0xFF));
    AddCharToBuffer((uint8_t)(((iLinkKeepTime+5)/10)&0xFF));

    iRet=DisposeSendAndReceive(NULL,0);
    DelBuffer();

    return iRet;
}

/****************************************************************
*                                                                *
*    功  能：KWP2000协议 发一帧数据到ECU并接收应答                *
*            (直到ECU不发才结束)                                    *
*                                                                *
*    参  数：wBusyFrameSum       忙帧7F个数，两个字节
            pToEcuData            发送到ECU数据                    *
*            pReceiveBuffer        接收缓冲区指针                    *
*            iLengthReceiveBuffer接收缓冲区长度                    *
*            iMaximumWaitTime    最大等待时间(单位:毫秒)            *
*                                                                *
*    返回值：成功    实际接收到的总长度                            *
*            失败    出错代码(小于0)                                 *
*                                                                *
****************************************************************/
int32_t ToyotaKwpSendDataToEcuGetMultiFrameAnswer(int32_t wBusyFrameSum,
                                uint8_t *pToEcuData,
                                uint8_t *pReceiveBuffer,
                                int32_t iLengthReceiveBuffer,
                                int32_t iMaximumWaitTime)
{
    int32_t iRet=GD_OK;
    int32_t iLengthSendToEcuData=CalculateKwpCommandLength(pToEcuData);

    NewBuffer();
    do{
        if(iLengthSendToEcuData<=0 || iLengthSendToEcuData>255){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer==0)
            pReceiveBuffer=NULL;
        if(iMaximumWaitTime>255*10 && iMaximumWaitTime<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        //logger("\n\n=====xja");
        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_KWP2000__SEND_1_FRAME_RECEIVE_N_FRAME_TY);
        AddCharToBuffer((uint8_t)(wBusyFrameSum>>8));
        AddCharToBuffer((uint8_t)(wBusyFrameSum&0xff));
        AddCharToBuffer((uint8_t)(iLengthSendToEcuData&0xFF));
        AddToBuffer(iLengthSendToEcuData,pToEcuData);
        AddCharToBuffer((uint8_t)(((iMaximumWaitTime+5)/10)&0xFF));
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

/***********************************************************************
 *    函数名:        SAE_J1708_SendDataToEcuGetAnswer()
 *    功能:        发送一帧数据给ECU, 并接收返回的一帧或多桢数据.
  *    参数:        iSendDataLength      ---     所要发送数据的长度.
 *                pDataToSend            ---        要发送的数据.
 *                pRecBuffer            ---        接收数据缓冲.
 *                iRecBufferLength    ---        接收缓冲区长度.
 *
 *
 *    返回值:        >  0         ---        接收的总帧数.
 *                <  0        ---        通讯出错.
 ***********************************************************************/
/* Command Word: */
int32_t    SAE_J1708_SendDataToEcuGetAnswer(int32_t iSendDataLength, uint8_t *pDataToSend, uint8_t *pRecBuffer,int32_t iRecBufferLength)
{

    int32_t iRet=GD_OK;

    NewBuffer();

    do{
        if(iSendDataLength<=0 || iSendDataLength>255){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength==0)
            pRecBuffer=NULL;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_VOVOL_CAN_SEND_N_FRAME_RECEIVE_N_FRAME);//vovol truck old id==620b

        AddToBuffer(iSendDataLength,pDataToSend);
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pRecBuffer,iRecBufferLength);
    }

    DelBuffer();

    return iRet;
}

int32_t CumminsDieselSAE1708_SendDataToEcuGetAnswer(
                uint8_t ucSendDataTimes,
                uint8_t *pReceiveBuffer,
                int32_t iLengthReceiveBuffer,
                int32_t iFrameNumber,
                ...)
{
    va_list args;
    int32_t iTemp,i;
    uint8_t *pTemp;

    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(ucSendDataTimes!=CA_ONCE_ONLY
            && ucSendDataTimes!=CA_REPEAT_UNTIL_NEW_COMMAND_BREAK)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iFrameNumber>255 || iFrameNumber<=0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);//old id==6128  for beijing bus
        AddCharToBuffer(CAC_CUMMINS_CAN_SEND_ONE_FRAME_RECEIVE_N_FRAME);
        AddCharToBuffer((uint8_t)(ucSendDataTimes&0xFF));
        AddCharToBuffer((uint8_t)(iFrameNumber&0xFF));

        va_start(args, iFrameNumber);
        for (i=0;i<iFrameNumber;i++){

            iTemp=va_arg(args,int32_t);
            if(iTemp>255 || iTemp<=0){
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            pTemp=va_arg(args,uint8_t*);             //SendCommand
            AddToBuffer((int32_t)iTemp,pTemp);
        }
        va_end(args);
        break;
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

int32_t CumminsDieselSAE1708_SendMultiFramesToEcuGetAnswer(
                uint8_t ucSendDataTimes,
                uint8_t *pReceiveBuffer,
                int32_t iLengthReceiveBuffer,
                int32_t iFrameNumber,
                uint8_t *pTemp
                )
{
    //va_list args;
    int32_t i,j = 0;

    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(ucSendDataTimes!=CA_ONCE_ONLY
            && ucSendDataTimes!=CA_REPEAT_UNTIL_NEW_COMMAND_BREAK)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iFrameNumber>255 || iFrameNumber<=0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);//old id==6128 for beijing bus
        AddCharToBuffer(CAC_CUMMINS_CAN_SEND_ONE_FRAME_RECEIVE_N_FRAME);
        AddCharToBuffer((uint8_t)(ucSendDataTimes&0xFF));
        AddCharToBuffer((uint8_t)(iFrameNumber&0xFF));

        j = 0;
        for (i=0;i<iFrameNumber;i++)
        {

            AddCharToBuffer(pTemp[j]);
            AddToBuffer(pTemp[j],pTemp+j+1);
            j = j + pTemp[j]+1;
        }
        break;
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

/****************************************************************
*
*   功  能：设置 CAN BUS 链路保持命令                                    *
*
*    参  数：pLinkCommand    链路保持命令缓冲区,如为NULL则停止
*                               链路保持
*               ucCommandLength 链路保持命令长度, 如为0则停止链路
*                              保持
*               ucLinkCommandReturnLength    链路保持ECU返回字节数
*               wLinkKeepTime   链路保持间隔时间,单位毫秒; 取值范
*                           围: 大于0, 小于等于2550毫秒
*
*   返回值：成功    GD_OK
*           失败    出错代码
*   说明:该函数与SetLinkKeep()除了命令字不同, 其余一样.
*
****************************************************************/
int32_t SetCanIIBusLinkKeep(uint8_t *pLinkCommand,
                int32_t iCommandLength,
                int32_t iLinkCommandReturnLength,
                int32_t iLinkKeepTime)
{
    int32_t iRet=GD_OK;

    if((iCommandLength>255-3)||(iCommandLength<0))
        return EC_PARAMETER_ERROR;
    if((iLinkCommandReturnLength>255)|| (iLinkCommandReturnLength<0))
        return EC_PARAMETER_ERROR;
    if((iLinkKeepTime>2550) || (iLinkKeepTime<0))
        return EC_PARAMETER_ERROR;

    if( (pLinkCommand==NULL) || (iCommandLength==0) ||
        (iLinkCommandReturnLength==0) || (iLinkKeepTime==0) )
    {
        pLinkCommand=NULL;
        iCommandLength=0;
        iLinkCommandReturnLength=0;
        iLinkKeepTime=0;
    }

    NewBuffer();
    AddCharToBuffer(CAC_CONTRAL_COMMAND);
    AddCharToBuffer(CAC_CAN2BUS_LINK_KEEP);  //6010H  FOR smart CAN BUS
    AddCharToBuffer((uint8_t)(iCommandLength&0xFF));
    if(iCommandLength!=0)
        AddToBuffer(iCommandLength,pLinkCommand);
    AddCharToBuffer((uint8_t)(iLinkCommandReturnLength&0xFF));
    AddCharToBuffer((uint8_t)(((iLinkKeepTime+5)/10)&0xFF));

    iRet=DisposeSendAndReceive(NULL,0);
    DelBuffer();

    return iRet;
}

/****************************************************************
*                                                                *
*    功  能：读取字节型数据,发送一个字节接收一个字节,直到结束条件止.        *
*                                                                *
*    参  数：ucMode    从ECU接收数据的模式                             *
*            模式一: 读取LANDROVER系统进入                         *
*            模式二: 读取故障码                            *
*            模式三: 清除故障码                                  *                                                  .                *
*           模式四: 读取数据流                                  *
*                                                *
*
*            pReceiveBuffer    接收缓冲区指针,内容为ECU返回的一帧    *
*                            数据.            *
*                                                                *
*            iLengthReceiveBuffer接收缓冲区长度                *
*                                                                *
*    返回值：成功    从ECU接收到的一帧数据的长度            *
*            失败    出错代码                        *
*    command word:6115                                                            *
****************************************************************/
int32_t ReadByteBytedataLand(uint8_t ucMode, uint8_t *pReceiveBuffer, int32_t iLengthReceiveBuffer)
{
    int32_t iRet;
    if( (ucMode<MINIMUM_MODE_RECEIVE_DATA_FROM_ECU)
            || (ucMode>MAXIMUM_MODE_RECEIVE_DATA_FROM_ECU))
    {
        return EC_PARAMETER_ERROR;
    }
        NewBuffer();
        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_LANDROVER__SEND_1_BTYE_RECEIVE_1_BYTE);
        AddCharToBuffer((uint8_t)(ucMode));

        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
        DelBuffer();
        return iRet;
    }



/****************************************************************
*                                                                *
*    功  能：VPW协议 发一帧数据到ECU并接收应答(直到ECU不发才        *
*            结束)                                                *
*                                                                *
*    参  数：ucIDFirst,ucIDSecond为过滤的ID，如果不用就设为0x7F  *
*            ucFrameNum            从ECU接收数据的帧数                *
*            ucSendDataTimes        指令发送次数                    *
*           wLengthSendToEcuData发送的数据长度                    *
*            pToEcuData            发送到ECU数据                    *
*            pReceiveBuffer        接收缓冲区指针                    *
*            iLengthReceiveBuffer接收缓冲区长度                    *
*                                                                *
*    返回值：成功    实际接收到的总长度                            *
*            失败    出错代码(小于0)                                 *
*                                                                *
****************************************************************/
int32_t VPW_SendDataToEcuGetMultiFrameAnswer(
                                uint8_t ucIDFirst,
                                uint8_t ucIDSecond,
                                uint8_t ucFrameNum,
                                int32_t iLengthSendToEcuData,
                                uint8_t *pToEcuData,
                                uint8_t *pReceiveBuffer,
                                int32_t iLengthReceiveBuffer)
{
    int32_t iRet=GD_OK;

    // VPW_Init_SendMultiDataToEcuGetMulti
    if( (ucFrameNum <= 0)||(ucFrameNum > 255))   //old vaule==31   20081211
	{
		return EC_PARAMETER_ERROR;
	}
	if( 1 )
	{
		NewBuffer();
		AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
		AddCharToBuffer(CAC_RECEIVE_DATA_FROM_ECU_VPW);
		AddCharToBuffer((uint8_t)(CA_ONCE_ONLY&0xFF));
		AddCharToBuffer(0);
		AddCharToBuffer(ucFrameNum);
		//AddCharToBuffer(ucTime);
		AddCharToBuffer(ucIDFirst);
		AddCharToBuffer(ucIDSecond);
	}

    // VPW_Add_SendDataToEcuGetAnswer
    uint8_t ucTemp;

	do
    {
		if(iLengthSendToEcuData<0 || iLengthSendToEcuData>255)
        {
			iRet=EC_PARAMETER_ERROR;
			break;
		}

		//Add frame number
		ucTemp=(*(GetBuffer()+3))&0xFF;
		if(ucTemp==255)
        {
			iRet=EC_OVER_MAXIMUM_NUMBER;
			break;
		}
		*(GetBuffer()+3)=ucTemp+1;

		AddCharToBuffer((uint8_t)(iLengthSendToEcuData&0xFF));
		AddToBuffer(iLengthSendToEcuData,pToEcuData);
		break;
	}
    while(0);

    // VPW_SendAndReceive_SendDataToEcuGetAnswer
    iRet = DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
	DelBuffer();

    return iRet;
}


/********************************************************************************
*                                                                                *
*    功  能：地址码通信方式(可以设定延时时间)                                       *
*                                                                                *
*    参  数：ucAddressCode        地址码                                            *
*            pReceiveBuffer        接收缓冲区                                        *
*            iLengthReceiveBuffer接收缓冲区长度                                    *
*           fBps                与ECU通信波特率                                 *
*           iTimeKW1Before      延时时间单位ms(0-255ms),0x55与KW1之间延时。     *
*            iTimeSendKW2ToEcu   延时时间单位ms (0-255ms),KW2 取返发回延时。        *
*           iLLineSwitch        发送完地址码后L LINE 是不是关掉,0为关掉.        *
*            bRecognizeBaudRate    需要识别并自动设置波特率                        *
*            bSecondByteReverseReturnToECU    第二个字节是否要求                    *
*                                            取反发回                            *
*            iReceiveFrameNumber    要接收的帧数,取值范围:0~255                     *
*                                (不包括取反发回的字节)                            *
*                                                                               *
*           后面参数依次为:                                                     *
*                                                                               *
*           接收的第1帧数据长度,类型:int32_t                                     *
*           接收的第2帧数据长度,类型:int32_t                                     *
*                   ...   ...                                                   *
*           接收的第iReceiveFrameNumber帧数据长度                                 *
*                                                                                *
*    返回值：成功    返回从ECU接收的版本信息长度,如无版本信息,                    *
*                    返回GD_OK                                                    *
*            失败    出错代码                                                        *
*                                                                                *
*********************************************************************************/
int32_t AddressCodeWayAdjustTime(
                uint8_t ucAddressCode,
                uint8_t *pReceiveBuffer,
                int32_t iLengthReceiveBuffer,
                float fBps,
                int8_t bRecognizeBaudRate,
                uint8_t iTimeKW1Before,
                uint8_t iTimeSendKW2ToEcu,
                int8_t iLLineSwitch,
                int8_t bSecondByteReverseReturnToECU,
                int32_t iReceiveFrameNumber,...
                )
{
    int32_t iRet=GD_OK;

    if(iReceiveFrameNumber<0 || iReceiveFrameNumber>255)
        return EC_PARAMETER_ERROR;

    NewBuffer();
    AddCharToBuffer(CAC_CONTRAL_COMMAND);
    AddCharToBuffer(CAC_ADJUST_TIME_ADDRESS_CODE_WAY);

    if(fabs(fBps-5)<0.0001)
    {
        AddCharToBuffer(1); //way 1              5bps
        AddCharToBuffer(ucAddressCode);
        AddCharToBuffer((uint8_t)(iLLineSwitch?0xFF:0x00));
        AddCharToBuffer((uint8_t)(bRecognizeBaudRate?0xFF:0x00));
    }
    else if(fabs(fBps-0xc8)<0.0001)
    {
        AddCharToBuffer(3); //way 3                200bps
        AddCharToBuffer(ucAddressCode);
        AddCharToBuffer((uint8_t)(iLLineSwitch?0xFF:0x00));
        AddCharToBuffer((uint8_t)(bRecognizeBaudRate?0xFF:0x00));
    }
    else
    {
        AddCharToBuffer(2); //way 2                其他
        AddCharToBuffer(ucAddressCode);
        AddCharToBuffer((uint8_t)(iLLineSwitch?0xFF:0x00));
            if(fBps>1000.0)
            {
                     AddCharToBuffer((uint8_t)((((int32_t)(65536-18432000/32/fBps))>>8)&0xFF));
                     AddCharToBuffer((uint8_t)(((int32_t)(65536-18432000/32/fBps))&0xFF));
            }
            else
            {      //2003.3.26 add
                     int32_t iTemp=(int32_t)(fBps+0.5);
                     AddCharToBuffer((uint8_t)((iTemp>>8)&0xFF));
                     AddCharToBuffer((uint8_t)(iTemp&0xFF));
            }
    }

    AddCharToBuffer((uint8_t)(iTimeKW1Before&0xFF));
        AddCharToBuffer((uint8_t)(iTimeSendKW2ToEcu&0xFF));
    AddCharToBuffer((uint8_t)(bSecondByteReverseReturnToECU?0xFF:0x00));
    AddCharToBuffer((uint8_t)(iReceiveFrameNumber&0xFF));

    if(iReceiveFrameNumber<=0)
    {
        AddCharToBuffer((uint8_t)(0));
    }
    else
    {
        int32_t i,iTemp;
        va_list args;
        va_start(args, iReceiveFrameNumber);
        for(i=0;i<iReceiveFrameNumber;i++)
        {
            iTemp=va_arg(args,int32_t);
            if((iTemp>255) || (iTemp<=0) )
            {
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
        }
        va_end(args);
    }

    if(iRet==GD_OK)
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    DelBuffer();

    return iRet;
}

/**********************************************************************************************************************************************
*function:CAN2.0 Send n cmd received dedicated m answer(s)
*Parameter:ucSendDataTimes- communication times,can be CA_ONCE_ONLY (only send once) or
*                                               CA_REPEAT_UNTIL_NEW_COMMAND_BREAK (keep send and receive until another new command be sent)
*          iReceiveFrameNumber - how many frame want to receive
*
*                            iReceiveFrameNumber = m - n + 1 ;
*                             m = total frames you want to receive
*                                  n = total frames you want to send
*                            m >= n
*                            if(iReceiveFrameNumber==0 && iSendFrameNumber==1) send only
*          iLengthSendToEcuData - the length of the command
*          pToEcuData- the address of the command to send
*          iLengthReceiveBuff- the length of the buffer to hold the answer
*          pReceiveBuffer-  the address of the buffer to hold the answer
*Retrun:The length of the answer
*Create by zjz@holdenv3100
**************************************************************************************************************************************************/
int32_t  HoldenCan20SendNFramesReceiveNDatas(uint8_t ucSendDataTimes,
                                int32_t iSendFrameNumber,
                                int32_t iReceiveFrameNumber,
                                int32_t iLengthReceiveBuffer,
                                uint8_t *pReceiveBuffer, ...
                                )
{
    int32_t iRet=GD_OK,i,iTemp;
    uint8_t *pTemp;
    va_list args;

    NewBuffer();
    do
    {
        if( ucSendDataTimes != CA_ONCE_ONLY
            && ucSendDataTimes != CA_REPEAT_UNTIL_NEW_COMMAND_BREAK )
        {
            iRet = EC_PARAMETER_ERROR;
            break;
        }

        if(iLengthReceiveBuffer<0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer==0)
            pReceiveBuffer=NULL;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);
        AddCharToBuffer(CAC_HDN_CAN20_SND_N_FRAME_RECV_N_FRAME);
        AddCharToBuffer((uint8_t)(ucSendDataTimes&0xFF));
        AddCharToBuffer((uint8_t)(iSendFrameNumber&0xff));//v3100 zjz
        AddCharToBuffer((uint8_t)(iReceiveFrameNumber&0xff));
        va_start(args, pReceiveBuffer);

        for(i=0;i<iSendFrameNumber;i++){
            iTemp = va_arg(args,int32_t); //length
            if(iTemp > 255 || iTemp <= 0){
                iRet = EC_PARAMETER_ERROR;
                break;
            }
            pTemp = va_arg(args,uint8_t*);
            //AddCharToBuffer((uint8_t)(iTemp&0xFF));
            AddToBuffer(iTemp,pTemp);
        }
        va_end(args);
        //AddToBuffer(iLengthSendToEcuData,pToEcuData);
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();


    return iRet;
}

/******************************************************************************************************************************
*  功能：CANBUS 发多帧收多帧
*
*  参数：iSendDataLength     发送数据长度
*        iReqFrames          发送帧数
*        *pDataToSend        发送命令数据缓冲区指针 里面的数据格式为55 aa 0b 08 fc 00 xx xx xx xx xx xx xx xx xx +.................
*        *pRecBuffer         接收命令数据缓冲区指针
*        iRecBufferLength    接收命令数据缓冲区长度
*        iAskFrames          接收帧数(如果为ff的话，表示下位机自动识别)
*
* 返回缓冲区内容为  帧数  每一桢的数据内容例如 01 55 aa 0b 08 fd 00 xx xx xx xx xx xx xx xx xx
* 返回值：成功     返回总长度
*          失败     出错代码
*********************************************************************************************************************************/
int32_t KnowFramesCanSendDataToECUGetAnswer(int32_t iSendDataLength, int32_t iReqFrames,uint8_t *pDataToSend, uint8_t *pRecBuffer,int32_t iRecBufferLength,int32_t iAskFrames)
{
     int32_t iRet=GD_OK;
     NewBuffer();
     do
     {
          if(iSendDataLength<=0 || iSendDataLength>255)
          {
               iRet=EC_PARAMETER_ERROR;
               break;
         }
         if(iRecBufferLength<0)
         {
               iRet=EC_PARAMETER_ERROR;
               break;
         }
         if(iRecBufferLength==0)
             pRecBuffer=NULL;

         AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);   //0x62
         AddCharToBuffer(CAC_DENGFENG_CAN_SEND_N_REC_N_FRAME);    //0x08
         AddCharToBuffer(iAskFrames);
         AddCharToBuffer(iReqFrames);
         AddToBuffer(iSendDataLength,pDataToSend);
     }while(0);
     if(iRet==GD_OK)
     {
          iRet=DisposeSendAndReceive(pRecBuffer,iRecBufferLength);
     }
     DelBuffer();
     return iRet;
}

/***********************************************************************
 *    函数名:         PorscheCanSendMulFrameGetMulFrame()
 *    功能:        porsche专用CAN BUS 通讯函数1：发多帧收多帧
 *                发送帧数由pDataToSend第1个字节决定
 *                接收帧数从接收到的数据判断,直到接收到最后一帧不是(xx)&(0x30H)==0x10。
 *                              遇到最后一帧是多帧开始帧时会自动发送 {B2}流控制帧激发连续帧
 *                具体见PORSCHE  CAN BUS协议中多帧格式
 *    参数:        iSendDataLength      ---     所要发送数据的长度.
 *                pDataToSend            ---        要发送的数据,第一个字节表示发送的帧数，
 *                                  ---     第二个字节为发送ID1
 *                                  ---     第二个字节为发送ID2
 *                                  ---     发送数据第一帧长度
 *                                  ---     发送第一帧数据
 *                                  ---     .............
 *                pRecBuffer            ---        接收数据缓冲.前两个字节表示接收到的帧数
 *                iRecBufferLength    ---        接收缓冲区长度.
 *
 *
 *    返回值:        >  0         ---        接收的总帧数.
 *                <  0        ---        通讯出错.
 ***********************************************************************/
/* Command Word: 6218h*/

int32_t    PorscheCanSendMulFrameGetMulFrame(int32_t iSendDataLength, uint8_t *pDataToSend, uint8_t *pRecBuffer,int32_t iRecBufferLength)
{

    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        //if(iSendDataLength<=0 || iSendDataLength>255){
        //    iRet=EC_PARAMETER_ERROR;
        //    break;
        //}
        if(iSendDataLength<=0) break;
        if(iRecBufferLength<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength==0)
            pRecBuffer=NULL;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_PURSCHE_CAN_SEND_N_FRAME_RECEIVE_N_FRAME);
    //    AddCharToBuffer((uint8_t)(iSendDataLength&0xFF));
        AddToBuffer((int32_t)iSendDataLength,pDataToSend);
    //    AddCharToBuffer((uint8_t)(((iMaxWaitTime+5)/10)&0xFF));
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pRecBuffer,(int32_t)iRecBufferLength);
    }
    DelBuffer();

    return iRet;
}

/****************************************************************
*                                                                *
*    功  能：地址码通信方式                                           *
*                                                                *
*    参  数：ucAddressCode        地址码                            *
*            pReceiveBuffer        接收缓冲区                        *
*            iLengthReceiveBuffer接收缓冲区长度                    *
*            //fBps                与ECU通信波特率                    *
*           iMode               通迅模式选择                    *
*            bRecognizeBaudRate    需要识别并自动设置波特率        *
*            bSecondByteReverseReturnToECU    第二个字节是否要求    *
*                                            取反发回            *
*            iReceiveFrameNumber    要接收的帧数,取值范围:0~255     *
*                                (不包括取反发回的字节)            *
*                                                               *
*           后面参数依次为:                                     *
*                                                               *
*           接收的第1帧数据长度,类型:int32_t                     *
*           接收的第2帧数据长度,类型:int32_t                     *
*                   ...   ...                                   *
*           接收的第iReceiveFrameNumber帧数据长度             *
*                                                                *
*    返回值：成功    返回从ECU接收的版本信息长度,如无版本信息,    *
*                    返回GD_OK                                    *
*            失败    出错代码                                        *
*                                                                *
****************************************************************/
int32_t AddressCodeWayChoiceMode(
                uint8_t ucAddressCode,
                uint8_t *pReceiveBuffer,
                int32_t iLengthReceiveBuffer,
                uint8_t  iMode,
                int8_t bRecognizeBaudRate,
                int8_t bSecondByteReverseReturnToECU,
                int32_t iReceiveFrameNumber,...
                )
{
    int32_t iRet=GD_OK;

    if(iReceiveFrameNumber<0 || iReceiveFrameNumber>255)
        return EC_PARAMETER_ERROR;

    NewBuffer();
    AddCharToBuffer(CAC_CONTRAL_COMMAND);
    AddCharToBuffer(CAC_ECU_ADDRESS_CODE_WAY2);

    AddCharToBuffer(iMode); //way 3以上使用可选模式
    AddCharToBuffer(ucAddressCode);
    AddCharToBuffer((uint8_t)(bRecognizeBaudRate?0xFF:0x00));
    AddCharToBuffer((uint8_t)(bSecondByteReverseReturnToECU?0xFF:0x00));
    AddCharToBuffer((uint8_t)(iReceiveFrameNumber&0xFF));

    if(iReceiveFrameNumber<=0){
        AddCharToBuffer((uint8_t)(0));
    }
    else{
        int32_t i,iTemp;
        va_list args;
        va_start(args, iReceiveFrameNumber);
        for(i=0;i<iReceiveFrameNumber;i++){
            iTemp=va_arg(args,int32_t);
            if((iTemp>255) || (iTemp<=0) ){
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
        }
        va_end(args);
    }

    if(iRet==GD_OK)
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    DelBuffer();

    return iRet;
}

/****************************************************************
*                                                                *
*    功  能：Micas协议    发多帧数据到ECU并接收多帧应答            *
*                                                                *
*    参  数：ucSendDataTimes    发送次数,可以是CA_ONCE_ONLY 或        *
*                            CA_REPEAT_UNTIL_NEW_COMMAND_BREAK    *
*            pReceiveBuffer    接收缓冲区指针,格式为:接收到的总    *
*                            帧数(1byte),第1帧数据长度(1byte),    *
*                            第1帧数据内容, ... 第n帧数据长度,    *
*                            第n帧数据内容.                        *
*                                                                *
*            iLengthReceiveBuffer接收缓冲区长度                    *
*            iFrameNumber        发送的帧数                        *
*                                                                *
*           后面参数依次为:                                     *
*                                                                *
*            发送的第1帧数据长度,类型:int32_t                     *
*            发送到ECU的第1帧数据,类型:uint8_t*                        *
*
*            发送的第2帧数据长度,类型:int32_t                     *
*            发送到ECU的第2帧数据,类型:uint8_t*                        *
*                ... ...                                            *
*            发送的第n帧数据长度,类型:int32_t                     *
*            发送到ECU的第n帧数据,类型:uint8_t*                        *
*                                                                *
*    返回值：成功    实际接收到的总长度                            *
*            失败    出错代码(小于0)                                 *
*                                                                *
****************************************************************/
int32_t Micas_SendDataToEcuGetAnswer(
        uint8_t ucSendDataTimes,
        uint8_t *pReceiveBuffer,
        int32_t iLengthReceiveBuffer,
        int32_t iFrameNumber,
        ...)
{
    va_list args;
    int32_t iTemp,i;
    uint8_t *pTemp;

    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(ucSendDataTimes!=CA_ONCE_ONLY
            && ucSendDataTimes!=CA_REPEAT_UNTIL_NEW_COMMAND_BREAK)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iFrameNumber>255 || iFrameNumber<=0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_MICAS_1_FRAME_RECEIVE_ANY_FRAME);
        AddCharToBuffer((uint8_t)(ucSendDataTimes&0xFF));
        AddCharToBuffer((uint8_t)(iFrameNumber&0xFF));

        va_start(args, iFrameNumber);
        for (i=0;i<iFrameNumber;i++){

            iTemp=va_arg(args,int32_t);
            if(iTemp>255 || iTemp<=0){
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            pTemp=va_arg(args,uint8_t*);             //SendCommand
            AddToBuffer(iTemp,pTemp);
        }
        va_end(args);
        break;
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}
/******************************************************************************
函数名称：RecOneFrameSendOneFrame
功能：专门负责接收super16主动发送的版本信息所有
增加日期：09.02.20 by zyzh
*******************************************************************************/
int32_t RecOneFrameSendOneFrame(int32_t siDataLength, uint8_t * pDataToSend,int32_t iLengthReceiveBuffer,uint8_t *pReceiveBuffer,uint8_t Cnt)
{
        int32_t siRet=GD_OK;
        NewBuffer();
        do{
        if( siDataLength<=0 || siDataLength >255 || pDataToSend==NULL )
        {
        siRet=EC_PARAMETER_ERROR;
        break;
        }
        AddCharToBuffer(0x62);
        AddCharToBuffer(0x1f);
        AddCharToBuffer(Cnt);
        AddCharToBuffer( (uint8_t)(siDataLength & 0xff) );
        AddToBuffer(siDataLength,pDataToSend);
        break;
        }while(0);
        if( siRet==GD_OK)
        siRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
        DelBuffer();
        return siRet;
}

//ziyingzhu add 2009-3-12
/****************************************************************
*                                                               *
*   功  能：设置BenzHMF链路保持命令                                    *
*                                                               *
*    参  数：pLinkCommand    链路保持命令缓冲区,如为NULL则停止   *
*                           链路保持                            *
*           ucCommandLength  链路保持命令长度, 如为0则停止链路   *
*                           保持                                *
*           ucLinkCommandReturnLength    链路保持ECU返回字节数   *
*           wLinkKeepTime   链路保持间隔时间,单位毫秒; 取值范   *
*                           围: 大于0, 小于等于2550毫秒         *
*                                                               *
*   返回值：成功    GD_OK                                       *
*           失败    出错代码                                    *
*                                                               *
****************************************************************/
int32_t SetBenzHMFLinkKeep(uint8_t *pLinkCommand,
                int32_t iCommandLength,
                int32_t iLinkCommandReturnLength,
                int32_t iLinkKeepTime)
{
    int32_t iRet=GD_OK;

    if((iCommandLength>255-3)||(iCommandLength<0))
        return EC_PARAMETER_ERROR;
    if((iLinkCommandReturnLength>255)|| (iLinkCommandReturnLength<0))
        return EC_PARAMETER_ERROR;
    if((iLinkKeepTime>2550) || (iLinkKeepTime<0))
        return EC_PARAMETER_ERROR;

    if( (pLinkCommand==NULL) || (iCommandLength==0) ||
        (iLinkCommandReturnLength==0) || (iLinkKeepTime==0) )
    {
        pLinkCommand=NULL;
        iCommandLength=0;
        iLinkCommandReturnLength=0;
        iLinkKeepTime=0;
    }

    NewBuffer();
    AddCharToBuffer(CAC_CONTRAL_COMMAND);
    AddCharToBuffer(CAC_ECU_LINK_KEEP_BENZ_HMF);
    AddCharToBuffer((uint8_t)(iCommandLength&0xFF));
    if(iCommandLength!=0)
        AddToBuffer(iCommandLength,pLinkCommand);
    AddCharToBuffer((uint8_t)(iLinkCommandReturnLength&0xFF));
    AddCharToBuffer((uint8_t)(((iLinkKeepTime+5)/10)&0xFF));

    iRet=DisposeSendAndReceive(NULL,0);
    DelBuffer();

    return iRet;
}

/****************************************************************
*                                                                *
*    功  能：KWP2000协议    发多帧数据到ECU并接收多帧应答 主要处理benz出现的7f问题            *
*                                                                *
*    参  数：ucSendDataTimes    发送次数,可以是CA_ONCE_ONLY 或        *
*                            CA_REPEAT_UNTIL_NEW_COMMAND_BREAK    *
*            pReceiveBuffer    接收缓冲区指针,格式为:接收到的总    *
*                            帧数(1byte),第1帧数据长度(1byte),    *
*                            第1帧数据内容, ... 第n帧数据长度,    *
*                            第n帧数据内容.                        *
*                                                                *
*            iLengthReceiveBuffer接收缓冲区长度                    *
*            iFrameNumber        发送的帧数                        *
*                                                                *
*           后面参数依次为:                                     *
*            发送到ECU的第1帧数据,类型:uint8_t*                        *
*            发送到ECU的第2帧数据,类型:uint8_t*                        *
*                ... ...                                            *
*            发送到ECU的第n帧数据,类型:uint8_t*                        *
*                                                                *
*    返回值：成功    实际接收到的总长度                            *
*            失败    出错代码(小于0)                                 *
*                                                                *
****************************************************************/
int32_t KwpSendDataToEcuGetAnswer_benz(
                uint8_t ucSendDataTimes,
                uint8_t *pReceiveBuffer,
                int32_t iLengthReceiveBuffer,
                int32_t iFrameNumber,
                ...)
{
    va_list args;
    int32_t iTemp,i;
    uint8_t *pTemp;

    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(ucSendDataTimes!=CA_ONCE_ONLY
            && ucSendDataTimes!=CA_REPEAT_UNTIL_NEW_COMMAND_BREAK)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iFrameNumber>255 || iFrameNumber<=0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_KWP2000_SEND_N_FRAME_RECEIVE_N_FRAME_BENZ);
        AddCharToBuffer((uint8_t)(ucSendDataTimes&0xFF));
        AddCharToBuffer((uint8_t)(iFrameNumber&0xFF));

        va_start(args, iFrameNumber);
        for (i=0;i<iFrameNumber;i++){
            pTemp=va_arg(args,uint8_t*);                 //SendCommand
            iTemp=CalculateKwpCommandLength(pTemp); //SendData Length
            if(iTemp>255 || iTemp<=0){
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            AddToBuffer(iTemp,pTemp);
        }
        va_end(args);
        break;
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

/********************************************************************************
*                                                                                *
*    功  能：地址码通信方式(可以设定延时时间)    解决benz版本信息字节位置不对的问题                                   *
*                                                                                *
*    参  数：ucAddressCode        地址码                                            *
*            pReceiveBuffer        接收缓冲区                                        *
*            iLengthReceiveBuffer接收缓冲区长度                                    *
*           fBps                与ECU通信波特率                                 *
*           iTimeKW1Before      延时时间单位ms(0-255ms),0x55与KW1之间延时。     *
*            iTimeSendKW2ToEcu   延时时间单位ms (0-255ms),KW2 取返发回延时。        *
*           iLLineSwitch        发送完地址码后L LINE 是不是关掉,0为关掉.        *
*            bRecognizeBaudRate    需要识别并自动设置波特率                        *
*            bSecondByteReverseReturnToECU    第二个字节是否要求                    *
*                                            取反发回                            *
*            iReceiveFrameNumber    要接收的帧数,取值范围:0~255                     *
*                                (不包括取反发回的字节)                            *
*                                                                               *
*           后面参数依次为:                                                     *
*                                                                               *
*           接收的第1帧数据长度,类型:int32_t                                     *
*           接收的第2帧数据长度,类型:int32_t                                     *
*                   ...   ...                                                   *
*           接收的第iReceiveFrameNumber帧数据长度                                 *
*                                                                                *
*    返回值：成功    返回从ECU接收的版本信息长度,如无版本信息,                    *
*                    返回GD_OK                                                    *
*            失败    出错代码                                                        *
*                                                                                *
*********************************************************************************/
int32_t AddressCodeWayAdjustTime_benz(
                uint8_t ucAddressCode,
                uint8_t *pReceiveBuffer,
                int32_t iLengthReceiveBuffer,
                float fBps,
                int8_t bRecognizeBaudRate,
                uint8_t iTimeKW1Before,
                uint8_t iTimeSendKW2ToEcu,
                int8_t iLLineSwitch,
                int8_t bSecondByteReverseReturnToECU,
                int32_t iReceiveFrameNumber,...
                )
{
    int32_t iRet=GD_OK;

    if(iReceiveFrameNumber<0 || iReceiveFrameNumber>255)
        return EC_PARAMETER_ERROR;

    NewBuffer();
    AddCharToBuffer(CAC_CONTRAL_COMMAND);
    AddCharToBuffer(CAC_ADJUST_TIME_ADDRESS_CODE_WAY_BENZ);

    if(fabs(fBps-5)<0.0001)
    {
        AddCharToBuffer(1); //way 1              5bps
        AddCharToBuffer(ucAddressCode);
        AddCharToBuffer((uint8_t)(iLLineSwitch?0xFF:0x00));
        AddCharToBuffer((uint8_t)(bRecognizeBaudRate?0xFF:0x00));
    }
    else if(fabs(fBps-0xc8)<0.0001)
    {
        AddCharToBuffer(3); //way 3                200bps
        AddCharToBuffer(ucAddressCode);
        AddCharToBuffer((uint8_t)(iLLineSwitch?0xFF:0x00));
        AddCharToBuffer((uint8_t)(bRecognizeBaudRate?0xFF:0x00));
    }
    else
    {
        AddCharToBuffer(2); //way 2                其他
        AddCharToBuffer(ucAddressCode);
        AddCharToBuffer((uint8_t)(iLLineSwitch?0xFF:0x00));
    if(fBps>1000.0)
    {
         AddCharToBuffer((uint8_t)((((int32_t)(65536-18432000/32/fBps))>>8)&0xFF));
         AddCharToBuffer((uint8_t)(((int32_t)(65536-18432000/32/fBps))&0xFF));
    }
    else
    {  //2003.3.26 add
         int32_t iTemp=(int32_t)(fBps+0.5);
         AddCharToBuffer((uint8_t)((iTemp>>8)&0xFF));
         AddCharToBuffer((uint8_t)(iTemp&0xFF));
    }
    }
    AddCharToBuffer((uint8_t)(iTimeKW1Before&0xFF));
    AddCharToBuffer((uint8_t)(iTimeSendKW2ToEcu&0xFF));
    AddCharToBuffer((uint8_t)(bSecondByteReverseReturnToECU?0xFF:0x00));
    AddCharToBuffer((uint8_t)(iReceiveFrameNumber&0xFF));

    if(iReceiveFrameNumber<=0)
    {
            AddCharToBuffer((uint8_t)(0));
    }
    else
    {
        int32_t i,iTemp;
        va_list args;
        va_start(args, iReceiveFrameNumber);
        for(i=0;i<iReceiveFrameNumber;i++)
        {
            iTemp=va_arg(args,int32_t);
            if((iTemp>255) || (iTemp<=0) )
            {
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
        }
        va_end(args);
    }

    if(iRet==GD_OK)
            iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    DelBuffer();

    return iRet;
}

/****************************************************************
*                                                                *
*    功  能：地址码通信方式    解决郭永恒的L线收发问题                                       *
*                                                                *
*    参  数：ucAddressCode        地址码                            *
*            pReceiveBuffer        接收缓冲区                        *
*            iLengthReceiveBuffer接收缓冲区长度                    *
*            iBps                与ECU通信波特率                    *
*            bRecognizeBaudRate    需要识别并自动设置波特率        *
*            bSecondByteReverseReturnToECU    第二个字节是否要求    *
*                                            取反发回            *
*            iReceiveFrameNumber    要接收的帧数,取值范围:0~255        *
*                                (不包括取反发回的字节)            *
*            iLengthEachFrame    接收每帧的长度                    *
*                                                                *
*    返回值：成功    返回从ECU接收的版本信息长度,如无版本信息,    *
*                    返回GD_OK                                    *
*            失败    出错代码                                        *
*                                                                *
*   说  明: 建议使用AddressCodeWay()代替本函数                  *
*                                                               *
****************************************************************/
int32_t AddressCodeCommunicationWay_Lline(
                uint8_t ucAddressCode,
                uint8_t *pReceiveBuffer,
                int32_t iLengthReceiveBuffer,
                float fBps,
                int8_t bRecognizeBaudRate,
                int8_t bSecondByteReverseReturnToECU,
                int32_t iReceiveFrameNumber,
                int32_t iLengthEachFrame
                )
{
    int32_t iRet=GD_OK;

    if(iReceiveFrameNumber<0 || iReceiveFrameNumber>255)
        return EC_PARAMETER_ERROR;
    if(iLengthEachFrame<=0 || iLengthEachFrame>255)
        return EC_PARAMETER_ERROR;

    NewBuffer();
    AddCharToBuffer(CAC_CONTRAL_COMMAND);
    AddCharToBuffer(CAC_ECU_ADDRESS_CODE_WAY_LLINE_ENABLE);
    if(fabs(fBps-5)<0.0001){
        AddCharToBuffer(1); //way 1
        AddCharToBuffer(ucAddressCode);
        AddCharToBuffer((uint8_t)(bRecognizeBaudRate?0xFF:0x00));
        AddCharToBuffer((uint8_t)(bSecondByteReverseReturnToECU?0xFF:0x00));
        AddCharToBuffer((uint8_t)(iReceiveFrameNumber&0xff));
        AddCharToBuffer((uint8_t)(iLengthEachFrame&0xff));
    }
    else
    {
        AddCharToBuffer(2); //way 2
        AddCharToBuffer(ucAddressCode);
        if(fBps>1000.0)
        {
            AddCharToBuffer((uint8_t)(+(((int32_t)(65536-18432000/32/fBps))>>8)&0xFF));
            AddCharToBuffer((uint8_t)(((int32_t)(65536-18432000/32/fBps))&0xFF));
        }
        else
        {      //2003.3.26 add
            int32_t iTemp=(int32_t)(fBps+0.5);
            AddCharToBuffer((uint8_t)((iTemp>>8)&0xFF));
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
        }

        AddCharToBuffer((uint8_t)(bSecondByteReverseReturnToECU?0xFF:0x00));
        AddCharToBuffer((uint8_t)(iReceiveFrameNumber&0xFF));
        AddCharToBuffer((uint8_t)(iLengthEachFrame&0xff));
    }

    iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    DelBuffer();

    return iRet;
}

/****************************************************************
*                                                                *
*    功  能：KWP2000协议 发一帧数据到ECU并接收应答    主要处理benz出现的7f问题            *
*            (直到ECU不发才结束)                                    *
*                                                                *
*    参  数：pToEcuData            发送到ECU数据                    *
*            pReceiveBuffer        接收缓冲区指针                    *
*            iLengthReceiveBuffer接收缓冲区长度                    *
*            iMaximumWaitTime    最大等待时间(单位:毫秒)            *
*                                                                *
*    返回值：成功    实际接收到的总长度                            *
*            失败    出错代码(小于0)                                 *
*                                                                *
****************************************************************/
int32_t KwpSendDataToEcuGetMultiFrameAnswer_benz(
                                uint8_t *pToEcuData,
                                uint8_t *pReceiveBuffer,
                                int32_t iLengthReceiveBuffer,
                                int32_t iMaximumWaitTime)
{
    int32_t iRet=GD_OK;
    int32_t iLengthSendToEcuData=CalculateKwpCommandLength(pToEcuData);

    NewBuffer();
    do{
        if(iLengthSendToEcuData<=0 || iLengthSendToEcuData>255){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer==0)
            pReceiveBuffer=NULL;
        if(iMaximumWaitTime>255*10 && iMaximumWaitTime<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_KWP2000__SEND_1_FRAME_RECEIVE_N_FRAME_BENZ);
        AddCharToBuffer((uint8_t)(iLengthSendToEcuData&0xFF));
        AddToBuffer(iLengthSendToEcuData,pToEcuData);
        AddCharToBuffer((uint8_t)(((iMaximumWaitTime+5)/10)&0xFF));
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

/****************************************************************
*                                                                *
*    功  能:发一帧数据到ECU并接收应答                *
*            (直到超时才结束)                                    *
*                                                                *
*    参  数：pToEcuData            发送到ECU数据                    *
*            pReceiveBuffer        接收缓冲区指针                    *
*            iLengthReceiveBuffer接收缓冲区长度                    *
*            iMaximumWaitTime    最大等待时间(单位:毫秒)            *
*                                                                *
*    返回值：成功    实际接收到的总长度                            *
*            失败    出错代码(小于0)                                 *
*                                                                *
****************************************************************/
//ziyingzhu 增加，发一帧收n个字节，直到超时为止2009-8-31 15:06
int32_t SendOneFrameToEcuGetMultiFrameAnswer(
                                uint8_t *pToEcuData,
                                int32_t iLengthSendToEcuData,
                                uint8_t *pReceiveBuffer,
                                int32_t iLengthReceiveBuffer,
                                int32_t iLengthReceiveWant,
                                int32_t iMaximumWaitTime)
{
    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(iLengthSendToEcuData<=0 || iLengthSendToEcuData>255){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iLengthReceiveBuffer==0)
            pReceiveBuffer=NULL;
        if(iMaximumWaitTime>255*10 && iMaximumWaitTime<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(SYS_SEND_ONE_FRAME_REC_N_BYTES_UNTILL_TIMEOVER);
        AddCharToBuffer((uint8_t)(iLengthSendToEcuData&0xFF));
        AddToBuffer(iLengthSendToEcuData,pToEcuData);

        AddCharToBuffer((uint8_t)((iLengthReceiveWant>>8)&0xFF));
        AddCharToBuffer((uint8_t)(iLengthReceiveWant&0xFF));

        AddCharToBuffer((uint8_t)(((iMaximumWaitTime+5)/10)&0xFF));

    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

/********************************************
pDataToSend: [打包总数] [第1包帧数] [第1包数据] [第2包帧数] [第2包数据]
pRecBuffer: [收到总数] [第1包长度(2个字节)] [第1包数据] [第2包长度(2个字节)] [第2包数据]
*********************************************/
int32_t CumminsSAE1939_SendMultiFramesToEcuGetAnswer(uint8_t ucSendDataTimes,int32_t iSendDataLength, uint8_t *pDataToSend, uint8_t *pRecBuffer,int32_t iRecBufferLength)
{
 int32_t iRet=GD_OK;
 NewBuffer();
 do{
  if(iSendDataLength<=0 || iSendDataLength>0xffff){
   iRet=EC_PARAMETER_ERROR;
   break;
  }
  if(iRecBufferLength<0){
   iRet=EC_PARAMETER_ERROR;
   break;
  }
  if(iRecBufferLength==0)
   pRecBuffer=NULL;
  AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
  AddCharToBuffer(CUMMINSSAE1939_SEND_MULTIFRAMES_TOECU_GETANSWER);
  AddCharToBuffer((uint8_t)(ucSendDataTimes&0xFF));
  AddToBuffer(iSendDataLength,pDataToSend);
 }while(0);
 if(iRet==GD_OK){
  iRet=DisposeSendAndReceive(pRecBuffer,iRecBufferLength);
 }
 DelBuffer();
 return iRet;
}

/****************************************************************
*                                                                *
*    功  能：KWP2000协议    发多帧数据到ECU并接收多帧应答 主要处理benz出现的7f问题            *
*                                                                *
*    参  数：ucSendDataTimes    发送次数,可以是CA_ONCE_ONLY 或        *
*                            CA_REPEAT_UNTIL_NEW_COMMAND_BREAK    *
*            pReceiveBuffer    接收缓冲区指针,格式为:接收到的总    *
*                            帧数(1byte),第1帧数据长度(1byte),    *
*                            第1帧数据内容, ... 第n帧数据长度,    *
*                            第n帧数据内容.                        *
*                                                                *
*            iLengthReceiveBuffer接收缓冲区长度                    *
*            iFrameNumber        发送的帧数                        *
*                                                                *
*           后面参数依次为:                                     *
*            发送到ECU的第1帧数据,类型:uint8_t*                        *
*            发送到ECU的第2帧数据,类型:uint8_t*                        *
*                ... ...                                            *
*            发送到ECU的第n帧数据,类型:uint8_t*                        *
*                                                                *
*    返回值：成功    实际接收到的总长度                            *
*            失败    出错代码(小于0)                                 *
*                                                                *
****************************************************************/
int32_t KwpSendDataToEcuGetAnswer_BMW(
                uint8_t ucSendDataTimes,
                uint8_t *pReceiveBuffer,
                int32_t iLengthReceiveBuffer,
                int32_t iFrameNumber,
                ...)
{
    va_list args;
    int32_t iTemp,i;
    uint8_t *pTemp;

    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(ucSendDataTimes!=CA_ONCE_ONLY
            && ucSendDataTimes!=CA_REPEAT_UNTIL_NEW_COMMAND_BREAK)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iFrameNumber>255 || iFrameNumber<=0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_KWP2000_SEND_N_FRAME_RECEIVE_N_FRAME_BMW);
        AddCharToBuffer((uint8_t)(ucSendDataTimes&0xFF));
        AddCharToBuffer((uint8_t)(iFrameNumber&0xFF));

        va_start(args, iFrameNumber);
        for (i=0;i<iFrameNumber;i++){
            pTemp=va_arg(args,uint8_t*);                 //SendCommand
            iTemp=CalculateKwpCommandLength_BMW(pTemp); //SendData Length
            if(iTemp>255 || iTemp<=0){
                iRet=EC_PARAMETER_ERROR;
                break;
            }
            AddCharToBuffer((uint8_t)(iTemp&0xFF));
            AddToBuffer(iTemp,pTemp);
        }
        va_end(args);
        break;
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pReceiveBuffer,iLengthReceiveBuffer);
    }
    DelBuffer();

    return iRet;
}

//ziyingzhu 增加 2009-12-23 11:27 专门给J1939使用，下位机来完成读码过滤等功能
int32_t SAE1939Function(uint8_t *pRecBuffer,int32_t iRecBufferLength,uint8_t Mode)
{
    int32_t iRet=GD_OK;
    NewBuffer();
    do
    {
        if(iRecBufferLength<0)
        {
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength==0)
            pRecBuffer=NULL;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE);   //0x61
        AddCharToBuffer(CAC_J1939FUNCTION);    //0x29
        AddCharToBuffer(Mode);
    }while(0);
    if(iRet==GD_OK)
    {
      iRet=DisposeSendAndReceive(pRecBuffer,iRecBufferLength);
    }
    DelBuffer();
    return iRet;
}

/****************************************************************
*                            *
*    功  能：测试DTS线                     *
*                            *
*    参数：DTSPin    选择DTS线引脚                   *
*                            *
*          pReceiveBuffer    接收缓冲区指针          *
*                            *
*             iLengthReceiveBuffer接收缓冲区长度        *
*                            *
*    返回值：成功 1                            *
*        失败    0                   *
*                                                       *
*                            *
****************************************************************/
//yaoming add 20100715 GX3/DIAGUN 用于IO测试的DTS线测试
int32_t TestDTS(uint8_t DTSPin,uint8_t *pRecBuffer,int32_t iRecBufferLength)
{
    int32_t iRet=GD_OK;
    //uint8_t bb[20];

    NewBuffer();
    do{
        AddCharToBuffer(CAC_CONTRAL_COMMAND);
        AddCharToBuffer(CAC_DTSTEST);
        AddCharToBuffer(DTSPin);

    }while(0);
    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pRecBuffer,iRecBufferLength);
    }


    DelBuffer();

    return iRet;
}
/***********************************************************************
 *    函数名:         CanSendMulFrameGetMoreSingleFrame()
 *    功能:        所有车通用一个函数据：发多帧收多帧,也可收取多个单帧
 *                发送帧数由pDataToSend第1个字节决定
 *              接收字节由pDataToSned第2个字节决定，<=255字节，可以由PC发下来指定字节决定，大于
 *              255个字节则必由下位机决定，由下位机定接收字节数可以将此字节设成0XFFH。当由下位机决定字节个数时，
 *                接收帧数从接收到的数据判断,直到接收到最后一帧不是多帧开始帧为止。
 *              遇到最后一帧是多帧开始帧时会自动发送0x30流控制帧激发连续帧。
 *                具体可以参见Toyota CAN BUS协议中多帧格式
 *    参数:        iSendDataLength      ---     所要发送数据的长度.
 *                pDataToSend            ---        要发送的数据.(第一个字节表示发送的帧数，第二个字节表示接收的字节数，
                                            第三个字节表示ID个数)发送数不带55+AA CS发送，55+AA CS由下位机发送。
 *                pRecBuffer            ---        接收数据缓冲.前两个字节表示接收到的帧数
 *                iRecBufferLength    ---        接收缓冲区长度.
                iRecFrameNumber     ---     接收单帧的数目
 *  说明：      当ECU回复10H时，此函数要与SetEnterFrame配合使用，发送ECU 30H确认帧。
 *
 *    返回值:        >  0         ---        接收的总帧数.
 *                <  0        ---        通讯出错.
 ***********************************************************************/
int32_t    CanSendMulFrameGetMoreSingleFrame(int32_t iSendDataLength, uint8_t *pDataToSend, uint8_t *pRecBuffer,int32_t iRecBufferLength,int32_t iRecFrameNumber)
{
    int32_t iRet=GD_OK;

    NewBuffer();
    do{
        if(iSendDataLength<=0 || iSendDataLength>255){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iSendDataLength<=0) break;
        if(iRecBufferLength<0){
            iRet=EC_PARAMETER_ERROR;
            break;
        }
        if(iRecBufferLength==0)
            pRecBuffer=NULL;

        AddCharToBuffer(CAC_DATA_SEND_RECEIVE_1);
        AddCharToBuffer(CAC_CAN_SEND_MULFRAME_GET_MULFRAME_MOREFRAME);
        AddCharToBuffer(iRecFrameNumber);
        AddToBuffer((int32_t)iSendDataLength,pDataToSend);
    //    AddCharToBuffer((uint8_t)(((iMaxWaitTime+5)/10)&0xFF));
    }while(0);

    if(iRet==GD_OK){
        iRet=DisposeSendAndReceive(pRecBuffer,(int32_t)iRecBufferLength);
    }
    DelBuffer();

    return iRet;
}

//------------------------------------------------------------------------------------------------
// Funtion: 设置 CAN 请求帧间距
// Input: Waittime - 请求帧间距
// Output: none
// Return: ID_FOK
// Other: 6020
//------------------------------------------------------------------------------------------------
uint8_t SetCanRequestFrametime( uint8_t Waittime )
{
    int32_t iRet=GD_OK;
    NewBuffer();
    AddCharToBuffer( CAC_CONTRAL_COMMAND );
    AddCharToBuffer( CAC_SETCANFRAMETIME );
    AddCharToBuffer( Waittime );
       iRet = DisposeSendAndReceive( NULL, 0 );
    DelBuffer();
    return iRet;
}

/***************************************************************
* 设置ISO 15765 CAN多帧通讯的30H确认帧
* 在初始化时设置即可，无需在每次发命令前设置
*
* 返回值：正确：TRUE，错误：FALSE
*
* 举例：设置30H确认帧：88 C6 D0 07 D0 30 00 00 00 00 00 00 00
*       注意要把ID放在高位
*       uint8_t pEnterFrameCommand[]={0x88,0xC6,0xD0,0x07,0xD0,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
*       MySetEnterFrame(pEnterFrameCommand);
***************************************************************/
int32_t CanSetEnterFrame(uint8_t ucCan30ConfigMode, uint8_t *pEnterFrameCommand)
{
	uint8_t command[19]={0x55,0xaa,0x0d,0x61,0x01};

	if(pEnterFrameCommand[0]&0x80)//扩展帧
		command[2] = 7 + (pEnterFrameCommand[0]&0x0f);
	else//标准帧
		command[2] = 5 + (pEnterFrameCommand[0]&0x0f);

	memcpy(command+5, pEnterFrameCommand, command[2]-2);
	command[command[2]+3] = CheckXOR(command+2, command[2]+1);//异或校验
	if(SetEnterFrameExt(ucCan30ConfigMode, command, command[2]+4)!=GD_OK)
		return FALSE;
	return TRUE;
}

int32_t CanSendAndRecv(int8_t isExFrame,uint8_t *canID, int32_t fillByte, uint8_t *pReqBuffer, uint8_t *pAnsBuffer,int32_t ansBuffLen)
{
	int32_t i = 0;
	uint8_t toSendBuff[0xFF*14+3];
	int32_t iSendDataLength = 0;
	uint8_t toRecvBuff[0xFF*14+3];
	int32_t iReqCount = (pReqBuffer[0]<<8) + pReqBuffer[1];
	int32_t iAnsCount = 0;

	if(iReqCount>(0xFF*7-1) || iReqCount==0)
		return -2;//目前超过255帧下位机不支持,下位机修改后，此处可修改

  if(fillByte != (int32_t)(-1))//填充00或FF
		memset(toSendBuff,fillByte,sizeof(toSendBuff));
	memset(toRecvBuff,0x00,sizeof(toRecvBuff));
	toSendBuff[0] = (iReqCount<8)? 1:((iReqCount+7)/7);//发送帧数

	if(isExFrame)//扩展帧
	{
		memcpy(toSendBuff+1, "\xFF\x04", 2);
		if(toSendBuff[0]==1)//发单帧
		{
			memcpy(toSendBuff+3, "\x0F\x61\x01\x88", 4);//用61 01发送
			memcpy(toSendBuff+7, canID, 4);
			memcpy(toSendBuff+11, pReqBuffer+1, iReqCount+1);
			if(fillByte == (int32_t)(-1))//不填充
				toSendBuff[6] = 0x80 + iReqCount + 1;
		}
		else//发多帧
		{
			for(i=0; i<toSendBuff[0]; i++)
			{
				if((i==0) || (i==toSendBuff[0]-1))
					memcpy(toSendBuff+3+i*16,"\x0F\x61\x01\x88",4);//第一帧和最后一帧用61 01发送
				else
					memcpy(toSendBuff+3+i*16,"\x0F\x61\x03\x88",4);//中间帧用61 03发送
				memcpy(toSendBuff+7+i*16, canID, 4);
				if(i==0)
					toSendBuff[11] = 0x10|pReqBuffer[0];//第一帧是1x帧
				else
					toSendBuff[11+16*i] = 0x20|(i&0x0F);//后续帧是2x帧

				if (i==(toSendBuff[0]-1))//最后一帧
				{
					memcpy(toSendBuff+12+16*i, pReqBuffer+1+7*i, iReqCount-i*7+1);
					if(fillByte == (int32_t)(-1))//不填充
						toSendBuff[6+16*i] = 0x80 + (iReqCount-i*7+1) + 1;
				}
				else
					memcpy(toSendBuff+12+16*i, pReqBuffer+1+7*i, 7);
			}
		}
		iSendDataLength = 3 + 16*toSendBuff[0];
	}
	else//标准帧
	{
		memcpy(toSendBuff+1, "\xFF\x02", 2);
		if(toSendBuff[0]==1)//发单帧
		{
			memcpy(toSendBuff+3, "\x0D\x61\x01\x08", 4);//用61 01发送
			memcpy(toSendBuff+7, canID, 2);
			memcpy(toSendBuff+9, pReqBuffer+1, iReqCount+1);
			if(fillByte == (int32_t)(-1))//不填充
				toSendBuff[6] = iReqCount + 1;
		}
		else//发多帧
		{
			for(i=0; i<toSendBuff[0]; i++)
			{
				if((i==0) || (i==toSendBuff[0]-1))
					memcpy(toSendBuff+3+i*14,"\x0D\x61\x01\x08",4);//第一帧和最后一帧用61 01发送
				else
					memcpy(toSendBuff+3+i*14,"\x0D\x61\x03\x08",4);//中间帧用61 03发送

				memcpy(toSendBuff+7+i*14, canID, 2);
				if(i==0)
					toSendBuff[9] = 0x10|pReqBuffer[0];//第一帧是1x帧
				else
					toSendBuff[9+14*i] = 0x20|(i&0x0F);//后续帧是2x帧

				if (i==toSendBuff[0]-1)//最后一帧
				{
					memcpy(toSendBuff+10+14*i, pReqBuffer+1+7*i,iReqCount-i*7+1);
					if(fillByte == (int32_t)(-1))//不填充
						toSendBuff[6+i*14] = (iReqCount-i*7+1) + 1;
				}
				else
					memcpy(toSendBuff+10+14*i, pReqBuffer+1+7*i,7);
			}
		}
		iSendDataLength = 3 + 14*toSendBuff[0];
	}

	if(CanSendMulFrameGetMulFrameGN(iSendDataLength,toSendBuff,toRecvBuff,sizeof(toRecvBuff))<0)
		return -1;

	if(isExFrame)//扩展帧
	{
		if(((toRecvBuff[0]<<8)+toRecvBuff[1]) == 1)//收单帧
		{
			pAnsBuffer[0] = 0;
			pAnsBuffer[1] = toRecvBuff[10];
			iAnsCount = (pAnsBuffer[0]<<8) + pAnsBuffer[1];
			if(iAnsCount > ansBuffLen)
				iAnsCount = ansBuffLen;
			memcpy(pAnsBuffer+2, toRecvBuff+11, iAnsCount);
		}
		else//收多帧
		{
			pAnsBuffer[0] = toRecvBuff[10] & 0x0f;
			pAnsBuffer[1] = toRecvBuff[11];
			iAnsCount = (pAnsBuffer[0]<<8) + pAnsBuffer[1];
			if(iAnsCount > ansBuffLen)
				iAnsCount = ansBuffLen;
			memcpy(pAnsBuffer+2, toRecvBuff+12, iAnsCount);
		}
	}
	else//标准帧
	{
		if(((toRecvBuff[0]<<8)+toRecvBuff[1]) == 1)//收单帧
		{
			pAnsBuffer[0] = 0;
			pAnsBuffer[1] = toRecvBuff[8];
			iAnsCount = (pAnsBuffer[0]<<8) + pAnsBuffer[1];
			if(iAnsCount > ansBuffLen)
				iAnsCount = ansBuffLen;
			memcpy(pAnsBuffer+2, toRecvBuff+9, iAnsCount);
		}
		else//收多帧
		{
			pAnsBuffer[0] = toRecvBuff[8] & 0x0f;
			pAnsBuffer[1] = toRecvBuff[9];
			iAnsCount = (pAnsBuffer[0]<<8) + pAnsBuffer[1];
			if(iAnsCount > (ansBuffLen-2))
				iAnsCount = (ansBuffLen-2);
			memcpy(pAnsBuffer+2, toRecvBuff+10, iAnsCount);
		}
	}

	return iAnsCount;
}

/******************************************************************************************************************
*                                                                                                                 *
*   功  能：设置链路保持命令                                                                                      *
*
*	参  数：  ucCan30ConfigMode 模式：                                                                              *
*            CAN_30_CONFIG_NORMAL = 0->与SetEnterFrame设置相同, 只发一帧30命令就接收21开始的多帧                  *
*            CAN_30_CONFIG_BS     = 1->根据30后第一个字节(BS:Block Size)来计算发一帧30H接收多少帧数据             *
*                                      (如雷诺的30命令为30 01就是发一帧30命令接收一帧数据,如为30 00就与0模式相同) *
*            CAN_30_CONFIG_ST     = 2->发多帧命令时根据读到的30命令后第二个字节(ST:Separation Time)               *
*                                       来计算发下一帧之前需要延时的时间(ms)                                      *
*                                       (如接收到的30命令为30 00 0A, 就是在发下一帧(21)之前需要延时10ms           *
*           注：CAN_30_CONFIG_BS | CAN_30_CONFIG_ST = 3 可以同时设置block size 和 Separation Time
*
*           pEnterFrameCommand  发送给ECU（10H）30H确认帧，带55+AA+LEN+DATA1+DATA2+...+DATAn+CS                   *
*                                                       *
*           ucCommandLength     发送确认帧的长度，长度不包括长度本身，即要发送数的实际长度                        *
*                                                                                                                 *
*   返回值：成功    GD_OK                                                                                         *
*           失败    出错代码                                                                                      *
*
*   liyangcheng add 20110114    0x601F                                                                            *
*******************************************************************************************************************/
int32_t SetEnterFrameExt(uint8_t ucCan30ConfigMode, uint8_t *pEnterFrameCommand,
				int32_t iCommandLength)
{
	int32_t iRet=GD_OK;

	if((ucCan30ConfigMode>255)||(ucCan30ConfigMode<0))
		return EC_PARAMETER_ERROR;
	if((iCommandLength>86-3)||(iCommandLength<0))
		return EC_PARAMETER_ERROR;

	NewBuffer();
	AddCharToBuffer(CAC_CONTRAL_COMMAND);
	AddCharToBuffer(CAN_MULTI_FLAG_30_CONFIG);
	AddCharToBuffer(ucCan30ConfigMode);
	AddCharToBuffer((uint8_t)(iCommandLength&0xFF));
	if(iCommandLength!=0)
		AddToBuffer(iCommandLength,pEnterFrameCommand);
	iRet=DisposeSendAndReceive(NULL,0);
	DelBuffer();

	return iRet;
}
#endif

