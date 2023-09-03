
#include "LedControl.h"

//the opcodes for the MAX7221 and MAX7219
#define OP_NOOP   0
#define OP_DIGIT0 1
#define OP_DIGIT1 2
#define OP_DIGIT2 3
#define OP_DIGIT3 4
#define OP_DIGIT4 5
#define OP_DIGIT5 6
#define OP_DIGIT6 7
#define OP_DIGIT7 8
#define OP_DECODEMODE  9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15


//初始化
LedControl::LedControl(int dataPin, int clkPin, int csPin) {

    SPI_MOSI=dataPin;
    SPI_CLK=clkPin;
    SPI_CS=csPin;

    pinMode(SPI_MOSI,OUTPUT);
    pinMode(SPI_CLK,OUTPUT);
    pinMode(SPI_CS,OUTPUT);

    digitalWrite(SPI_CS,HIGH);//先拉高Load，禁止数据输入

    SPI_MOSI=dataPin;

    //清空点阵缓存
    for(int i=0;i<64;i++){
	status[i]=0x00;
    }
	//设置测试模式为常规操作，不做检测0，做检测就是1
        spiTransfer(OP_DISPLAYTEST,0);

        //设置显示x坐标列的范围，弄到最大0-7
        setScanLimit(7);

        //设置译码模式为不译码0
        spiTransfer(OP_DECODEMODE,0);

        clearDisplay();

        //初始化为全灭状态
        shutdown(true);
	zhonglifangxiang=3;
	//fanxiang_flag=0;

}


//关闭显示
void LedControl::shutdown(bool b) {

    if(b)
        spiTransfer(OP_SHUTDOWN,0);
    else
        spiTransfer(OP_SHUTDOWN,1);
}

void LedControl::setScanLimit(int limit) {

    if(limit>=0 && limit<8)
        spiTransfer(OP_SCANLIMIT,limit);
}

void LedControl::setIntensity(int intensity) {

    if(intensity>=0 && intensity<16)	
        spiTransfer(OP_INTENSITY,intensity);
}

void LedControl::clearDisplay() {//把显存数组清零，然后上传

    for(int i=0;i<8;i++) {
	LedBuffer[i]=0;
        status[i]=0;
        spiTransfer(i+1,status[i]);
    }
}
void LedControl::clearDisplay_work() {//把显存数组清零，然后上传

    for(int i=0;i<8;i++) {
	LedBuffer_work[i]=0;
    }
}

//在后级显存里写入指定坐标信息，数组最左下面为原点
void LedControl::setLed(int x, int y, byte state) {//左下角为原点
	//坐标系方向变换
	//充电口为下（默认坐标系是充电口朝右，所以需要把坐标系逆时针旋转90度）
    byte x_m=1;//用来做移位运算缓存
    byte x_b=LedBuffer[7-y];
    //检查参数合理性
    if(x<0 || x>7 || y<0 || y>7)
        return;
    if(state){
		
		LedBuffer[7-y]=x_b|(x_m<<(7-x));//点亮

	}else if(state==0){
		
		LedBuffer[7-y]=x_b&(~(x_m<<(7-x)));//熄灭
	}

}
//在初级显存里写入指定坐标信息，数组最左下面为原点
void LedControl::setLed_work(int x, int y, byte state) {//左下角为原点
	//坐标系方向变换
	//充电口为下（默认坐标系是充电口朝右，所以需要把坐标系逆时针旋转90度）
    byte x_m=1;//用来做移位运算缓存
    byte x_b=LedBuffer_work[7-y];
    //检查参数合理性
    if(x<0 || x>7 || y<0 || y>7){return;}

    if(state){
		
		LedBuffer_work[7-y]=x_b|(x_m<<(7-x));//点亮

	}else if(state==0){
		
		LedBuffer_work[7-y]=x_b&(~(x_m<<(7-x)));//熄灭
	}

}

//点阵驱动芯片传输
void LedControl::spiTransfer(volatile byte opcode, volatile byte data) {//（x列，该列的数据）
    //Create an array with the data to shift out


    for(int i=0;i<2;i++)
        spidata[i]=(byte)0;
    //put our device data into the array
    spidata[1]=opcode;
    spidata[0]=data;
    //enable the line 

    digitalWrite(SPI_CS,LOW);//写入数据之前先把Load拉低
    //Now shift out the data 
    for(int i=2;i>0;i--)
        shiftOut(SPI_MOSI,SPI_CLK,MSBFIRST,spidata[i-1]);
    //latch the data onto the display
    digitalWrite(SPI_CS,HIGH);//写完之后，拉高Load载入数据

}    
//上传后级显存内容到屏幕上
void LedControl::UpLoad(){

		for(byte i=1;i<9;i++){
    			spiTransfer(i,LedBuffer[i-1]);
		}

}

//判断在后级缓存里指定坐标像素的状态
byte LedControl::getLedState(byte x, byte y){//这个用于判断用于上传的显存里指定坐标像素的状态


        byte x_m=1;//用来做移位运算缓存
	byte x_b=0;
    	if(x>=0&&x<=7&&y>=0&&y<=7){
		x_b=LedBuffer[7-y]&(x_m<<(7-x));
	}else{return 3;}//如果参数不合法就返回3

	if(x_b){return 1;}else{return 0;}

}
//判断在外部缓存里指定坐标像素的状态
byte LedControl::getLedState_work(int x, int y, byte view[8]){//获取外部显存数组指定坐标的状态,这个用于像素互动判断


        byte x_m=1;//用来做移位运算缓存
	byte x_b=0;
    	if(x>=0&&x<=7&&y>=0&&y<=7){
		x_b=view[7-y]&(x_m<<(7-x));
	}else{return 3;}//如果参数不合法就返回3

	if(x_b){return 1;}else{return 0;}

}

//把指定画面转存到初级显存里
void LedControl::bitmap_work(byte buffer[8]){//负责把游戏运行显存的内容拷贝到初级显存



			for(int i=0;i<8;i++){
				LedBuffer_work[i]=buffer[i];
			}


}

//把指定画面转存到后级显存里
void LedControl::bitmap(byte buffer[8]){//位图显示，用于显示指定的位图，根据屏幕姿态控制显示方向，可以实现画面跟随重力自动旋转,fangxiang为上下左右对应的1342

	//默认重力方向为下3

	switch(zhonglifangxiang){//根据当前方向和按键输入，更新蛇头移动方向
		
		case 1://上方朝下时

				for(int j=0;j<8;j++){
						LedBuffer[j]=0;
						for(int i=0;i<8;i++){
							LedBuffer[j]|=((((buffer[7-j]>>i)&1))<<(7-i));
		
						}
				}
			

			break;

		case 4://左方朝下时ok
			for(int i=0;i<8;i++){
				LedBuffer[7-i]=0;
				for(int j=0;j<8;j++){
					LedBuffer[7-i]|=((buffer[7-j]&(0b00000001<<i))>>i)<<(7-j);
					
				}
			}
			break;

		case 3://下方朝下时【默认方向，就直接写入，不做变换】ok
			for(int i=0;i<8;i++){
				LedBuffer[i]=buffer[i];
			}

			break;

		case 2://右方朝下时ok

			for(int i=0;i<8;i++){
				LedBuffer[7-i]=0;

				for(int j=0;j<8;j++){
					LedBuffer[7-i]|=((buffer[j]&(0b10000000>>i))<<i)>>j;
					
				}
			}
			
			break;

	}

}

//把后级缓存里的内容按某个方向滚动一格
void LedControl::roll(byte lr,byte ud){//左右，上下，某个方向不滚动的话就设置为0

	//定义：左滚动1，右滚动2，上滚动3，下滚动4
	if(lr==1){

		for(int i=0;i<8;i++){
			LedBuffer[i]=LedBuffer[i]<<1;
		}

	}

	if(lr==2){

		for(int i=0;i<8;i++){
			LedBuffer[i]=LedBuffer[i]>>1;
		}

	}

	if(ud==3){

		for(int i=0;i<7;i++){
			
				LedBuffer[i]=LedBuffer[i+1];
			
		}
		LedBuffer[7]=0;

	}

	if(ud==4){

		for(int i=7;i>0;i--){
			
				LedBuffer[i]=LedBuffer[i-1];
			
		}
		LedBuffer[0]=0;

	}
}

