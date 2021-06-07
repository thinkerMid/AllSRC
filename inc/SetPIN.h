#ifndef __SET_PIN_H
#define __SET_PIN_H
#define CON_ERROR                               0X7F
#define CON_BASIC                               0X10
#define CON_CANV20                              0X20
#define CON_TOPV30                              0x34
#define CON_Super16V30                          0X30
#define CON_DiaGunV50                           0X50
#define FUNC_CanDouble                          0x03
#define FUNC_CanSingle                          0x02
#define FUNC_IO                                 0x01
#define FUNC_BUSLINE                            0x00
#define FUNC_LLINE                              0x0E
#define FUNC_TDS                                0x0F
#define FUNC_VOLVO_K                            0x10
#define FUNC_DTS                                0x11
#define FUNC_RESET                              0xFF
//yaoming add 20100722 for heavy_duty
#define FUNC_HEAVYDUTY_J1708                    0x12
#define FUNC_HEAVYDUTY_RS232                    0x13
//#define FUNC_HEAVYDUTY_KLINE                  0x14
#define RT_OK                                   0
#define RT_ERROR                                1
#define IO_IN                                   0
#define IO_OUT                                  1
#define TOP_CANBUS                              0x05
#define TOP_COMPOUD_IO                          0x08
#define TOP_BUSLINE                             0x09
#define TOP_DTS                                 0x0A
#define TOP_TDS                                 0x0B
#define TOP_LLINE                               0x0C
#define TOP_VOLVO_K                             0x0D
#define FUNC_ENABLE                             0XFF
#define FUNC_DISABLE                            0X00
#define OBD16_PIN1                              0x01
#define OBD16_PIN2                              0x02
#define OBD16_PIN3                              0x03
#define OBD16_PIN6                              0x06
#define OBD16_PIN7                              0x07
#define OBD16_PIN8                              0x08
#define OBD16_PIN9                              0x09
#define OBD16_PIN10                             0x0A
#define OBD16_PIN11                             0x0B
#define OBD16_PIN12                             0x0C
#define OBD16_PIN13                             0x0D
#define OBD16_PIN14                             0x0E
#define OBD16_PIN15                             0x0F
#define CHANGE_PLUG                             0x88
#define CANSET_FILID                            0X01
#define CANSET_BITRATE                          0X02
#define CANSET_WORKMODE                         0X04
#define CANSET_MASKFILID                        0X06
typedef struct
{
    char IsExFrame;     //帧格式: 扩展帧(0XFF),标准帧(0X00)
    char WorkMode;      //工作模式:只听模式(0XFF),正常工作模式(0X00)
    char BaudRate;      //波特率:1M(0x01),800K(0X02),500K(0X03),250K(0X04),125K(0X05),62.5K(0X06),50K(0X07),25K(0X08),100K(0X09),96K(0X0A),33.3K(0X0B)
    //char FilterId[50];    //设置过滤ID和掩码
    //ziyingzhu 修改 2010-2-4 17:30
    char FilterId[256]; //设置过滤ID和掩码
    char Pin;           //引脚:可自行定义使用
} CAN_COM;
char DiaGun_SetIOLine(char FuncType,unsigned char IsEnable,char InputPin,char OutputPin);
/********************************************************************************************************************************************
* 函数名称: char SelectPIN(char ConnectorType,char FuncType,char IsEnable,char InputPin,char OutputPin)                                     *
* 函数功能: 综合设置CANBUSII,TOPIII,SUPER-16,OBD 16接头引脚功能                                                                             *
* 函数参数: char ConnectorType  接头类型:根据Connector_Identify()函数返回值                                                                 *
*           char FuncType   引脚功能类型,如下九种其一                                                                                       *
*                           FUNC_IO||FUNC_CanDouble||FUNC_CanSingle||FUNC_BUSLINE||FUNC_LLINE||FUNC_TDS||FUNC_VOLVO_K||FUNC_DTS ||FUNC_RESET*
*           char IsEnable   对应功能(DTS,TDS,L线,VOLVO K线,BUS+/-)是否使能: 值为 FUNC_ENABLE 或 FUNC_DISABLE                                *
*           char InputPin   接头接收数据引脚号: 1-15 (电源16,地4,5除外)                                                                     *
*           char OutputPin  接头接收数据引脚号: 1-15 (电源16,地4,5除外)                                                                     *
* 返 回 值:                                                                                                                                 *
*           RT_OK           设置成功                                                                                                        *
*           CHANGE_PLUG     该功能当前接头无法实现,需要更换接头                                                                             *
*           其他--          设置失败                                                                                                        *
* 修改记录: 1. 20080722     JJH                                                                                                             *
*********************************************************************************************************************************************/
char SelectPIN(char ConnectorType,char FuncType,char IsEnable,char InputPin,char OutputPin);
/********************************************************************************************************************************************
* 函数名称: uint8_t CanCmdCheckXor(uint8_t *InputCmd)                                                                                             *
* 函数功能: CAN命令异或校验计算函数.                                                                                                        *
* 函数参数: uint8_t *InputCmd: 命令缓冲区                                                                                                      *
*           命令格式: 0x55,0xaa,0x0b,0x08,0xfc,0x00,0x02,0x1a,0x97,0x00,0x00,0x00,0x00,0x00,0x70                                            *
* 返 回 值: 命令总长度                                                                                                                      *
* 修改记录: 1. 20080722     JJH                                                                                                             *
*********************************************************************************************************************************************/
unsigned char CanCmdCheckXor(unsigned char *InputCmd);
/********************************************************************************************************************************************
* 函数名称: uint8_t CanSetParameter(uint8_t ConnectorType,uint8_t SetType,CAN_COM *CanParam)                                                         *
* 函数功能: 设置CAN通讯参数功能:设置过滤ID(包含设置掩码),设置ECU通讯波特率,设置工作模式                                                     *
*           CanSetParameterALL()函数区别在于单独设置CAN参数                                                                                 *
* 函数参数: uint8_t ConnectorType 接头类型:CON_Super16V30 || CON_TOPV30 || CON_DiaGunV50 ||CON_CANV20                                          *
*           uint8_t SetType       设置类型:CANSET_FILID   || CANSET_BITRATE|| CANSET_WORKMODE                                                  *
*           CAN_COM *CanParam  设置参数结构                                                                                                 *
* 返 回 值: RT_OK 设置成功,其他-设置失败                                                                                                    *
* 修改记录: 1. 20080807     JJH                                                                                                             *
*********************************************************************************************************************************************/
unsigned char CanSetParameter(unsigned char ConnectorType,unsigned char SetType,CAN_COM *CanParam);
/********************************************************************************************************************************************
* 函数名称: uint8_t CanSetParameterNormal(uint8_t ConnectorType,CAN_COM *CanParam)                                                                *
* 函数功能: 设置CAN通讯参数:根据CanParam全部设置完过滤ID(包含掩码),波特率,工作模式                                                          *
* 函数参数: uint8_t ConnectorType 接头类型:CON_Super16V30 || CON_TOPV30 || CON_DiaGunV50 ||CON_CANV20                                          *
*           CAN_COM *CanParam  设置参数结构                                                                                                 *
* 返 回 值: RT_OK 设置成功,其他-设置失败                                                                                                    *
* 修改记录: 1. 20080807     JJH                                                                                                             *
*********************************************************************************************************************************************/
//ziyingzhu 修改2009-9-21 22:32 将波特率的设置放在前面，解决super16出现的掩码设置失败的问题
//工程师如果单独使用掩码设置函数，则要求设置掩码前必须先设置波特率，也就是波特率设置必须在掩码之前！！！因为super16的限制而造成的
unsigned char CanSetParameterNormal(unsigned char ConnectorType,CAN_COM *CanParam);
#endif
