
#include <avr/pgmspace.h>
#include <Wire.h>
#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif


class ip5306ck{
    private:

	//检测引脚
	int LED_1;
	int LED_2;
	int LED_3;


	//记录引脚电平
	int ss1;
	int ss2;
	int ss3;


	//统计四个灯亮灯的占空比（暂时设定的是检测1000次）
	int z1;
	int z2;
	int z3;
	int z4;


	//只要亮灯占比超过设定值就定义为亮，低于设定值就定义为灭，这几个变量直接给出指示灯的实际状态
	byte led1;
	byte led2;
	byte led3;
	byte led4;
	

	



    public:
	byte charge_state;//指示是否处在充电状态
	ip5306ck();//初始化函数
	byte led_state();//统计一千次四个3个引脚的电平，通过占空比统计出四个灯的亮灭状态
	byte bat_state();//判断当前电池电量（低电量或者充满）以及是否正在充电，低电量返回0，
};

