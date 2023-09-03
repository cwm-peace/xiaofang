
#include <avr/pgmspace.h>
#include <Wire.h>
#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
//定义一条蛇

class she{
    private:
	//头和尾的坐标

	byte fangxiang;//用来标记当前蛇的运动方向,上1，右2，下3，左4

	byte she_body[64];//用来存储蛇身体坐标信息，定义：* * ｜0 0 0｜0 0 0			  
	  //                                         不 用 |x坐 标|y坐标

	byte tou_adr;//保存最新头在body数组里的位置
	byte wei_adr;//保存最新尾在body数组里的位置
	void refresh_body();//更新she_body[64]数组里的数据
	void pengzhuang_jiance();//检测是否超出边界，撞到自己，吃到食物，蛇头每更新一次就运行一次
	void she_body_reset();//重新整理蛇身体数组的顺序
	void food_create();//创建食物坐标，不能与当前蛇身重叠
    public:
	byte defen;//记录游戏得分
	byte food_xy;//食物的坐标，定义同上
	byte tou_x;//取值范围0-7，外部绘图使用
	byte tou_y;//取值范围0-7，外部绘图使用
	byte wei_x;//取值范围0-7,外部绘图使用
	byte wei_y;//取值范围0-7，外部绘图使用
	byte body_add;//标记蛇身是否增长
	byte food_flag;//标记是否吃到食物

	byte she_body_length;//定义蛇身体长度

	byte she_life_state;//标记蛇的生命状态，生存1还是死亡0
	byte tou_xy;//整合头坐标为字节
	byte wei_xy;
	byte wei_xy_del;

	she();//构造函数，初始化函数，记住这个函数不要有类型，它就是个光杆司令
	void key_fangxiang(byte key_flag);//根据按键标志位修改蛇的运动方向标识位
	void move();//根据fangxiang标志位移动一次
	
	void reset_she();//重置函数
	
	

};

