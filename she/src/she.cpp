#include "she.h"
////贪吃蛇类
she::she(){//构造函数

	//初始化头和尾的坐标，刚开始长度为零，所以头和尾重叠
	for(int i=0;i<64;i++){she_body[i]=0;}//清空身体信息
	tou_adr=0;//保存最新头在body数组里的位置
	wei_adr=0;//保存最新尾在body数组里的位置
	tou_x=3;
	tou_y=3;
	tou_xy=(tou_x<<3)|tou_y;
	she_body[tou_adr]=tou_xy;
	wei_xy=tou_xy;
	wei_x=wei_xy>>3;
	wei_y=wei_xy&0B00000111;
	defen=0;
	fangxiang=1;//初始化移动方向上1，右2，下3，左4

	//随机初始化食物坐标
	food_create();
	body_add=0;
	she_body_length=1;//定义蛇身体长度
	she_life_state=1;


}


//重置蛇
void she::reset_she(){
		//初始化头和尾的坐标，刚开始长度为零，所以头和尾重叠
	for(int i=0;i<64;i++){she_body[i]=0;}//清空身体信息
	tou_adr=0;//保存最新头在body数组里的位置
	wei_adr=0;//保存最新尾在body数组里的位置
	tou_x=3;
	tou_y=3;
	tou_xy=(tou_x<<3)|tou_y;
	she_body[tou_adr]=tou_xy;
	wei_xy=tou_xy;
	wei_x=wei_xy>>3;
	wei_y=wei_xy&0B00000111;
	defen=0;
	fangxiang=1;//初始化移动方向上1，右2，下3，左4

	//随机初始化食物坐标
	food_create();
	body_add=0;
	she_body_length=1;//定义蛇身体长度
	she_life_state=1;
	//food_flag=0;//标记是否吃到食物



}

//ok
void she::key_fangxiang(byte key_flag){//根据按键标志位修改蛇的运动方向fangxiang，外部标识位key_scan.key_flag,上1，右2，下3，左4


	switch(fangxiang){//根据当前方向和按键输入，更新蛇头移动方向
		
		case 1:

			switch(key_flag){
				case 2:
					fangxiang=2;
					break;
				case 4:
					fangxiang=4;
					break;
			}
			break;

		case 2:
			switch(key_flag){
				case 1:
					fangxiang=1;
					break;

				case 3:
					fangxiang=3;
					break;
			}
			break;

		case 3:
			switch(key_flag){
				case 2:
					fangxiang=2;
					break;

				case 4:
					fangxiang=4;
					break;
			}
			break;

		case 4:
			switch(key_flag){
				case 1:
					fangxiang=1;
					break;
				case 3:
					fangxiang=3;
					break;

			}
			break;





	}

     


}


//ok
void she::move(){//得到一个新的蛇头坐标



	switch(fangxiang){

		case 1://向上，则y递增



				tou_y++;
				tou_xy=(tou_x<<3)|tou_y;
			
			
			break;
		case 2://向右，则x递增

				tou_x++;
				tou_xy=(tou_x<<3)|tou_y;
			

			break;
		case 3://向下，则y递减

				tou_y--;
				tou_xy=(tou_x<<3)|tou_y;
		
			break;

		case 4://向左，则x递减

				tou_x--;
				tou_xy=(tou_x<<3)|tou_y;
			
			break;
	}

	pengzhuang_jiance();//根据当前新的坐标，判断蛇的状态，要么死，要么吃到食物，要么什么都不发生

}


//ok
void she::pengzhuang_jiance(){//检测当前头坐标是否超出边界，是否撞到自己，是否吃到食物,被move函数调用
	


	//如果当前新坐标超出屏幕边缘，死路一条
	if(tou_x>7||tou_x<0||tou_y>7||tou_y<0){

		she_life_state=0;//
	}
	
	//判断蛇是不是撞上了自己
	if(she_life_state==1){
		for(int i=0;i<she_body_length;i++){

			if(tou_xy==she_body[i]){//如果撞上自己，死路一条
					she_life_state=0;
					i=she_body_length;
					
			}
		}
	}				
	//没撞上自己？那看是不是撞上食物
	if(she_life_state==1){
		if(tou_xy==food_xy){//如果坐标和食物重合
							
			if(she_body_length<63){
				
				food_flag=1;//吃到食物标志为1
				
				defen++;
				//delay(1000);
			}	
								
		}
	}	

	if(she_life_state==1){
		//更新一次body数组
		refresh_body();//根据
		
	}
}


//ok
void she::refresh_body(){//更新she_body[64]数组里的数据，该函数在头坐标每改变一次的时候也运行一次

	switch(food_flag){
		
		case 0://当什么都没发生的时候

			//保存蛇尾坐标信息
			wei_xy=she_body[wei_adr];

			//更新蛇尾位置
			if(wei_adr<she_body_length-1){
				wei_adr++;//如果没触边就变为后面一个位置
			
			}else if(wei_adr==she_body_length-1){
				wei_adr=0;//如果触边，就归零
			}
			
			//更新蛇头位置
			if(tou_adr<she_body_length-1){
				tou_adr++;//如果没触边就变为后面一个位置
			
			}else if(tou_adr==she_body_length-1){
				tou_adr=0;//如果触边，就归零
			}

			//在新蛇头位置写入当前蛇头坐标信息
			she_body[tou_adr]=tou_xy;
			
			//头尾坐标解算，用于绘制
			tou_x=tou_xy>>3;
			tou_y=tou_xy&0B00000111;
			wei_x=wei_xy>>3;
			wei_y=wei_xy&0B00000111;
			body_add=0;

			break;
			
		case 1://当吃到食物的时候ok
			
				
			she_body_reset();//先整理蛇身数组

			tou_adr=she_body_length;//更新蛇头位置

			she_body[tou_adr]=tou_xy; //更新蛇头坐标
			
			wei_adr=0;//重置蛇尾位置尾0
			wei_xy=she_body[wei_adr];
			she_body_length++;//蛇身长度加1		
			//头尾坐标解算，用于绘制
			tou_x=tou_xy>>3;
			tou_y=tou_xy&0B00000111;
			wei_x=wei_xy>>3;
			wei_y=wei_xy&0B00000111;

			food_create();//重新生成食物
			body_add=1;

			break;
			

	}
	
}



void she::she_body_reset(){

	byte she_body_temp[64];//一个临时数组用来重新整理蛇身数据的顺序，整理完了再按顺序写回原数组

	for(int i=wei_adr;i<she_body_length;i++){
		she_body_temp[i-wei_adr]=she_body[i];//把蛇尾坐标到蛇身长度所及位置的数据存从零存入，

	}
	for(int i=0;i<=tou_adr;i++){
		she_body_temp[i+she_body_length-wei_adr]=she_body[i];//把蛇头左边的数据也整理进去

	}

	for(int i=0;i<she_body_length;i++){//把数据再复制回原数组

	she_body[i]=she_body_temp[i];


	}
	

}


void she::food_create(){
	
			//更新食物坐标
			byte food;
			byte flag=1;
		while(flag){
			food=0;
			//randomSeed(analogRead(A0));//根据模拟口更新随机数种子
			food=food|(random(0,7)<<3);//生成x坐标
			food|=random(0,7);//生成y坐标
			flag=0;
			for(int i=0;i<she_body_length;i++){
				if(food==she_body[i]){
					flag=1;//如果坐标和身体重合，就立即退出循环重新开始生成
					
					break;
				}
				
				
			}
		}
		
		food_xy=food;
		
			
		food_flag=0;//食物标志归零

}
