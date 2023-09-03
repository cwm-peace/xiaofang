#include <avr/pgmspace.h>
#include <Wire.h>
#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

class iic_16k {

    private :
        	int iic_SCL;//flash时钟线
		int iic_SDA;//flash数据线
		int iic_WP;//flash写保护
		int dtime;//延时时长
		byte flag;//用于标记传输位
		


		void iic_start();//起始信号
		void iic_stop();//结束信号
		

    public:

		iic_16k();//初始化函数
		//单字节写入
		void iic_write_byte(byte Dadr,byte Badr,byte data);//(页地址，字节地址，数据)在指定地址写入一个字节的内容
		//单字节读取
		byte iic_read_byte(byte Dadr,byte Badr);//()读取指定位置的一个字节
		//顺序字节读取
		void iic_read_map(byte Dadr,byte Badr,byte * map[16]);//()地图加载函数
		byte iic_get_ack();//读取响应
		void iic_send_ack();//发送响应
		void iic_reset();//重置激活flash

};

