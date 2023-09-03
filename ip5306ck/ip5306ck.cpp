
#include "ip5306ck.h"


ip5306ck::ip5306ck(){//()初始化函数
	//检测引脚
	LED_1=A0;
	LED_2=A1;
	LED_3=A2;

  	//定义电量读取引脚
  	pinMode(LED_1,INPUT);
  	pinMode(LED_2,INPUT);
  	pinMode(LED_3,INPUT);


}


byte ip5306ck::led_state(){//获取电量

	//统计四个灯亮灯的占空比（暂时设定的是检测1000次）
		z1=0;
		z2=0;
		z3=0;
		z4=0;


		for(int i=0;i<1000;i++){

			ss1=digitalRead(LED_1);
			ss2=digitalRead(LED_2);

			if(analogRead(LED_3)>500){
				ss3=1;
			}else{
				ss3=0;
			}

			if(ss1&&!ss3){z1++;}
			if(!ss1&&ss3){z2++;}
			if(ss2&&!ss3){z3++;}
			if(!ss2&&ss3){z4++;}
			if(ss1&&!ss2){z4++;z1++;}
			if(!ss1&&ss2){z2++;z3++;}

		}



		//Serial.print("LED_1= ");
		if(z1>400){//每个灯亮占比大于400，就视为点亮状态
  			led1=1;
  			//Serial.print("亮");
  			//Serial.print("  ");
		}else{
  			led1=0;
  			//Serial.print("灭");
  			//Serial.print("  ");
		}


		//Serial.print("LED_2= ");
		if(z2>400){
  			led2=1;
  			//Serial.print("亮");
  			//Serial.print("  ");
		}else{
  			led2=0;
  			//Serial.print("灭");
  			//Serial.print("  ");
		}

		//Serial.print("LED_3= ");
		if(z3>400){
  			led3=1;
  			//Serial.print("亮");
  			//Serial.print("  ");
		}else{
  			led3=0;
  			//Serial.print("灭");
  			//Serial.print("  ");
		}

		//Serial.print("LED_4= ");
		if(z4>400){
  			led4=1;
  			//Serial.print("亮");
  			//Serial.println("  ");
		}else{
  			led4=0;
  			//Serial.print("灭");
  			//Serial.println("  ");
		}


		if(led1&&led2&&led3&&led4){//四灯全亮，充电充满，或者放电大于75%。
			//return 0b00001111;//=15
			return 6;
		}


		if(led1&&led2&&led3&&!led4){//充电，大于等于75%，放电50%-75%
			//return 0b00001110;//=14
			return 4;
		}


		if(led1&&led2&&!led3&&!led4){//充电50%-75%，放电25%-50%
			//return 0b00001100;//=12
			return 2;
		}


		if(led1&&!led2&&!led3&&!led4){//充电25%-50%，放电3%-25%
			//return 0b00001000;//=8
			return 1;

		}


		if(!led1&&!led2&&!led3&&!led4){//充电小于25%，放电0%-3%
			//return 0b00000000;//=0
			return 0;
		}
		
		return 0;//默认返回0


}


byte ip5306ck::bat_state(){//判断当前电池电量（低电量或者充满）以及是否正在充电，低电量返回0
	charge_state=0;
	byte state1;//
	byte state2;//


	//判断是否正在充电
		//读取十次四个灯的状态
		state1=led_state();	
		for(int i=0;i<10;i++){//增加第二次检测的周期，提高对变化检测的准确性
			state2=led_state();
			if(state2!=state1){//扫描十次，只要有一次跟state1不一样，就判定为正在充电，就立马返回充电状态的电量	
				charge_state=1;//在充电，把充电指示位置1
				i=100;
				return state1>state2?state1:state2;//如果两次不一样，就返回大的那个
				
			}
			
		}

		
		return state1;//返回当前电量值

			
}



