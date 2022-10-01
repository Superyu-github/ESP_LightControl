/*
 * @Author: error: git config user.name && git config user.email & please set dead value or install git
 * @Date: 2022-09-10 12:18:27
 * @LastEditors: superyu 577928831@qq.com
 * @LastEditTime: 2022-10-01 23:24:42
 * @FilePath: \ESP_LightControl\ESP_LightControl\src\main.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#define BLINKER_WIFI

#include <Blinker.h>
#define PIN_LED_B 1
#define PIN_LED_Y 3
#define LED_I 2

typedef struct
{
  bool power;                       //电源状态
  bool flow;                        //渐变开关状态
  uint16_t light_B;                 //黄灯亮度
  uint16_t light_Y;                 //蓝灯亮度
  uint16_t light_All;               //未用
  uint32_t light_start_time;        //开灯时间戳
  uint32_t light_on_time;           //亮灯时间
  uint32_t light_on_timeout = 10;   //亮灯时间限制
  uint32_t light_flow_time_set = 5; //渐变时间设置
  uint32_t light_flow_start_time;   //渐变开始时间
  uint32_t light_flow_time;         //渐变发生时间

} LED_state_t;

char auth[] = "";
char ssid[] = "";
char pswd[] = "";

// 新建组件对象
BlinkerButton Button_P("btn-power");
BlinkerButton Button_flow("btn-flow");
BlinkerSlider Slider_B("ran-b");
BlinkerSlider Slider_Y("ran-y");
BlinkerSlider Slider_L("ran-light");
BlinkerSlider Slider_light_on_timeout("ran-light-on-timeout");
BlinkerSlider Slider_flow_time("ran-flow-time");
BlinkerNumber Num_light_on_time("num-light-on-time");

LED_state_t LED_state;

/**
 * @brief 开关按键回调函数
 *
 * @param state
 */

void button_P_callback(const String &state)
{

  if (state == "on")
  {
    LED_state.power = true;
    LED_state.light_start_time = Blinker.time(); //记录开灯时的时间
    Button_P.print("on");
  }
  else
  {
    LED_state.power = false;
    Button_P.print("off");
  }
}
/**
 * @brief 渐变开关回调函数
 *
 * @param state
 */
void Button_flow_callback(const String &state)
{
  if (state == "on")
  {
    LED_state.flow = true;
    Button_flow.print("on");
    LED_state.light_flow_start_time = Blinker.time();
  }
  else
  {
    LED_state.flow = false;
    Button_flow.print("off");
    LED_state.light_flow_time = 0;
  }
}
/**
 * @brief 黄灯滑条回调函数
 *
 * @param value
 */
void Slider_B_callback(int32_t value) //滑条回调函数，滑条值范围0~1000
{
  LED_state.light_B = value;
}
/**
 * @brief 蓝灯滑条回调函数
 *
 * @param value
 */
void Slider_Y_callback(int32_t value) //滑条回调函数，滑条值范围0~1000
{
  LED_state.light_Y = value;
}
/**
 * @brief 亮灯时间限制设置回调函数
 *
 * @param value
 */
void Slider_light_on_timeout_callback(int32_t value)
{
  LED_state.light_on_timeout = value;
}
/**
 * @brief 设置渐变时间回调函数
 *
 * @param value
 */
void Slider_flow_time_callback(int32_t value)
{
  LED_state.light_flow_time_set = value;
}
/**
 * @brief 心跳包回调函数，用作APP显示同步和安全检查
 *
 */
void heartbeat_callback()
{
  /*安全检查*/
  if (LED_state.power)
  {
    uint32_t time_now = Blinker.time();                              //获取当前时间
    LED_state.light_on_time = time_now - LED_state.light_start_time; //计算亮灯时间
    if (LED_state.light_on_time > 60 * LED_state.light_on_timeout)
    {
      LED_state.power = false; //安全起见，防止灯过热，超过timeout分钟立刻关灯
      LED_state.light_on_time = 0;
    }
  }
  else
  {
    LED_state.light_on_time = 0;
  }

  /*同步显示*/
  Slider_B.print(LED_state.light_B);
  Slider_Y.print(LED_state.light_Y);
  Slider_light_on_timeout.print(LED_state.light_on_timeout);
  Slider_flow_time.print(LED_state.light_flow_time_set);
  Num_light_on_time.print(LED_state.light_on_time / 60);
  if (LED_state.power == true)
    Button_P.print("on");
  else
    Button_P.print("off");
  if (LED_state.flow == true)
    Button_flow.print("on");
  else
    Button_flow.print("off");
}
/**
 * @brief LED输出控制
 *
 */
void LED_controller(void)
{
  if (LED_state.power && !LED_state.flow) //电源开 非渐变状态
  {
    analogWrite(PIN_LED_B, (1000 - LED_state.light_B));
    analogWrite(PIN_LED_Y, (1000 - LED_state.light_Y));
  }
  else if (LED_state.power && LED_state.flow) //电源开 渐变状态
  {
    uint32_t time_now = Blinker.time();                                                     //获取当前时间
    LED_state.light_flow_time = time_now - LED_state.light_flow_start_time;                 //计算渐变发生时间
    uint16_t flow_scale = LED_state.light_flow_time / (LED_state.light_flow_time_set * 60); //计算渐变比例
    if (flow_scale > 1)                                                                     //渐变满，清零、关渐变
    {
      LED_state.light_flow_time = 0;
      LED_state.flow = false;
    }
    analogWrite(PIN_LED_B, (1000 - LED_state.light_B * flow_scale));
    analogWrite(PIN_LED_Y, (1000 - LED_state.light_Y * flow_scale));
  }
  else //电源关
  {
    digitalWrite(PIN_LED_B, HIGH);
    digitalWrite(PIN_LED_Y, HIGH);
  }
}
/**
 * @brief 检查设备连接状态,同步指示灯显示
 *
 */
void ConnectCheck()
{
  if (Blinker.connected())
  {
    digitalWrite(LED_I, HIGH);
  }
  else
  {
    digitalWrite(LED_I, LOW);
  }
}

void setup()
{
  // 初始化串口
  // Serial.begin(115200);
  // BLINKER_DEBUG.stream(Serial);
  // BLINKER_DEBUG.debugAll();

  // 初始化所有LED的IO
  pinMode(PIN_LED_B, OUTPUT); //蓝灯
  pinMode(PIN_LED_Y, OUTPUT); //黄灯
  pinMode(LED_I, OUTPUT);     // ESP8266指示灯
  digitalWrite(PIN_LED_B, HIGH);
  digitalWrite(PIN_LED_Y, HIGH);
  digitalWrite(LED_I, HIGH);
  analogWriteFreq(90000); //修改PWM频率
  analogWriteRange(1000); //修改PWM范围
  // 初始化blinker
  Blinker.begin(auth, ssid, pswd);
  Button_P.attach(button_P_callback);
  Button_flow.attach(Button_flow_callback);
  Slider_B.attach(Slider_B_callback);
  Slider_Y.attach(Slider_Y_callback);
  Slider_light_on_timeout.attach(Slider_light_on_timeout_callback);
  Slider_flow_time.attach(Slider_flow_time_callback);
  Blinker.attachHeartbeat(heartbeat_callback);
}

void loop()
{
  Blinker.run();
  ConnectCheck();
  LED_controller();
}
