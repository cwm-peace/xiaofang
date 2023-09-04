# xiaofang
小方1.0-巴掌宽护胸毛（供个人爱好者学习研究，请勿商用）
这是一个桌面电子沙漏小摆件，分辨率8x8，使用一个方块像素点阵做显示，尺寸32mmx32mmx15mm。


主要工程文件时xiaofang_v1.0.ino，是ArduinoIDE里写的，直接用Arduino打开即可。然后需要把这几个文件夹放到Arduino的libraries文件夹里，这几个文件夹都是使用到的相关库文件（basicMPU6050、iic_16k、ip5306ck、LedControl、LowPower、she）。
库文件描述：

basicMPU6050,这个库主要用于驱动MUP6050
iic_16k，这个库用于驱动AT24C6存储芯片
ip5306ck，这个用于读取电池电量信息
LedControl，这个用于驱动点阵
LowPower，这个用于开启ATMEGA328P低功耗
she，这个主要是存放的贪吃蛇的定义以及部分功能

以上几个库一个都不能少。
ATMEGA328P的引导程序烧录教程在“如何用Arduino给ATMEGA328p裸片烧录引导程序”这个文件夹里，跟着操作就搞定了。


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



重要的事单独列出来说

1.点阵驱动里有个函数，它的名字叫setIntensity(int intensity)，参数范围0-15，但千万千万不要把参数设置到4以上太长时间，不然很快就会冒烟。点阵全功率电流太大，电路设计没法承载。
2.电路板放主控芯片那面的所有零件都可以用加热盘一次性焊接好，另外一面有一个点阵驱动芯片和几个电容电阻需要手动焊上去。
千万注意，点阵不要急着焊上去，其余零件都焊接好之后，就要先通过板子上预留的bootloader烧录接口（isp接口）给328P烧录引导程序。
引导程序烧录好之后就可以直接用type-c的数据线连接进行编程了。
然后在xiaofang_v1.0.ino里这个位置“//如果要调试就保留下面这句，否则就注释掉它（1.跳过充电循环，因为插着usb时它会一直充电，停在这个循环里。2.关闭陀螺仪校正，这个过程很费时间。）//#define DEBUG”，
把“//#define DEBUG”前面的“//”删掉，把程序上传到芯片里，它会自动运行，点亮所有led。
这个时候，再把点阵插到电路板对应的孔里，用手压一压，尽可能让每一个引脚都接触上（事实上不可能），
然后反过来看点阵情况，调整按压力度和角度，看是不是每一个灯都能亮（不一定同时亮，只要能亮就行），
如果都能亮，才确定点阵方向放对了，这时再断电，焊死。
因为这里你一旦放错了方向，点阵不会正常工作，然后你还拆不下来，等于干废一块板子。

别问我为什么知道，实践出真知

测试点阵方向ok之后，就可以恢复“#define DEBUG”前面的两条杠“//“，再重新上传完整代码进去，然后焊接上电池，万事大吉！！！！

2023.9.4  备注

突然想起来，由于你们刚做好的flash里面对应的地址上没有存储其余七个地图数据，第一关过了之后大概会卡死。等我稍微改一下代码吧，实在抱歉，漏了这一点。因为其余七个地图需要手动一个一个地用写入函数写进去，写完之后就不用管了，迷宫才不会卡死。
