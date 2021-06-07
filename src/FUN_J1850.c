

//------------------------------------------------------------------------------
//  Purpose: 指令函数集 - 总线通讯类（J1850 VPW PWM）
//  Funtion: 完成所有物理层总线参数设置函数，主要集中与 指令ID 6100 - 6200
//  Dependent:
//  Designer:
//  Date.Ver:
//------------------------------------------------------------------------------
#ifndef FUN_J1850_C
#define FUN_J1850_C
#include "FUN_J1850.h"
#include "CMPHY_VPW.h"
#include "CMPHY_IOCtrl.h"
#include "CMPHY_PWM.h"
#include "gdstd.h"
#include "CMPHY_Relay.h"
#include "CMPHY_Timer.h"

//------------------------------------------------------------------------------
// Funtion: VPW协议发一收一
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 610E 00 01 05 6C 10 F5 00 20 CHK
//------------------------------------------------------------------------------
int VpwSendOneAndReceiveOneFrame_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char framesize, framesize1;
    unsigned char framenum;
    static int framepos, framepos1;
    unsigned int firstms;
    static unsigned char Isans;
    CMPHY_Relay Relaybuf;

    // 00 Datastreammode
    if (0 == argv[0])
    {
        Isans = 1;
    }
    else if (1 == Isans)
    {
        Isans = 0;
        return framepos;
    }

    framesize = argv[2];
    CMPHY_Relay_Init(&Relaybuf);
    Relaybuf.CommType = COMM_VPW_TYPE;              //通讯类型串行口
    Relaybuf.LevelType = COMM_LEVEL_12V;            //12V
    CMPHY_Relay_Reset();
    CMPHY_Relay_Set(&Relaybuf);
    VPW_Init();
    firstms = 0;
    framepos = 3;
    framenum = argv[1];

    //Allms = 0;
    if (framenum == 1) // SB 发一收多
    {
        if (false == VPW_Send(&argv[3], framesize))
        {
            ans[0] = 0xff;
            ans[1] = 0x02;
            return 2;
        }

        framesize = 0;
        framenum = 0;

        while ((firstms < 200) && (framenum < 20))
        {
            framesize = VPW_Receive(&ans[framepos], 200, &firstms, 200);

            if (framesize)
            {
                ans[framepos - 1] = framesize;
                framepos += framesize;
                framepos++;
                framenum++;
                firstms = 0;
            }
        }

        ans[1] = framenum;

        if (!framenum)
        {
            ans[0] = 0xff;
            ans[1] = 0x02;
            return 2;
        }
    }
    else // SB 发多收多
    {
        framepos1 = 0;
        ans[1] = framenum;

        while (framenum)
        {
            // ff 02 06 6c 10 f1 01 00 CHK 06 6c 10 f1 01 00 CHK
            framesize = argv[2 + framepos1];

            if (false == VPW_Send(&argv[3 + framepos1], framesize))
            {
                ans[0] = 0xff;
                ans[1] = 0x02;
                return 2;
            }

            framepos1++;
            framepos1 += framesize;

            // 17 02 07 6c f1 10 ee 01 e1 27 07 6c f1 10 ee 01 e1 27
            framesize1 = VPW_Receive(&ans[framepos], 200, &firstms, 200);

            if (framesize1)
            {
                ans[framepos - 1] = framesize1;
                framepos += framesize1;
                framepos++;
            }
            else
            {
                ans[0] = 0xff;
                ans[1] = 0x02;
                return 2;
            }

            framenum--;
        }
    }

    ans[0] = 0;
    framepos--;
    return framepos;
}


//------------------------------------------------------------------------------
// Funtion: VPW协议发多 收多
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6127 ,ff,01,10,6c,f1,05,6c,25,f1,20,c5,6d,
//          BUG  08 6C 10 F1 12 03 FF FF E6 ;Times: 1319601 us 
//               0A 6C F1 10 52 03 00 02 00 00 79 ;Times: 17894 us 
//               0A 6C F1 10 52 03 00 03 01 00 BA ;Times: 12541 us 
//               09 6C F1 10 52 03 00 04 00 E9 ;Times: 11655 us 
//               09 6C F1 10 52 03 00 05 48 5E ;Times: 12319 us 
//               09 6C F1 10 52 03 00 06 80 57 ;Times: 12556 us 
//               09 6C F1 10 52 03 00 07 91 CB ;Times: 13194 us 
//               09 6C F1 10 52 03 00 0B 65 B7 ;Times: 11926 us 
//               0A 6C F1 10 52 03 00 0C 00 00 70 ;Times: 13271 us 
//               09 6C F1 10 52 03 00 0D 00 FF ;Times: 11696 us 
//               08 6C 10 F1 12 03 FF FF E6 ;Times: 12086 us 
//               0A 6C F1 10 52 03 00 10 09 0F CF ;Times: 7804 us 
//      SendCmd: 55,aa,f0,f8,00,13,3f,27,01,61,27,ff,01,50,6c,f1,08,6c,10,f1,12,03,ff,ff,e6,05,
//      ReceiveCmd: ff,02,
//------------------------------------------------------------------------------
int VpwSendMultiFrameAndReceiveMultiFrameKnownAckFrameNumber_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char framesize;
    unsigned char framenum;
    static int framepos, framepos1;
    unsigned int firstms;
    unsigned int Allms;
    unsigned char filt_id[2];
    static unsigned char Isans;
    unsigned char MaxRecv;
    CMPHY_Relay Relaybuf;

    // 00 Datastreammode
    if (0 == argv[0])
    {
        Isans = 1;
    }
    else if (1 == Isans)
    {
        Isans = 0;
        return framepos;
    }

    CMPHY_Relay_Init(&Relaybuf);
    Relaybuf.CommType = COMM_VPW_TYPE;              //通讯类型串行口
    Relaybuf.LevelType = COMM_LEVEL_12V;            //12V
    CMPHY_Relay_Reset();
    CMPHY_Relay_Set(&Relaybuf);
    VPW_Init();
    framenum = argv[1];
    MaxRecv = argv[2];
    filt_id[0] = argv[3];
    filt_id[1] = argv[4];
    Allms = 0;
    firstms = 0;
    framepos = 3;
    framepos1 = 5;

    while (framenum) // SB 发
    {
        //61,27,ff,02,10,7f,7f,0a,6c,10,f1,2a,14,fe,fd,fc,fb,35,0a,6c,10,f1,2a,
        //24,fa,f9,f8,f7,e3,a4,
        framesize = argv[framepos1];
        framepos1++;
        delay(25);

        if (false == VPW_Send(&argv[framepos1], framesize))
        {
            ans[0] = 0xff;
            ans[1] = 0x02;
            return 2;
        }

        framenum--;
        framepos1 += framesize;
    }

    framesize = 0;
    framenum = 0;

    //while ((Allms < 500) && (framenum < MaxRecv))
    while ((Allms < 1400) && (framenum < MaxRecv)) //IDA显示这个值是1400 源码时500 lh
    {
        //00,02,0c,6c,f1,10,6a,fa,00,80,88,03,15,00,d6,
        framesize = VPW_Receive(&ans[framepos], 200, &firstms, 500);

        if (framesize && (ans[framepos] == filt_id[0]) && (ans[framepos + 1] == filt_id[1]))
        {
            ans[framepos - 1] = framesize;
            framepos += framesize;
            framepos++;
            framenum++;
           // Allms = 0;
           Allms = 1000;//IDA反汇编出来这里是1000
        }
        else if (framesize && (0x7F == filt_id[0]) && (0x7F == filt_id[1]))
        {
            ans[framepos - 1] = framesize;
            framepos += framesize;
            framepos++;
            framenum++;
            Allms = 0;
        }
        else
        {
            Allms += firstms;
        }
    }

    ans[1] = framenum;

    if (!framenum)
    {
        ans[0] = 0xff;
        ans[1] = 0x02;
        return 2;
    }

    ans[0] = 0;
    framepos--;
    return framepos;
}


//------------------------------------------------------------------------------
// Funtion: PWM协议发一收多
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : Recv: 610C ff,01,06,61,6a,f1,01,00,0a,64,
//          Send: 00,01,0b,c4,f5,10,53,01,02,01,03,01,04,07,
//------------------------------------------------------------------------------
int PWM_SendDataToEcuGetAnswer_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char framesize, framesize1;
    unsigned char framenum;
    static int framepos, framepos1;
    unsigned int firstms;
    static unsigned char Isans;
    unsigned int FrameID;
    unsigned char Addrress;
    CMPHY_Relay Relaybuf;

    // 00 Datastreammode
    if (0 == argv[0])
    {
        Isans = 1;
    }
    else if (1 == Isans)
    {
        Isans = 0;
        return framepos;
    }

    framesize = argv[2];
    CMPHY_Relay_Init(&Relaybuf);
    Relaybuf.CommType = COMM_PWM_TYPE;              //通讯类型串行口
    Relaybuf.LevelType = COMM_LEVEL_12V;            //12V
    CMPHY_Relay_Reset();
    CMPHY_Relay_Set(&Relaybuf);
    PWM_Init();
    framepos = 3;
    framenum = argv[1];
	//IDA里面是先判断再发送数据

	if ( argv[3] != 0x61 || argv[4] != 0x6A )
	  {
		if (argv[3] == 0xC4 )
		  FrameID = argv[4] + (((unsigned __int8)argv[5] + 0xC400) << 8);
		else
		  FrameID = 0;
	  }
	  else
	  {
		FrameID = 0x416BF0;
	  }



	
    if (framenum == 1) // SB 发一收多
    {
        if (false == PWM_Send(&argv[3], framesize, true, &Addrress))
        {
            delay(1);

            // 总线被其他节点 抢占，重试 一次
            if (false == PWM_Send(&argv[3], framesize, true, &Addrress))
            {
                //ans[ 0 ] = 0xff;
                //ans[ 1 ] = 0x02;
                //return 2;
            }
        }

        framesize = 0;
        framenum = 0;

        // OBDII 在IDA中这里被提到外面去了 lh
        //if ((0x61 == argv[3]) && (0x6a == argv[4]))
        //{
        ///    FrameID = 0x416B;
        //}
        //else if (0xC4 == argv[3]) // Ford PWM
        ///{
       ///     FrameID = 0xC400 + argv[5];
      	// }
       // else
       // {
       //     FrameID = 0;
       // }

        Addrress = argv[5];
        firstms = 0;

        //while( 1 )
        //{
        framesize = PWM_Receive(&ans[framepos], 200, &firstms, 500, true, FrameID, Addrress);

        if (firstms >= 500)
        {
            //break;
        }
        else
        {
            ans[framepos - 1] = framesize;
            framepos += framesize;
            framepos++;
            framenum++;
        }

        //}
        ans[1] = framenum;

        if (!framenum)
        {
            ans[0] = 0xff;
            ans[1] = 0x02;
            return 2;
        }
    }
    else // SB 发多收多
    {
        framepos1 = 0;
        ans[1] = framenum;

        while (framenum)
        {
            // ff 02 06 6c 10 f1 01 00 CHK 06 6c 10 f1 01 00 CHK
            framesize = argv[2 + framepos1];

            if (false == PWM_Send(&argv[3 + framepos1], framesize, true, &Addrress))
            {
                // 总线被其他节点 抢占，重试 一次
                delay(1);

                if (false == PWM_Send(&argv[3 + framepos1], framesize, true, &Addrress))
                {
                    //ans[ 0 ] = 0xff;
                    //ans[ 1 ] = 0x02;
                    //return 2;
                }
            }

            framepos1++;
            framepos1 += framesize;

            // OBDII  和IDA 反编译出来的有区别 FrameID的处理方式不一样
            if ((0x61 == argv[3]) && (0x6a == argv[4]))
            {
                FrameID = 0x416B;
            }
            else if (0xC4 == argv[3]) // Ford PWM
            {
                FrameID = 0xC400 + argv[5];
            }
            else
            {
                FrameID = 0;
            }

            Addrress = argv[5];
            firstms = 0;
            framesize1 = PWM_Receive(&ans[framepos], 200, &firstms, 500, true, FrameID, Addrress);
            ans[framepos - 1] = framesize1;
            framepos += framesize1;
            framepos++;

            // 17 02 07 6c f1 10 ee 01 e1 27 07 6c f1 10 ee 01 e1 27
            if (!framesize1)
            {
                ans[0] = 0xff;
                ans[1] = 0x02;
                return 2;
            }
            else
            {
                // NULL
            }

            framenum--;
        }
    }

    ans[0] = 0;
    framepos--;
    return framepos;
}


//------------------------------------------------------------------------------
// Funtion: PWM协议发一收多
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : 6112 07,c4,10,f5,22,02,00,5b,03,2d,
//------------------------------------------------------------------------------
int PwmSendOneAndReceiveMultiFrame_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char framesize;
    unsigned char framenum;
    int framepos;
    unsigned int firstms;
    unsigned int FrameID;
    unsigned char Addrress;
    CMPHY_Relay Relaybuf;

    framesize = argv[0];
    CMPHY_Relay_Init(&Relaybuf);
    Relaybuf.CommType = COMM_PWM_TYPE;              //通讯类型串行口
    Relaybuf.LevelType = COMM_LEVEL_12V;            //12V
    CMPHY_Relay_Reset();
    CMPHY_Relay_Set(&Relaybuf);
    PWM_Init();
    firstms = 0;
    framepos = 3;

	//修改lh
	if ( argv[1] != 0x61 || argv[2] != 0x6A )
	{
		if ( argv[1] == 0xC4 )
			FrameID = Argv[2] + ((argv[3] + 0xC400) << 8);
		else
			FrameID = 0;
	}
	else
	{
		FrameID = 0x416BF0; //这个FrameID 是设置回复的字节的前面的两个字节 6B 41 xx
	}




    if (false == PWM_Send(&argv[1], framesize, true, &Addrress))
    {
        delay(1);

        // 总线被其他节点 抢占，重试 一次
        if (false == PWM_Send(&argv[1], framesize, true, &Addrress))
        {
            //ans[ 0 ] = 0xff;
            //ans[ 1 ] = 0x02;
            //return 2;
        }
    }

    framesize = 0;
    framenum = 0;

    // OBDII
    //if ((0x61 == argv[1]) && (0x6a == argv[2]))
   // {
     //   FrameID = 0x416B;
   // }
   // else if (0xC4 == argv[1]) // Ford PWM
   // {
   //     FrameID = 0xC400 + argv[3];
   // }
   // else
   // {
   //     FrameID = 0;
   // }

    Addrress = argv[3];
    firstms = 0;

    while (1)
    {
        framesize = PWM_Receive(&ans[framepos], 200, &firstms, 200, true,
            FrameID, Addrress);

        if (firstms >= 200)
        {
            break;
        }

        ans[framepos - 1] = framesize;
        framepos += framesize;
        framepos++;
        framenum++;
    }

    ans[1] = framenum;

    if (!framenum)
    {
        ans[0] = 0xff;
        ans[1] = 0x02;
        return 2;
    }

    ans[0] = 0;
    framepos--;
    return framepos;
}


//------------------------------------------------------------------------------
// Funtion: JAGUAR 专用 PWM 协议 发一帧数据到ECU 并接收应答(直到ECU 不发才结束)
// Input  : argc - 参数长度
//          argv - 参数
// Output : ans - 回复到上位机的数据
// Return : 回复到上位机的数据长度
// Info   : recv: 61,20,01,f5,10,05,c4,10,f5,13,c7,50
//------------------------------------------------------------------------------
int JAGUARPWM_SendDataToEcuGetMultiFrameAnswer_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char framesize;
    unsigned char framenum;
    static int framepos;
    unsigned int firstms;
    unsigned char Addrress;
    static unsigned char Isans;
    unsigned int FrameID;
    CMPHY_Relay Relaybuf;

    // 55,aa,f0,f8,00,0f,06,27,01,61,20,c4,f5,10,05,c4,10,f5,13,c7,50,e7,
    // 00 Datastreammode
    if (0 == argv[0])
    {
        Isans = 1;
    }
    else if (1 == Isans)
    {
        Isans = 0;
        return framepos;
    }

    // c4,f5,10,05,c4,10,f5,13,c7,50,
    framesize = argv[3];
    CMPHY_Relay_Init(&Relaybuf);
    Relaybuf.CommType = COMM_PWM_TYPE;              //通讯类型串行口
    Relaybuf.LevelType = COMM_LEVEL_12V;            //12V
    CMPHY_Relay_Reset();
    CMPHY_Relay_Set(&Relaybuf);
    PWM_Init();
    firstms = 0;
    framepos = 3;

    //MaxRecv = argv[ 2 ];
    //FrameID = argv[0] *0x100 + argv[1];IDA反汇编FrameID的获取不一样
    FrameID = Argv[2] + ((Argv[1] + (*Argv << 8)) << 8);//lh 参考反汇编 修改后

    //Allms = 0;
    if (false == PWM_Send(&argv[4], framesize, true, &Addrress))
    {
        delay(1);

        // 总线被其他节点 抢占，重试 一次
        if (false == PWM_Send(&argv[4], framesize, true, &Addrress))
        {
            //ans[ 0 ] = 0xff;
            //ans[ 1 ] = 0x02;
            //return 2;
        }
    }

    framesize = 0;
    framenum = 0;

    //Allms = 0;
    Addrress = argv[6];

    while (1)
    {
        framesize = PWM_Receive(&ans[framepos], 200, &firstms, 200, true,
            FrameID, Addrress);

        if (firstms >= 200)
        {
            break;
        }

        ans[framepos - 1] = framesize;
        framepos += framesize;
        framepos++;
        framenum++;
    }

    ans[1] = framenum;

    if (!framenum)
    {
        ans[0] = 0xff;
        ans[1] = 0x02;
        return 2;
    }

    ans[0] = 0;
    return framepos;
}


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
int VPW_HDN_SendDataToEcuGetMultiFrameAnswer_1(int argc, unsigned char * argv, unsigned char * ans)
{
    int send_framesize, framesize;
    CMPHY_Relay Relaybuf;
    unsigned int firstms;

    CMPHY_Relay_Init(&Relaybuf);
    Relaybuf.CommType = COMM_PWM_TYPE;              //通讯类型串行口
    Relaybuf.LevelType = COMM_LEVEL_12V;            //12V
    CMPHY_Relay_Reset();
    CMPHY_Relay_Set(&Relaybuf);
    VPW_Init();
    send_framesize = argv[0];

    if (false == VPW_Send(&argv[1], send_framesize))
    {
        ans[0] = 0xff;
        ans[1] = 0x02;
        return 2;
    }

    framesize = VPW_Receive(&ans[3], 200, &firstms, 200);

    if (!framesize)
    {
        ans[0] = 0xff;
        ans[1] = 0x02;
        return 2;
    }

    ans[0] = 0;
    ans[1] = 1;
    ans[2] = framesize;
    return (framesize + 3);
}


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
//611D
//------------------------------------------------------------------------------
int VpwSendMultiFrameAndReceiveMultiFrameForHolden_1(int argc, unsigned char * argv, unsigned char * ans)
{
    unsigned char framesize, framesize1;
    unsigned char framenum;
    static int framepos, framepos1;
    unsigned int firstms;

    //unsigned int Allms;
    static unsigned char Isans;
    CMPHY_Relay Relaybuf;

    // 00 Datastreammode
    if (0 == argv[0])
    {
        Isans = 1;
    }
    else if (1 == Isans)
    {
        Isans = 0;
        return framepos;
    }

    framesize = argv[2];
    CMPHY_Relay_Init(&Relaybuf);
    Relaybuf.CommType = COMM_VPW_TYPE;              //通讯类型串行口
    Relaybuf.LevelType = COMM_LEVEL_12V;            //12V
    CMPHY_Relay_Reset();
    CMPHY_Relay_Set(&Relaybuf);
    VPW_Init();
    firstms = 0;
    framepos = 3;
    framenum = argv[1];

    //Allms = 0;
    if (framenum == 1) // SB 发一收多
    {
        if (false == VPW_Send(&argv[3], framesize))
        {
            ans[0] = 0xff;
            ans[1] = 0x02;
            return 2;
        }

        framesize = 0;
        framenum = 0;

        while ((firstms < 200) && (framenum < 20))
        {
            framesize = VPW_Receive(&ans[framepos], 200, &firstms, 200);

            if (framesize)
            {
                ans[framepos - 1] = framesize;
                framepos += framesize;
                framepos++;
                framenum++;
                firstms = 0;
            }
        }

        ans[1] = framenum;

        if (!framenum)
        {
            ans[0] = 0xff;
            ans[1] = 0x02;
            return 2;
        }
    }
    else // SB 发多收多
    {
        framepos1 = 0;
        ans[1] = framenum;

        while (framenum)
        {
            // ff 02 06 6c 10 f1 01 00 CHK 06 6c 10 f1 01 00 CHK
            framesize = argv[2 + framepos1];

            if (false == VPW_Send(&argv[3 + framepos1], framesize))
            {
                ans[0] = 0xff;
                ans[1] = 0x02;
                return 2;
            }

            framepos1++;
            framepos1 += framesize;

            // 17 02 07 6c f1 10 ee 01 e1 27 07 6c f1 10 ee 01 e1 27
            framesize1 = VPW_Receive(&ans[framepos], 200, &firstms, 200);

            if (framesize1)
            {
                ans[framepos - 1] = framesize1;
                framepos += framesize1;
                framepos++;
            }
            else
            {
                ans[0] = 0xff;
                ans[1] = 0x02;
                return 2;
            }

            framenum--;
        }
    }

    ans[0] = 0;
    framepos--;
    return framepos;
}


#endif

//--------------------------------------------------------- End Of File --------
