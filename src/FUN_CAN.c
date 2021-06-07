

//------------------------------------------------------------------------------
//  Purpose: 指令函数集 - 总线通讯类（CANBUS）
//  Funtion: 完成所有物理层总线参数设置函数，主要集中与 指令ID 6100 - 6200
//  Dependent:
//  Designer:
//  Date.Ver:
//canbus下位机存在的问题:1.没有设置过滤ID的功能。
//                       2.下位机接收数据的策略有所变化.LPC2368实现的策略是。当
//                         进行发送数据的时候.清空canbus缓冲区里面的内容，canbus
//                         的接受通过中断进行实现.当读取数据的时候，读取缓冲区里
//                         面的内容就可以.
//                       3.canbus的时间参数的设置.在LPC2368里面CANBUS的时间参数
//                       4.要统一一下返回数据的含义。
//                         #define receive_send_ok 0x00
//                         #define send_fail       0x01
//                         #define receive_fail    0x02
//                         #define time_over       0x03//接受或者超时可以上位机
//                                                      多发送几次进行处理。
//                       5.返回给上位机的数据格式是:
//                         数据:0x00 0x帧数1,0x帧数2,长度,内容,长度,内容
//                         设置:0xff,0x00 通讯成功.
//                              0xff,0x01 通讯失败.
//                       6.在lpc2368的下位机里面，6203指令忽略了0x61,0x01,0x61,0x03.等的作用
//                       7.建议：统一发送缓冲区的数据格式，自定义通讯链路层。
//                       8.接受缓冲区的数据格式也应该一定:
//                         目前有两种接受缓冲区的数据格式:
//                            aa:
//                            格式一. 00 03 0B 08 FD 20 FE 87 5F C0 97 6F 90 97
//                            0B 08 FD 20 FD 73 68 3B 3D 97 2C 00 0B 08 FD 20 FC C4 A8
//                            bb:(aa:)
//                            格式二. 00 02 0f 55 AA 0B 08 FD 00 02 7B 01 00 00 00 00 00 xx
//                                    0f 55 AA 0B 08 FD 00 02 7B 01 00 00 00 00 00 xx
//                           格式三.  00 13 55 AA 0B 08 FD 00 10 82 61 00 4C 46
//                                    50 48 34 41 42 43 35 36 39 30 31 38 34 32
//                                    33 FF FF FF FF 2A FF FF FF FF 03 35 FF FF FF FF FF 41 FF
// 整套CANBUS存在的风险是:在GX3为了与X431进行兼容,进行的处理是CANBUS，采用向接头发送数据，也
// 就是0X61,0X01,0X61,0X03 但是在新的下位机里面，忽略了这一点.
//------------------------------------------------------------------------------
#ifndef FUN_CAN_C
#define FUN_CAN_C
#include "FUN_CAN.h"
#include "FUN_Base.h"
extern CMPHY_Relay SC_RelayState; //继电器相关数据参数
extern SC_CMLISO15765para SC_Canpara;
extern SC_CML_ISO14230 SC_TagKWP2000;
extern SC_CML_ISO15765 SC_TagISO15765;
extern SET_LINK_KEEP Keep_Link;
extern unsigned char timer_open_flag;
extern SC_CMLISO15765para CAN_SetPara;
extern CAN_CONFIGPARA PHY_CAN_SetPara;
extern CAN_RXMessage Can_RXM;
extern unsigned char szFOK[4];
extern unsigned char szFNG[4];
extern unsigned char szFBREAK[4];
extern unsigned char szFNOWAIT[4];
extern unsigned char szErrorFrame[7];
extern SC_CMLISO15765ENTERFRAME CanEnterFrame;
char ucCan30ConfigMode; //流控制的模式控制字

//------------------------------------------------------------------------------
// Funtion: 设置 CAN 标准帧流控制帧
// Input  : argc - 参数长度
//          argv - 的数据格式是:
//        : 长度 0x55 0xaa len 0x61 0x01 0x08 .... cs
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : recv: 60,18,
//------------------------------------------------------------------------------
int SetEnterFrameData_1(int argc, unsigned char * argv, unsigned char * ans)
{
    CanEnterFrame.dataLenth = argv[0];

    if (CanEnterFrame.dataLenth == 0) //修改tm
    {
        memmove(ans, szFBREAK, sizeof(szFBREAK));
        return 2;
    }

    memmove(CanEnterFrame.data, argv + 6, argv[3] -2);
    memmove(ans, szFOK, sizeof(szFOK));
    return 2;
}


//------------------------------------------------------------------------------
// Funtion: 设置 CAN 扩展帧流控制帧
// Input  : argc - 参数长度
//          argv - 模式 + len + 0x55 + 0xaa + len + 0x61 + 0x01 + ....cs
//   模式：
//    0->与SetEnterFrame设置相同, 只发一帧30命令就接收21开始的多帧
//    1->根据30后第一个字节(BS:Block Size)来计算发一帧30H接收多少帧数据
//     如雷诺的30命令为30 01就是发一帧30命令接收一帧数据,如为30 00就与0模式相同)
//    2->发多帧命令时根据读到的30命令后第二个字节(ST:Separation Time)
//       来计算发下一帧之前需要延时的时间(ms)
//      (如接收到的30命令为30 00 0A, 就是在发下一帧(21)之前需要延时10ms
//    3 可以同时设置block size 和 Separation Time
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6018
//------------------------------------------------------------------------------
int SetEnterFrameDataExt_1(int argc, unsigned char * argv, unsigned char * ans)
{
    ucCan30ConfigMode = argv[0];
    CanEnterFrame.dataLenth = argv[1];

    if (CanEnterFrame.dataLenth == 0x00)
    {
        PutDataToAns(szFNG, ans, sizeof(szFNG));
        return ID_FNG;
    }
    else
    {
        memmove(CanEnterFrame.data, &argv[7], argv[4]);
        memmove(ans, szFOK, sizeof(szFOK));
    }

    return ID_FOK;
}


//------------------------------------------------------------------------------
// Funtion: 欧宝专用can发多收多
// Input  : argc - 参数长度
//          argv - (0x61 + 0x1f ) + 发送次数 + 帧数 + 接受帧数 + 第一帧内容（
//                 长度（不包括校验字节） + 内容）
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : recv: 60,1f,ff,01,01,08,61,01,04,09,68,21,01,00,00,0b
//                      1) 2) 3) 4) 5) 6) 7) 8) 9) A) B) C) D) E)
//          1) 数据流开关
//          2) 发送帧数
//          3) 接收帧数
//          4) 发送第一帧长度
//          5)6)7)8)9)A)B)C)D)E) 发送数据
//          send: 01,0d,08,fd,00,01,02,03,04,05,06,07,08
//                1) 2) 3) 4) 5) 6) 7) 8) 9) A) B) C) D)
//          1) 接收到帧数
//          2) 第一帧长度
//          3)4)5)6)7)8)9)A)B)C)D) 接收到数据
// 在使用这个函数的时候要注意的是：
// 1.在这里长度的字节要注意一下.
// 2.不支持扩展帧.
//6120
//------------------------------------------------------------------------------
int HoldenCan20SendOneFrameReceiveDatas_1(int argc, unsigned char * argv,
    unsigned char * ans)
{
    struct data_info
    {
        unsigned char   number;
        unsigned char   timers;
        unsigned char   length_pos;
        unsigned char   length;
        unsigned char   data[20];
    } send_info, receive_info;


    unsigned char * ans_head = NULL;                //注意在进行定义指针的时候一定要进行初始化。
    unsigned char specialdata[4] =
    {
        0
    };
    int total_length = 0;

    set_time0Stop();
    ans_head = ans;
    ans = ans + 3;
    send_info.timers = argv[0];
    send_info.number = argv[1];
    receive_info.number = argv[2];
    send_info.length_pos = 3;                       //13 61 01 08 ..........
    ans_head[2] = receive_info.number;              //接受的帧数

    while (1)
    {
        while (send_info.number)
        {
            memmove(send_info.data, argv + send_info.length_pos + 3, 11); //不支持扩展帧。
            CANSendOneFrame_Ext(send_info.data, 2, specialdata); //发送CANBUS数据
            send_info.length_pos += (argv[send_info.length_pos] +2);
            send_info.number--;
            break;
        }

        while (receive_info.number)
        {
            receive_info.length = ReadOneFrameFromCANBuf_Ext(&receive_info.data[1], 2, specialdata, 3, 500);

            if (receive_info.length == false)
            {
                if (timer_open_flag == 1)
                {
                    TimerStart(TIMER0);
                }

                ans_head[0] = 0x00;
                ans_head[1] = 0xff;
                ans_head[2] = 0x02;
                return 2;
            }

            receive_info.data[0] = receive_info.length;
            memmove(ans, receive_info.data, receive_info.length + 1);
            ans += receive_info.length + 1;
            total_length += (receive_info.length + 1); //返回的长度+长度本身字节；
            receive_info.number--;
            break;
        }

        if ((send_info.number == 0) && (receive_info.number == 0))
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            ans_head[0] = 0x00;
            ans_head[1] = 0x00;
            return total_length + 3; //总长度 + 字节位个数
        }
    }
}


//------------------------------------------------------------------------------
// Funtion: Can发多帧收多帧，收帧数未知
// Input  : argc - 参数长度
//          argv - 参数
//          发送缓冲区的数据格式 : 发送帧数 + 接受帧数(0xff由下位机进行发送) +
//          ID个数 + 长度 + 0x61 + 0x01 ....
//
// Output : ans - 回复到上位机的数据 0x01,0x55,0xaa,
// Return : 0x01
// Info   : 6203
//1. 实例   :
//        发送缓冲区: 0x02 0xff 0x02 0x0d 0x61 0x01 0x08 0xid1 0xid2 0x01,0x01,
//                    0x00,0x00,0x00,0x00
//        接受缓冲区:
//        LPC2368接受缓冲区的内容:
//        0x00,帧数,0x55,0xaa,0x0b,data,data....注意数据中不包含0x21,0x22.
//        函数实现的策略和LPC2368的不一样。功能是一样的。
//        这里要注意的是:流控制命令不包含在发送里面
//2. 函数实现的功能：canbus发送多帧并接受多帧
//3. 时间参数的确认要在另外的函数中进行设置
//4. 函数本身是无法确认接收的帧数的。
//   原因是：在进行多帧接受的时候可能会出现0x10 + len ，，，，的情况.
//5. 流控命令由流控命令函数进行设定。
//6. 2012-12-19修改返回长度字节————测试通过。
//7. 修改了参数的长度,
//------------------------------------------------------------------------------
int CANSendAndReceiveUnknownFrame_1(int argc, unsigned char * argv,
    unsigned char * ans)
{
    unsigned int send_number = 0;
    unsigned int send_pos = 0;
    unsigned int send_only = false;
    unsigned char send_data[14] =
    {
        0
    };

    //在发送的数据区有11和14个字节的
    unsigned char receive_number = 0;
    unsigned char receive_data[20] =
    {
        0
    };
    unsigned int receive_length = 0;
    unsigned int id_number = 0;
    unsigned char specialdata[4] =
    {
        0
    };
    unsigned int i = 0;
    unsigned char * ans_head = &ans[0];
    unsigned int send_length = 0;
    unsigned int flow_control_flag = 0;
    unsigned int re_flag = true;
    int total_length = 0;
    int BlockLenth;

    ans = ans + 3;
    id_number = argv[2];
    send_number = argv[0];
    send_length = argv[3] -2;                       //固定的每帧的长度（有校验字节的）
    send_pos = 3;

    //--------关闭链路--------
    set_time0Stop();
    send_only = ((argv[3] == 0x61) && (argv[4] == 0x03)) ? true: false;

    //////////////完成多帧的发送///////////////////////////////
    for (i = 0; i < send_number; i++)
    {
        memset(send_data, 0, sizeof(send_data));
        memmove(send_data, argv + send_pos + 3, send_length);

        //62,03,02,ff,04,0f,61,01,88,fc,00,00,00,10,0b,01,02,03,04,05,06,0f,61,01,88,fc,00,00,00,21,07,08,09,0a,0b,00,00,a2,
        if (CANSendOneFrame_Ext(send_data, id_number, specialdata) == false)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            ans_head[0] = szFBREAK[0];
            ans_head[1] = szFBREAK[1];
            return 0x02;
        }
        else //发送成功
        {
            if ((specialdata[0] &0x30) == 0x10) //说明是发多帧接受一个流控制
            {
                while (1)
                {
                    if (false == ReadOneFrameFromCANBuf_Ext(receive_data, id_number, specialdata, 1, 500))
                    {
                        if (timer_open_flag == 1)
                        {
                            TimerStart(TIMER0);
                        }

                        ans_head[0] = szFBREAK[0];
                        ans_head[1] = szFBREAK[1];
                        return 0x02; //这里的500是根据LPC2368来设置的
                    }

                    if ((specialdata[0] &0x30) == 0x30) //接受到了流控制命令
                    {
                        break;
                    }
                }
            }
        }

        send_pos += argv[send_pos] +1;
    }

    if (send_only == true)
    {
        ans[0] = 0x00;
        ans[1] = 0x00;
        ans[2] = 0x01;
        ans[3] = 0x55;
        ans[4] = 0xaa;
        ans[5] = 0x01;
        ans[6] = 0x00;
        ans[7] = 0x01;
        return 0xff;
    }

    receive_length = ReadOneFrameFromCANBuf_Ext(receive_data, id_number, specialdata, 1, 500); //接受第一帧

    if (receive_length == 0) //第一帧接受失败
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        ans_head[0] = szFBREAK[0];
        ans_head[1] = szFBREAK[1];
        return 0x02;
    }
    else //接受成功
    {
        while (re_flag == true)
        {
            if ((specialdata[0] &0xf0) == 0x10) //多帧命令
            {
                memmove(ans, receive_data, receive_length); //对于单帧来说发送缓冲区和接受缓冲区的长度是一样的. 0x55,0xaa,0xlen,0x08
                total_length += (receive_length);
                BlockLenth = (specialdata[0] &0x0f) +specialdata[1];
                ans += (receive_length);            //0x55,0xaa,0x0b,0x08,0x
                receive_number++;
                flow_control_flag = true;

                while (1)
                {
                    if (flow_control_flag == true) //确认是否要发送流控制命令
                    {
                        if (CANSendOneFrame_Ext(CanEnterFrame.data, id_number, specialdata) == false) //发送流控制命令
                        {
                            if (timer_open_flag == 1)
                            {
                                TimerStart(TIMER0);
                            }

                            ans_head[0] = szFBREAK[0];
                            ans_head[1] = szFBREAK[1];
                            return 0x02;
                        }
                    }

                    receive_length = ReadOneFrameFromCANBuf_Ext(receive_data, id_number, specialdata, 2, 500);

                    if (receive_length == 0)
                    {
                        if (total_length > BlockLenth)
                        {
                            re_flag = false;
                            break;
                        }
                        else
                        {
                            if (timer_open_flag == 1)
                            {
                                TimerStart(TIMER0);
                            }

                            ans_head[0] = szFBREAK[0];
                            ans_head[1] = szFBREAK[1];
                            return 0x02;
                        }
                    }
                    else
                    {
                        if (receive_data[0] < 0x20) //在小于0x20的时候只处理了0x10的情况
                        {
                            if ((receive_data[0] &0xf0) != 0x10)
                            {
                                re_flag = false;
                                break;
                            }
                            else
                            {
                                if (total_length > BlockLenth)
                                {
                                    re_flag = false;
                                    break;
                                }

                                flow_control_flag = true;
                                receive_number++;
                                memmove(ans, receive_data + 4, receive_length - 4);
                                total_length += (receive_length - 4);
                                ans = ans + receive_length - 4;
                            }
                        }
                        else
                        {
                            flow_control_flag = false;
                            receive_number++;
                            memmove(ans, receive_data + 1, receive_length - 1);
                            total_length += (receive_length - 1);
                            ans += receive_length - 1;
                        }
                    }
                }
            }
            else //接受到单帧
            {
                if ((specialdata[1] == 0x7f) && (specialdata[2] == 0x78))
                {
                    receive_length = ReadOneFrameFromCANBuf_Ext(receive_data, id_number, specialdata, 1, 500); //接受第一帧

                    if (receive_length == 0) //第一帧接受失败
                    {
                        if (timer_open_flag == 1)
                        {
                            TimerStart(TIMER0);
                        }

                        ans_head[0] = szFBREAK[0];
                        ans_head[1] = szFBREAK[1];
                        return 0x02;
                    }
                }
                else
                {
                    memmove(ans, receive_data, receive_length);
                    total_length += (receive_length);
                    receive_number++;
                    break;
                }
            }
        }

        ans_head[0] = 0x00;
        ans_head[1] = receive_number / 0x100;
        ans_head[2] = receive_number & 0x00ff;

        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        return total_length + 3;
    }
}


//------------------------------------------------------------------------------
// Funtion: 欧宝专用can发多收多
// Input  : argc - 参数长度
//          argv -  发送帧数 + 接受帧数 + 过滤ID的个数 + 第一帧长度 + 第一帧内容
//                  0x03 0x02 0x02 0x0d 0x61 0x01 0x08 id1 id2 0x10 0x08 0x01 0x02
//                  0x03 0x04 0x05 0x06 0x0d 0x61 0x03 0x08 id1 id2 0x21 0x07 0x08
//                  0x09 0x0a 0x0b 0x0c 0x0d 0x0d 0x61 0x01 0x08 id1 id2 0x22 0x0d
//                  0x0e 0x0f 0x00 0x00 0x00 0x00
// Output : ans - 回复到上位机的数据。
//          0x00,帧数,0x55,0xaa,0x0b,data,data....注意数据中不包含0x21,0x22.
//            00 03 0B 08 FD 20 FE 87 5F C0 97 6F 90 97 0B 08 FD 20 FD 73 68 3B 3D
//          97 2C 00 0B 08 FD 20 FC C4 A8
// Return : 回复到上位机的数据长度
// Info   : 6207
// 注意的问题:在接受多帧的时候没有发送流控制命令
// 1.2012-12-19修改返回的数据包含的是数据长度。
//------------------------------------------------------------------------------
int GMOPEL_CanSendMulFrameGetMulFrame_1(int argc, unsigned char * argv,
    unsigned char * ans)
{
    unsigned int send_number = argv[0];
    unsigned int send_length_pos = 0;
    unsigned int send_length = 0;
    unsigned char send_data[20] =
    {
        0
    };

    //在发送的数据区有11和14个字节的
    unsigned char receive_number = 0;               //下位机接受的帧数
    unsigned char receive_data[20] =
    {
        0
    };
    unsigned int receive_length = 0;
    unsigned int id_number = 0;
    unsigned char specialdata[4] =
    {
        0
    };
    unsigned int i = 0;
    unsigned char * ans_head = NULL;                //注意在进行定义指针的时候一定要进行初始化。
    int total_length = 0;

    ans_head = ans;
    ans = ans + 3;
    id_number = argv[2];
    send_length_pos = 3;
    send_number = argv[0];                          //发送的帧数
    receive_number = argv[1];                       //接受的帧数
    send_length = id_number + 8 + 1;
    set_time0Stop();

    ////////////////////完成多帧的发送///////////////////////////////
    for (i = 0; i < send_number; i++)
    {
        memset(send_data, 0, sizeof(send_data));
        memmove(send_data, argv + send_length_pos + 3, send_length);

        if (CANSendOneFrame_Ext(send_data, id_number, specialdata) == false)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            ans_head[0] = 0xff;
            ans_head[1] = 0x02;
            return 2;
        }
        else //发送成功
        {
            if (specialdata[0] &0x10) //说明是发送多帧
            {
                do
                {
                    ReadOneFrameFromCANBuf_Ext(receive_data, id_number, specialdata, 2, 500); //接受流控制命令
                }
                while(specialdata[0] != 0x30);
            }
        }

        send_length_pos += argv[send_length_pos] +1; //2,1,2,3,1,2,3
    }




	//sub_80ABFB8()//多出一个这个函数 具体功能参考IDA

    /////////////////////////////////////接受///////////////////////////////
    for (i = 0; i < receive_number; i++)
    {
        receive_length = ReadOneFrameFromCANBuf_Ext(&receive_data[1], id_number, specialdata, 3, 500);

        if (receive_length == 0)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            ans_head[0] = 0xff;
            ans_head[1] = 0x02;
            return 2;
        }
        else
        {
            receive_data[0] = receive_length;
            memmove(ans, receive_data, receive_length + 1);
            ans += (receive_length + 1);
            total_length += receive_length + 1;
        }
    }

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    ans_head[0] = 0x00;
    ans_head[1] = receive_number / 0x100;
    ans_head[2] = receive_number & 0x00ff;
    return total_length + 3;
}


//------------------------------------------------------------------------------
// Funtion:
// Input  : argc - 参数长度
//          argv - 数据格式
//          0x62 + 0x05 + 长度 + 内容 + ID高位 + ID 低位
// Output : ans - 回复到上位机的数据。
// Return : 回复到上位机的数据长度
// Info   : 6205
// 注意的问题:在接受多帧的时候没有发送流控制命令。
//Req:05 EA 20 10 00 02 1A 9B
//Ans:01 60 00 B1
//Ans:08 60 00 20 00 30 5a 9b 34 46 30
//Ans:08 60 00 21 39 31 30 35 35 32 53
//Ans:08 60 00 22 20 20 30 30 31 30 10
//Ans:08 60 00 23 00 00 00 02 32 15 ec
//Ans:08 60 00 24 10 d6 32 2e 34 6c 20
//Ans:08 60 00 25 56 36 20 4d 50 49 20
//Ans:08 60 00 26 20 20 20 20 20 20 20
//Ans:02 60 00 17 20
//Req:01 EA 20 B8
//目前实现的功能是:
//1.在接受到0x10的时候发送确认帧.并且不保存接受数据
//2.2012-12-19进行了返回接受数据的修改.
//上位机接受缓冲区的数据格式是:
//10,55,aa,0c,61,01,07,e8,00,10,00,04,31,b8,00,00,1e(校验位),e8,00,60,
//------------------------------------------------------------------------------
int VWCANSendDataToEcuGetMultiFrameAnswer1_1(int argc, unsigned char * argv,
    unsigned char * ans)
{
    unsigned int send_length = 0;
    unsigned char send_data[20] =
    {
        0
    };

    //在发送的数据区有11和14个字节的
    unsigned char receive_number = 0;               //下位机接受的帧数
    unsigned char receive_data[20] =
    {
        0
    };
    unsigned int receive_length = 0;
    unsigned int id_number = 0;
    unsigned char specialdata[4] =
    {
        0
    };
    unsigned char * ans_head = NULL;                //注意在进行定义指针的时候一定要进行初始化。
    unsigned char key_word1 = 0, key_word2 = 0;

    //确认帧
    unsigned char CONFIG_FRAME[] =
    {
        0X01, 0X00, 0X00, 0X00
    };
    unsigned char VW_MULTI_FRM[] =
    {
        0x06, 0x00, 0x00, 0xa1, 0x0f, 0x8a, 0xff, 0x32, 0xff
    };
    unsigned char flag1 = 0xff;
    unsigned char flag2 = 0;
    unsigned char flag3 = 0;
    int total_length = 0;

    ans_head = ans;
    ans = ans + 2;
    send_length = argv[0];                          //发送数据的长度
    id_number = 2;
    CONFIG_FRAME[1] = argv[send_length + 1];
    CONFIG_FRAME[2] = argv[send_length + 2];
    VW_MULTI_FRM[1] = argv[send_length + 1];
    VW_MULTI_FRM[2] = argv[send_length + 2];

    if (send_length == 0)
    {
        ans_head[0] = 0xff;
        ans_head[1] = 0x02;
        return 2;
    }

    set_time0Stop();
    memset(send_data, 0, sizeof(send_data));

    //10,55,aa,0c,61,01,07,e8,00,10,00,04,31,b8,00,00,1e(校验位),e8,00,60,
    memmove(send_data, argv + 6, argv[6] +3);       //保存发送的数据

	//if ( MEMORY[0x2000CF68] )  IDA反汇编出来的代码有一个这样的 lh
		 // Delay_sub_80861A0(MEMORY[0x2000CF68]);
	//else
		  //Delay_sub_80861A0(10);


    //发送一帧的内容
    if (CANSendOneFrame_Ext(send_data, id_number, specialdata) == false)
    {
        TimerStart(TIMER0);
        ans_head[0] = 0xff;
        ans_head[1] = 0x02;
        return 2;
    }

    //接受数据
    receive_length = ReadOneFrameFromCANBuf_Ext(&receive_data[1], id_number, specialdata, 1, 500); //读取确认帧。

    if (receive_length == 0) //读取失败
    {
        TimerStart(TIMER0);
        ans_head[0] = 0xff;
        ans_head[1] = 0x02;
        return 2;
    }

    while (1)
    {
        receive_length = ReadOneFrameFromCANBuf_Ext(&receive_data[1], id_number, specialdata, 2, 1500);

        if (receive_length == 0) //读取失败
        {
            TimerStart(TIMER0);
            ans_head[0] = 0xff;
            ans_head[1] = 0x02;
            return 2;
        }
        else
        {
            key_word1 = receive_data[1];            //00 55 aa len 08 id1 id2
            key_word2 = receive_data[2];            //

            if ((key_word1 & 0x10) == 0x10) //说明发送确认帧后就结束
            {
                flag1 = 0xff;
                CONFIG_FRAME[3] = (((key_word1 + 1) & 0x0f) | 0xb0);

                if (CANSendOneFrame_Ext(CONFIG_FRAME, id_number, specialdata) == false) //发送确认帧
                {
                    TimerStart(TIMER0);
                    ans_head[0] = 0xff;
                    ans_head[1] = 0x02;
                    return 2;
                }

                if ((receive_data[4]!=0x7F) && (receive_data[6]!=0x78))
                {
                    if (flag2 == 0xff) //说明上一帧是0x2x。
                    {
                        TimerStart(TIMER0);
                        receive_number++;
                        receive_data[0] = receive_length;
                        memmove(ans, receive_data, receive_length + 1);
                        total_length += receive_length + 1;
                        ans_head[0] = 0;
                        ans_head[1] = receive_number;
                        return total_length + 2;
                    }
                    else //说明大帧接受没有完毕
                    {
                        if (flag3 == 0xff)
                        {
                            receive_number++;
                            receive_data[0] = receive_length;
                            memmove(ans, receive_data, receive_length + 1);
                            ans += (receive_length + 1);
                            total_length += receive_length + 1;
                            flag3 = 0;
                            continue;
                        }
                        else
                        {
                            if ((key_word2 & 0xf0) == 0x00) //当前帧是结束帧
                            {
                                TimerStart(TIMER0);
                                receive_number++;
                                receive_data[0] = receive_length;
                                memmove(ans, receive_data, receive_length + 1);
                                total_length += receive_length + 1;
                                ans_head[0] = 0;
                                ans_head[1] = receive_number;
                                return total_length + 2;
                            }
                            else
                            {
                                //这里是不保存.
                                receive_number++;
                                receive_data[0] = receive_length;
                                memmove(ans, receive_data, receive_length + 1);
                                ans += (receive_length + 1);
                                total_length += receive_length + 1;
                                continue;
                            }
                        }
                    }
                }
            }
            else if ((key_word1 & 0x20) == 0x20)
            {
                if ((key_word1 & 0x80) == 0x80)
                {
                    if (CANSendOneFrame_Ext(VW_MULTI_FRM, id_number, specialdata) == false) //发送确认帧
                    {
                        TimerStart(TIMER0);
                        ans_head[0] = 0xff;
                        ans_head[1] = 0x02;
                        return 2;
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    flag3 = 0xff;                   //说明接受的是0x2x

                    if (flag1 == 0xff)
                    {
                        flag1 = 0;

                        if ((key_word2 & 0xf0) == 0x00)
                        {
                            flag2 = 0xff;
                        }
                    }

                    receive_number++;
                    receive_data[0] = receive_length;
                    memmove(ans, receive_data, receive_length + 1);
                    ans += (receive_length + 1);
                    total_length += receive_length + 1;
                    continue;
                }
            }
            else
            {
                CONFIG_FRAME[3] = (((key_word1 + 1) & 0x0f) | 0xb0);

                if (CANSendOneFrame_Ext(CONFIG_FRAME, id_number, specialdata) == false) //发送确认帧
                {
                    TimerStart(TIMER0);
                    ans_head[0] = 0xff;
                    ans_head[1] = 0x02;
                    return 2;
                }

                receive_number++;
                receive_data[0] = receive_length;
                memmove(ans, receive_data, receive_length + 1);
                ans += (receive_length + 1);
                total_length += receive_length + 1;
                continue;
            }
        }
    }
}


//------------------------------------------------------------------------------
// Funtion:
// Input  : argc - 参数长度
//          argv - 数据格式
//         (0x62 + 0x01 )  + 0x55 + 0xaa +0x0d + 0x61 +0x01 + 0x08 +.........
//         0x55 + 0xaa +0x0d.......
// 发送缓冲区实例
//         : 55 aa 0d 61 01 08 FC 00 02 21 00 00 00 00 00 00 xx
//           (长度里面不包含校验的长度)
//           55 aa 0d 61 01 08 FC 00 30 00 00 00 00 00 00 00 xx
// Output : ans - 回复到上位机的数据。
//        : 数据格式是->02 0f 55 AA 0B 08 FD 00 02 7B 01 00 00 00 00 00 xx
//                         0f 55 AA 0B 08 FD 00 02 7B 01 00 00 00 00 00 xx
// Return : 0xff(接受成功)
//          0x02(接受失败)
// Info   : 6201
// 注意事项:
// 在这里使用方法是:发一帧收多帧,在函数中包涵流控制命令。在部分老下位机中函数中不包含
// 流控制命令.例如6203.
// 不支持扩展帧。
//------------------------------------------------------------------------------
int Can20SendTwoFrameQuickGetInfo_1(int argc, unsigned char * argv,
    unsigned char * ans)
{
    struct data_info
    {
        unsigned char   number;
        unsigned char   timers;
        unsigned char   length_pos;
        unsigned char   length;
        unsigned char   data[20];
    } send_info, receive_info;


    unsigned char * ans_head = ans;                 //注意在进行定义指针的时候一定要进行初始化。
    unsigned char specialdata[4] =
    {
        0
    };
    unsigned int total_length = 0;

    ans = ans + 3;

    //send_info.length_pos = 5; 2012-12-19//增加扩展帧进行修改
    send_info.length_pos = 2;                       //表示的是长度信息所在的位置.
    receive_info.number = 0;
    set_time0Stop();
    memmove(send_info.data, argv + send_info.length_pos + 3, argv[send_info.length_pos] -2);
    send_info.length_pos += argv[send_info.length_pos] +1 + 3;

    if (CANSendOneFrame_Ext(send_info.data, 2, specialdata) == false) //发送第一帧数据
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        ans_head[0] = 0xff;
        ans_head[1] = 0x02;
        return 2;
    }

    do
    {
        receive_info.length = ReadOneFrameFromCANBuf_Ext(&receive_info.data[1], 2, specialdata, 1, 500); //这里的时间的设置是不是正确的？

        if ((specialdata[1] == 0x7f) && (specialdata[2] == 0x78))
        {
            continue;
        }
        else
        {
            receive_info.data[0] = receive_info.length;
            memmove(ans, receive_info.data, receive_info.length + 1);
            ans += (receive_info.length + 1);
            total_length = receive_info.length + 1;
            receive_info.number++;
            break;
        }
    }
    while(1);

    //memmove(send_info.data, argv+send_info.length_pos, 11);
    memmove(send_info.data, argv + send_info.length_pos + 3, argv[send_info.length_pos] -2);

    if (CANSendOneFrame_Ext(send_info.data, 2, specialdata) == false) //流控制命令
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        ans_head[0] = 0xff;
        ans_head[1] = 0x02;
        return 2;
    }

    while (1)
    {
        receive_info.length = ReadOneFrameFromCANBuf_Ext(&receive_info.data[1], 2, specialdata, 1, 500); //这里的时间的设置是不是正确的？

        if (receive_info.length == false)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            ans_head[0] = 0;
            ans_head[1] = 0;
            ans_head[2] = receive_info.number;
            return total_length + 3;
        }
        else
        {
            receive_info.data[0] = receive_info.length;
            memmove(ans, receive_info.data, receive_info.length + 1);
            ans += (receive_info.length + 1);
            total_length += receive_info.length + 1;
            receive_info.number++;
        }
    }
}


//------------------------------------------------------------------------------
// Funtion:
// Input  : argc - 参数长度
//          argv - 数据格式
//         (0x62 + 0x02 ) 帧数 + 从ECU接受的帧数(当用0XFF表示的时候，就说明是下位机自动判断)
//         + ID的个数支持扩展帧 + 长度 + 61 + 01 + 帧内容。
// 实例   : 01 02 55 aa 0D 61 01 08 FC 00 02 10 81 00 00 00 00 00 (发一帧的数据格式)
//        : 03 02 55 aa 0D 61 01 08 FC 00 10 12 3B 00 4C 46 50 48 0D 61 03 08 FC
//          00 21 35 41 42 43 31 33 39 0D 61 01 08 FC 00 22 FF E3 D6 FF 30 00 00
//          (多帧数据的存放格式)
// Output : ans - 回复到上位机的数据。
//        : 数据格式是->00 01 55 AA 0B 08 FD 00 02 7B 00 00 00 00 00 00(一帧数据格式)
//                    ->00 13 55 AA 0B 08 FD 00 10 82 61 00 4C 46 50 48 34
//                      41 42 43 35 36 39 30 31 38 34 32 33 FF FF FF FF 2A FF FF
//                      FF FF 03 35 FF FF FF FF FF 41 FF
//                      FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
//                      FF FF FF FF FF FF FF FF FF FF
//                      FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
//                      FF FF FF FF FF FF FF FF FF FF
//                      FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
//                      FF FF FF FF FF FF FF FF FF FF FF FF E4 81 FF 30
// Return : 0xff
// Info   : 6202
// 注意事项:
// 1.接受的帧数默认设置为0xff，
// 2.发送完一帧到接受下一帧的时间是设置的是默认的是500(这个地方可能有问题)，
// 3.实现的机制是:首先发送多帧，
// 4.对于收到的多帧中，计数0x20小于0x20时没有进行处理。
// 5.这是一个比较普通的发一帧收多帧,发一帧收多帧,发多帧收多帧.
// 6.发的帧不包括流控制命令。
// 7.下位机不需要设置流控制指令。
//------------------------------------------------------------------------------
int CANSendAndReceiveMultiFrame_1(int argc, unsigned char * argv,
    unsigned char * ans)
{
    unsigned int send_length_pos = 0;
    unsigned char send_number = 0;
    unsigned char i = 0;
    unsigned char send_data[20] =
    {
        0
    };

    //在发送的数据区有11和14个字节的
    unsigned int receive_number = 0;                //下位机接受的帧数
    unsigned char receive_data[20] =
    {
        0
    };

    //
    unsigned int receive_length = 0;
    unsigned int id_number = 0;
    unsigned char specialdata[4] =
    {
        0
    };
    unsigned char * ans_head = NULL;                //注意在进行定义指针的时候一定要进行初始化。
    int total_length = 0;
    unsigned char flow_con[20] =
    {
        0
    };
    char flow_control = 0;

    send_number = argv[0];
    id_number = argv[1];

    if (id_number > 4)
    {
        id_number = 4;
    }

    send_length_pos = 7;
    ans_head = ans;
    ans = ans + 3;

    //流控命令
    // 0x01,0x02,0x55,0xaa,0x0d,0x61,0x03,0x08,0xfc,0x00,0x....
    // 0x55,0xaa,0x0d,0x61,0x03,0x08,0xfc,0x00,0x....
    memmove(flow_con, argv + send_length_pos, (argv[send_length_pos] &0x0f) +id_number + 1); //08 id1 id2
    flow_con[id_number + 1] = 0x30;
    memset(flow_con + id_number + 2, 0, 7);
    set_time0Stop();

    /*---------------------数据发送完毕------------------------------*/
    for (i = 0; i < send_number; i++)
    {
        memmove(send_data, argv + send_length_pos, (argv[send_length_pos] &0x0f) +id_number + 1);

        if (CANSendOneFrame_Ext(send_data, id_number, specialdata) == false) //发送确认帧
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            ans_head[0] = 0xff;
            ans_head[1] = 0x02;
            return 2;
        }

        if ((specialdata[0] &0x30) == 0x10) //说明是发多帧
        {
            while (1)
            {
                if (false == ReadOneFrameFromCANBuf_Ext(receive_data, id_number, specialdata, 1, 500))
                {
                    if (timer_open_flag == 1)
                    {
                        TimerStart(TIMER0);
                    }

                    ans_head[0] = szFBREAK[0];
                    ans_head[1] = szFBREAK[1];
                    return 0x02; //这里的500是根据LPC2368来设置的
                }

                if ((specialdata[0] &0x30) == 0x30) //接收到了流控制命令
                {
                    break;
                }
            }
        } // 8 + 2 + 2 + 2 + 1;

        send_length_pos += (argv[send_length_pos] &0x0f) + 2 + id_number + 2 + 2; //下一帧开始的地方.
    }

    /*-----------------以上是数据发送完毕----------------------------------*/
    while (1)
    {
        /*--------------------接受第一帧数据-------------------------------*/
        receive_length = ReadOneFrameFromCANBuf_Ext(receive_data, id_number, specialdata, 1, 500);

        if (receive_length == false) //这里就是接受失败
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            ans_head[0] = 0xff;
            ans_head[1] = 0x02;
            return 2;
        }

        if ((specialdata[0] &0x10) == 0x10) //说明是接受多帧
        {
            receive_number++;
            memmove(ans, receive_data, receive_length + 1);
            ans += receive_length;
            total_length += receive_length;         //总长度+已经获取的长度。
            flow_control = 1;

            while (1)
            {
                if (flow_control == 1)
                {
                    if (CANSendOneFrame_Ext(flow_con, id_number, specialdata) == false) //流控
                    {
                        if (timer_open_flag == 1)
                        {
                            TimerStart(TIMER0);
                        }

                        ans_head[0] = 0xff;
                        ans_head[1] = 0x02;
                        return 2;
                    }

                    flow_control = 0;
                }

                receive_length = ReadOneFrameFromCANBuf_Ext(&receive_data[0], id_number, specialdata, 2, 500);

                if (receive_length == false)
                {
                    if (receive_number > 0)
                    {
                        if (timer_open_flag == 1)
                        {
                            TimerStart(TIMER0);
                        }

                        ans_head[0] = 0;
                        ans_head[1] = (receive_number / 0x100) & 0xff;
                        ans_head[2] = receive_number & 0xff;
                        return total_length + 3;
                    }
                    else
                    {
                        if (timer_open_flag == 1)
                        {
                            TimerStart(TIMER0);
                        }

                        ans_head[0] = 0xff;
                        ans_head[1] = 0x02;
                        return 2;
                    }
                }
                else
                {
                    if (specialdata[0] < 0x20)
                    {
                        if (specialdata[0] == 0x10)
                        {
                            flow_control = 1;
                            receive_number++;
                            memmove(ans, receive_data + 4, receive_length - 4);
                            ans += receive_length - 4;
                            total_length += receive_length - 4;
                        }
                    }
                    else
                    {
                        receive_number++;
                        memmove(ans, receive_data + 1, receive_length - 1);
                        ans += receive_length - 1;
                        total_length += receive_length - 1;
                    }
                }
            }
        }
        else //接受一帧
        {
            if ((specialdata[1] == 0x7f) && (specialdata[2] == 0x78)) //说明是繁忙帧
            {
                continue;
            }
            else
            {
                if (timer_open_flag == 1)
                {
                    TimerStart(TIMER0);
                }

                receive_number++;
                memmove(ans, receive_data, receive_length);
                total_length += receive_length;
                ans_head[0] = 0;
                ans_head[1] = 0;                    //receive_number/0x100;
                ans_head[2] = receive_number & 0xff;
                return total_length + 3;
            }
        }
    }
}


//------------------------------------------------------------------------------
// Funtion:
// Input  : argc - 参数长度
//          argv - 数据格式
//          (0x62 0x13) 总长度, 0x55, 0xaa, 长度 0x61 0x01 data........
//        : 注意这里的总长度是什么意思？
// 实例   : 0x0f 0x55 0xaa 0x0d 0x61 0x01 0x08 0xfc 0x00 0x02 0x62 0x013 0x00
//          0x00 0x00 0xcs
// Output : ans 的数据格式是：
//        : 0x00,0x00,0x帧数,0x55,0xaa,0x0d,0x08,0xfc,0x00,0x02,....
//        : 实例:0x00,0x00,0x02,0x55,0xaa,0x0d,0x08,0xfc,0x00,0x10,0x08,0x01,0x02,
//               0x03,0x04,0x05,0x06,0x07,0x08
// Return : 0xff
// Info   : 6213
//          send: 62,13,01,55,aa,0d,61,01,08,fc,00,02,a2,06,00,00,00,00,00,9a,
//          recv: 00,00,04,55,aa,0b,08,fd,00,10,14,e2,06,47,52,58,39,23,20,20,
//                31,4e,5a,46,45,20,04,02,41,6b,01,47,52,58,39,
// 函数没有进行测试。
//　实现的是丰田a2canbus的发一帧收多帧。
//  在这里主要的问题是:进行接收多帧的时候，如果下位机的数据起始位是小于0X20应该怎么处理.
//  在这里只支持标准帧。
//  没有进行老下位机和新下位机的测试。
//------------------------------------------------------------------------------
int ToyotaA2ModeCanMultiframe_1(int argc, unsigned char * argv,
    unsigned char * ans)
{
    struct data_info
    {
        unsigned char   number;
        unsigned char   timers;
        unsigned char   length_pos;
        unsigned char   length;
        unsigned char   data[20];
    } send_info;


    unsigned char receive_number = 0;
    unsigned char receive_data[20] =
    {
        0
    };
    unsigned int receive_length = 0;
    unsigned int id_number = 0;
    unsigned char specialdata[4] =
    {
        0
    };
    unsigned char * ans_head = &ans[0];
    unsigned int flow_control_flag = 0;
    unsigned int re_flag = true;
    int Blocknum;
    unsigned char flow_contral[11] =
    {
        0x08, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    int total_length = 0;

    ans = ans + 3;

    //receive_info.number = 0;
    set_time0Stop();
    memmove(send_info.data, argv + 6, 11);          //只发送一帧.
    flow_contral[1] = send_info.data[1];
    flow_contral[2] = send_info.data[2];

    if (CANSendOneFrame_Ext(send_info.data, 2, specialdata) == false) //
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        ans_head[0] = 0xff;
        ans_head[1] = 0x02;
        return 2;
    }

    receive_length = ReadOneFrameFromCANBuf_Ext(receive_data, id_number, specialdata, 1, 500); //接受第一帧

    if (receive_length == 0) //第一帧接受失败
    {
        if (timer_open_flag == 1)
        {
            TimerStart(TIMER0);
        }

        ans_head[0] = szFBREAK[0];
        ans_head[1] = szFBREAK[1];
        re_flag = true;
        return 0x02;
    }

    while (re_flag == true)
    {
        if ((specialdata[0] &0xf0) == 0x10) //多帧命令
        {
            memmove(ans, receive_data, receive_length); //对于单帧来说发送缓冲区和接受缓冲区的长度是一样的. 0x55,0xaa,0xlen,0x08
            total_length += (receive_length);
            Blocknum = (int) (((specialdata[0] &0x0f) +specialdata[1]) / 7);

            if (((specialdata[0] &0x0f) +specialdata[1]) % 7)
            {
                Blocknum++;
            }

            ans += (receive_length);                //0x55,0xaa,0x0b,0x08,0x
            receive_number++;
            flow_control_flag = true;

            while (1)
            {
                if (flow_control_flag == true) //确认是否要发送流控制命令
                {
                    if (CANSendOneFrame_Ext(flow_contral, 2, specialdata) == false) //发送流控制命令
                    {
                        if (timer_open_flag == 1)
                        {
                            TimerStart(TIMER0);
                        }

                        ans_head[0] = szFBREAK[0];
                        ans_head[1] = szFBREAK[1];
                        return 0x02;
                    }
                }

                receive_length = ReadOneFrameFromCANBuf_Ext(receive_data, id_number, specialdata, 2, 500);

                if (receive_length == 0)
                {
                    if (timer_open_flag == 1)
                    {
                        TimerStart(TIMER0);
                    }

                    ans_head[0] = szFBREAK[0];
                    ans_head[1] = szFBREAK[1];
                    return 0x02;
                }
                else
                {
                    if (receive_data[0] < 0x20) //在小于0x20的时候只处理了0x10的情况
                    {
                        if ((receive_data[0] &0xf0) != 0x10)
                        {
                            re_flag = false;
                            break;
                        }
                        else
                        {
                            flow_control_flag = true;
                            receive_number++;
                            memmove(ans, receive_data + 4, receive_length - 4);
                            total_length += (receive_length - 4);
                            ans = ans + receive_length - 4;
                        }
                    }
                    else
                    {
                        flow_control_flag = false;
                        receive_number++;
                        memmove(ans, receive_data + 1, receive_length - 1);
                        total_length += (receive_length - 1);
                        ans += receive_length - 1;
                    }
                }

                if (receive_number >= Blocknum)
                {
                    re_flag = false;
                    break;
                }
            }
        }
        else //接受到单帧
        {
            if ((specialdata[1] == 0x7f) && (specialdata[2] == 0x78))
            {
                receive_length = ReadOneFrameFromCANBuf_Ext(receive_data, id_number, specialdata, 1, 500); //接受第一帧

                if (receive_length == 0) //第一帧接受失败
                {
                    if (timer_open_flag == 1)
                    {
                        TimerStart(TIMER0);
                    }

                    ans_head[0] = szFBREAK[0];
                    ans_head[1] = szFBREAK[1];
                    return 0x02;
                }
            }
            else
            {
                memmove(ans, receive_data, receive_length);
                total_length += (receive_length);
                receive_number++;
                break;
            }
        }
    }

    ans_head[0] = 0x00;
    ans_head[1] = receive_number / 0x100;
    ans_head[2] = receive_number & 0x00ff;

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    return total_length + 3;
}


//------------------------------------------------------------------------------
// Funtion : CAN_RecieveMultiFrame_Porsche
// Input  : argc - 参数长度
//             argv - 数据格式
//             (0x62 0x18) + 发送帧数 +
//          过滤ID1 + 过滤id2 + 总长度 + 0x55 0xaa  长度  0x61 0x01 data...cs.
//          总长度 + 0x55 0xaa 长度   0x61 0x01 长度 + id1 + id2 + 0XXX........
//        : 注意这里的总长度是什么意思？
// 实例   : 0x0f 0x55 0xaa 0x0d 0x61 0x01 0x08 0xfc 0x00 0x02 0x62 0x013 0x00 0x00 0x00 0xcs
// Output : ans 的数据格式是：
//        : 0x00,0x00,0x帧数,KWP的数据组成部分
//        : 0x00,0x01,0x83,0xF1,0x58,021,0x01,0x02,0x00;
// Return : 0xff
// Info   : 6218
// 注意的问题:在老下位机里面，canbus有校验位
//------------------------------------------------------------------------------
int CAN_RecieveMultiFrame_Porsche_1(int argc, unsigned char * argv,
    unsigned char * ans)
{
    struct data_info
    {
        unsigned char   number;
        unsigned char   timers;
        unsigned char   length_pos;
        unsigned char   pos;
        unsigned char   length;
        unsigned char   data[20];
    } send_info, receive_info;


    unsigned char send_enter_frame[] =
    {
        0x01, 0x00, 0x00, 0x00
    };

    //当接收的标志位的高位时0,1的时候发送0表示继续接受1表示接受完毕
    unsigned char send_busy_frame[] =
    {
        0x01, 0x00, 0x00, 0x30
    };

    //当接收的标志位是0x30的时候下位机进行发送.
    unsigned char specialdata[4] =
    {
        0
    };
    unsigned char total_length = 0;
    int total_length_porsche = 0;

    set_time0Stop();
    receive_info.pos = 4;
    receive_info.length_pos = 3;
    send_info.number = argv[0];                     //接受的帧数
    send_info.length_pos = 3;                       //长度的位置
    send_enter_frame[1] = argv[1];
    send_busy_frame[1] = argv[1];
    send_enter_frame[2] = argv[2];
    send_busy_frame[2] = argv[2];
    receive_info.number = 0;

    do
    {
        memmove(send_info.data, argv + send_info.length_pos + 6, argv[send_info.length_pos] -6); //长度减去3忽略cs
        CANSendOneFrame_Ext(send_info.data, 2, specialdata);
        argv += (argv[send_info.length_pos] +1);
    }
    while(--send_info.number);

    while (1)
    {
        receive_info.length = ReadOneFrameFromCANBuf_Ext(&receive_info.data[1], 2, specialdata, 3, 500);

        if (receive_info.length == 0)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            ans[0] = 0;
            ans[1] = 0;
            ans[2] = receive_info.number;
            return 0xff;
        }
        else
        {
            receive_info.data[0] = receive_info.length;

            switch (receive_info.data[4] &0x30)
            {
                case 0x00:
                    if (total_length == 0)
                    {
                        memmove(&ans[receive_info.pos], &receive_info.data[1], receive_info.length - 4);
                        total_length += receive_info.length - 4; //每一帧的长度
                        ans[receive_info.length_pos] = total_length;
                        total_length_porsche += receive_info.length - 3; //总长度
                        receive_info.number++;      //总帧数

                        if (timer_open_flag == 1)
                        {
                            TimerStart(TIMER0);
                        }

                        ans[0] = 0;
                        ans[1] = 0;
                        ans[2] = receive_info.number;
                        return total_length_porsche + 3;
                    }
                    else
                    {
                        send_enter_frame[3] = receive_info.data[4] +0xb1;
                        memmove(&ans[receive_info.pos], receive_info.data + 5, receive_info.data[1] -1);
                        total_length += receive_info.data[1] -1;
                        total_length_porsche += receive_info.data[1] -1;
                        receive_info.pos += (receive_info.data[1] -1);
                        delay(50);
                        CANSendOneFrame_Ext(send_enter_frame, 2, specialdata); //发送确认帧
                    }

                    break;

                case 0x10: //大帧发送结束.X431向ECU发送确认帧
                    send_enter_frame[3] = (receive_info.data[4] &0x0f) + 0xb1;
                    memmove(&ans[receive_info.pos], receive_info.data + 5, receive_info.data[1] -1);
                    total_length += receive_info.data[1] -1;
                    total_length_porsche += receive_info.data[1] -1;
                    receive_info.number++;
                    ans[receive_info.length_pos] = total_length;
                    delay(50);
                    CANSendOneFrame_Ext(send_enter_frame, 2, specialdata); //发送确认帧

                    if (timer_open_flag == 1)
                    {
                        TimerStart(TIMER0);
                    }

                    ans[0] = 0;
                    ans[1] = 0;
                    ans[2] = receive_info.number;
                    return total_length_porsche + 3 + receive_info.number;

                case 0x20: //大帧继续发送,但是不要发送确认帧。
                    if ((receive_info.data[4] &0xf0) == 0xa0)
                    {
                        memmove(&ans[receive_info.pos], &receive_info.data[1], receive_info.data[1] -1);
                        total_length += receive_info.data[1] -1;
                        total_length_porsche += receive_info.data[1] -1;
                        receive_info.number++;
                        ans[receive_info.length_pos] = total_length;

                        if (timer_open_flag == 1)
                        {
                            TimerStart(TIMER0);
                        }

                        ans[0] = 0;
                        ans[1] = 0;
                        ans[2] = receive_info.number;
                        return total_length_porsche + 3 + receive_info.number;
                    }
                    else
                    {
                        memmove(&ans[receive_info.pos], receive_info.data + 5, receive_info.data[1] -1);
                        total_length += receive_info.data[1] -1;
                        receive_info.pos += receive_info.data[1] -1;
                        total_length_porsche += receive_info.data[1] -1;
                    }

                    break;

                case 0x30:
                    if ((receive_info.data[8] == 0x7f) && (receive_info.data[10] == 0x78))
                    {
                        //delay( 500);
                        CANSendOneFrame_Ext(send_busy_frame, 2, specialdata); //发送确认帧
                        break;                      //继续接受
                    }
                    else
                    {
                        memmove(&ans[receive_info.pos], receive_info.data + 5, receive_info.data[1] -1);
                        total_length += receive_info.data[1] -1; //每一帧的长度
                        ans[receive_info.length_pos] = total_length; //
                        total_length_porsche += receive_info.data[1] -1;
                        receive_info.pos += receive_info.data[1];
                        receive_info.length_pos += (total_length + 1);
                        total_length = 0;
                        receive_info.number++;

                        //delay( 500);
                        CANSendOneFrame_Ext(send_busy_frame, 2, specialdata); //发送确认帧
                    }

                    break;

                default:
                    break;
            }
        }
    }
}


//------------------------------------------------------------------------------
// Funtion : CANSendAndReceiveUnknownFrame_benz
// Input   : argc - 参数长度
//           argv - 数据格式
//           发送缓冲区(argv)的数据格式->0x62|0x25|发送的帧数|接受的数据长度|
//           发送的ID个数|发送长度(不包括校验位)|0x61|0x01|canbus数据。
//           发送缓冲区的数据举例:
//           0x02 0xff 0x02 0x0d 0x61 0x01 0x08 0xfd 0x00 0x10 0x08 0x21 0x01 0x02 0x03 0x04 0x05
//           0x0d 0x61 0x01 0x08 0xfd 0x00 0x06 0x07 0x00 0x00 0x00 0x00 0x00 0x00.
// Output  : ans
//         :接受缓冲区的数据格式:0x00|帧数|0x55|0xaa|0x0d|canbus数据
//         :数据格式举例: 0x00 0x03 0x55 0xAA 0x0B 0x08 0xFD 0x00 0x10
//                        0x14 0x61 0xE0 0x00 0x05 0x19 0x03 0x02 0x44
//          0xFF 0xFF 0x02 0x49 0x02 0x66 0xFF 0xFF 0xFF 0xFF 0x00 0x00 0xF1
// Return : 0xff
// Info   : 6225
// 注意的问题:在老下位机里面.
// 注意在这里所有的接受的数据的长度都是下位机自己判断的.
// 这里与6203不同的时在进行
//------------------------------------------------------------------------------
int CANSendAndReceiveUnknownFrame_benz_1(int argc, unsigned char * argv, unsigned char * ans)
{
    struct data_info
    {
        unsigned char   number;
        unsigned char   timers;
        unsigned char   pos;
        unsigned char   length;
        unsigned char   data[20];
        unsigned int    id_number;
    } send_info, receive_info;


    //int total_length  = 0;
    int total_length_benz = 0;
    unsigned char specialdata[4] =
    {
        0
    };
    unsigned int control_flow_flag = true;
    unsigned int send_only = false;
    unsigned int re_flag = true;
    signed int BlockLenth;

    set_time0Stop();
    send_info.pos = 3;
    send_info.number = argv[0];
    receive_info.number = 0;
    send_info.id_number = argv[2];
    send_info.id_number = (send_info.id_number == 2) ? 2: 4;
    receive_info.pos = 3;
    send_only = ((argv[send_info.pos + 1] == 0x61) && (argv[send_info.pos + 1] == 0x03)) ? true: false;

    do
    {
        memmove(send_info.data, &argv[send_info.pos + 3], argv[send_info.pos] -2);
        CANSendOneFrame_Ext(send_info.data, send_info.id_number, specialdata);

        if ((specialdata[0] &0x30) == 0x10)
        {
            while (1)
            {
                ReadOneFrameFromCANBuf_Ext(receive_info.data, send_info.id_number, specialdata, 3, 500);

                if (specialdata[0] == 0x30)
                {
                    break;
                }
            }
        }

        send_info.pos += argv[send_info.pos] +1;
    }
    while(--send_info.number);

    /////////////////////////////////////////
    if (send_only == true)
    {
        ans[0] = 0x00;
        ans[1] = 0x00;
        ans[2] = 0x01;
        ans[3] = 0x55;
        ans[4] = 0xaa;
        ans[5] = 0x01;
        ans[6] = 0x00;
        ans[7] = 0x01;
        return 0xff;
    }

    ////////////////////////////////////////
    //do{ //改过
    while (re_flag == true)
    {
        receive_info.length = ReadOneFrameFromCANBuf_Ext(receive_info.data, send_info.id_number, specialdata, 1, 500); //接受第一帧数据

        if (receive_info.length == 0)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            ans[0] = 0xff;
            ans[1] = 0x02;
            return 0x02;
        }

        receive_info.number++;
        memmove(&ans[receive_info.pos], receive_info.data, receive_info.length);
        total_length_benz += receive_info.length;
        receive_info.pos += receive_info.length;

        if ((specialdata[0] &0x10) == 0x10) //开始接受多帧
        {
            BlockLenth = (specialdata[0] &0x0f) +specialdata[1];
            BlockLenth = BlockLenth + 5 + send_info.id_number - receive_info.length;
            control_flow_flag = true;               //开启流控

            //total_length = (specialdata[ 0 ]&0x0f) + specialdata[ 1 ] - (receive_info.length - 5 - send_info.id_number - 2);//剩下的长度
            do
            {
                if (control_flow_flag == true) //流控开关进行控制是否发送流控
                {
                    CANSendOneFrame_Ext(CanEnterFrame.data, send_info.id_number, specialdata); //发送流控制命令
                }

                receive_info.length = ReadOneFrameFromCANBuf_Ext(receive_info.data, send_info.id_number, specialdata, 2, 500); //接受第一帧数据

                if (BlockLenth <= 1)
                {
                    re_flag = false;
                    break;
                }

                if (receive_info.length == 0x00)
                {
                    if (timer_open_flag == 1)
                    {
                        TimerStart(TIMER0);
                    }

                    ans[0] = 0xff;
                    ans[1] = 0  x02;
                    return 0x02;
                }

                if (specialdata[0] < 0x20) //判断是否小于0x20
                {
                    if ((specialdata[0] &0x10) != 0x10)
                    {
                        re_flag = false;
                        break;                      //跳出循环
                    }
                    else
                    {	//等于0x10 就把接收的数据存起来 然后又发送流控制帧
                        receive_info.number++;
                        memmove(&ans[receive_info.pos], receive_info.data + 1, receive_info.length - 4);
                        receive_info.pos += receive_info.length - 1;
                        total_length_benz += receive_info.length - 1;
                        BlockLenth = BlockLenth + 1 - receive_info.length;

                        //total_length = (specialdata[ 0 ]&0x0f) + specialdata[ 1 ] - (receive_info.length - 2);//剩下的长度
                        control_flow_flag = true;

                        /////////////////
                        CanEnterFrame.data[4] = 0x01;//设置流控制帧的数据

                        ////////////////
                        continue;
                    }
                }
                else if ((specialdata[0] == 0x20) || (specialdata[0] == 0x28)) //判断是否等于0x28||0x20
                {
                //等于0x20 或者0x28 也要发送流控制帧吗
                    ///////////////////
                    CanEnterFrame.data[4] = 0x02;

                    //////////////////
                    control_flow_flag = true;       //需要发送流控
                }
                else
                {
                    control_flow_flag = false;
                }

                receive_info.number++;
                memmove(&ans[receive_info.pos], receive_info.data + 1, receive_info.length - 1);
                total_length_benz += receive_info.length - 1;
                receive_info.pos += receive_info.length - 1;
                BlockLenth = BlockLenth + 1 - receive_info.length;

                //total_length  -= (receive_info.length - 1);//剩下的长度
            } //while(total_length >= 0); //在这里等于0时还进行循环一次.
            while(1);                               //这里无法用长度进行控制。
        }
        else //接受一帧
        {
            if ((specialdata[1] == 0x7f) || (specialdata[2] == 0x78))
            {
                continue;
            }
            else
            {
                re_flag = false;
                break;
            }
        }
    }

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    ans[0] = 0;
    ans[1] = 0;
    ans[2] = receive_info.number;
    return total_length_benz += 3;
}


//------------------------------------------------------------------------------
// Funtion : CanSendMulFrameGetMoreSingleFrame
// Input   : argc - 参数长度
//           argv - 数据格式
//           发送缓冲区(argv)的数据格式->0x62|0x25|发送的帧数|接受的数据长度|
//           发送的ID个数|发送长度(不包括校验位)|0x61|0x01|canbus数据。
//           发送缓冲区的数据举例:
//           0x单帧的个数 0x02 0xff 0x02 0x0d 0x61 0x01 0x08 0xfd 0x00 0x10 0x08
//                        0x21 0x01 0x02 0x03 0x04 0x05 0x0d 0x61 0x01 0x08 0xfd
//                        0x00 0x06 0x07 0x00 0x00 0x00 0x00 0x00 0x00.
// Output  : ans
//         :接受缓冲区的数据格式:0x00|帧数|0x55|0xaa|0x0d|canbus数据
//         :数据格式举例: 0x00 0x03 0x55 0xAA 0x0B 0x08 0xFD 0x00 0x10
//                        0x14 0x61 0xE0 0x00 0x05 0x19 0x03
//                        0x02 0x44 0xFF 0xFF 0x02 0x49 0x02
//                        0x66 0xFF 0xFF 0xFF 0xFF 0x00 0x00 0xF1
// Return : 0xff
// Info   : 6227
// 注意的问题:在老下位机里面.
// 注意在这里所有的接受的数据的长度都是下位机自己判断的.
// 这里与6203不同的时在进行
// 这里主要处理的是连续的多帧.
// 能够接收独立的多帧
// 1.2012-12-19添加了返回数据的长度的功能.
//------------------------------------------------------------------------------
int CanSendMulFrameGetMoreSingleFrame_1(int argc, unsigned char * argv,
    unsigned char * ans)
{
    struct data_info
    {
        unsigned char   number;
        unsigned char   timers;
        unsigned char   pos;
        unsigned char   length;
        unsigned char   data[20];
        unsigned int    id_number;
    } send_info, receive_info;


    unsigned char specialdata[4] =
    {
        0
    };
    unsigned int control_flow_flag = true;
    unsigned int send_only = false;
    unsigned int re_number = 0;
    unsigned int re_flag = true;
    int total_length = 0;

    set_time0Stop();
    re_number = argv[0];
    send_info.pos = 4;
    send_info.number = argv[1];
    receive_info.number = 0;
    send_info.id_number = argv[3];
    send_info.id_number = (send_info.id_number == 2) ? 2: 4;
    receive_info.pos = 3;
    send_only = ((argv[send_info.pos + 1] == 0x61) && (argv[send_info.pos + 1] == 0x03)) ? true: false;

    do
    {
        memmove(send_info.data, &argv[send_info.pos + 3], argv[send_info.pos] -2);
        CANSendOneFrame_Ext(send_info.data, send_info.id_number, specialdata);

        if (specialdata[0] == 0x10)
        {
            while (1)
            {
                ReadOneFrameFromCANBuf_Ext(receive_info.data, send_info.id_number, specialdata, 3, 500);

                if (specialdata[0] == 0x30)
                {
                    break;
                }
            }
        }

        send_info.pos += argv[send_info.pos] +1;
    }
    while(--send_info.number);

    if (send_only == true)
    {
        ans[0] = 0x00;
        ans[1] = 0x00;
        ans[2] = 0x01;
        ans[3] = 0x55;
        ans[4] = 0xaa;
        ans[5] = 0x01;
        ans[6] = 0x00;
        ans[7] = 0x01;
        return 0xff;
    }

    //do{//这里有进行修改要注意测试
    while (re_flag == true)
    {
        receive_info.length = ReadOneFrameFromCANBuf_Ext(receive_info.data, send_info.id_number, specialdata, 1, 500); //接受第一帧数据

        if (receive_info.length == 0)
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            ans[0] = 0xff;
            ans[1] = 0x02;
            return 0x02;
        }

        if ((specialdata[0] &0x10) == 0x10) //开始接受多帧
        {
            receive_info.number++;
            memmove(&ans[receive_info.pos], receive_info.data, receive_info.length); //去掉校验位
            total_length += receive_info.length;
            receive_info.pos += receive_info.length;
            control_flow_flag = true;               //开启流控

            do
            {
                if (control_flow_flag == true) //流控开关进行控制是否发送流控
                {
                    CANSendOneFrame_Ext(CanEnterFrame.data, send_info.id_number, specialdata); //发送流控制命令
                }

                receive_info.length = ReadOneFrameFromCANBuf_Ext(receive_info.data, send_info.id_number, specialdata, 2, 500); //接受第一帧数据

                if (receive_info.length == 0x00)
                {
                    if (receive_info.number == 0x00)
                    {
                        if (timer_open_flag == 1)
                        {
                            TimerStart(TIMER0);
                        }

                        ans[0] = 0xff;
                        ans[1] = 0x02;
                        return 0x02;
                    }
                    else
                    {
                        if (timer_open_flag == 1)
                        {
                            TimerStart(TIMER0);
                        }

                        ans[0] = 0xff;
                        ans[1] = 0x02;
                        return 0x02;
                    }
                }

                if (specialdata[0] < 0x20) //判断是否小于0x20
                {
                    if ((specialdata[0] &0x10) != 0x10)
                    {
                        re_flag = false;
                        break;                      //跳出循环
                    }
                    else
                    {
                        receive_info.number++;
                        memmove(&ans[receive_info.pos], receive_info.data + 4, receive_info.length - 4);
                        receive_info.pos += receive_info.length - 4;
                        total_length += receive_info.length - 4;
                        control_flow_flag = true;
                        continue;
                    }
                }
                else
                {
                    control_flow_flag = false;
                }

                receive_info.number++;
                memmove(&ans[receive_info.pos], receive_info.data + 1, receive_info.length - 1);
                receive_info.pos += receive_info.length - 1;
                total_length += receive_info.length - 1;
            }
            while(1);                               //这里无法用长度进行控制。
        }
        else //接受一帧
        {
            if ((specialdata[1] == 0x7f) || (specialdata[2] == 0x78))
            {
                continue;
            }
            else
            {
                receive_info.number++;
                memmove(&ans[receive_info.pos], receive_info.data, receive_info.length - 1); //去掉校验位
                receive_info.pos += receive_info.length - 1;
                total_length += receive_info.length - 1;

                if (receive_info.number < re_number)
                {
                    continue;
                }
                else
                {
                    re_flag = false;
                    break;
                }
            }
        }
    }

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    ans[0] = 0;
    ans[1] = 0;
    ans[2] = receive_info.number;
    return total_length += 3;
}


//------------------------------------------------------------------------------
// Funtion: Can发多帧收多帧，收帧数未知
// Input  : argc - 参数长度
//          argv - 参数
//          发送缓冲区的数据格式 : 发送帧数 + 接受帧数(0xff由下位机进行发送) +
//          ID个数 + 长度 + 0x61 + 0x01 ....
//
// Output : ans - 回复到上位机的数据 0x01,0x55,0xaa,
// Return : 0x01
// Info   : 6208
//1. 实例   :
//        发送缓冲区: 0x02 0xff 0x02 0x0d 0x61 0x01 0x08 0xid1 0xid2 0x01,0x01,
//                    0x00,0x00,0x00,0x00
//        接受缓冲区:
//        LPC2368接受缓冲区的内容:
//        0x00,帧数,0x55,0xaa,0x0b,data,data....注意数据中不包含0x21,0x22.
//发多帧收多帧(指定帧或者自动识别)，发第一帧的时候，ECU可能没有准备好，这个时候不一定会回30
//发完之后，接收一帧，此时可能出现7f 78，需要反复接收
//发多帧的打包，第一帧和最后一帧要6101，中间用6103
//接收帧数为ff的话，直到接收超时才返回;不为ff的话，则返回指定的接收帧数，即使接收不够，将实际接收的帧数返回
//固定2个ID或者4个ID
//ziyingzhu 修改并在平台上测试成功 2009-9-16 9:44
//里面对CAN帧操作的函数，其他模块可以借鉴
//返回数据格式帧数+每帧数据
//发送多帧的时候，按照15765的流控.接收帧只进行7f 78的判断(接收多帧假设不需要发送流控的30帧)
//能否完成发单帧，收多帧，进行7f 78的判断
//------------------------------------------------------------------------------
int DongFengSendMultiFrameAndReceiveOneFrame_1(int argc, unsigned char * argv,
    unsigned char * ans)
{
    unsigned int send_length_pos = 0;
    unsigned char send_number = 0;
    unsigned char i = 0;
    unsigned char send_data[20] =
    {
        0
    };

    //在发送的数据区有11和14个字节的
    unsigned int receive_number = 0;                //下位机接受的帧数
    unsigned char receive_data[20] =
    {
        0
    };

    //
    unsigned int receive_length = 0;
    unsigned int id_number = 0;
    unsigned char specialdata[4] =
    {
        0
    };
    unsigned char * ans_head = NULL;                //注意在进行定义指针的时候一定要进行初始化。
    int total_length = 0;

    send_number = argv[1];
    receive_number = argv[0];
    id_number = 2;
    send_length_pos = 7;
    ans_head = ans;
    ans = ans + 3;

    //对55 aa 0d 61 01 08 fc 00 xx xx xx xx xx xx xx xx xx
    set_time0Stop();

    /*---------------------数据发送完毕------------------------------*/
    for (i = 0; i < send_number; i++)
    {
        memmove(send_data, argv + send_length_pos, (argv[send_length_pos] &0x0f) +id_number + 1);

        if (CANSendOneFrame_Ext(send_data, id_number, specialdata) == false) //发送确认帧
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            ans_head[0] = 0xff;
            ans_head[1] = 0x02;
            return 2;
        }

        while (1)
        {
            if (false == ReadOneFrameFromCANBuf_Ext(receive_data, id_number, specialdata, 1, 500))
            {
                if (timer_open_flag == 1)
                {
                    TimerStart(TIMER0);
                }

                ans_head[0] = szFBREAK[0];
                ans_head[1] = szFBREAK[1];
                return 0x02; //这里的500是根据LPC2368来设置的
            }

            if ((specialdata[0] &0x30) == 0x30) //接受到了流控制命令
            {
                break;
            }
        }

        // 1 + 8 + 2 + 1 + 2 + 3;
        send_length_pos += (argv[send_length_pos] &0x0f) +id_number + 7; //下一帧开始的地方.
        delay(2);
    }

    /*-----------------以上是数据发送完毕----------------------------------*/
    send_length_pos = 2;
    ans_head[1] = receive_number;

    while (receive_number)
    {
        /*--------------------接受第一帧数据-------------------------------*/
        receive_length = ReadOneFrameFromCANBuf_Ext(receive_data, id_number, specialdata, 1, 500);

        if (receive_length == false) //这里就是接受失败
        {
            if (timer_open_flag == 1)
            {
                TimerStart(TIMER0);
            }

            if (0 == total_length)
            {
                ans_head[0] = 0xff;
                ans_head[1] = 0x02;
                return 2;
            }
            else
            {
                ans_head[0] = 0;
                return total_length + 3;
            }
        }

        if ((specialdata[1] == 0x7f) && (specialdata[2] == 0x78)) //说明是繁忙帧
        {
            continue;
        }
        else
        {
            receive_length++;                       // jiaoyan
            receive_number--;
            memmove(&ans_head[send_length_pos], receive_data, receive_length);
            send_length_pos += receive_length;
            total_length += receive_length;
        }
    }

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    if (0 == total_length)
    {
        ans_head[0] = 0xff;
        ans_head[1] = 0x02;
        return 2;
    }
    else
    {
        ans_head[0] = 0;
        return total_length + 2;
    }
}


//------------------------------------------------------------------------------
// Funtion: GM发多帧
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 620D
//          00,03,fc,00,08,10,0e,2c,fe,00,04,00,0d,08,21,00,0f,11,52,12,b1,13,08,22,1f,00,00,00,00,00,00,bd,
//------------------------------------------------------------------------------
int GMSendMultiFrameToECUGetAnswer_1(int argc, unsigned char * argv,
    unsigned char * ans)
{
    int sendframenum, rcvpos = 5, waitingtime = 500;
    unsigned char framesize, busyid[3];
    unsigned char pCanbuf[30];
    unsigned char pCanSendbuf[30];

    set_time0Stop();
    delay(2);

    //发送总帧数
    sendframenum = argv[0] *256;
    sendframenum += argv[1];
    pCanSendbuf[1] = argv[2];
    pCanSendbuf[2] = argv[3];
    pCanSendbuf[0] = argv[4];
    memcpy(pCanSendbuf + 3, argv + 5, pCanSendbuf[0]);
    rcvpos += pCanSendbuf[0];
    CANSendOneFrame_Ext(pCanSendbuf, 2, busyid);
    sendframenum--;

    if (sendframenum != 0)
    {
        //反复读，直到接收到30确认帧为止
        do
        {
            if (ReadOneFrameFromCANBuf_Ext(pCanbuf, 2, busyid, 0, 2000) == false)
            {
                ans[0] = 0xff;
                ans[1] = 0x02;
                return 2;
            }

            //如果是30帧的话，将剩余的发送完毕
            if (busyid[0] == 0x30)
            {
                break;
            }
        }
        while(1);

        while (sendframenum--)
        {
            pCanSendbuf[0] = argv[rcvpos++];
            memcpy(pCanSendbuf + 3, argv + rcvpos, pCanSendbuf[0]);
            rcvpos += pCanSendbuf[0];
            CANSendOneFrame_Ext(pCanSendbuf, 2, busyid);
            delay(2);
        }
    }

    do
    {
        if ((framesize = ReadOneFrameFromCANBuf_Ext(pCanbuf, 2, busyid, 1, waitingtime)) == FALSE)
        {
            ans[0] = 0xff;
            ans[1] = 0x02;
            return 2;
        }

        if ((busyid[1] == 0x7f) && (busyid[2] == 0x78))
        {
            waitingtime = 5000;
            continue;
        }

        memmove(&ans[3], pCanbuf, framesize);
        rcvpos += framesize;
        break;
    }
    while(1);

    if (timer_open_flag == 1)
    {
        TimerStart(TIMER0);
    }

    ans[0] = 0;
    ans[1] = 0;
    ans[2] = 1;
    return (framesize + 3);
}


//------------------------------------------------------------------------------
// Funtion: J1939 卡车CAN (BUG)
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6223
//          55,aa,00,12,ff,ed,62,23,ff,01,01,43,00,00,00,00,00,02,ff,00,
//          00,01,08,00,44,00,00,02,44,48,ff,ff,
//------------------------------------------------------------------------------
int CumminsSAE1939_SendMultiFramesToEcuGetAnswer_1(int argc, unsigned char * argv,
    unsigned char * ans)
{
    int receiveframenum, sendframenum, multiframenum, i, j;
    char group = 0;
    int rcvpos = 0;
    char sendframe_18EC00F9[20] =
    {
        0x0f, 0x61, 0x01, 0x88, 0xc7, 0x60, 0x07, 0xc8
    };
    char sendframe_18EB00F9[20] =
    {
        0x0f, 0x61, 0x01, 0x88, 0xc7, 0x58, 0x07, 0xc8
    };
    char sendframe_1CEC00F9_11[20] =
    {
        0x0f, 0x61, 0x01, 0x88, 0xe7, 0x60, 0x07, 0xc8, 0x11, 0x02, 0x01, 0xff, 0xff, 0x00, 0xef, 0x00
    };
    char sendframe_1CEC00F9_13[20] =
    {
        0x0f, 0x61, 0x01, 0x88, 0xe7, 0x60, 0x07, 0xc8, 0x13, 0x09, 0x09, 0x02, 0xff, 0x00, 0xef, 0x00
    };
    char receivebuf[15] =
    {
        0
    };
    set_time0Stop();                                // 2008-12-2, 500->STOPLINKTIME
    delay(2);

    //发送的多帧帧数

    group = 1;
    if((multiframenum = argv[ group++ ]) == 0)
    {
        if(timer_open_flag == 1)
        TimerStart( TIMER0 );
        ans[ 0 ] = 0xff;
        ans[ 1 ] = 0x02;
        return 2 ;
    }
    rcvpos = 3;
    for(i=0;i<multiframenum;i++)
    {
        //发送过程
        if((sendframenum = argv[ group++ ]) == 0)
        {
            if(timer_open_flag == 1)
            TimerStart( TIMER0 );
            ans[ 0 ] = 0xff;
            ans[ 1 ] = 0x02;
            return 2 ;
        }
        //发送多帧第一帧 10 18EC00F9
        memcpy( sendframe_18EC00F9+8, argv+(group), 8 );
        group += 9;
        CANSendOneAndReceiveOneFrame(sendframe_18EC00F9,0);
        //接收11确认帧
        if(false == ReadOneFrameFromCANBuf(receivebuf, 200))
        {
            if(timer_open_flag == 1)
            TimerStart( TIMER0 );
            ans[ 0 ] = 0xff;
            ans[ 1 ] = 0x02;
            return 2 ;
        }
        sendframenum--;
        //发送一个多帧 18EB00F9
        for(j=0;j<sendframenum;j++)
        {
            memcpy(sendframe_18EB00F9+8,argv+(group),8);
            group += 9;
            CANSendOneAndReceiveOneFrame(sendframe_18EB00F9,0);
            Sleep(2);
        }
        //接收13确认帧
        if(false == ReadOneFrameFromCANBuf(receivebuf, 200))
        {
            if(timer_open_flag == 1)
            TimerStart( TIMER0 );
            ans[ 0 ] = 0xff;
            ans[ 1 ] = 0x02;
            return 2 ;
        }
        //接收过程
        //接收10帧，知道接收的帧数和有效数据长度
        if(false == ReadOneFrameFromCANBuf(receivebuf, 200))
        {
            if(timer_open_flag == 1)
            TimerStart( TIMER0 );
            ans[ 0 ] = 0xff;
            ans[ 1 ] = 0x02;
            return 2 ;
        }
        //发送确认帧 11 1CEC00F9
        sendframe_1CEC00F9_11[9] = receivebuf[8];
        receiveframenum = receivebuf[8];
        sendframe_1CEC00F9_13[9] = receivebuf[6];
        sendframe_1CEC00F9_13[10] = receivebuf[7];
        sendframe_1CEC00F9_13[11] = receivebuf[8];
        CANSendOneAndReceiveOneFrame(sendframe_1CEC00F9_11,0);
        //接收多帧
        for(j=0;j<receiveframenum;j++)
        {
            if(false == ReadOneFrameFromCANBuf(receivebuf, 200))
            {
                if(timer_open_flag == 1)
                TimerStart( TIMER0 );
                ans[ 0 ] = 0xff;
                ans[ 1 ] = 0x02;
                return 2 ;
            }
            memcpy( &ans[ rcvpos ], receivebuf + 6, 7 );
            ans[ rcvpos - 1 ] = 7;
            rcvpos += 7;
            //PageWriteExtMem(rcvpos+7*j,receivebuf+6,7);
        }
        //发送确认帧 13 1CEC00F9
        CANSendOneAndReceiveOneFrame(sendframe_1CEC00F9_13,0);
        len = sendframe_1CEC00F9_13[10]*256+sendframe_1CEC00F9_13[9];
    }
    ans[ 0 ] = 0;
    ans[ 1 ] = receiveframenum;
    if(timer_open_flag == 1)
            TimerStart( TIMER0 );
    return 0xff;
}


//------------------------------------------------------------------------------
// Funtion: 无效果
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6114 send: ff,00, 表示成功
//------------------------------------------------------------------------------
int CanbusSendTwoFrameToEcuGetAnswerOldCon10_1(int argc, unsigned char * argv,
    unsigned char * ans)
{
    ans[0] = 0xff;
    ans[1] = 0x00;
    return 2;
}


#endif

//--------------------------------------------------------- End Of File --------
