/*
 *    LedControl.h - A library for controling Leds with a MAX7219/MAX7221
 *    Copyright (c) 2007 Eberhard Fahle
 * 
 *    Permission is hereby granted, free of charge, to any person
 *    obtaining a copy of this software and associated documentation
 *    files (the "Software"), to deal in the Software without
 *    restriction, including without limitation the rights to use,
 *    copy, modify, merge, publish, distribute, sublicense, and/or sell
 *    copies of the Software, and to permit persons to whom the
 *    Software is furnished to do so, subject to the following
 *    conditions:
 * 
 *    This permission notice shall be included in all copies or 
 *    substantial portions of the Software.
 * 
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *    OTHER DEALINGS IN THE SOFTWARE.
 */
 
#ifndef LedControl_h
#define LedControl_h

#include <avr/pgmspace.h>
#include <Wire.h>
#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

class LedControl {
    private :
        /* The array for shifting the data to the devices */
        byte spidata[2];
        /* 向max7219点阵传输信息 */

        void spiTransfer(byte opcode, byte data);

	

        /* 设置点阵的缓存 */
        byte status[64];
        /* 数据输出*/
        int SPI_MOSI;
        /* 这里接时钟信号 */
        int SPI_CLK;
        /* 低电平选中设备 */
        int SPI_CS;
        /* 所使用的最大设备数量 */
        int maxDevices=1;


	/*后级显存，用来作为最终上传的数据*/  // y
	byte LedBuffer[8]={0B00000000,//0
			   0B00000000,//1
			   0B00000000,//2
			   0B00000000,//3
			   0B00000000,//4
			   0B00000000,//5
		  	   0B00000000,//6
			   0B00000000// 7
//                         x 01234567
			   };

    public:
	//byte fanxiang_flag;//0不反相，1反相
	byte zhonglifangxiang;//设置重力方向
	/*初级显存，用来像素互动*/       // y
	byte LedBuffer_work[8]={
			   0B00000000,//0
			   0B00000000,//1
			   0B00000000,//2
			   0B00000000,//3
			   0B00000000,//4
			   0B00000000,//5
		  	   0B00000000,//6
			   0B00000000// 7
//                         x 01234567
			   };

        LedControl(int dataPin, int clkPin, int csPin);//只有一块8*8点阵，所以不需要设备数量这个参数




        void shutdown(bool status);//关闭led屏


        void setScanLimit(int limit);//==没搞懂这个函数是什么功能

        void setIntensity(int intensity);//设置led屏的亮度，亮度范围0-15

        void clearDisplay();//熄灭所有灯
        void clearDisplay_work();//熄灭所有灯

        void setLed(int x, int y, byte state);//设置指定坐标单个led的亮灭，row就是坐标x，col就是坐标y

	 //刷新全屏信息
	void UpLoad();
	//获取缓存里指定坐标的状态，返回1或者0
	byte getLedState(byte x, byte y);
	byte getLedState_work(int x, int y, byte view[8]);
	void setLed_work(int x, int y, byte state);//增加重力方向，根据重力方向自动变换坐标系方向
	

	//图形显示，把图形数组传递给显存数组
	void bitmap(byte buffer[8]);//按指定方向变化画面防线写入后级显存，根据屏幕姿态控制显示方向，可以实现画面跟随重力自动旋转,fangxiang为上下左右对应的1342
	void bitmap_work(byte buffer[8]);//负责把游戏运行显存的内容拷贝到初级显存里

	//按某个方向滚动
	void roll(byte lr,byte ud);//左右，上下，左右速度，上下速度
	//void Led_rain(byte flag);//数字雨屏保
	byte byte_fanzhuan(byte b);//镜像一个字节


};
#endif