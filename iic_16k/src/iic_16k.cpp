
#include "iic_16k.h"


//初始化函数
iic_16k::iic_16k(){//初始化函数
        iic_SCL=19;//flash时钟线
	iic_SDA=18;//flash数据线
	iic_WP=17;//flash写保护	
	
	
	dtime=1;//延时x微秒

	pinMode(iic_WP,OUTPUT);
	pinMode(iic_SCL,OUTPUT);
	pinMode(iic_SDA,OUTPUT);

	digitalWrite(iic_WP,LOW);//允许读写操作
	delayMicroseconds(dtime);
	digitalWrite(iic_SCL,LOW);//时钟拉低，防止误判
	delayMicroseconds(dtime);
	//总线进入待机模式
	digitalWrite(iic_SDA,HIGH);//拉高SDA
	delayMicroseconds(dtime);
	digitalWrite(iic_SCL,HIGH);//拉高SCL
	delayMicroseconds(dtime);
}


//重置激活函数

void iic_16k::iic_reset(){
	iic_start();

	//拉高SDA
	digitalWrite(iic_SDA,HIGH);//拉高SDA
	delayMicroseconds(dtime);
   for(int i=0;i<9;i++){

	//拉高SCL
	digitalWrite(iic_SCL,HIGH);//拉高SCL
	delayMicroseconds(dtime);
	//拉低SCL
	digitalWrite(iic_SCL,LOW);//时钟拉低，为后续传输做好准备
	delayMicroseconds(dtime);
   }

	iic_start();
	iic_stop();


}
//起始信号函数
void iic_16k::iic_start(){//起始信号,时钟高电平时，数据线从高到低跳变
	
	//拉低时钟
	digitalWrite(iic_SCL,LOW);//时钟拉低，防止误判
	delayMicroseconds(dtime);	
	//恢复SDA为输出模式
	pinMode(iic_SDA,OUTPUT);
	delayMicroseconds(dtime);
	//拉高SDA
	digitalWrite(iic_SDA,HIGH);//拉高SDA
	delayMicroseconds(dtime);
	//拉高SCL
	digitalWrite(iic_SCL,HIGH);//拉高SCL
	delayMicroseconds(dtime);
	//拉低SDA
	digitalWrite(iic_SDA,LOW);//拉低SDA
	delayMicroseconds(dtime);
	//拉低SCL
	digitalWrite(iic_SCL,LOW);//时钟拉低，为后续传输做好准备
	delayMicroseconds(dtime);
}

void iic_16k::iic_stop(){//结束信号，时钟高电平时，数据线从低到高跳变

	//拉低时钟
	digitalWrite(iic_SCL,LOW);//时钟拉低，防止误判
	delayMicroseconds(dtime);	
	//恢复SDA为输出模式
	pinMode(iic_SDA,OUTPUT);
	delayMicroseconds(dtime);
	//拉低SDA
	digitalWrite(iic_SDA,LOW);//拉低SDA
	delayMicroseconds(dtime);
	//拉高SCL
	digitalWrite(iic_SCL,HIGH);//拉高SCL
	delayMicroseconds(dtime);
	//拉高SDA
	digitalWrite(iic_SDA,HIGH);//拉高SDA
	delayMicroseconds(dtime);
	//delay(100);
	
	//至此总线恢复到待机状态都是高电平

}

byte iic_16k::iic_get_ack(){//读取响应
	//临时存储SDA电平
	byte ack;

	//SDA变为输入
	pinMode(iic_SDA,INPUT);

	//标准延时
	delayMicroseconds(dtime);

	//SCL高电平
	digitalWrite(iic_SCL,HIGH);//拉高SCL

	//标准延时
	delayMicroseconds(dtime);

	//判断SDA电平
	if(digitalRead(iic_SDA)){

		ack=1;
	}else{
		ack=0;
	}

	//SCL低电平
	digitalWrite(iic_SCL,LOW);//拉低SCL
	//返回结果
	return ack;
}
void iic_16k::iic_send_ack(){//发送响应

	//SDA变为输出
	pinMode(iic_SDA,OUTPUT);

	//标准延时
	delayMicroseconds(dtime);

	//拉低SDA
	digitalWrite(iic_SDA,LOW);//拉低SDA

	//标准延时
	delayMicroseconds(dtime);

	//拉高SCL
	digitalWrite(iic_SCL,HIGH);//拉高SCL


}


void iic_16k::iic_write_byte(byte Dadr,byte Badr,byte data){//(8页页地址（0-7），256个字节地址（0-255），数据)在指定地址写入一个字节的内容(页地址的前4位是固定的1010，后三位组成页地址共8页，最后一位是读写位。0是写操作，1是读操作)
	Wire.end();	
	//复原页地址
	Dadr=0b10100000|(Dadr<<1);
	digitalWrite(iic_WP,LOW);//允许读写操作
	iic_reset();
	//标准延时
	delayMicroseconds(dtime);

	//起始信号
	iic_start();
	

	//发送设备地址+页地址+写信号
	for(int i=0;i<8;i++){
  		flag=(Dadr<<i)&0b10000000;//逐位判断是1还是0，决定后面是高电平还是低电平
  		if(flag){
			digitalWrite(iic_SDA,HIGH);
		}else{
			digitalWrite(iic_SDA,LOW);
		}

		delayMicroseconds(dtime);

  		digitalWrite(iic_SCL,HIGH);

		delayMicroseconds(dtime);

  		digitalWrite(iic_SCL,LOW);

		delayMicroseconds(dtime);
	}	




while(iic_get_ack()){
;
}




	//发送字节地址
  	digitalWrite(iic_SCL,LOW);
	pinMode(iic_SDA,OUTPUT);
	//标准延时
	delayMicroseconds(dtime);

	for(int i=0;i<8;i++){
  		flag=(Badr<<i)&0b10000000;
  		if(flag){
			digitalWrite(iic_SDA,HIGH);
		}else{
			digitalWrite(iic_SDA,LOW);
		}

  		digitalWrite(iic_SCL,HIGH);

		delayMicroseconds(dtime);

  		digitalWrite(iic_SCL,LOW);

		delayMicroseconds(dtime);

	}


while(iic_get_ack()){
;
}


	//发送数据
	digitalWrite(iic_SCL,LOW);
  	pinMode(iic_SDA,OUTPUT);
	delayMicroseconds(dtime);

	for(int i=0;i<8;i++) {
  		flag=(data<<i)&0b10000000;
  		if(flag){
			digitalWrite(iic_SDA,HIGH);
		}else{
			digitalWrite(iic_SDA,LOW);
		}

		delayMicroseconds(dtime);

  		digitalWrite(iic_SCL,HIGH);

		delayMicroseconds(dtime);

  		digitalWrite(iic_SCL,LOW);

		delayMicroseconds(dtime);
	}


while(iic_get_ack()){
;
}



	//结束信号
	iic_stop();
	
	digitalWrite(iic_WP,HIGH);//写保护
	delay(5);//这个写入周期时间一定要大于5毫秒



}

//////////随机读取
byte iic_16k::iic_read_byte(byte Dadr,byte Badr){//()读取指定位置的一个字节
Wire.end();
digitalWrite(iic_WP,LOW);//允许读写操作

iic_reset();

	//起始信号
	iic_start();
	//复原页地址
	Dadr=0b10100000|(Dadr<<1);
//发送设备地址+页地址+写信号
	for(int i=0;i<8;i++){
  		flag=(Dadr<<i)&0b10000000;
  		if(flag){
			digitalWrite(iic_SDA,HIGH);
		}else{
			digitalWrite(iic_SDA,LOW);
		}

		delayMicroseconds(dtime);

 		digitalWrite(iic_SCL,HIGH);

		delayMicroseconds(dtime);

  		digitalWrite(iic_SCL,LOW);

		delayMicroseconds(dtime);
	}

while(iic_get_ack()){
;
}

//发送字节地址

  	digitalWrite(iic_SCL,LOW);
  	delayMicroseconds(dtime);
  	pinMode(iic_SDA,OUTPUT);
  	delayMicroseconds(dtime);
	for(int i=0;i<8;i++){
  		flag=(Badr<<i)&0b10000000;
  		if(flag){
			digitalWrite(iic_SDA,HIGH);
		}else{
			digitalWrite(iic_SDA,LOW);
		}


		delayMicroseconds(dtime);

  		digitalWrite(iic_SCL,HIGH);
		delayMicroseconds(dtime);
  		digitalWrite(iic_SCL,LOW);
		delayMicroseconds(dtime);
	}

while(iic_get_ack()){
;
}



//起始信号
	iic_start();


//发送设备地址+读信号
	Dadr=Dadr+1;

	for(int i=0;i<8;i++){
  		flag=(Dadr<<i)&0b10000000;
  		if(flag){
			digitalWrite(iic_SDA,HIGH);
		}else{
			digitalWrite(iic_SDA,LOW);
		}

		delayMicroseconds(dtime);
  		digitalWrite(iic_SCL,HIGH);
		delayMicroseconds(dtime);
  		digitalWrite(iic_SCL,LOW);
		delayMicroseconds(dtime);//--------
	}



while(iic_get_ack()){
;
}



//读取8位数据，高位优先
	byte Rdata=0;
	for(int i=7;i>=0;i--){
  		digitalWrite(iic_SCL,HIGH);
  		delayMicroseconds(dtime);

  		if(digitalRead(iic_SDA)==1){
			Rdata=Rdata|(0b00000001<<i);
		}

  		digitalWrite(iic_SCL,LOW);

  		delayMicroseconds(dtime);
	}


	
	//结束信号
	iic_stop();

	delayMicroseconds(dtime);

	digitalWrite(iic_WP,HIGH);//写保护

	return Rdata;

}


