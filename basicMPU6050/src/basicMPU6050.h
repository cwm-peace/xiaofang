#include "Arduino.h"
#include <Wire.h>

#ifndef basicMPU6050_h
#define basicMPU6050_h

//------------------ Common terms -------------------- 

// Sensor values
#define             MPU_ADDRESS_LOW     0x68    //小方已把A0下拉，所以iic地址为0x68            
#define             MPU_ADDRESS_HIGH    0x69
#define             ACCEL_LSB_0         16384.0
#define             N_AXIS              3

//---------------- Default settings ------------------ [ No characters after backlash! ]

constexpr float     DEFAULT_SCALE = 1;                  // Default scale of Gyro and Accelerometer

#define TEMPLATE_DEFAULT                            \
    uint8_t         DLPF_CFG    = 6                 ,   /* Low pass filter setting - 0 to 6         */  \
    uint8_t         FS_SEL      = 0                 ,   /* Gyro sensitivity - 0 to 3                */  \
    uint8_t         AFS_SEL     = 0                 ,   /* Accelerometer sensitivity - 0 to 3       */  \
    bool            ADDRESS_A0  = LOW               ,   /* State of A0 pin to set I2C address       */  \
    int16_t         AX_OFS      = 0                 ,   /* Accel X offset                           */  \
    int16_t         AY_OFS      = 0                 ,   /* Accel Y offset                           */  \
    int16_t         AZ_OFS      = 0                 ,   /* Accel Z offset                           */  \
    const float*    AX_S        = &DEFAULT_SCALE    ,   /* Accel X Multiplier                       */  \
    const float*    AY_S        = &DEFAULT_SCALE    ,   /* Accel Y Multiplier                       */  \
    const float*    AZ_S        = &DEFAULT_SCALE    ,   /* Accel Z Multiplier                       */  \
    const float*    GX_S        = &DEFAULT_SCALE    ,   /* Gyro X Multiplier                        */  \
    const float*    GY_S        = &DEFAULT_SCALE    ,   /* Gyro Y Multiplier                        */  \
    const float*    GZ_S        = &DEFAULT_SCALE    ,   /* Gyro Z Multiplier                        */  \
    uint16_t        GYRO_BAND   = 64                ,   /* Standard deviation of raw gyro signals   */  \
    uint32_t        N_BIAS      = 5000                  /* Samples of average used to calibrate gyro*/  

//--------------- Template Parameters ---------------- [ No characters after backlash! ]
    
#define TEMPLATE_TYPE                               \
    uint8_t         DLPF_CFG                        ,\
    uint8_t         FS_SEL                          ,\
    uint8_t         AFS_SEL                         ,\
    bool            ADDRESS_A0                      ,\
    int16_t         AX_OFS                          ,\
    int16_t         AY_OFS                          ,\
    int16_t         AZ_OFS                          ,\
    const float*    AX_S                            ,\
    const float*    AY_S                            ,\
    const float*    AZ_S                            ,\
    const float*    GX_S                            ,\
    const float*    GY_S                            ,\
    const float*    GZ_S                            ,\
    uint16_t        GYRO_BAND                       ,\
    uint32_t        N_BIAS                          

#define TEMPLATE_INPUTS                             \
                    DLPF_CFG                        ,\
                    FS_SEL                          ,\
                    AFS_SEL                         ,\
                    ADDRESS_A0                      ,\
                    AX_OFS                          ,\
                    AY_OFS                          ,\
                    AZ_OFS                          ,\
                    AX_S                            ,\
                    AY_S                            ,\
                    AZ_S                            ,\
                    GX_S                            ,\
                    GY_S                            ,\
                    GZ_S                            ,\
                    GYRO_BAND                       ,\
                    N_BIAS                           

//---------------- Class definition ------------------ 
        
template <TEMPLATE_DEFAULT>
class basicMPU6050 {
   private:       
    float mean[N_AXIS] = {0};    
    float var[N_AXIS] = {0};
    
    // 常规设置
    static const uint8_t MPU_ADDRESS;//地址
    static const float ACCEL_LSB;
    static const float GYRO_LSB;
    static const float MEAN;
    
    // iic通信
    void setRegister(uint8_t, uint8_t);//设置寄存器
    void readRegister(uint8_t);//读取寄存器
    int readWire();

   public: 
    void setup();
    void set_int(); 

    //原始测量数据
    
    //加速度
    int rawAx();
    int rawAy();
    int rawAz();

    //温度
    int rawTemp();

    //陀螺仪
    int rawGx();
    int rawGy();
    int rawGz();

    //缩放测量

    //加速度
    float ax();
    float ay();
    float az();

    // 温度
    float temp();

    // 陀螺仪
    float gx();
    float gy();
    float gz();
    
    //-- 陀螺仪偏差估计       
    void setBias();
    void updateBias();
///////////////////////////////////自定义新增函数///////////////////////////////////
	//void set_motion_int();//设置运动中断



///////////////////////////////////自定义新增函数///////////////////////////////////
};

#include "basicMPU6050.tpp"
 
#endif
