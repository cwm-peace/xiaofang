//小方1.0-巴掌宽护胸毛
/*
千叮咛万嘱咐：

我就是业余爱好，代码水平是真不行，我已经尽我所能写了备注，但遗憾的是有时候依然连我自己都读不懂，各位自求多福吧。
有些参数没搞明白之前最好不要改，不然会出些莫名其妙的问题。现在的代码刚好能跑，目前没有太大的bug，相对比较流畅。
我也在抽空不断优化，如果你是代码老司机可以帮忙优化优化。

我说说代码的大概原理

所有功能都放在一个大循环里，每个功能有个标志位变量，当这个变量为1时，表明当前选择的是这个功能，其他功能的标志位会被置零，所以在大循环里反复执行这个功能。
然后在每个功能里设置了手势检测，如果读取到进入菜单的操作（向后甩）就会立即进入菜单切换界面，通过左右倾斜选择功能，向上倾斜就确定进入该功能。这个菜单选择
操作就会设置功能的标志位变量，让其中一个为1，其他全为零。（除了温度、代码版本、声音开关这几个不用，这几个进入后，会自动回到进入菜单时的功能里。）


各功能描述
插上usb就会进入充电界面，会一直显示充电动画，直到充满，这个时候无法使用任何功能。只有拔下来才能进入后面的功能，每次插电会自动重启主控，拔下来后会开始自动校正6050的偏差，这个大概会等待10秒左右。
但后续休眠被唤醒就很快。


为了节约用电，给主控和6050都设置了无操作超时就进入休眠模式，但即便如此也只能待机两天。
在所有功能里，只要倒扣就会立即进入休眠，需要摇一摇唤醒。菜单状态里倒扣不会休眠。
任何姿态下，静止不动超过一定时间也会进入休眠，需要摇一摇唤醒。（温度显示除外）

关于显示驱动原理
设置了两级显示数据的缓存数组，两个数组大小一样。初级显存用来做一些碰撞检测之类的，次级显存用于最终上传到点阵驱动里显示出来。这样可以实现指定像素的闪烁效果而不会影响到游戏里的碰撞检测。


迷宫：
迷宫，大概原理就是用一个大数组作为大地图，屏幕分辨率只有8x8，所以肯定是无法显示完的，所以我定义了一个视野的概念，也就是点阵能显示的只是大地图的一部分，这部分视野
会随着玩家的移动而在大地图里移动，显示出大地图里对应区域的画面信息。每个地图设置了自己的终点区域，玩家只要到这个区域里就会通关，然后随机读取存储在flash里的其余七个地图之一。
你可以把玩家看作是一个钢珠，在地图里随着重力滚动，你往哪个方向倾斜它就会滚过去，所以玩的时候需要平放。

贪吃蛇：

贪吃蛇的实现原理我自己都说不清楚，反正是一点点地就拼凑出来了，大家空了自己看吧。也是重力感应，你往哪边倾斜，它就往哪边走，但不能倒退。
死亡时会显示得分，得分显示画面会跟随重力方向自动翻转。如果得分超过flash里记录的最高分，就会更新记录。破纪录动画还没写。

卜卦和摇骰子：

这个很简单，进去后摇一摇会随机出现对应画面。

沙漏：

上面的沙子掉完后会启动闹铃，闹铃状态下有任何移动就会退出闹铃然后重新开始。计时不太准确，但能设置每个沙子掉落的间隔时长，变量为shalou_speed，数字越大就越慢。
沙子掉落过程中，整个画面会随着放置方向自动改变画面方向，让未掉落的沙子保持在上方。实现方法就是在过程中加入重力方向判断的函数。



待优化

//表情没有做跟随重力方向显示的修改，只能固定一个方向显示
*/
//硬件定义---------------------------------------------------------------------------硬件定义//
//小喇叭输出 10 
//视野坐标系定义： 左下角为原点（0，0）到（7，7）                                            
//充电口所在方向定义为右，也就是ay（）小于0，左边大于0. 上ax()小于0，下ay（）大于0   
//玩家坐标遵循视野坐标系，玩家坐标范围4x4，左下角坐标定义为玩家的位置坐标，相对坐标范围（0，0）到（1，1）

//迷宫地图坐标系定义：
//大地图面积为6x6个视野范围，左上角定义为原点（0，0），右下角为（47，47），
//外围一圈视野区域定义为墙，是全部点亮的区域，无法通过

//陀螺仪的轴向和屏幕的方向关系（充电口为右）：
//imu.ax()对应上下，imu.ay（）对应左右
//硬件定义---------------------------------------------------------------------------硬件定义//



/*程序结构描述

显存：

显存分两级，初级view（是全局变量）、后级LedBuffer（类私有变量）。view用于储存游戏里出现的实体信息，但不一定会显示出来，LedBuffer的数据是最终显示的数据。

点阵驱动：
void setIntensity(int intensity);//设置显示屏的亮度，亮度范围0-15，千万不要尝试把亮度开到5以上持续5秒以上，很快就会冒烟（电路设计需要优化，电流承载能力不够）
void clearDisplay();//清空显示屏
void setLed(int x, int y, byte state);//设置指定坐标单个led的亮灭，row就是坐标x，col就是坐标y
void setLed_work(int x, int y, byte state);在后级显存里设置像素
byte getLedState(int x, int y);	//获取后级显存里指定坐标的状态，返回1或者0
byte getLedState_out(int x, int y, byte view[8]);//获取初级显存里指定坐标的状态，返回1或者0
void bitmap(byte buffer[8]);	//把位图信息拷贝到后级显存
void roll(byte lr,byte ud);//	//按某个方向滚动后级显存里的信息，左右，上下，左右速度，上下速度
void UpLoad();负责把LedBuffer里的数据显示到屏幕上



MPU_6050驱动：
低功耗运动中断：set_int() ，进入低功耗
加速度：ax(),ay(),az(),分别获取三个轴向的加速度，范围是-1到1的float格式数据.（原始数据是rawAx(),rawAy(),rawAz(),得到一个int格式的数据）
角速度：gx(),gy(),gz(),分别获取三个轴向的角速度，范围是？（原始数据是rawGx()，rawGy() ，rawGz(),得到一个int格式的数据 ,-40000~40000）
温度：temp()获取当前温度，单位摄氏度。（原始数据rawTemp()，获取温度数据，int格式）
误差：setBias() ，updateBias() ，这个没搞懂是啥意思


AT24C6（2k Byte空间，8页，每页256字节）：
随机地址写一个字节：void iic_write_byte(byte Dadr,byte Badr,byte data);//(页地址（0-7），字节地址（0-255），数据)在指定地址写入一个字节的内容
随机地址读一个字节：byte iic_read_byte(byte Dadr,byte Badr);//()读取指定位置的一个字节
声音开关存储地址：0页，136字节
贪吃蛇得分储存地址：0页，135字节

存储结构：


页地址   字节地址

0-地图0，贪吃蛇得分（0，135）声音开关（0，136）,138-255空
1-地图1，135-255，空
2-地图2，135-255，空
3-地图3，135-255，空
4-地图4，135-255，空
5-地图5，135-255，空
6-地图6，135-255，空
7-地图7，135-255，空


电源管理（ip5306ck）:
byte led_state();//统计一千次3个引脚的电平，通过占空比统计出四个灯的亮灭状态
byte bat_state();//判断当前电池电量（低电量或者充满）以及是否正在充电，低电量返回0，
充电状态判断：charge_state，1为充电，0为未充电

*/


///////////////////////////////////////////////////////////////////////////下面面是全局变量////////////////////////////////////////////////////////////////

#include "LowPower.h"//低功耗库
#include "she.h"//贪吃蛇类定义
#include "iic_16k.h"//AT24C16存储芯片驱动
#include "ip5306ck.h"//电源管理芯片驱动
#include "LedControl.h"//点阵驱动
#include <basicMPU6050.h> //陀螺仪驱动

/*
//备注：请安下面的顺序执行代码编译
//全部调试好后，下面三步涉及到的定义就注释掉，再编译上传
*/


//第一步，打开这个定义，这个用来安装点阵的时候测试点阵，点阵ok后就注释掉
#define DEBUG_LEDTEST


//第二步，打开这个定义，写入地图数据
//MAP_WRITE定义被开启时，执行地图数据写入操作，写入完成之后就注释掉，如果是首次写入完成会有短促的滴滴声，
//如果已经写过了会有长的滴滴声，总之任何声音出现了，说明地图数据已经写好了，就可以注释掉这个定义了
//#define MAP_WRITE


//第三步，自由调试，自选
//如果要调试就保留下面这句，否则就注释掉它（1.跳过充电循环，因为插着usb时它会一直充电，停在这个循环里。2.关闭陀螺仪校正，这个过程很费时间。）
//#define DEBUG


//第四步，串口调试开关，自选
//#define Serial_DEBUG
//像素结构体


#ifdef DEBUG_LEDTEST
  #define DEBUG
#endif

#ifdef MAP_WRITE
  #include "migong_ditu.h"
  struct migong_map0 mmp0;
  struct migong_map1 mmp1;
  struct migong_map2 mmp2;
  struct migong_map3 mmp3;
  struct migong_map4 mmp4;
  struct migong_map5 mmp5;
  struct migong_map6 mmp6;
  struct migong_map7 mmp7;
#endif


struct Mypiexl{
	byte x;//初始坐标
	byte y;
	byte x_old;//保留上一次的坐标
	byte y_old;
	byte x_now;//当前坐标
	byte y_now;

	//下面两个参数记录当前的速度和速度方向
	float speed_x_now;//两个轴向的速度
	float speed_y_now;
	float speed_x;//两个轴向的初始速度
	float speed_y;
	byte over_flag;//是否已经静止，静止为1，非静止为0
	
};

//迷宫地图结构体
struct migong_maps{

  byte qiangbi_0[8]={
              	0b11111111,//0
			          0b11111111,//1
			          0b11111111,//2
			          0b11111111,//3
			          0b11111111,//4
			          0b11111111,//5
		  	        0b11111111,//6
			          0b11111111// 7

  };

  byte * qiangbi[8]={
              	&qiangbi_0[0],//0
			          &qiangbi_0[1],//1
			          &qiangbi_0[2],//2
			          &qiangbi_0[3],//3
			          &qiangbi_0[4],//4
			          &qiangbi_0[5],//5
		  	        &qiangbi_0[6],//6
			          &qiangbi_0[7]// 7

  };


  byte map_data[128]={//迷宫大地图显存
  0b10000000,0b00000111,0b11111111,0b11111111,
  0b10000000,0b00000100,0b11110000,0b00000001,
  0b10011111,0b11111100,0b11110000,0b00000001,
  0b10000000,0b00000000,0b00000011,0b11110001,
  0b10000000,0b00000000,0b00000011,0b00000000,
  0b10011110,0b01111100,0b00110011,0b00000000,
  0b10010000,0b00000100,0b00110011,0b00000100,
  0b10010000,0b00000000,0b00000011,0b00000100,
  0b11110011,0b11110000,0b00000011,0b11100100,
  0b10000000,0b00110011,0b11111111,0b11100100,
  0b10000000,0b00111110,0b00001111,0b11100100,
  0b10011111,0b00111110,0b00001100,0b00000000,
  0b10011111,0b00000000,0b01001100,0b00000000,
  0b10000001,0b00000000,0b01001100,0b11111111,
  0b10000001,0b11111111,0b11001100,0b00000001,
  0b11111000,0b00000001,0b11001100,0b00000001,
  0b11111000,0b00000001,0b00000000,0b11111001,
  0b11111111,0b11111001,0b00000000,0b10000001,
  0b10000001,0b11100001,0b11001111,0b10000001,
  0b10000001,0b11100001,0b11111111,0b10011001,
  0b10011000,0b00100110,0b00000000,0b00011001,
  0b10011000,0b00100110,0b00000000,0b00011001,
  0b10011111,0b00000110,0b01110011,0b11111001,
  0b10000011,0b00000110,0b01110000,0b00001111,
  0b10000000,0b11100110,0b01110000,0b00001111,
  0b11100000,0b00000110,0b01111111,0b11001000,
  0b11100000,0b00000110,0b01000000,0b00001000,
  0b11100110,0b01100110,0b01000000,0b00001001,
  0b11100110,0b01100000,0b01001111,0b00111001,
  0b10000110,0b00100000,0b01000001,0b00000001,
  0b10000110,0b00111111,0b11000001,0b00000001,
  0b11111111,0b11111111,0b11000001,0b11111111,
  };

  byte * map_1[8]={
                &map_data[0],
                &map_data[4],
                &map_data[8],
                &map_data[12],
                &map_data[16],
                &map_data[20],
                &map_data[24],
                &map_data[28]
  };
  byte * map_2[8]={
                &map_data[1],
                &map_data[5],
                &map_data[9],
                &map_data[13],
                &map_data[17],
                &map_data[21],
                &map_data[25],
                &map_data[29]
  };
  byte * map_3[8]={
                &map_data[2],
                &map_data[6],
                &map_data[10],
                &map_data[14],
                &map_data[18],
                &map_data[22],
                &map_data[26],
                &map_data[30]
  };
  byte * map_4[8]={
                &map_data[3],
                &map_data[7],
                &map_data[11],
                &map_data[15],
                &map_data[19],
                &map_data[23],
                &map_data[27],
                &map_data[31]
  };

  byte * map_5[8]={
                &map_data[32],
                &map_data[36],
                &map_data[40],
                &map_data[44],
                &map_data[48],
                &map_data[52],
                &map_data[56],
                &map_data[60]
  };
  byte * map_6[8]={
                &map_data[33],
                &map_data[37],
                &map_data[41],
                &map_data[45],
                &map_data[49],
                &map_data[53],
                &map_data[57],
                &map_data[61]
  };
  byte * map_7[8]={
                &map_data[34],
                &map_data[38],
                &map_data[42],
                &map_data[46],
                &map_data[50],
                &map_data[54],
                &map_data[58],
                &map_data[62]
  };
  byte * map_8[8]={
                &map_data[35],
                &map_data[39],
                &map_data[43],
                &map_data[47],
                &map_data[51],
                &map_data[55],
                &map_data[59],
                &map_data[63]
  };


  byte * map_9[8]={
                &map_data[64],
                &map_data[68],
                &map_data[72],
                &map_data[76],
                &map_data[80],
                &map_data[84],
                &map_data[88],
                &map_data[92]
  };
  byte * map_10[8]={
                &map_data[65],
                &map_data[69],
                &map_data[73],
                &map_data[77],
                &map_data[81],
                &map_data[85],
                &map_data[89],
                &map_data[93]
  };
  byte * map_11[8]={
                &map_data[66],
                &map_data[70],
                &map_data[74],
                &map_data[78],
                &map_data[82],
                &map_data[86],
                &map_data[90],
                &map_data[94]
  };
  byte * map_12[8]={
                &map_data[67],
                &map_data[71],
                &map_data[75],
                &map_data[79],
                &map_data[83],
                &map_data[87],
                &map_data[91],
                &map_data[95]
  };



  byte * map_13[8]={
                &map_data[96],
                &map_data[100],
                &map_data[104],
                &map_data[108],
                &map_data[112],
                &map_data[116],
                &map_data[120],
                &map_data[124]
  };
  byte * map_14[8]={
                &map_data[97],
                &map_data[101],
                &map_data[105],
                &map_data[109],
                &map_data[113],
                &map_data[117],
                &map_data[121],
                &map_data[125]
  };
  byte * map_15[8]={
                &map_data[98],
                &map_data[102],
                &map_data[106],
                &map_data[110],
                &map_data[114],
                &map_data[118],
                &map_data[122],
                &map_data[126]
  };
  byte * map_16[8]={
                &map_data[99],
                &map_data[103],
                &map_data[107],
                &map_data[111],
                &map_data[115],
                &map_data[119],
                &map_data[123],
                &map_data[127]
  };


  byte * * map_all[6][6]={//总地图数组，[y][x],左上角为原点，全局坐标范围（0，0）到（47，47）
               {qiangbi,qiangbi,qiangbi,qiangbi,qiangbi,qiangbi},
               {qiangbi, map_1, map_2, map_3,  map_4,   qiangbi},
               {qiangbi, map_5, map_6, map_7,  map_8,   qiangbi},
               {qiangbi, map_9,map_10,map_11, map_12,   qiangbi},  
               {qiangbi, map_13,map_14,map_15,map_16,   qiangbi},  
               {qiangbi,qiangbi,qiangbi,qiangbi,qiangbi,qiangbi}
  };



  //初始化玩家坐标
  byte px=2;
  byte py=1;
  //初始化视野坐标
  byte x=24;
  byte y=32;
  //初始化目的地闪烁区域,地图全局坐标，左上方为原点
  byte mubiao_x=19;
  byte mubiao_y=8;
  byte map_index=0;//地图序号，从0开始

};




byte versions=1;//代码版本
byte v_page=0;//代码版本信息存储页地址
byte v_addr=137;//代码版本信息存储字节地址


basicMPU6050 <> imu;//创建陀螺仪实例
LedControl lc=LedControl(3,13,4);//创建点阵驱动类(DIN,CLK,CS)[19,18,17]
she myshe=she();//创建蛇实例
iic_16k myflash=iic_16k();//创建内存读写实例
ip5306ck bat=ip5306ck();//创建电量检测类

//当开启地图写入的时候，这些代码都不用编译，不然空间不够

//======================================================条件编译================================================================
#ifndef MAP_WRITE
byte view[]={0,0,0,0,0,0,0,0};//迷宫的视野范围，也是做为初级显存用，这里的数据可以经过处理后再送入LedBuffer显存，于是可以做一些画面实体互动或者碰撞检测之类的

byte she_defen_page=0;//贪吃蛇得分存储页地址
byte she_defen_addr=135;//贪吃蛇得分存储字地址
byte beeper_page=0;//声音开关存储页地址
byte beeper_addr=136;//声音开关存储字地址

byte beeper=1;//喇叭开关

int mpu_key_flag=1;//设置蛇初始运动方向为向上运动
int mpu_key_flag_a;
int mpu_key_flag_old;//记录上一次按键状态

unsigned long null_time;//无输入限时，超时自动关闭屏幕，直到有输入才复位。
unsigned long timeout=15000;//休眠倒计时每单位毫秒

int x_huiwu=0;//视野水平坐标，如果是四个屏幕范围则取值（0-24）
int power_down_flag=0;//是否进入低功耗的标志位
float input_old;//保存上一次的输入,用于蛇和迷宫游戏的休眠判断
float input_new;//保留当前输入，用于蛇和迷宫游戏的休眠判断
int speed=300;//设置蛇的初始速度为400
int she_state=0;//蛇的生命状态，1为生，0为死
int migong_state=0;//迷宫的生命状态，1为生，0为死
const int t_xy_time=4;//地图坐标闪烁频率
const int t_time=2;//玩家闪烁频率
float pax;//迷宫专用暂存陀螺仪输出结果
float pay;
int guaxiang=0;//八卦选中开关
int shaizi=0;//骰子选中开关
int shalou=1;//沙漏计时器开关
int shoudiantong=0;//手电筒开关
int tem=0;//温度计开关
int beeper_state=0;//声音操作标记，操作过会被置1，没操作过会被置0。
byte map_0[8];//map_offset()函数专用的存储空间

byte lishidefen;//用来读取贪吃蛇历史最高纪录

int game_old_state=4;//存储上次退出菜单时所处位置，再次返回菜单的时候回到该位置（0迷宫，1贪吃蛇，2卜卦，3骰子，4沙漏，5声音，6版本号）

struct Mypiexl piexl_1;//定义一个像素结构
struct migong_maps migong_map;// 定义一个迷宫地图结构体
int migong_speed=16;//12;//6;//这个参数控制迷宫移动延时，数字越大动作就越慢
int shalou_speed=1000;//9600;//1920;//这个参数控制沙漏每个像素掉落的间隔时间
float ay=9.8;//沙漏下落加速度
float time_speed=0.25;//0.15;//沙漏时间切片
float time_pass=0;
int gaodu=350;//沙漏实际高度
byte zhonglifangxiang=3;//用来定义重力方向,现在用方向3就正确，用别的就有问题
byte zhonglifangxiang_old=zhonglifangxiang;

float a_x;//用于姿态判断
float a_y;//用于姿态判断


byte xx=5;//沙漏专用
byte yy=7;//沙漏专用
byte view_tem[8];//用于进入菜单时临时存储初级显存



//真实物理像素结构体


//数字字库
byte num_0[]={  
                0B11100000,//0//0%-3%
			          0B10100000,//1
			          0B10100000,//2
			          0B10100000,//3
			          0B11100000,//4
			          0B00000000,//5
		  	        0B00000000,//6
			          0B00000000// 7
  //            x 01234567

};
byte num_1[]={  
                0B01000000,//0//0%-3%
			          0B01000000,//1
			          0B01000000,//2
			          0B01000000,//3
			          0B01000000,//4
			          0B00000000,//5
		  	        0B00000000,//6
			          0B00000000// 7
  //            x 01234567

};
byte num_2[]={  
                0B11100000,//0//0%-3%
			          0B00100000,//1
			          0B11100000,//2
			          0B10000000,//3
			          0B11100000,//4
			          0B00000000,//5
		  	        0B00000000,//6
			          0B00000000// 7
  //            x 01234567

};
byte num_3[]={  
                0B11100000,//0//0%-3%
			          0B00100000,//1
			          0B11100000,//2
			          0B00100000,//3
			          0B11100000,//4
			          0B00000000,//5
		  	        0B00000000,//6
			          0B00000000// 7
  //            x 01234567

};
byte num_4[]={  
                0B10100000,//0//0%-3%
			          0B10100000,//1
			          0B11100000,//2
			          0B00100000,//3
			          0B00100000,//4
			          0B00000000,//5
		  	        0B00000000,//6
			          0B00000000// 7
  //            x 01234567

};
byte num_5[]={  
                0B11100000,//0
			          0B10000000,//1
			          0B11100000,//2
			          0B00100000,//3
			          0B11100000,//4
			          0B00000000,//5
		  	        0B00000000,//6
			          0B00000000// 7
  //            x 01234567

};
byte num_6[]={  
                0B11100000,//0//0%-3%
			          0B10000000,//1
			          0B11100000,//2
			          0B10100000,//3
			          0B11100000,//4
			          0B00000000,//5
		  	        0B00000000,//6
			          0B00000000// 7
  //            x 01234567

};
byte num_7[]={  
                0B11100000,//0//0%-3%
			          0B00100000,//1
			          0B00100000,//2
			          0B00100000,//3
			          0B00100000,//4
			          0B00000000,//5
		  	        0B00000000,//6
			          0B00000000// 7
  //            x 01234567

};
byte num_8[]={  
                0B11100000,//0//0%-3%
			          0B10100000,//1
			          0B11100000,//2
			          0B10100000,//3
			          0B11100000,//4
			          0B00000000,//5
		  	        0B00000000,//6
			          0B00000000// 7
  //              x 01234567

};
byte num_9[]={  
                0B11100000,//0//0%-3%
			          0B10100000,//1
			          0B11100000,//2
			          0B00100000,//3
			          0B11100000,//4
			          0B00000000,//5
		  	        0B00000000,//6
			          0B00000000// 7
  //            x 01234567

};
//数字字库数组汇总
byte * num_bitmap[10]={num_0,num_1,num_2,num_3,num_4,num_5,num_6,num_7,num_8,num_9};

//游戏界面
byte game_0[]={ 
                0B00000000,//0  迷宫
			          0B01010110,//1
			          0B01011010,//2
			          0B01000010,//3
			          0B00111010,//4
			          0B00100010,//5
		  	        0B01101110,//6
			          0B00000000// 7
  //              x 01234567

};
byte game_1[]={ 
                0B00000000,//0  贪吃蛇
			          0B01110100,//1
			          0B01000000,//2
			          0B01111110,//3
			          0B00000010,//4
			          0B00011110,//5
		  	        0B00010000,//6
			          0B00000000// 7
  //              x 01234567

};
byte game_2[]={ //八卦
                0B00000000,//0
			          0B01100110,//1
			          0B00000000,//2
			          0B01100110,//3
			          0B00000000,//4
			          0B01111110,//5
		  	        0B00000000,//6
			          0B00000000// 7
    //    
};
byte game_3[]={ //摇骰子
                0B00000000,//0
			          0B01100110,//1
			          0B01100110,//2
			          0B00011000,//3
			          0B00011000,//4
			          0B01100110,//5
		  	        0B01100110,//6
			          0B00000000// 7
  //            x 01234567
};
byte game_4[]={ //沙漏计时器
                0B00000000,//0  
			          0B01111110,//1
			          0B00111100,//2
			          0B00011000,//3
			          0B00011000,//4
			          0B00111100,//5
		  	        0B01111110,//6
			          0B00000000// 7
  //              x 01234567

};

byte game_5[]={ //温度
                0B00000000,//0  
			          0B01100000,//1
			          0B01101110,//2
			          0B00010000,//3
			          0B00010000,//4
			          0B00010000,//5
		  	        0B00001110,//6
			          0B00000000// 7
  //            x 01234567

};
byte game_6[]={ //声音开
                0B00000000,//0  
			          0B00011000,//1
			          0B00001100,//2
			          0B00001010,//3
			          0B00011000,//4
			          0B00111000,//5
		  	        0B00110000,//6
			          0B00000000// 7
  //            x 01234567

};
byte game_7[]={ //代码版本
  0b00000000,
  0b00000000,
  0b01100110,
  0b01100110,
  0b00000000,
  0b01000010,
  0b00111100,
  0b00000000,

};

byte *game_bitmap[]={game_0,game_1,game_2,game_3,game_4,game_5,game_6,game_7,game_2,game_2,game_2};//这里很奇怪，如果尾巴上不多放几个成员，就会出现显示不完整的问题，不知道为什么

//电池电量
byte bat_0[]={  
                0B00000000,//0
			          0B00011000,//1
			          0B00100100,//2
			          0B00100100,//3
			          0B00100100,//4
			          0B00100100,//5
		  	        0B00111100,//6
			          0B00000000// 7
  //            x 01234567

};
byte bat_1[]={  
                0B00000000,//0
			          0B00011000,//1
			          0B00100100,//2
			          0B00100100,//3
			          0B00100100,//4
			          0B00111100,//5
		  	        0B00111100,//6
			          0B00000000// 7
  //            x 01234567

};
byte bat_2[]={  
                0B00000000,//0
			          0B00011000,//1
			          0B00100100,//2
			          0B00100100,//3
			          0B00111100,//4
			          0B00111100,//5
		  	        0B00111100,//6
			          0B00000000// 7
  //            x 01234567

};
byte bat_3[]={  
                0B00000000,//0
			          0B00011000,//1
			          0B00100100,//2
			          0B00111100,//3
			          0B00111100,//4
			          0B00111100,//5
		  	        0B00111100,//6
			          0B00000000// 7
  //            x 01234567
};
byte bat_4[]={  
                0B00000000,//0
			          0B00011000,//1
			          0B00111100,//2
			          0B00111100,//3
			          0B00111100,//4
			          0B00111100,//5
		  	        0B00111100,//6
			          0B00000000// 7
  //            x 01234567

};
byte bat_5[]={  
                0B00000000,//0
			          0B00011000,//1
			          0B00111100,//2
			          0B00111100,//3
			          0B00111100,//4
			          0B00111100,//5
		  	        0B00111100,//6
			          0B00000000// 7
  //            x 01234567

};
byte bat_6[]={  
                0B00000000,//0//
			          0B00011000,//1
			          0B00111100,//2
			          0B00111100,//3
			          0B00111100,//4
			          0B00111100,//5
		  	        0B00111100,//6
			          0B00000000// 7
  //            x 01234567

};
byte *bat_bitmap[7]={bat_0,bat_1,bat_2,bat_3,bat_4,bat_5,bat_6};


//摇骰子图形
byte dian1[8]={
                0B00000000,//0
			          0B00011000,//1
			          0B00111100,//2
			          0B01111110,//3
			          0B01111110,//4
			          0B00111100,//5
		  	        0B00011000,//6
			          0B00000000// 7
  //            x 01234567
  };

byte dian2[8]={
                0B00000110,//0
			          0B00001111,//1
			          0B00001111,//2
			          0B00000110,//3
			          0B01100000,//4
			          0B11110000,//5
		  	        0B11110000,//6
			          0B01100000// 7
  //            x 01234567
  };

byte dian3[8]={
                0B00011000,//0
			          0B00111100,//1
			          0B00111100,//2
			          0B00011000,//3
			          0B11000011,//4
			          0B11100111,//5
		  	        0B11100111,//6
			          0B11100111// 7
  //            x 01234567
  };

byte dian4[8]={
                0B11100111,//0
			          0B11100111,//1
			          0B11100111,//2
			          0B00000000,//3
			          0B00000000,//4
			          0B11100111,//5
		  	        0B11100111,//6
			          0B11100111// 7
  //            x 01234567
	};

byte dian5[8]={
                0B11100111,//0
			          0B11100111,//1
			          0B11011011,//2
			          0B00111100,//3
			          0B00111100,//4
			          0B11011011,//5
		  	        0B11100111,//6
			          0B11100111// 7
  //            x 01234567
	};

byte dian6[8]={
                0B11100111,//0
			          0B11100111,//1
			          0B00000000,//2
			          0B11100111,//3
			          0B11100111,//4
			          0B00000000,//5
		  	        0B11100111,//6
			          0B11100111// 7
  //            x 01234567
	};

byte *shaizi_all[6]={dian1,dian2,dian3,dian4,dian5,dian6};   



//八卦图形

  //乾
byte qian[8]={
                0B11111111,//0
			          0B11111111,//1
			          0B00000000,//2
			          0B11111111,//3
			          0B11111111,//4
			          0B00000000,//5
		  	        0B11111111,//6
			          0B11111111// 7
    //          x 01234567
	};
  //坤
byte kun[8]={ 
                0B11100111,//0
			          0B11100111,//1
			          0B00000000,//2
			          0B11100111,//3
			          0B11100111,//4
			          0B00000000,//5
		  	        0B11100111,//6
			          0B11100111// 7
    //          x 01234567
	};
  //震
byte zhen[8]={
                0B11100111,//0
			          0B11100111,//1
			          0B00000000,//2
			          0B11100111,//3
			          0B11100111,//4
			          0B00000000,//5
		  	        0B11111111,//6
			          0B11111111// 7
    //          x 01234567
	};
  //艮
byte gen[8]={ 
                0B11111111,//0
			          0B11111111,//1
			          0B00000000,//2
			          0B11100111,//3
			          0B11100111,//4
			          0B00000000,//5
		  	        0B11100111,//6
			          0B11100111// 7
    //          x 01234567
	};
  //离
byte li[8]={
                0B11111111,//0
			          0B11111111,//1
			          0B00000000,//2
			          0B11100111,//3
			          0B11100111,//4
			          0B00000000,//5
		  	        0B11111111,//6
			          0B11111111// 7
    //          x 01234567
	};
  //坎
byte kan[8]={ 
                0B11100111,//0
			          0B11100111,//1
			          0B00000000,//2
			          0B11111111,//3
			          0B11111111,//4
			          0B00000000,//5
		  	        0B11100111,//6
			          0B11100111// 7
    //          x 01234567
	};
  //兑
byte dui[8]={ 
                0B11100111,//0
			          0B11100111,//1
			          0B00000000,//2
			          0B11111111,//3
			          0B11111111,//4
			          0B00000000,//5
		  	        0B11111111,//6
			          0B11111111// 7
    //          x 01234567
	};
  //巽
byte xun[8]={ 
                0B11111111,//0
			          0B11111111,//1
			          0B00000000,//2
			          0B11111111,//3
			          0B11111111,//4
			          0B00000000,//5
		  	        0B11100111,//6
			          0B11100111// 7
    //          x 01234567
	};

byte *bagua[]={qian,kun,zhen,gen,li,kan,dui,xun};



//全屏点亮图，用于沙漏计时结束的函数
byte all_led_on[]={
  0b11111111,
  0b11111111,
  0b11111111,
  0b11111111,
  0b11111111,
  0b11111111,
  0b11111111,
  0b11111111

};
///////////////焊接点阵前先用这个测试函数验证点阵的引脚方向是否装对，如果装对了，所有灯都能亮///////////////

//点阵测试函数，焊接点阵之前用一次
void LED_TEST(){
  lc.setIntensity(0);
  while(1){
          lc.bitmap(all_led_on);
          lc.UpLoad(); 
  }
}
#endif
//======================================================条件编译================================================================

///////////////////////////////////////////////////////////////////////////上面是全局变量////////////////////////////////////////////////////////////////





/////////////////////////////////////////////////////////////////////////下面是自定义函数区域///////////////////////////////////////////////////////////////////



//======================================================条件编译================================================================
#ifndef MAP_WRITE
////////////////////研发中的功能//////////////////    
//用指针方式操作数组，功能：偏移图像
void map_offset(byte * map_1,byte * map_2,byte x,byte y){//原数组，目标数组，偏移，传递一个数组的指针给函数，返回一个数组的指针，但这个数组要定义为全局变量才行


  if(x!=0){
    for(int i=0;i<8;i++){
      map_2[i]=map_1[i]>>x;
    }
  }else if(x==0){
    for(int i=0;i<8;i++){
      map_2[i]=map_1[i];
    }
  }


  if(y!=0){
    for(int i=7;i>=0;i--){
      if(i>=y){
        map_2[i]=map_2[i-y];
      }else if(i<y){
        map_2[i]=0b00000000;
      }
    }
  }

    
}
////////////////////研发中的功能//////////////////


//沙漏结束时的画面和声音
void shalou_over(){
  move_or_not();
  lc.clearDisplay();
  lc.setIntensity(8);
  int dt=800;
  int i=0;
  int timeout_over=25000;//这里设置闹钟持续时长，如果没有任何操作在这个时间倒计时结束时，闹钟也结束，避免一直卡在这里。
  while((!move_or_not())&&timeout_over){
    
    if(i==0){
      for(int i=0;i<3;i++){
        tone(10,6000,100);
        lc.bitmap(all_led_on);
        lc.UpLoad();
        delay(1);
        lc.clearDisplay();   
        delay(100);
        noTone(10);
        delay(20);
      }
      lc.setIntensity(2);  
      lc.bitmap(all_led_on);
      lc.UpLoad();
      tone(10,6000,150);  
      delay(50);  
      lc.clearDisplay();      
      delay(100);
      noTone(10);
      lc.setIntensity(8);
      i=dt;
    }
    i--;
    if(timeout_over>0){ 

      timeout_over--;
    }
  }
  
  lc.setIntensity(0);
}


//表情会跟随重力方向变化
//休眠表情
void biaoqing_shufu(){
  byte beeper_old=beeper;
  beeper=1;
  byte eyes_x=1;
  byte eyes_y=4;

  //while(1){//动作循环开始
  ///////////东张西望
  //呆滞嘴
  lc.clearDisplay_work();
  //lc.clearDisplay();
  zui_daizhi_draw();
  //呆滞眼
  eyes_dai_draw(eyes_x,eyes_y);
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();
       //设置每次动画结束后的间隔时长
        for(int i=0;i<500;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }

  if(beeper){
    tone(10,6000,50);
  }
  //呆滞眼左看
  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_dai_draw(eyes_x-1,eyes_y);
  zui_daizhi_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

       //设置每次动画结束后的间隔时长
        for(int i=0;i<400;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }


  //呆滞眼右看
  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_dai_draw(eyes_x,eyes_y);
  zui_daizhi_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();
       //设置每次动画结束后的间隔时长
        for(int i=0;i<10;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }


  lc.clearDisplay_work();
  
  if(beeper){
   tone(10,6000,50);
  }
  eyes_dai_draw(eyes_x+1,eyes_y);
  zui_daizhi_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

       //设置每次动画结束后的间隔时长
        for(int i=0;i<500;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }


  //眼神复位
  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_dai_draw(eyes_x,eyes_y);
  zui_daizhi_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();
       //设置每次动画结束后的间隔时长
        for(int i=0;i<500;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }


  /////////////眨眼后微笑再眨眼
  //微笑
  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_dai_draw(eyes_x,eyes_y);
  zui_xiao_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();
       //设置每次动画结束后的间隔时长
        for(int i=0;i<1000;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }


  //眨眼
  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_bi_draw();
  zui_xiao_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();
       //设置每次动画结束后的间隔时长
        for(int i=0;i<80;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }

  lc.clearDisplay_work();
  //lc.clearDisplay();
  if(beeper){
    tone(10,6000,50);
  }
  eyes_dai_draw(eyes_x,eyes_y);
  zui_xiao_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

        //设置每次动画结束后的间隔时长
        for(int i=0;i<500;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }
  //眨眼
  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_bi_draw();
  zui_xiao_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();
       //设置每次动画结束后的间隔时长
        for(int i=0;i<80;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }

  lc.clearDisplay_work();
  //lc.clearDisplay();
  if(beeper){
    tone(10,6000,50);
  }
  eyes_dai_draw(eyes_x,eyes_y);
  zui_xiao_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

       //设置每次动画结束后的间隔时长
        for(int i=0;i<500;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }
  //眨眼


  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_bi_draw();
  zui_xiao_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();
       //设置每次动画结束后的间隔时长
        for(int i=0;i<80;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }

  lc.clearDisplay_work();
  //lc.clearDisplay();
  if(beeper){
    tone(10,6000,50);
  }
  eyes_dai_draw(eyes_x,eyes_y);
  zui_xiao_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

       //设置每次动画结束后的间隔时长
        for(int i=0;i<900;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }


  /////////////平静地说话
  //呆滞嘴
  lc.clearDisplay_work();
  //lc.clearDisplay();
  if(beeper){
    tone(10,random(3000,9000),50);
  }
  eyes_dai_draw(eyes_x,eyes_y);
  zui_daizhi_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

       //设置每次动画结束后的间隔时长
        for(int i=0;i<100;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }
  //嘟嘴


  lc.clearDisplay_work();
  //lc.clearDisplay();
  if(beeper){
    tone(10,random(3000,9000),50);
  }
  eyes_dai_draw(eyes_x,eyes_y);
  zui_duzui_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

       //设置每次动画结束后的间隔时长
        for(int i=0;i<200;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }
  //呆滞嘴


  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_dai_draw(eyes_x,eyes_y);
  zui_daizhi_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();
  //嘟嘴


  lc.clearDisplay_work();
  //lc.clearDisplay();
  if(beeper){
    tone(10,random(3000,9000),50);
  }
  eyes_dai_draw(eyes_x,eyes_y);
  zui_duzui_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

       //设置每次动画结束后的间隔时长
        for(int i=0;i<200;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }
  //呆滞嘴


  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_dai_draw(eyes_x,eyes_y);
  zui_daizhi_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();
  //嘟嘴



  lc.clearDisplay_work();
  //lc.clearDisplay();
  if(beeper){
    tone(10,random(3000,9000),50);
  }
  eyes_dai_draw(eyes_x,eyes_y);
  zui_duzui_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

       //设置每次动画结束后的间隔时长
        for(int i=0;i<200;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }
  //呆滞嘴


  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_dai_draw(eyes_x,eyes_y);
  zui_daizhi_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

  //说第二句

  //呆滞嘴
  lc.clearDisplay_work();
  //lc.clearDisplay();
  if(beeper){
    tone(10,random(3000,9000),50);
  }
  eyes_dai_draw(eyes_x,eyes_y);
  zui_daizhi_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

       //设置每次动画结束后的间隔时长
        for(int i=0;i<100;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }
  //嘟嘴


  lc.clearDisplay_work();
  //lc.clearDisplay();
  if(beeper){
    tone(10,random(3000,9000),50);
  }
  eyes_dai_draw(eyes_x,eyes_y);
  zui_duzui_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

       //设置每次动画结束后的间隔时长
        for(int i=0;i<200;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }
  //呆滞嘴


  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_dai_draw(eyes_x,eyes_y);
  zui_daizhi_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();
  //嘟嘴


  lc.clearDisplay_work();
  //lc.clearDisplay();
  if(beeper){
    tone(10,random(3000,9000),50);
  }
  eyes_dai_draw(eyes_x,eyes_y);
  zui_duzui_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

       //设置每次动画结束后的间隔时长
        for(int i=0;i<200;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }
  //呆滞嘴


  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_dai_draw(eyes_x,eyes_y);
  zui_daizhi_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();
  //嘟嘴



  lc.clearDisplay_work();
  //lc.clearDisplay();
  if(beeper){
    tone(10,random(3000,9000),50);
  }
  eyes_dai_draw(eyes_x,eyes_y);
  zui_duzui_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

       //设置每次动画结束后的间隔时长
        for(int i=0;i<200;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }
  //呆滞嘴


  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_dai_draw(eyes_x,eyes_y);
  zui_daizhi_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();



       //设置每次动画结束后的间隔时长
        for(int i=0;i<800;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }

  ///////////////眨眼等待
  //眨眼
  lc.clearDisplay_work();
  //lc.clearDisplay();
  if(beeper){
    tone(10,8000,50);
  }
  eyes_bi_draw();
  zui_daizhi_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

       //设置每次动画结束后的间隔时长
        for(int i=0;i<100;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }


  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_dai_draw(eyes_x,eyes_y);
  zui_daizhi_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();
       //设置每次动画结束后的间隔时长
        for(int i=0;i<700;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }
  //眨眼

  lc.clearDisplay_work();
  //lc.clearDisplay();
  if(beeper){
    tone(10,5000,50);
  }
  eyes_bi_draw();
  zui_daizhi_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

       //设置每次动画结束后的间隔时长
        for(int i=0;i<100;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }


  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_dai_draw(eyes_x,eyes_y);
  zui_daizhi_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();
       //设置每次动画结束后的间隔时长
        for(int i=0;i<1000;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }

  beeper=beeper_old;
  //重复整个过程
  //}//循环结束
}//函数结束

//唤醒表情
void biaoqing_wakeup(){
  
  byte beeper_old=beeper;
  beeper=1;
  byte eyes_x=1;
  byte eyes_y=4;
  lc.clearDisplay_work();
  ///////////////眨眼等待

  //眨眼
  //lc.clearDisplay();
  if(beeper){
    tone(10,8000,50);
  }
  eyes_bi_draw();
  zui_daizhi_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

       //设置每次动画结束后的间隔时长
        for(int i=0;i<100;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }

  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_dai_draw(eyes_x,eyes_y);
  zui_daizhi_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();
       //设置每次动画结束后的间隔时长
        for(int i=0;i<700;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }
  //眨眼
  lc.clearDisplay_work();
  //lc.clearDisplay();
  if(beeper){
    tone(10,5000,50);
  }
  eyes_bi_draw();
  zui_daizhi_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

       //设置每次动画结束后的间隔时长
        for(int i=0;i<100;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }
  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_dai_draw(eyes_x,eyes_y);
  zui_daizhi_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();
       //设置每次动画结束后的间隔时长
        for(int i=0;i<1000;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(lc.LedBuffer_work);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }

  beeper=beeper_old;

}

void biaoqing_she_pojilu(){
  byte beeper_old=beeper;
  beeper=1;
  byte eyes_x=1;
  byte eyes_y=4;


  //惊讶嘴
  lc.clearDisplay_work();
  zui_jingkong_draw();
  //呆滞眼
  eyes_dai_draw(eyes_x,eyes_y);
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();
  delay(500);

 
  //眨眼
  lc.clearDisplay_work();
  if(beeper){
    tone(10,8000,50);
  }
  eyes_bi_draw();
  zui_jingkong_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

  delay(100);
  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_dai_draw(eyes_x,eyes_y);
  zui_jingkong_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();
  delay(700);

  //眨眼
  lc.clearDisplay_work();
  if(beeper){
    tone(10,8000,50);
  }
  eyes_bi_draw();
  zui_jingkong_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

  delay(100);
  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_dai_draw(eyes_x,eyes_y);
  zui_jingkong_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();
  delay(700);

  //眨眼
  lc.clearDisplay_work();
  if(beeper){
    tone(10,8000,50);
  }
  eyes_bi_draw();
  zui_jingkong_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();

  delay(100);
  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_dai_draw(eyes_x,eyes_y);
  zui_jingkong_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();
  delay(700);


  //微笑
  lc.clearDisplay_work();
  //lc.clearDisplay();
  eyes_dai_draw(eyes_x,eyes_y);
  zui_xiao_draw();
  zhonglifangxiang_panduan();
  lc.bitmap(lc.LedBuffer_work);
  lc.UpLoad();
  delay(1000);
  beeper=beeper_old;



}
//5种眼睛和6种嘴形，可生成30种表情
//画眼睛=======================
  //呆滞眼/////////////////1

void eyes_dai_draw(byte eyes_x,byte eyes_y){
  //画左眼
  lc.setLed_work(eyes_x,eyes_y,1);
  lc.setLed_work(eyes_x,eyes_y+1,1);
  lc.setLed_work(eyes_x+1,eyes_y,1);
  lc.setLed_work(eyes_x+1,eyes_y+1,1);
  //画右眼
  lc.setLed_work(eyes_x+4,eyes_y,1);
  lc.setLed_work(eyes_x+4,eyes_y+1,1);
  lc.setLed_work(eyes_x+5,eyes_y,1);
  lc.setLed_work(eyes_x+5,eyes_y+1,1);
}


  //闭眼/////////////////2

void eyes_bi_draw(){
  //画左眼
  lc.setLed_work(1,4,1);
  lc.setLed_work(2,4,1);
  lc.setLed_work(0,4,1);

  //画右眼
  lc.setLed_work(5,4,1);
  lc.setLed_work(6,4,1);
  lc.setLed_work(7,4,1);

}
//大笑眼3
void eyes_xiao_draw(){
  //画左眼
  lc.setLed_work(0,4,1);
  lc.setLed_work(1,5,1);
  lc.setLed_work(2,4,1);
  //画右眼
  lc.setLed_work(5,4,1);
  lc.setLed_work(6,5,1);
  lc.setLed_work(7,4,1);

}
//生气眼4
void eyes_shengqi_draw(){
  //画左眼
  lc.setLed_work(1,4,1);
  lc.setLed_work(1,6,1);
  lc.setLed_work(2,5,1);
  lc.setLed_work(3,4,1);
  //画右眼
  lc.setLed_work(4,4,1);
  lc.setLed_work(5,5,1);
  lc.setLed_work(6,6,1);
  lc.setLed_work(6,4,1);

}
  //虚眼/////////////////5
void eyes_xu_draw(byte eyes_x){
  //画左眼
  lc.setLed_work(eyes_x, 3, 1);//画眼珠
  lc.setLed_work(1,4,1);
  lc.setLed_work(2,4,1);
  lc.setLed_work(0,4,1);

  //画右眼
  lc.setLed(eyes_x+5, 3, 1);//画眼珠
  lc.setLed(5,4,1);
  lc.setLed(6,4,1);
  lc.setLed(7,4,1);

}



//画嘴巴=======================
  //画嘴巴呆滞1
void zui_daizhi_draw(){
  lc.setLed_work(3,2,1);
  lc.setLed_work(4,2,1);
}

  //画嘴巴无奈2
void zui_wulai_draw(){
  lc.setLed_work(2,1,1);
  lc.setLed_work(3,1,1);
  lc.setLed_work(4,1,1);
  lc.setLed_work(5,1,1);
}


  //画嘴巴嘟嘴3
void zui_duzui_draw(){
  lc.setLed_work(3,1,1);
  lc.setLed_work(3,2,1);
  lc.setLed_work(4,1,1);
  lc.setLed_work(4,2,1);
}
  //画嘴巴惊恐，张大嘴巴4
void zui_jingkong_draw(){
  lc.setLed_work(2,1,1);
  lc.setLed_work(2,2,1);
  lc.setLed_work(3,0,1);
  lc.setLed_work(3,3,1);
  lc.setLed_work(4,0,1);
  lc.setLed_work(4,3,1);
  lc.setLed_work(5,1,1);
  lc.setLed_work(5,2,1);
}


  //画嘴巴笑5
void zui_xiao_draw(){
  lc.setLed_work(3,1,1);
  lc.setLed_work(4,1,1);
  lc.setLed_work(2,2,1);
  lc.setLed_work(5,2,1);
}
  //画嘴巴生气6
void zui_shegnqi_draw(){
  lc.setLed_work(3,2,1);
  lc.setLed_work(4,2,1);
  lc.setLed_work(2,1,1);
  lc.setLed_work(5,1,1);
}


//////////////////////////////////////////////音效函数//////////////////////////

void caidan_beeper(){//菜单选择音效
  if(beeper){
      tone(10,1500,100);
      delay(200);
  }else{
    delay(200);
  }
}
void caidan_beeper2(){//菜单选择音效
  if(beeper){
      tone(10,100,50);
      noTone(10);
      delay(50);
  }
  
}
void caidan_beeper3(){//确认音效
  if(beeper){
    for(int i=400;i<2000;i+=100){
      tone(10,i,50);
      delay(10);
    }
  }
}
void caidan_beeper4(){//进入菜单音效
  if(beeper){
    for(int i=3000;i>200;i-=200){
      tone(10,i,50);
      delay(10);
    }
  }
}
void bagua_beeper(){//八卦音效
  if(beeper){
    for(int i=3000;i>200;i-=400){
      tone(10,i,50);
      delay(10);
    }
  }
}
void my_beeper(){//迷宫移动发声函数
    if(beeper){
      tone(10,5000,50);
      delay(50);
      noTone(10);
    }else{
      delay(50);//为了不影响游戏速度
    }
  
}
void power_sleep_beeper(){//休眠开启音效，捕手声音开关控制
  //if(beeper){
      tone(10,8000,100);
      delay(100);
      tone(10,2500,100);
      delay(100);
      tone(10,800,100);
      delay(100);
      noTone(10); 
  //}
}
void power_on_beerper(){//开机声音，不受声音开关控制

   //if(beeper){
      tone(10,800,200);//引脚、频率、持续时长
      delay(200);
      tone(10,2500,100);
      delay(100);
      tone(10,8000,200);
      delay(200);
      noTone(10);
   //}
      
}
void wakeup_beerper(){//唤醒声音,不受声音开关控制
  //if(beeper){
      tone(10,1500,200);
      delay(200);
      tone(10,8000,200);
      delay(200);
      noTone(10);
  //}
}
/////////////////////////////////////////////音效函数//////////////////////////
#endif
//======================================================条件编译================================================================









///////////////////////////////系统函数////////////////////

void version_w(){//代码版本管理
  myflash.iic_write_byte(v_page,v_addr,versions);   
}

//flash读函数
byte flash_r(byte page,byte addr){//页，字节
         //Wire.end();//空出iic总线      
         return  myflash.iic_read_byte(page,addr);    
}
//flash写函数
void flash_w(byte page,byte addr,byte data){//页，字节，数据,    
         myflash.iic_write_byte(page,addr,data);    
}



//======================================================条件编译================================================================
#ifdef MAP_WRITE
//迷宫地图写入函数
void map_w(){
  byte map_w_YN;//用于判断是否已经写如果地图，如果写如果就不执行后续代码
  map_w_YN=myflash.iic_read_byte(7,255);//读取flash最末尾一字节的值，如果等于100，就说明之前已经写过了，否则就写入
  if(map_w_YN!=100){
    //写入mmp0
    for(int i=0;i<128;i++){
        myflash.iic_write_byte(mmp0.map_index,i,mmp0.map_data[i]);    
    }
    //写入玩家坐标信息
    myflash.iic_write_byte(mmp0.map_index,128,mmp0.px); 
    myflash.iic_write_byte(mmp0.map_index,129,mmp0.py); 
    //写入视野初始化坐标
    myflash.iic_write_byte(mmp0.map_index,130,mmp0.x); 
    myflash.iic_write_byte(mmp0.map_index,131,mmp0.y); 
    //写入目标区域坐标
    myflash.iic_write_byte(mmp0.map_index,132,mmp0.mubiao_x); 
    myflash.iic_write_byte(mmp0.map_index,133,mmp0.mubiao_y); 
  

  //写入mmp1
    for(int i=0;i<128;i++){
        myflash.iic_write_byte(mmp1.map_index,i,mmp1.map_data[i]);    
    }
    //写入玩家坐标信息
    myflash.iic_write_byte(mmp1.map_index,128,mmp1.px); 
    myflash.iic_write_byte(mmp1.map_index,129,mmp1.py); 
    //写入视野初始化坐标
    myflash.iic_write_byte(mmp1.map_index,130,mmp1.x); 
    myflash.iic_write_byte(mmp1.map_index,131,mmp1.y); 
    //写入目标区域坐标
    myflash.iic_write_byte(mmp1.map_index,132,mmp1.mubiao_x); 
    myflash.iic_write_byte(mmp1.map_index,133,mmp1.mubiao_y); 


  //写入mmp2
    for(int i=0;i<128;i++){
        myflash.iic_write_byte(mmp2.map_index,i,mmp2.map_data[i]);    
    }
    //写入玩家坐标信息
    myflash.iic_write_byte(mmp2.map_index,128,mmp2.px); 
    myflash.iic_write_byte(mmp2.map_index,129,mmp2.py); 
    //写入视野初始化坐标
    myflash.iic_write_byte(mmp2.map_index,130,mmp2.x); 
    myflash.iic_write_byte(mmp2.map_index,131,mmp2.y); 
    //写入目标区域坐标
    myflash.iic_write_byte(mmp2.map_index,132,mmp2.mubiao_x); 
    myflash.iic_write_byte(mmp2.map_index,133,mmp2.mubiao_y); 



  //写入mmp3
    for(int i=0;i<128;i++){
        myflash.iic_write_byte(mmp3.map_index,i,mmp3.map_data[i]);    
    }
    //写入玩家坐标信息
    myflash.iic_write_byte(mmp3.map_index,128,mmp3.px); 
    myflash.iic_write_byte(mmp3.map_index,129,mmp3.py); 
    //写入视野初始化坐标
    myflash.iic_write_byte(mmp3.map_index,130,mmp3.x); 
    myflash.iic_write_byte(mmp3.map_index,131,mmp3.y); 
    //写入目标区域坐标
    myflash.iic_write_byte(mmp3.map_index,132,mmp3.mubiao_x); 
    myflash.iic_write_byte(mmp3.map_index,133,mmp3.mubiao_y); 


  //写入mmp4
    for(int i=0;i<128;i++){
        myflash.iic_write_byte(mmp4.map_index,i,mmp4.map_data[i]);    
    }
    //写入玩家坐标信息
    myflash.iic_write_byte(mmp4.map_index,128,mmp4.px); 
    myflash.iic_write_byte(mmp4.map_index,129,mmp4.py); 
    //写入视野初始化坐标
    myflash.iic_write_byte(mmp4.map_index,130,mmp4.x); 
    myflash.iic_write_byte(mmp4.map_index,131,mmp4.y); 
    //写入目标区域坐标
    myflash.iic_write_byte(mmp4.map_index,132,mmp4.mubiao_x); 
    myflash.iic_write_byte(mmp4.map_index,133,mmp4.mubiao_y);  

  //写入mmp5
    for(int i=0;i<128;i++){
        myflash.iic_write_byte(mmp5.map_index,i,mmp5.map_data[i]);    
    }
    //写入玩家坐标信息
    myflash.iic_write_byte(mmp5.map_index,128,mmp5.px); 
    myflash.iic_write_byte(mmp5.map_index,129,mmp5.py); 
    //写入视野初始化坐标
    myflash.iic_write_byte(mmp5.map_index,130,mmp5.x); 
    myflash.iic_write_byte(mmp5.map_index,131,mmp5.y); 
    //写入目标区域坐标
    myflash.iic_write_byte(mmp5.map_index,132,mmp5.mubiao_x); 
    myflash.iic_write_byte(mmp5.map_index,133,mmp5.mubiao_y); 

    //写入mmp6
    for(int i=0;i<128;i++){
        myflash.iic_write_byte(mmp6.map_index,i,mmp6.map_data[i]);    
    }
    //写入玩家坐标信息
    myflash.iic_write_byte(mmp6.map_index,128,mmp6.px); 
    myflash.iic_write_byte(mmp6.map_index,129,mmp6.py); 
    //写入视野初始化坐标
    myflash.iic_write_byte(mmp6.map_index,130,mmp6.x); 
    myflash.iic_write_byte(mmp6.map_index,131,mmp6.y); 
    //写入目标区域坐标
    myflash.iic_write_byte(mmp6.map_index,132,mmp6.mubiao_x); 
    myflash.iic_write_byte(mmp6.map_index,133,mmp6.mubiao_y); 


  //写入mmp7
    for(int i=0;i<128;i++){
        myflash.iic_write_byte(mmp7.map_index,i,mmp7.map_data[i]);    
    }
    //写入玩家坐标信息
    myflash.iic_write_byte(mmp7.map_index,128,mmp7.px); 
    myflash.iic_write_byte(mmp7.map_index,129,mmp7.py); 
    //写入视野初始化坐标
    myflash.iic_write_byte(mmp7.map_index,130,mmp7.x); 
    myflash.iic_write_byte(mmp7.map_index,131,mmp7.y); 
    //写入目标区域坐标
    myflash.iic_write_byte(mmp7.map_index,132,mmp7.mubiao_x); 
    myflash.iic_write_byte(mmp7.map_index,133,mmp7.mubiao_y); 

    //写入完成之后，在最后一个字节写入标记位
    myflash.iic_write_byte(7,255,100); 
    //写入完成后声音提醒
    while(1){
      tone(10,6000,100);
      delay(100);
      noTone(10);
      delay(50);
    }
    delay(1000);

  }else{
    while(1){
      tone(10,6000,1000);
      delay(1000);
      noTone(10);
      delay(50);
    }
  }

}
#endif
//======================================================条件编译================================================================







//======================================================条件编译================================================================
#ifndef MAP_WRITE
//迷宫加载地图函数

void map_r(byte map_index){
  for(byte i=0;i<128;i++){
      
        migong_map.map_data[i]=flash_r(map_index,i);
      
  }

  //myflash.iic_read_map(migong_map_dadr,0,migong_map.map_all_r);
  //写入玩家初始化坐标，地址128，129
  migong_map.px=flash_r(map_index,128);//x
  migong_map.py=flash_r(map_index,129);//y
  //写入初始化视野，地址130，131
  migong_map.x=flash_r(map_index,130);//x
  migong_map.y=flash_r(map_index,131);//y
  //写入目标区域坐标，地址132，133
  migong_map.mubiao_x=flash_r(map_index,132);//x
  migong_map.mubiao_y=flash_r(map_index,133);//y
  //写入地图编号，地址134
  migong_map.map_index=flash_r(map_index,134);
}


//开启休眠模式
void power_down(){   
    null_time=millis();//休眠倒计时归零  
     power_down_flag=1;//是否进入低功耗的标志位 
     power_sleep_beeper();//休眠音效

          for(int i=0;i<8;i++){//存储初级显存到缓存里
            view_tem[i]=lc.LedBuffer_work[i];
          }
     biaoqing_shufu();
     imu.set_int();//开启陀螺仪低功耗中断输出
     attachInterrupt(digitalPinToInterrupt(2),wakeUp, CHANGE);//开启中断
     lc.shutdown(true);//关闭点阵屏
     delay(2000);//等两秒
     LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); //主控芯片进入低功耗模式
        //这里是唤醒后执行的动作
        wakeup_beerper();//唤醒音效
        imu.setup();//重新初始化陀螺仪
        delay(100);
        //zhonglifangxiang_panduan();//检测一下当前的重力方向
        lc.shutdown(false);//开启点阵屏
        biaoqing_wakeup();
        //see_bat();//查看电量
        for(int i=0;i<8;i++){//存储初级显存到缓存里
            lc.LedBuffer_work[i]=view_tem[i];//复原休眠时的显存
        }

               
      //激活电量检测信号输出，免得芯片没反应过来
      digitalWrite(9,LOW);
      delay(100);
      digitalWrite(9,HIGH);
      delay(100);
      digitalWrite(9,LOW);
      delay(100);
      digitalWrite(9,HIGH);
      delay(100);
      digitalWrite(9,LOW);
      delay(100);
      digitalWrite(9,HIGH);
      delay(300);
      int bat_v=bat.bat_state();
      lc.bitmap(bat_bitmap[bat_v]);//  让充电池上方对着充电口
      lc.UpLoad();
        //设置每次动画结束后的间隔时长
        for(int i=0;i<3000;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(bat_bitmap[bat_v]);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }
        power_down_flag=0;//低功耗标识位改为0，表示退出了低功耗模式
        null_time=millis();//休眠倒计时归零       
        lc.clearDisplay();//清屏
        if(!tem){
          lc.bitmap(lc.LedBuffer_work);//复原退出时的画面
          lc.UpLoad();
        }
        if(migong_state||she_state){//如果之前是从是迷宫或者蛇进入的休眠，那恢复时需要强制恢复重力方向为默认的3，否则控制方向会乱
          lc.zhonglifangxiang=3;
        }
     detachInterrupt(digitalPinToInterrupt(2)); //取消中断
}

//唤醒函数
void wakeUp(){//中断唤醒函数,这里面不要放任何函数，不然无法正常唤醒



}
//电量显示函数,跟随重力
void  see_bat(){//显示电量
      //lc.zhonglifangxiang=3;//这里强制恢复重力初始方向，也作为提示用户，根据电池朝向确定屏幕方向
      byte bat_v;
      lc.clearDisplay();//清屏
      //激活电量检测信号输出
      digitalWrite(9,LOW);
      delay(100);
      digitalWrite(9,HIGH);
      delay(100);
      digitalWrite(9,LOW);
      delay(100);
      digitalWrite(9,HIGH);
      delay(100);
      digitalWrite(9,LOW);
      delay(100);
      digitalWrite(9,HIGH);
      delay(300);

      //判断是否处在充电状态

      bat_v=bat.bat_state();//记下当前电量以及是否在充电
      null_time=millis();//休眠倒计时重置
      #ifdef DEBUG 
      
          while(0){//测试代码
      #else 
          while(bat.charge_state){//1表示在充电，0表示未充电，如果处在充电状态，就播放充电动画
      #endif
        mpu_6050_keyinput();
        for(int i=0;i<=bat_v;i++){
            if(beeper){
              tone(10,2000+i*600,50);
            }
            zhonglifangxiang_panduan();
            lc.bitmap(bat_bitmap[i]);//  让充电池上方对着充电口
            lc.UpLoad();
            delay(50);//设置充电动画速度
        }


        //设置每次动画结束后的间隔时长
        for(int i=0;i<500;i++){
            zhonglifangxiang_panduan();
            if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(bat_bitmap[bat_v]);//  让充电池上方对着充电口
                    lc.UpLoad();
            }
        }


        bat_v=bat.bat_state();//再次检测当前电量，以及判断充电是否结束
        if((input_new>input_old+0.01)||(input_new<input_old-0.01)){//检测是否静止，只要有轻微移动就重置计时器
              null_time=millis();//休眠倒计时重置
              lc.shutdown(false);//亮屏
              beeper=1;
        }else{
              if(millis()-null_time>timeout){//如果静止时长超过设定的时间，就进入休眠
                  beeper=0;
                  
                  lc.shutdown(true);//熄屏
                  
              }
        }
      }
      if(beeper){
          //playRtttlBlocking(TONE_PIN,battery_charge_over);//充电结束音乐
      }
      //充满后显示表情
      beeper=myflash.iic_read_byte(beeper_page,beeper_addr);
      lc.shutdown(false);
      biaoqing_wakeup();
      zhonglifangxiang_panduan();
      lc.bitmap(bat_bitmap[bat_v]);
      lc.UpLoad();
}


//重力方向判断

void zhonglifangxiang_panduan(){
  zhonglifangxiang_old=lc.zhonglifangxiang;
  a_x=imu.ax();
  a_y=imu.ay();

          if(abs(a_x)>0.5||abs(a_y)>0.5){
            if(abs(a_x)>abs(a_y)){
              if(a_x<-0.5){
                lc.zhonglifangxiang=1;
              }
              if(a_x>0.5){
                lc.zhonglifangxiang=3;
              }

            }

            if(abs(a_x)<abs(a_y)){
              if(a_y<-0.5){
                lc.zhonglifangxiang=2;
              }
            if(a_y>0.5){
                lc.zhonglifangxiang=4;
              }

            }
        }
}
//判断是否处于倒扣状态，判断是否进入菜单，如果没有任何操作，就返回1，菜单确认返回0
int daokou(){

  int game_old;//存储上一次的游戏图形编号
  int game=game_old_state;//初始游戏编号
  byte m=0;
  byte s=0;
  if(imu.az()>=0.9){//这个参数判断游戏机是否处在倒扣状态

  delay(100);
  for(int i=0;i<500;i++){

    if(imu.az()<0){//进入菜单
        int ok=1;//菜单选择循环条件，未确认之前为1，确认后为0
        if(shalou){
          s=1;
          for(int i=0;i<8;i++){//存储初级显存到缓存里
            view_tem[i]=lc.LedBuffer_work[i];
          }
        }
        if(migong_state){
          m=1;
          for(int i=0;i<8;i++){//存储初级显存到缓存里
            view_tem[i]=lc.LedBuffer_work[i];
          }
        }
        //根据输入修改框选项
        //显示游戏菜单画面,设计一个动画
       
        
        if((migong_state&&game==0)||(she_state&&game==1)){
          for(int i=0;i<300;i++){
              zhonglifangxiang_panduan();
          }
          lc.clearDisplay(); 
          }
        lc.clearDisplay(); 
          
        
        lc.zhonglifangxiang=zhonglifangxiang_old;//动态设定菜单的显示方向
        lc.bitmap(game_bitmap[game]);
        lc.UpLoad();

        //这里可以加入一个进入菜单的音效，然后加点延时，免得画面不稳定
        if(beeper){
          //playRtttlBlocking(TONE_PIN,into_caidan);//进入菜单的音效
        }
        delay(500);

        null_time=millis();//记下当前时间点

        while(ok){//进入菜单选择循环
          
            if(millis()-null_time<10000){//如果距记下的时间点超过十秒就休眠，用于无操作时自动退出
                //扫描输入
                //根据重力方向选择扫描哪个轴
                //根据操作方向，变化游戏编号

                switch(lc.zhonglifangxiang){
                      case 1:
                          //重力方向为1时
                          if(imu.ay()>0.3){//右倾斜
                              null_time=millis();
                              if(game>0){
                                game_old=game;
                                game--;

                                  //这里的动画可以想法优化
                                  for(int i=0;i<9;i++){//移动八次
                                      for(int j=0;j<8;j++){

                                          lc.LedBuffer_work[j]=(game_bitmap[game][j]<<(8-i))|(game_bitmap[game_old][j]>>i);//把相邻两个数组向左移动拼接在一起形成一个新数组
                                      }
                            
                                      lc.bitmap(lc.LedBuffer_work);
                                      lc.UpLoad();
                                      delay(20);
                                  } 
                                  caidan_beeper();
                        

                                  delay(100);
                              }

                          }else if(imu.ay()<-0.3){//左倾斜
                                null_time=millis();
                                if(game<7){
                                  game_old=game;
                                  game++;  

                                  //这里的动画可以想法优化
                                  for(int i=0;i<9;i++){//移动八次
                                      for(int j=0;j<8;j++){
                                
                                          lc.LedBuffer_work[j]=(game_bitmap[game_old][j]<<i)|(game_bitmap[game][j]>>(8-i));//把相邻两个数组向右移动拼接在一起形成一个新数组
                                      }
                            
                                      lc.bitmap(lc.LedBuffer_work);
                                      lc.UpLoad();
                                      delay(20);
                                  } 
                                  caidan_beeper();
                        

                                  delay(100);
                        
                                }  

                          }
                      break;

                      case 2:
                          //重力方向为2时
                          if(imu.ax()<-0.3){//右倾斜
                              null_time=millis();
                              if(game>0){
                                game_old=game;
                                game--;

                                  //这里的动画可以想法优化
                                  for(int i=0;i<9;i++){//移动八次
                                      for(int j=0;j<8;j++){

                                          lc.LedBuffer_work[j]=(game_bitmap[game][j]<<(8-i))|(game_bitmap[game_old][j]>>i);//把相邻两个数组向左移动拼接在一起形成一个新数组
                                      }
                            
                                      lc.bitmap(lc.LedBuffer_work);
                                      lc.UpLoad();
                                      delay(20);
                                  } 
                                  caidan_beeper();
                        

                                  delay(100);
                              }

                          }else if(imu.ax()>0.3){//左倾斜
                                null_time=millis();
                                if(game<7){
                                  game_old=game;
                                  game++;  

                                  //这里的动画可以想法优化
                                  for(int i=0;i<9;i++){//移动八次
                                      for(int j=0;j<8;j++){
                                
                                          lc.LedBuffer_work[j]=(game_bitmap[game_old][j]<<i)|(game_bitmap[game][j]>>(8-i));//把相邻两个数组向右移动拼接在一起形成一个新数组
                                      }
                            
                                      lc.bitmap(lc.LedBuffer_work);
                                      lc.UpLoad();
                                      delay(20);
                                  } 
                                  caidan_beeper();
                        

                                  delay(100);
                        
                                }  

                          }
                      break;

                      case 3:
                          //重力方向为3时
                          if(imu.ay()<-0.3){//右倾斜
                              null_time=millis();
                              if(game>0){
                                game_old=game;
                                game--;

                                  //这里的动画可以想法优化
                                  for(int i=0;i<9;i++){//移动八次
                                      for(int j=0;j<8;j++){

                                          lc.LedBuffer_work[j]=(game_bitmap[game][j]<<(8-i))|(game_bitmap[game_old][j]>>i);//把相邻两个数组向左移动拼接在一起形成一个新数组
                                      }
                            
                                      lc.bitmap(lc.LedBuffer_work);
                                      lc.UpLoad();
                                      delay(20);
                                  } 
                                  caidan_beeper();
                        

                                  delay(100);
                              }

                          }else if(imu.ay()>0.3){//左倾斜
                                null_time=millis();
                                if(game<7){
                                  game_old=game;
                                  game++;  

                                  //这里的动画可以想法优化
                                  for(int i=0;i<9;i++){//移动八次
                                      for(int j=0;j<8;j++){
                                
                                          lc.LedBuffer_work[j]=(game_bitmap[game_old][j]<<i)|(game_bitmap[game][j]>>(8-i));//把相邻两个数组向右移动拼接在一起形成一个新数组
                                      }
                            
                                      lc.bitmap(lc.LedBuffer_work);
                                      lc.UpLoad();
                                      delay(20);
                                  } 
                                  caidan_beeper();
                        

                                  delay(100);
                        
                                }  

                          }
                      break;

                      case 4:
                          //重力方向为4时
                          if(imu.ax()>0.3){//右倾斜
                              null_time=millis();
                              if(game>0){
                                game_old=game;
                                game--;

                                  //这里的动画可以想法优化
                                  for(int i=0;i<9;i++){//移动八次
                                      for(int j=0;j<8;j++){

                                          lc.LedBuffer_work[j]=(game_bitmap[game][j]<<(8-i))|(game_bitmap[game_old][j]>>i);//把相邻两个数组向左移动拼接在一起形成一个新数组
                                      }
                            
                                      lc.bitmap(lc.LedBuffer_work);
                                      lc.UpLoad();
                                      delay(20);
                                  } 
                                  caidan_beeper();
                        

                                  delay(100);
                              }

                          }else if(imu.ax()<-0.3){//左倾斜
                                null_time=millis();
                                if(game<7){
                                  game_old=game;
                                  game++;  

                                  //这里的动画可以想法优化
                                  for(int i=0;i<9;i++){//移动八次
                                      for(int j=0;j<8;j++){
                                
                                          lc.LedBuffer_work[j]=(game_bitmap[game_old][j]<<i)|(game_bitmap[game][j]>>(8-i));//把相邻两个数组向右移动拼接在一起形成一个新数组
                                      }
                            
                                      lc.bitmap(lc.LedBuffer_work);
                                      lc.UpLoad();
                                      delay(20);
                                  } 
                                  caidan_beeper();
                        

                                  delay(100);
                        
                                }  

                          }
                      break;       
                 }



                        
                byte queren_flag=0;//确认选择
                switch(lc.zhonglifangxiang){
                  case 1:
                        if(imu.ax()>0.3){
                          queren_flag=1;
                        }
                  break;
                  case 2:
                        if(imu.ay()>0.3){
                          queren_flag=1;
                        }
                  break;              
                  case 3:
                        if(imu.ax()<-0.3){
                          queren_flag=1;
                        }
                  break;
                  case 4:
                        if(imu.ay()<-0.3){
                          queren_flag=1;
                        }
                  break;
                }

                //重力方向为3时
                if(queren_flag){//确认选择

                    switch (game){

                      case 0://迷宫
                        she_state=0;//蛇的生命状态，1为生，0为死
                        myshe.she_life_state=0;
                        guaxiang=0;
                        migong_state=1;//迷宫的生命状态，1为生，0为死
                        shaizi=0;
                        shalou=0;//沙漏开关
                        game_old_state=game;
                        shoudiantong=0;
                        tem=0;
                        beeper_state=0;
                        
                        break;

                      case 1://贪吃蛇
                        migong_state=0;//迷宫的生命状态，1为生，0为死
                        guaxiang=0;
                        she_state=1;//蛇的生命状态，1为生，0为死
                        myshe.she_life_state=1;
                        shaizi=0;
                        shalou=0;//沙漏开关
                        game_old_state=game;
                        shoudiantong=0;
                        tem=0;
                        beeper_state=0;
                        
                        break;

                      case 2://卜卦
                        she_state=0;//蛇的生命状态，1为生，0为死
                        myshe.she_life_state=0;
                        guaxiang=1;
                        migong_state=0;//迷宫的生命状态，1为生，0为死
                        shaizi=0;
                        shalou=0;//沙漏开关
                        game_old_state=game;
                        shoudiantong=0;
                        tem=0;
                        beeper_state=0;
                        break;

                      case 3://摇骰子
                        she_state=0;//蛇的生命状态，1为生，0为死
                        myshe.she_life_state=0;
                        guaxiang=0;
                        migong_state=0;//迷宫的生命状态，1为生，0为死
                        shaizi=1;
                        shalou=0;//沙漏开关
                        game_old_state=game;
                        shoudiantong=0;
                        tem=0;
                        beeper_state=0;
                        break;

                      case 4://沙漏计时器
                        she_state=0;//蛇的生命状态，1为生，0为死
                        myshe.she_life_state=0;
                        guaxiang=0;
                        migong_state=0;//迷宫的生命状态，1为生，0为死
                        shaizi=0;
                        shalou=1;//沙漏开关
                        game_old_state=game;
                        shoudiantong=0;
                        tem=0;
                        beeper_state=1;
                        break;

                      case 5://温度显示
                        xianshiwendu(imu.temp());
                        delay(3000);
                        beeper_state=1;

                        

                        break;

                      case 6://声音开关
                        if(beeper){//声音开关切换
                            beeper=0;
                        }else{
                            beeper=1;
                        }
                         
                        flash_w(beeper_page,beeper_addr,beeper);
                        beeper_state=1;
                        

                        break;

                      case 7://代码版本

                         
                        xianshiwendu(flash_r(v_page,v_addr));
                        delay(3000);
                        beeper_state=1;
                        

                        break;

                    }


                    ok=0;//完成选择就把循环标志位变成0
                    i=500;
                    //向上滚屏,这个工作在后级显存里
                    caidan_beeper3();




                    for(int i=0;i<9;i++){
                      switch (lc.zhonglifangxiang) {//根据重力方向确定滚动方向
                      case 1:
                      lc.roll(0,4);//定义：左滚动1，右滚动2，上滚动3，下滚动4
                      break;
                      case 2:
                      lc.roll(1,0);//定义：左滚动1，右滚动2，上滚动3，下滚动4
                      break;       
                      case 3:
                      lc.roll(0,3);//定义：左滚动1，右滚动2，上滚动3，下滚动4  
                      break;
                      case 4:
                      lc.roll(2,0);//定义：左滚动1，右滚动2，上滚动3，下滚动4
                      break;   
                      }

                      
                       lc.UpLoad();
                        delay(18);
                    }
                    

                    if(migong_state||she_state){//强制恢复迷宫和贪吃蛇的重力方向
                      lc.zhonglifangxiang=3;
                    }


                    if((s&&shalou)||(m&&migong_state)){
                            for(int i=0;i<8;i++){//恢复退出时的初级显存数据
                                lc.LedBuffer_work[i]=view_tem[i];
                                
                          
                            }
                            lc.bitmap(lc.LedBuffer_work);
                            lc.UpLoad();
                            

                    }else{
                              //清空初级缓存里的内容，这里的作用是清除从沙漏切换到其他功能时画面上的乱码
                              //这里加个判断，可以解决沙漏退出马上又进沙漏会卡死的现象，找了大半天，终于找到问题所在了
                              
                                 for(int xi=0;xi<8;xi++){
                                      lc.LedBuffer_work[xi]=0;
                                  }
                              

                              lc.bitmap(lc.LedBuffer_work);
                              lc.clearDisplay();
                              //xianshidefen(22);
                    }


                   // (migong_state&&game_old==0&&beeper_state==1)
                    null_time=millis();//重置静态时间点起点
                    
                    return 0;//完成选择返回0
                }
            }else{
                
                power_down();
            }
        }//选择循环结束
      

    }
  }
    
    power_down();
  
  }

  return 1;//未做任何操作返回1
}
///////////////////////////////系统函数////////////////////


//显示得分
void xianshidefen(byte defen){


  byte shiwei;
  byte gewei;
  shiwei=defen/10;
  gewei=defen%10;


  lc.clearDisplay();//清空显示
  //把十位写入后级显存
  lc.bitmap_work(num_bitmap[shiwei]);
  //个位偏移，并放入临时数组
  map_offset(num_bitmap[gewei],view,3,1);

  //图像合成
  for(int i=0;i<8;i++){
    lc.LedBuffer_work[i]|=view[i];//把两个图像进行或运算合成
  }
  //画面整体偏移
  map_offset(lc.LedBuffer_work,view,1,1);

  zhonglifangxiang_panduan();
  lc.bitmap(view);
  lc.UpLoad(); 

    for(int i=0;i<1500;i++){
      zhonglifangxiang_panduan();
      lc.bitmap(view);
      lc.UpLoad(); 
      if(zhonglifangxiang_old!=lc.zhonglifangxiang){
          lc.bitmap(view);
          lc.UpLoad(); 
      }
    }
  
  lc.clearDisplay();//清空显示
  lc.zhonglifangxiang=3;//强制复位重力方向
}
//显示温度
void xianshiwendu(byte defen){

  byte shiwei;
  byte gewei;
  shiwei=defen/10;
  gewei=defen%10;
  lc.clearDisplay();//清空显示
  //把十位写入后级显存
  lc.bitmap_work(num_bitmap[shiwei]);
  //个位偏移，并放入临时数组
  map_offset(num_bitmap[gewei],view,3,1);

  //图像合成
  for(int i=0;i<8;i++){
    lc.LedBuffer_work[i]|=view[i];//把两个图像进行或运算合成
  }
  //画面整体偏移
  map_offset(lc.LedBuffer_work,view,1,1);
  lc.bitmap(view);
  lc.UpLoad(); 

}

byte jingzhijiance(){

  input_old=input_new;//存储上一次陀螺仪输出，用于迷宫静止检测功能
      pax=imu.ay();
      pay=imu.ax();
  input_new=abs(pax)+abs(pay);//两个轴向上的倾斜累加起来，提高休眠检测的准确性

  if((input_new>input_old+0.01)||(input_new<input_old-0.01)){//检测是否静止，只要有轻微移动就重置计时器
        null_time=millis();//休眠倒计时重置
        return 1;
  }else{
          if(millis()-null_time>timeout){//如果静止时长超过设定的时间，就进入休眠
              //biaoqing_shufu();
              power_down();//进入休眠
          }
        }

}

byte move_or_not(){

  input_old=input_new;//存储上一次陀螺仪输出，用于迷宫静止检测功能
      pax=imu.ay();
      pay=imu.ax();
  input_new=abs(pax)+abs(pay);//两个轴向上的倾斜累加起来，提高休眠检测的准确性

  if(abs(input_new-input_old)>0.1||imu.az()>=0.9){//检测是否静止，只要有轻微移动或者倒扣就重置计时器
        null_time=millis();//休眠倒计时重置
        lc.setIntensity(0);
        return 1;//如果有移动就返回1
  }else{
      return 0;//如果没移动就返回0
  }

}




    



//////////////////////////////---------------------------------已启用功能///////////////////
//摇骰子
void yaoshaizi(){
      //lc.clearDisplay();
        if(millis()-null_time>timeout){//如果静止时长超过设定的时间，就进入休眠
          //Serial.println("准备关机");
          biaoqing_shufu();
          power_down();//屏保
        }
      daokou();
      
      float kx;
      float ky;
      kx=imu.ax();
      ky=imu.ay();
      int g;
      if(kx>0.5){
        
        for(int i=0;i<100;i++){
          if(imu.ax()<-0.5){
            zhonglifangxiang_panduan();
            lc.clearDisplay();  
            bagua_beeper();
            g=random(0,6);
            lc.bitmap(shaizi_all[g]);
            lc.UpLoad();
            i=50;
          }
        }
        null_time=millis();
      }

      if(ky>0.5){
        
        for(int i=0;i<100;i++){
          if(imu.ay()<-0.5){
            zhonglifangxiang_panduan();
            lc.clearDisplay();  
            bagua_beeper();
            g=random(0,6);
            lc.bitmap(shaizi_all[g]);
            lc.UpLoad();
            i=50;
          }
        }
        null_time=millis();
      }
      zhonglifangxiang_panduan();
      if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(shaizi_all[g]);
                    lc.UpLoad();
      }

}

//贪吃蛇输入控制，用加速度控制输入，功能已ok
int mpu_6050_keyinput_she(int laofangxiang){//需要传递当前蛇的前进方向作为参数，决定判断x还是y轴的变化
  float lingmindu=0.3;//这里设置操控灵敏度
  float lingmindu_n=-0.3;//这里设置操控灵敏度
  daokou();//优先检查是否倒扣，如果倒扣就马上休眠
  if(laofangxiang==1||laofangxiang==3){//如果当前前进方向是上或者下，就只判断x轴，也就是左右
    if(imu.ay()<lingmindu_n){//这里设置操控灵敏度
      return 2;//右
    }else if(imu.ay()>lingmindu){
      return 4;//左
    }
  }else if(laofangxiang==2||laofangxiang==4){//如果当前前进方向是左右，就判断y轴，也就是上下
    if(imu.ax()>lingmindu){
      return 3;//下
    }else if(imu.ax()<lingmindu_n){
      return 1;//上
    }
  }
  return laofangxiang;

}

////////迷宫专用函数/////////////////////////////////////////////
//迷宫输入控制函数[上1，下3，左4，右2，【上左14，上右12，下左34，下右32】0未输入
byte mpu_6050_keyinput(){
  input_old=input_new;//存储上一次陀螺仪输出，用于迷宫静止检测功能
  for(int i=0;i<migong_speed;i++){//这里用于控制移动灵敏度
      pax=imu.ay();
      pay=imu.ax();
      if(daokou()==0){//如果监测到返回菜单的操作，就马上退出循环，提高切换菜单操作的灵敏度
        i=migong_speed;
        return 1;
      }
  }
  input_new=abs(pax)+abs(pay);//两个轴向上的倾斜累加起来，提高休眠检测的准确性
  return 0;
}
//设置指定全局坐标在视野内的坐标的状态
void set_view_xy(int bx,int by,int x,int y,byte state){//视野全局x，视野全局y，指定全局坐标x,指定全局坐标y
  int v_x;
  int v_y;
  //判断指定坐标是否在视野范围内
  if(x>=bx&&x<bx+8&&y>=by&&y<by+8){
  //把指定的全局坐标转换为视野坐标
  v_x=x-bx;
  v_y=7-y+by;
  //根据state设置该坐标的状态
  lc.setLed_work(v_x,v_y,state);
  }


}
//迷宫通关判断，判断当前玩家是否处在目标区域
byte pass_xy(int x_1,int y_1,int x_2,int y_2,int px,int py){//目标坐标范围，玩家坐标

    if(px>=x_1&&px<=x_2&&py>=y_1&&py<=y_2){
      return 1;
    }else{
      return 0;
    }

}
void get_view(int x,int y){//根据视野的左上角坐标位置，得到视野的内容

  //判断坐标在全局地图中在哪个地图块,得到块坐标和块内子坐标
  byte map_block_x;//得到块坐标
  byte map_block_y;//得到块坐标
  byte map_block_x_x;//得到块内的子坐标
  byte map_block_y_y;//得到块内的子坐标
  
  if(x>7){
    if((x+1)%8){
      map_block_x=((x+1)/8);
      map_block_x_x=((x+1)%8)-1;//得到块内的子坐标
    }else{
      map_block_x=((x+1)/8)-1;
      map_block_x_x=7;
    }
  }else{
      map_block_x=0;
      map_block_x_x=x;
  }
  if(y>7){
    if((y+1)%8){
      map_block_y=((y+1)/8);
      map_block_y_y=((y+1)%8)-1;//得到块内的子坐标
    }else{
      map_block_y=((y+1)/8)-1;
      map_block_y_y=7;
    }
  }else{
      map_block_y=0;
      map_block_y_y=y;
  }

  //把当前块所在区域内容复制到视野数组里

  for(int i=0;i<8-map_block_y_y;i++){

      view[i]=(*(migong_map.map_all[map_block_y][map_block_x][map_block_y_y+i])<<map_block_x_x)|(*(migong_map.map_all[map_block_y][map_block_x+1][map_block_y_y+i])>>(8-map_block_x_x));//[y][x]

  }


  //把当前块下方的复制进视野数组

  for(int i=0;i<map_block_y_y;i++){
      
      view[8-map_block_y_y+i]=(*(migong_map.map_all[map_block_y+1][map_block_x][i])<<map_block_x_x)|(*(migong_map.map_all[map_block_y+1][map_block_x+1][i])>>(8-map_block_x_x));//[y][x]


  }
  
}
//玩家图标，根据坐标移动图标
void player(byte x,byte y,byte state){//四个像素，左下角为位置坐标,x,y范围：0-7
  lc.setLed_work(x,y,state);
  lc.setLed_work(x,y+1,state);
  lc.setLed_work(x+1,y,state);
  lc.setLed_work(x+1,y+1,state);
}
///////////迷宫专用函数/////////////////////////////////////////////


//////////////沙漏函数//////////

//这个函数用来初始化每个像素
void piexl_int(struct Mypiexl * piexl,byte x,byte y,float speed_x,float speed_y){
    (*piexl).x=x;
    (*piexl).y=y;
    (*piexl).x_now=x;
    (*piexl).y_now=y;
    (*piexl).x_old=x;
    (*piexl).y_old=y;
    (*piexl).speed_x=speed_x;
    (*piexl).speed_y=speed_y;
    (*piexl).speed_x_now=speed_x;
    (*piexl).speed_y_now=speed_y;
    (*piexl).over_flag=0;
}

//这个函数用来更新每一个像素的状态
void move_piexl(struct Mypiexl * piexl){//把像素结构的指针做为参数，才能实际修改信息


  float y_l=0;//路程变化量
  byte yyyy;
  //如果像素的静止标志位为0时，才执行下面的所有代码，用于控制一个像素是否完成了整个下落过程
  if(!(*piexl).over_flag){



  //一个像素掉落并静止的整个过程
  //当前y坐标大于等于1，当前坐标下面一个坐标为空白，shalou参数为0
  while(((*piexl).y_now>=1||!lc.getLedState_work((*piexl).x_now,(*piexl).y_now-1,lc.LedBuffer_work))&&shalou){
    //下落过程
    if((!lc.getLedState_work((*piexl).x_now,(*piexl).y_now-1,lc.LedBuffer_work))&&shalou){//当下方为空，且沙漏标志位是非零时
        if((*piexl).y_now>0){//当没有触底时

          //计算当前速度
          (*piexl).speed_y_now=ay*time_pass;//原始速度基础上累加加速度在切片内新增的速度
          //计算新的位置
          y_l=map((map((*piexl).y,0,7,0,gaodu)-0.5*ay*time_pass*time_pass),0,gaodu,0,7);//得到在y轴上相对于起始位置移动的距离，这里假设视野范围实际尺寸为100x100，计算距离地面的绝对位置
          (*piexl).y_old=(*piexl).y_now;
          (*piexl).y_now=y_l;
          //Serial.println(y_l);//调试代码
          lc.setLed_work((*piexl).x_old,(*piexl).y_old,0);//擦掉旧坐标
          //下面区域用于画上新坐标
          lc.setLed_work((*piexl).x_now,(*piexl).y_now,1);//写上新坐标
          lc.bitmap(lc.LedBuffer_work);
          lc.UpLoad();

          zhonglifangxiang_panduan();
          if(lc.zhonglifangxiang!=zhonglifangxiang_old){//让画面跟随重力方向自动变化,当重力方向变化时才更新显示，可以避免闪烁
              lc.bitmap(lc.LedBuffer_work);
              lc.UpLoad();
          }
          daokou();
          

        }
        
    }//下落过程结束


    yyyy=(*piexl).y_now;//保存当前反弹的y轴坐标
    //反弹过程
    if(((*piexl).y_now==0||lc.getLedState_work((*piexl).x_now,(*piexl).y_now-1,lc.LedBuffer_work))&&shalou){//当高度为零或者下方有像素且沙漏标志位为非零时


        if(((*piexl).speed_y_now>10&&y_l<15)&&beeper){//这里控制反弹音次数
              tone(10,4000,50);
              delay(50);
              noTone(10);
        }else{
          delay(50);
        }


        time_pass=0;
        (*piexl).speed_y_now*=0.1*random(6,10);//反弹系数随机，模仿杂乱的地面


        while(((*piexl).speed_y_now>=ay*time_pass)&&shalou){

              time_pass+=time_speed;
              //计算当前反弹高度
              y_l=(*piexl).speed_y_now*time_pass-0.5*ay*time_pass*time_pass+map(yyyy,0,7,0,gaodu);//优化了反弹高度需要加上反弹位置的高度
              (*piexl).y_old=(*piexl).y_now;
              (*piexl).y_now=map(y_l,0,gaodu,0,7);
              if(!lc.getLedState_work((*piexl).x_now,(*piexl).y_now,lc.LedBuffer_work)){
                  lc.setLed_work((*piexl).x_old,(*piexl).y_old,0);//擦掉上一个坐标
                  //下面区域用于画上新坐标
                  lc.setLed_work((*piexl).x_now,(*piexl).y_now,1);//画上新坐标
              } 

              zhonglifangxiang_panduan();
              lc.bitmap(lc.LedBuffer_work);
              lc.UpLoad(); 
              daokou();        
        }

        time_pass=0;
        (*piexl).y=(*piexl).y_now;
      
    }//反弹过程结束
    time_pass+=time_speed;//计时器递增

  }//一个像素掉落的整个循环过程结束

    if(shalou){
      (*piexl).over_flag=1;//像素是否落下来并静止的判断符号，1为静止
  

      //这里是两个像素掉落之间的间隙，这里控制间隔时间
      for(int i=0;i<shalou_speed;i++){//这里设置沙漏的延时,可以设置像素掉落之间的间隔时间，这个参数大概能延时5秒
          if(shalou==0){//如果已经退出，立即结束这个循环
              i=shalou_speed;
          }else{//如果正常进行，就检测菜单操作和重力检测
              daokou();
              zhonglifangxiang_panduan();
              if(lc.zhonglifangxiang!=zhonglifangxiang_old){
                  lc.bitmap(lc.LedBuffer_work);
                  lc.UpLoad();
              }
          }
      }
    }
  }//如果像素静止，则不执行所有的代码。只有当像素处在非静止时才会运行上面的所有代码
}

//沙漏初始化函数
void shalou_chushihua(){


    /////下面是沙漏初始化过程

    lc.clearDisplay();
    //初始化第一个像素的坐标
    xx=5;
    yy=7;
    //赋予随机数种子
    randomSeed(imu.rawAy());//给随机数函数一个随机的种子

    //清空两级显存
    for(int yj=0;yj<8;yj++){
        for(int xi=0;xi<8;xi++){
            lc.setLed_work(xi,yj,0);
        }
    }
    lc.bitmap(lc.LedBuffer_work);
    lc.UpLoad(); 
    //检测一次重力方向
    zhonglifangxiang_panduan(); 

    //绘制沙漏上半部分
    for(int yj=4;yj<8;yj++){
      for(int xi=0;xi<8;xi++){
          lc.setLed_work(xi,yj,1);
      }
    }
    lc.bitmap(lc.LedBuffer_work);
    lc.UpLoad(); 

    //显示完成后等一秒再开始
    delay(1000);

    //闪烁三次配音效后开始
    bagua_beeper();
    lc.setIntensity(1);//(点阵地址，亮度0-31)初始化点阵屏亮度
    delay(50);
    lc.setIntensity(0);//(点阵地址，亮度0-31)初始化点阵屏亮度
    bagua_beeper();
    lc.setIntensity(1);//(点阵地址，亮度0-31)初始化点阵屏亮度
    delay(50);
    lc.setIntensity(0);//(点阵地址，亮度0-31)初始化点阵屏亮度
    bagua_beeper();
    lc.setIntensity(1);//(点阵地址，亮度0-31)初始化点阵屏亮度
    delay(50);
    lc.setIntensity(0);//(点阵地址，亮度0-31)初始化点阵屏亮度
    delay(1500);
    /////上面是沙漏初始化过程
}
//////////////沙漏函数//////////


//////////////////////////////-----------------------------------已启用功能///////////////////

/////////////////////////////////////////////////////////////////////////上面是自定义函数区域///////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------下面是执行区域------------------------------------------------------------------------------
#endif
//======================================================条件编译================================================================








void setup() {


//======================================================条件编译================================================================

  version_w();//写入代码版本号
  #ifdef Serial_DEBUG
  Serial.begin(38400);//一旦开启串口程序主体就会出现显示乱码，估计是串口和显示屏信号冲突了
  #endif
  pinMode(10,OUTPUT);//声音输出
  pinMode(9,OUTPUT);//ip5306_key,电量显示，持续拉低10秒会关闭电源输出，等于自杀，自身就无法唤醒。这时只有给它type-c通一下电让其重启才行。
  pinMode(2,INPUT);//读取mpu6050的运动中断信号，用于休眠时唤醒
  
  lc.shutdown(false);//这个是设置点阵正常工作
  lc.clearDisplay();//清空显示
  lc.setIntensity(0);//(点阵地址，亮度0-31)初始化点阵屏亮度


  #ifndef MAP_WRITE
  lc.zhonglifangxiang=zhonglifangxiang;
  null_time=millis();

  lishidefen=myflash.iic_read_byte(she_defen_page,she_defen_addr);//启动的时候先读取贪吃蛇历史记录备用，这样就不用每次结束时都读取一次浪费时间
  beeper=myflash.iic_read_byte(beeper_page,beeper_addr);//启动的时候先读取声音开关记录备用，保留上次的操作记录

  power_on_beerper(); //开机音乐
  //初始化mpu6050
  imu.setup();
  see_bat();//查看电量

  #endif
//======================================================条件编译================================================================


  #ifndef DEBUG
  // 设置mpu6050偏差
  imu.setBias();
  //Serial.println("校准完成");
  #endif


/////////////////调试2
//======================================================条件编译================================================================
  #ifdef MAP_WRITE
  map_w();
  #endif
//======================================================条件编译================================================================




  #ifdef DEBUG_LEDTEST
    //点阵测试函数，用完注释掉就行了
    LED_TEST();
  #endif



}//setup结束



void loop() { 
  //======================================================条件编译================================================================
#ifndef MAP_WRITE
  //////////////////////////////////////////////////贪吃蛇游戏//////////////////////////////////////////


  while(she_state){
    lc.zhonglifangxiang=3;//贪吃蛇模式下锁定坐标系方向
    myshe.defen=0;//初始化蛇的得分为0.
    speed=300;//这里设置贪吃蛇的速度上限。
    myshe.reset_she();//重新初始化蛇。
    mpu_key_flag=1;
    while(she_state){//蛇游戏循环。
      lc.clearDisplay();
      lc.clearDisplay_work();    
    while(myshe.she_life_state){//蛇活着的循环

          //扫描输入，不管有没有输入都会循环扫描多次用于延时
        for(int i=0;i<speed-map(myshe.defen,0,32,0,speed);i++){//这里根据得分来设置蛇的移动速度，得分越高，速度越快
            mpu_key_flag_a=mpu_6050_keyinput_she(mpu_key_flag);
        }
        mpu_key_flag=mpu_key_flag_a;

        if(she_state){
            //////////////////////////////////////////////////////休眠判断
            input_old=input_new;
            input_new=max(imu.ax(),imu.ay());
            if((input_new>input_old+0.03)||(input_new<input_old-0.03)){
                null_time=millis();
            }else{
                if(millis()-null_time>timeout){
                    power_down();//休眠
                }
            }
            //=========================================================================================================================
            //3根据输入更新蛇移动方向she.key_fangxiang(key.key_flag);
            myshe.key_fangxiang(mpu_key_flag);//int key_flag=0;//上1，下3，左4，右2，0未输入
            //4移动蛇的坐标she.move
            my_beeper();//正常移动时的音效
            myshe.move();//更新蛇身体的坐标信息，用于后面绘制蛇的身体
            //5画头lc.setLed(tou_x,tou_y,1);擦尾，当蛇身长度为1时，不擦尾，否则擦尾lc.setLed(wei_del_x,wei_del_y,0)
            //这里的工作模式是直接把画面信息写入显存数组LedBuffer，这里的数据下一步就会在调用lc.UpLoad()函数时被显示在点阵屏上
            lc.setLed_work(myshe.food_xy>>3,myshe.food_xy&0B00000111,1);//画食物
            lc.setLed_work(myshe.tou_x,myshe.tou_y,1);//画头
            if(!myshe.body_add){
                lc.setLed_work(myshe.wei_x,myshe.wei_y,0);//擦尾
            }
            lc.bitmap(lc.LedBuffer_work);
            lc.UpLoad();//上传显示
            
            if(myshe.body_add&&beeper){//得分音效和画面效果
                lc.setIntensity(3);
                delay(100);
                tone(10,2000,1000);
                delay(15);
                noTone(10);
                lc.setIntensity(0);
                delay(50);
                tone(10,3000,1000);
                delay(15);
                noTone(10);       
                lc.setIntensity(3);
                delay(25);
                tone(10,2000,1000);
                delay(15);
                noTone(10);       
                lc.setIntensity(0);
            }
            //6如果还活着，就跳过从新从2画头开始，否则结束，统计得分，暂停，按任意键回到1初始化蛇
        }

    }
    if(she_state){
      if(beeper){//死亡音效
        tone(10,500,1000);
        delay(160);
        tone(10,300,1000);
        delay(160);
        tone(10,100,1000);
        delay(200);
        noTone(10);
      }    
        //playRtttlBlocking(TONE_PIN,she_pass_sound);  
        Wire.end();//空出iic总线
        //破纪录
        if(myshe.defen>lishidefen){//如果得分比历史记录高，则记录当前得分，否则不记录
            myflash.iic_write_byte(0,0,myshe.defen);//记录最终得分到0页，0字节位置。页地址(0-15)、字节地址(0-255)、数据(页地址的前4位是固定的1010，后三位组成页地址共8页，最后一位是读写位。0是写操作，1是读操作)
            lishidefen=myshe.defen;//更新历史最高分到比较变量里
            
            //这里再放一个画面
            biaoqing_she_pojilu();
            xianshidefen(myshe.defen);//显示当前得分
            //播放破纪录音乐
            if(beeper){
                
            }
        }else{
          xianshidefen(myshe.defen);//显示当前得分
        }

      

      myshe.reset_she();//重新初始化蛇
      lc.clearDisplay();
    }

    }//蛇循环结束
  }//蛇条件判断
  //=============================================================================
  ////////////////////////////////////////////////////迷宫游戏////////////////////////////////
  while(migong_state){//迷宫游戏开始
      lc.zhonglifangxiang=3;
      lc.clearDisplay();
      lc.clearDisplay_work();
      ////////////////////迷宫游戏变量///////////////////////////////////////////////////////////////////////////
      byte px_0;//用来暂存玩家根据输入得到的新坐标，如果新坐标内无障碍物才会赋予给玩家坐标
      byte py_0;
      byte x_0;
      byte y_0;
      byte p_x;//玩家的全局坐标
      byte p_y;

 
      int t;//玩家闪烁倒计时变量，这个变量会在每次循环里递减，直到低于阈值就会翻转地图指定区域的显示方式（熄灭或者点亮）


      int t_xy;//地图闪烁倒计时变量，这个变量会在每次循环里递减，直到低于阈值就会翻转地图指定区域的显示方式（熄灭或者点亮）
      byte state_flag;//地图闪烁区域显示状态位，当t_xy低于阈值时会在0和1之间翻转
      byte state_flag_p;//玩家闪烁区域显示状态位，当t低于阈值时会在0和1之间翻转

      ////////////////////迷宫游戏变量////////////////////////////////////////////////////////////////////////////////
      x_0=migong_map.x;
      y_0=migong_map.y;
      //注意这里有个坐标变换
      //视野框在全局的坐标这个事以左上角为原点看的
      //但视野框内的坐标是以左下角为原点看的，所以玩家的坐标也是以左下角为原点，
      //所以在把玩家的相对坐标转换为全局坐标时，y轴坐标是相反的，需要反转一下用7减去玩家的纵坐标，
      //然后再和视野原点坐标相加，从而得到玩家的全局坐标
      //搞不明白当初为什么我要这么设计……太弯弯绕了

      //px=2;//玩家初始坐标（视野坐标系，左下角是原点）
      //py=2;//玩家初始坐标（视野坐标系，左下角是原点）
      px_0=migong_map.px;
      py_0=migong_map.py;
      mpu_key_flag=0;//初始化方向
      t=t_time;//复位玩家闪烁递减参数
      t_xy=t_xy_time;//复位地图闪烁递减参数

      state_flag=0;//地图闪烁标记，0灭，1亮，各自的倒计时结束时，这个标志会翻转
      state_flag_p=0;//地图闪烁标记，0灭，1亮，各自的倒计时结束时，这个标志会翻转

      int px_old;//记录玩家上一次的全局坐标
      int py_old;//记录玩家上一次的全局坐标


      //初始化显示
      get_view(migong_map.x,migong_map.y);//根据视野初始坐标（视野的左上角坐标）获取视野
      //这个函数的工作原理：在大地图里截取以左上角坐标向右下方8x8面积的信息，把这些信息写入初级显存view


      lc.bitmap_work(view);//把视野（初级显存view）复制进Ledbufer_work
      player(migong_map.px,migong_map.py,1);//根据坐标，在Ledbuffer_work里绘制玩家，第三个参数是亮灭，可以实现闪烁
      lc.bitmap(lc.LedBuffer_work);//负责把初级显存内容按重力旋转后拷贝入次级显存
      lc.UpLoad();//上传并显示
      null_time=millis();//休眠倒计时归零

      while(migong_state){/////////////////////////////////////////////////////////////开始迷宫游戏循环////////////////////
          t--;//玩家闪烁截止时间递减
          t_xy--;//地图闪烁截止时间递减

              //根据当前玩家的全局坐标绘制玩家和视野
              get_view(migong_map.x,migong_map.y);//获取视野
              lc.bitmap_work(view);//把视野复制进Ledbufer_work
              player(migong_map.px,migong_map.py,1);//根据显示状态位，重新在Ledbuffer_work里绘制玩家
              lc.bitmap(lc.LedBuffer_work);//负责把初级显存内容按重力旋转后拷贝入次级显存
              lc.UpLoad();


          if(t_xy==0){//地图闪烁，当倒计时结束，状态位切换
              if(state_flag==0){
                  state_flag=1;
              }else{
                  state_flag=0;
              }
              //终点闪烁区域
              set_view_xy(migong_map.x,migong_map.y,migong_map.mubiao_x,migong_map.mubiao_y,state_flag);
              set_view_xy(migong_map.x,migong_map.y,migong_map.mubiao_x+1,migong_map.mubiao_y,state_flag);
              set_view_xy(migong_map.x,migong_map.y,migong_map.mubiao_x,migong_map.mubiao_y+1,state_flag);
              set_view_xy(migong_map.x,migong_map.y,migong_map.mubiao_x+1,migong_map.mubiao_y+1,state_flag);
              lc.bitmap(lc.LedBuffer_work);
              lc.UpLoad();
              //delay(10);
              t_xy=t_xy_time;//重置地图闪烁倒计时
          }

          if(t==0){//玩家闪烁
              if(state_flag_p==0){
                  state_flag_p=1;
              
                  player(migong_map.px,migong_map.py,state_flag_p);//根据显示状态位，重新在Ledbuffer_work里绘制玩家
                  lc.bitmap(lc.LedBuffer_work);//负责把初级显存内容按重力旋转后拷贝入次级显存
                  lc.UpLoad();
                  delay(50);
              }else{
                  state_flag_p=0;
                  player(migong_map.px,migong_map.py,state_flag_p);//根据显示状态位，重新在Ledbuffer_work里绘制玩家
                  lc.bitmap(lc.LedBuffer_work);//负责把初级显存内容按重力旋转后拷贝入次级显存
                  lc.UpLoad();
                  delay(5);
              }



              t=t_time;
              
          }  

              lc.bitmap(lc.LedBuffer_work);//把视野复制进Ledbufer
              lc.UpLoad();//上传并显示
         
          px_old=p_x;//把当前玩家的全局坐标存起来
          py_old=p_y;//把当前玩家的全局坐标存起来

          //计算新的全局坐标，说下原理：
          //        玩家的全局坐标等于玩家在视野里的坐标和视野的坐标计算得来
          p_x=migong_map.x+migong_map.px;//视野的横坐标加上玩家在视野里的横坐标，就得到玩家的全局横坐标
          p_y=migong_map.y+7-migong_map.py;//视野的纵坐标加7，再减去玩家在视野里的纵坐标就得到玩家的全局坐标


          //通关判断
          //解释一下这个函数的功能：
          //前面两个坐标和后面两个坐标对角线所在矩形的范围就是目标范围
          //最后一个坐标就是玩家的全局坐标，该函数能够判断玩家当前是否处在目标范围内


          //通关判断，如果通关就执行滚屏动画，否则就跳过
          if(pass_xy(migong_map.mubiao_x,migong_map.mubiao_y,migong_map.mubiao_x+1,migong_map.mubiao_y+1,p_x,p_y)){//判断玩家是否在目标区域范围内，如果在范围内，则执行下面的通关动画
              //哦～是通关滚屏动画，动画完成后清屏
              
              
              for(int i=0;i<8;i++){
                  for(int j=0;j<8;j++){
                      lc.setLed_work(j,7-i,0); 
                      delay(10);
                  }
                 
                  lc.bitmap(lc.LedBuffer_work);//把视野复制进Ledbufer
                  lc.UpLoad();
              }

              for(int i=0;i<8;i++){
                  for(int j=0;j<8;j++){
                      lc.setLed_work(j,7-i,1);
                      delay(10);
                  }
                  
                  lc.bitmap(lc.LedBuffer_work);//把视野复制进Ledbufer
                  lc.UpLoad();
              }

              for(int i=0;i<8;i++){
                  for(int j=0;j<8;j++){
                      lc.setLed_work(j,7-i,0); 
                      delay(10);
                  }
                  
                  lc.bitmap(lc.LedBuffer_work);//把视野复制进Ledbufer
                  lc.UpLoad();
              }
              byte r;
              r=byte(random(0,8));
              while(r==migong_map.map_index){
                  r=random(0,8);
                }//随机生成一个和当前地图编号不同的地图
              
                  map_r(r);//随机加载新地图

              


              
          }

          


          //如果玩家当前的全局坐标和老坐标不一样，就发声音
          if((px_old!=p_x)||(py_old!=p_y)){//如果玩家移动，就发声
              my_beeper();

          }
          
                      
          
          ////////////////////////////////////////////////////////////////////////////////////////////////////////////休眠判断

          if((input_new>input_old+0.01)||(input_new<input_old-0.01)){//检测是否静止，只要有轻微移动就重置计时器
              null_time=millis();//休眠倒计时重置
          }else{
              if(millis()-null_time>timeout){//如果静止时长超过设定的时间，就进入休眠
                 
                  power_down();//进入休眠
              }
          }


          if(!mpu_6050_keyinput()){//重新获取陀螺仪输出               
              //根据输入，得到一个新的坐标
              if(pax>0.1){//往左倾斜
                px_0--;
                x_0--;
              }else if(pax<-0.1){//往右倾斜    
                px_0++;
                x_0++;
              }
              if(pay>0.1){//往下倾斜
                py_0--;
                y_0++;
              }else if(pay<-0.1){//往上倾斜
                py_0++;
                y_0--;
              }
          }
          //判断该新坐标玩家所处范围内是否有障碍，如果有则不更新实际坐标，如果没有就更新实际坐标
          if(!(lc.getLedState_work(migong_map.px,py_0,lc.LedBuffer_work)||lc.getLedState_work(migong_map.px+1,py_0,lc.LedBuffer_work)||lc.getLedState_work(migong_map.px,py_0+1,lc.LedBuffer_work)||lc.getLedState_work(migong_map.px+1,py_0+1,lc.LedBuffer_work))){//
            
           
                if(py_0>=2&&py_0<=4){

                    migong_map.py=py_0;//
                }
                //这个参数限定视野框在全局地图里的移动范围，保证视野框不会超出全局地图
                if(py_0<2||py_0>4){
                    if(y_0>=6&&y_0<=34){
                       
                       migong_map.y=y_0;
                    }
                }
          }


          if(!(lc.getLedState_work(px_0,migong_map.py,lc.LedBuffer_work)||lc.getLedState_work(px_0+1,migong_map.py,lc.LedBuffer_work)||lc.getLedState_work(px_0,migong_map.py+1,lc.LedBuffer_work)||lc.getLedState_work(px_0+1,migong_map.py+1,lc.LedBuffer_work))){//
                if(px_0>=2&&px_0<=4){
                 
                    migong_map.px=px_0;//
                }
                if(px_0<2||px_0>4){
                    if(x_0>=6&&x_0<=34){
                       migong_map.x=x_0;//
                    }
                }
                
          }


          x_0=migong_map.x;
          y_0=migong_map.y;
          px_0=migong_map.px;
          py_0=migong_map.py;
         
    }//迷宫循环
  }//迷宫开始条件判断
  while(guaxiang){//卜卦
      //lc.clearDisplay();
      if(millis()-null_time>timeout){//如果静止时长超过设定的时间，就进入休眠
          //Serial.println("准备关机");
          biaoqing_shufu();
          power_down();//屏保
      }
      daokou();
      
      float kx;
      float ky;
      kx=imu.ax();
      ky=imu.ay();
      int g;
      if(kx>0.5){
        
        for(int i=0;i<100;i++){
          if(imu.ax()<-0.5){
              zhonglifangxiang_panduan();
              lc.clearDisplay();  
              bagua_beeper();
              g=random(0,8);
              lc.bitmap(bagua[g]);
              lc.UpLoad();
              i=50;
          }
        }

        null_time=millis();
      }


      if(ky>0.5){
        
        for(int i=0;i<100;i++){
          if(imu.ay()<-0.5){
              zhonglifangxiang_panduan();
              lc.clearDisplay();  
              bagua_beeper();
              g=random(0,8);
              lc.bitmap(bagua[g]);
              lc.UpLoad();
              i=50;
          }
        }
        zhonglifangxiang_panduan();
        if(zhonglifangxiang_old!=lc.zhonglifangxiang){//当重力方向改变时刷新画面
                    lc.bitmap(bagua[g]);
                    lc.UpLoad();
        }
        null_time=millis();
      }





  }
  //摇骰子
  while(shaizi){
    yaoshaizi();
  }
  //沙漏计时器
  while(shalou){

    //沙漏初始化
    shalou_chushihua();

    //这里循环32次，随机在LedBuffer_work里找一个已存在的像素开始执行下落动画
    for(int i=0;i<32;i++){//沙子开始掉落


        //在点亮的像素里随机找一个像素，如果它下方是空的就全屏闪烁然后开始往下掉，如果退出游戏则略过本段代码，并且退出上级循环，跳到全部沙子挑落结束的位置
        if(i<=31&&shalou){
            //在留存的像素里随机找一个像素的坐标，这里依赖初级显存，必须保证显存里有点亮的像素，不然会卡死在这个循环里
            while(!(lc.getLedState_work(xx,yy,lc.LedBuffer_work)&&!lc.getLedState_work(xx,yy-1,lc.LedBuffer_work))&&shalou/*如果在剩下的沙子范围内且下方为空，或者沙漏退出，就退出循环*/){//这里是个死循环，如果上面没有像素了，会一直卡死在这里

                xx=random(0,8);
                yy=random(4,8);
                
            }

        //闪烁一下选中的像素,判断条件：当沙漏正常进行时，否则跳过
          if(shalou){
            if(beeper){
                tone(10,8000,50);
            }
            lc.setLed_work(xx,yy,0);
            lc.bitmap(lc.LedBuffer_work);
            lc.UpLoad(); 
            delay(50);
             if(beeper){
                noTone(10);
             }
            lc.setLed_work(xx,yy,1);
            lc.bitmap(lc.LedBuffer_work);
            lc.UpLoad(); 
            delay(100);

            lc.setLed_work(xx,yy,0);
            lc.bitmap(lc.LedBuffer_work);
            lc.UpLoad(); 
          }
        }else{
          i=32;


        }
        //根据生成的像素坐标，初始化一个像素参数
        piexl_int(&piexl_1,xx,yy,0,0);
        //执行这个像素的掉落过程
        move_piexl(&piexl_1);//这里面会做返回操作和重力变换检测，如果监测到返回菜单的操作，后面的代码应该立即跳过
  
    }//全部沙子掉落结束

    if(shalou){//如果已经退出游戏，则略过这段代码
        
        //这一小段代码是为了实现掉落结束的时候画面重力跟随，可有可无
        zhonglifangxiang_panduan();
          if(lc.zhonglifangxiang!=zhonglifangxiang_old){
              lc.bitmap(lc.LedBuffer_work);
              lc.UpLoad();
          }
        shalou_over();//沙漏结束的声光提醒，直到拿起才结束

          //清空两级显存
        for(int yj=0;yj<8;yj++){
            for(int xi=0;xi<8;xi++){
              lc.setLed_work(xi,yj,0);
            }
        }

        lc.bitmap(lc.LedBuffer_work);
        lc.clearDisplay();
        delay(300);
        //重力方向判断
        
    } 

    
  }//沙漏结尾
#endif

//======================================================条件编译================================================================
}//loop循环结束，万事万物都有终点，代码也一样
