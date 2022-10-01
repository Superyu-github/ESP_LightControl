/*
 * @Author: error: git config user.name && git config user.email & please set dead value or install git
 * @Date: 2022-09-10 12:18:27
 * @LastEditors: superyu 577928831@qq.com
 * @LastEditTime: 2022-10-01 21:19:35
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
  bool power;                     //电源状态
  uint16_t light_B;               //黄灯亮度
  uint16_t light_Y;               //蓝灯亮度
  uint16_t light_All;             //未用
  uint32_t light_start_time;      //开灯时间戳
  uint32_t light_on_time;         //亮灯时间
  uint32_t light_on_timeout = 10; //亮灯时间限制

} LED_state_t;

char auth[] = "4d5842643f7e";
char ssid[] = "PDCN";
char pswd[] = "niuzi203203";

// 新建组件对象
BlinkerButton Button_P("btn-power");
BlinkerSlider Slider_B("ran-b");
BlinkerSlider Slider_Y("ran-y");
BlinkerSlider Slider_L("ran-light");
BlinkerSlider Slider_light_on_timeout("ran-light-on-timeout");
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
 * @brief 亮灯时间限制设置回调
 *
 * @param value
 */
void Slider_light_on_timeout_callback(int32_t value)
{
  LED_state.light_on_timeout = value;
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
    uint32_t time_now = Blinker.time();
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
  Num_light_on_time.print(LED_state.light_on_time / 60);
  if (LED_state.power == true)
    Button_P.print("on");
  else
    Button_P.print("off");
}
/**
 * @brief LED输出控制
 *
 */
void LED_controller(void)
{
  if (LED_state.power)
  {
    analogWrite(PIN_LED_B, (1000 - LED_state.light_B));
    analogWrite(PIN_LED_Y, (1000 - LED_state.light_Y));
  }
  else
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
  Slider_B.attach(Slider_B_callback);
  Slider_Y.attach(Slider_Y_callback);
  Slider_light_on_timeout.attach(Slider_light_on_timeout_callback);
  Blinker.attachHeartbeat(heartbeat_callback);
}

void loop()
{
  Blinker.run();
  ConnectCheck();
  LED_controller();
}
