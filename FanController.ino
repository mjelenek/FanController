#include "PID_v1.h"
#include "CommandHandler.h"
#include "EEPROMStoreISR.h"
#include "MemoryFree.h"

#define TIMING_DEBUG
//#define SAVE_DEBUG
//#define TEMPERATURES_DEBUG
#define FREE_MEMORY_DEBUG
#define COUNT_MILLLIS_BY_DIVIDE_MICROS
#define USE_TEMP_CACHE
#define USE_PWM_CACHE

#ifdef USE_PWM_CACHE
#define USE_FAN_NUMBER_DECLARATION ,byte fanNumber
#define USE_FAN_NUMBER ,fanNumber
#else
#define USE_FAN_NUMBER_DECLARATION
#define USE_FAN_NUMBER
#endif

#ifdef TIMING_DEBUG
#define WARN_MICROSECONDS_DEBUG 800
unsigned long timeInCode;
unsigned long timeTotal;
unsigned int to400;
unsigned int to600;
unsigned int to800;
unsigned int to1000;
unsigned int to1200;
unsigned int over1200;
byte timeCounting = 0;
byte timeCountingStartFlag = 0;

void printTimingResult(){
  Serial.print(F("Time in code: "));
  Serial.print(100 * (float)timeInCode / (float)timeTotal, 2);
  Serial.println(F("%"));
  Serial.print(F("Average delay: "));
  Serial.println((unsigned long)timeInCode >> 9);
  Serial.print(F("<400-"));
  Serial.println(to400);
  Serial.print(F("<600-"));
  Serial.println(to600);
  Serial.print(F("<800-"));
  Serial.println(to800);
  if(to1000 > 0){
    Serial.print(F("<1000-"));
    Serial.println(to1000);
  }
  if(to1200 > 0){
    Serial.print(F("<1200-"));
    Serial.println(to1200);
  }
  if(over1200 > 0){
    Serial.print(F(">1200-"));
    Serial.println(over1200);
  }
}
#endif

#ifdef FREE_MEMORY_DEBUG
volatile int freemem = 2000;
#endif

//one iteration microseconds
#define ITERATION_MICROSECONDS 2000
#define WARN_MICROSECONDS 1600
#define DELAY_THRESHOLD 10000

//by multimeter
#define ANALOGREFERENCEVOLTAGE 3.3
// voltage to thermistor
#define VOLTAGETHERMISTOR 3.3
// resistance of resistor in series with thermistor(value measured by multimeter)
#define RT0 9990
#define RT1 9990

// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// resistance at nominal temperature
#define THERMISTORNOMINAL 10000      
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950

#define RPMSENSOR0 7
#define RPMSENSOR1 8
#define RPMSENSOR2 2
#define RPMSENSOR3 4
#define RPMSENSOR4 12
#define RPMSENSOR5 19   // A5

//PWM output pins
#define PWM0 3  //OC2B
#define PWM1 5  //OC0B
#define PWM2 6  //OC0A
#define PWM3 9  //OC1A
#define PWM4 10 //OC1B
#define PWM5 11 //OC2A
byte PWMOUT[] = {PWM0, PWM1, PWM2, PWM3, PWM4, PWM5};

#define LED_OUT 13
#define LED_OUT_1 PORTB |= _BV(PB5)
#define LED_OUT_0 PORTB &= ~_BV(PB5)
#define LED_OUT_SET {LED_OUT_1;} else {LED_OUT_0;}

#define MAX_ALLOWED_TEMP 100

class PWMConfiguration
{
public:
  // 0 - analogInput, 1 - constPWM, 2 - PWM by temperatures, 3 - constRPM, 4 - RPM by temperatures
  byte pwmDrive;
  // pwm when pwmDrive == 1
  byte constPwm;
  // settings when pwmDrive == 2 or pwmDrive == 4
  byte tSelect; // select temperature sensor 0 - T0, 1 - T1, 2  - (T1+T2)/2
  // settings when pwmDrive == 2
  byte tPwm[5];
  byte pwm[5];
  // settings when pwmDrive == 3
  unsigned short constRpm;
  // settings when pwmDrive == 4
  byte tRpm[5];
  unsigned short rpm[5];
  // pid parameters, real value is parameter / 200
  byte kp;
  byte ki;
  byte kd;
  byte minPidPwm; // minimal value of PWM when speed is driven by PID. Can not be 0, because fans with pwm power=0 instantly indicates 0rpm.

  void Reset()
  {
    pwmDrive = 1;
    constPwm = 120;
    tSelect = 0;
    tPwm[0] = 32;
    pwm[0] = 90;
    tPwm[1] = 50;
    pwm[1] = 200;
    constRpm = 700;
    tRpm[0] = 32;
    rpm[0] = 600;
    tRpm[1] = 50;
    rpm[1] = 1400;
    // PID parameters (real value is parameter value / 200)
    kp = 40;
    ki = 30;
    kd = 5;
    minPidPwm = 15;
  }

  void set(byte pwmDrive1, byte constPwm1, byte tSelect1,
    byte t0, byte pwm0,
    byte t1, byte pwm1,
    byte t2, byte pwm2,
    byte t3, byte pwm3,
    byte t4, byte pwm4)
  {
    if(pwmDrive1 >= 0 && pwmDrive1 <= 4){
      pwmDrive = pwmDrive1;
    }
    constPwm = constPwm1;
    if(tSelect1 >= 0 && tSelect1 <= 2){
      tSelect = tSelect1;
    }
    if(t0 <= 60 && t0 <= t1){
      tPwm[0] = t0;
      pwm[0] = pwm0;
    }
    if(t1 <= 60 && t0 <= t1){
      tPwm[1] = t1;
      pwm[1] = pwm1;
    }
    tPwm[2] = tPwm[3] = tPwm[4] = 0;
    pwm[2] = pwm[3] = pwm[4] = 0;
    
    if(t2 <= 60 && (t1 <= t2)){
      tPwm[2] = t2;
      pwm[2] = pwm2;
    } else {
      return;
    }
    if(t3 <= 60 && (t2 <= t3)){
      tPwm[3] = t3;
      pwm[3] = pwm3;
    } else {
      return;
    }
    if(t4 <= 60 && (t3 <= t4)){
      tPwm[4] = t4;
      pwm[4] = pwm4;
    }
  }

  void setPid(unsigned short constRpm1, byte kp1, byte ki1, byte kd1, byte minPidPwm1,
    byte t0, unsigned short rpm0,
    byte t1, unsigned short rpm1,
    byte t2, unsigned short rpm2,
    byte t3, unsigned short rpm3,
    byte t4, unsigned short rpm4)
  {
    constRpm = constRpm1;
    kp = kp1;
    ki = ki1;
    kd = kd1;
    minPidPwm = minPidPwm1;
    if(t0 <= 60 && t0 <= t1){
      tRpm[0] = t0;
      rpm[0] = rpm0;
    }
    if(t0 <= 60 && t0 <= t1){
      tRpm[1] = t1;
      rpm[1] = rpm1;
    }
    tRpm[2] = tRpm[3] = tRpm[4] = 0;
    rpm[2] = rpm[3] = rpm[4] = 0;
    
    if(t2 <= 60 && t1 <= t2){
      tRpm[2] = t2;
      rpm[2] = rpm2;
    } else {
      return;
    }
    if(t3 <= 60 && t2 <= t3){
      tRpm[3] = t3;
      rpm[3] = rpm3;
    } else {
      return;
    }
    if(t4 <= 60 && t3 <= t4){
      tRpm[4] = t4;
      rpm[4] = rpm4;
    }
  }

  void guiStat(){
    Serial.write(pwmDrive);
    Serial.write(constPwm);
    Serial.write(tSelect);
    for(int i = 0; i <= 4; i++){
      Serial.write(tPwm[i]);
      Serial.write(pwm[i]);
    }
    serialWriteInt(constRpm);
    for(int i = 0; i <= 4; i++){
      Serial.write(tRpm[i]);
      serialWriteInt(rpm[i]);
    }
    Serial.write(kp);
    Serial.write(ki);
    Serial.write(kd);
    Serial.write(minPidPwm);
  }
};

EEPROMStore<PWMConfiguration> ConfigurationPWM0;
EEPROMStore<PWMConfiguration> ConfigurationPWM1;
EEPROMStore<PWMConfiguration> ConfigurationPWM2;
EEPROMStore<PWMConfiguration> ConfigurationPWM3;
EEPROMStore<PWMConfiguration> ConfigurationPWM4;
EEPROMStore<PWMConfiguration> ConfigurationPWM5;
PWMConfiguration *ConfigurationPWM[] = {&ConfigurationPWM0.Data.m_UserData, &ConfigurationPWM1.Data.m_UserData,
                                        &ConfigurationPWM2.Data.m_UserData, &ConfigurationPWM3.Data.m_UserData,
                                        &ConfigurationPWM4.Data.m_UserData, &ConfigurationPWM5.Data.m_UserData};

class ControllerConfiguration
{
public:
  // sensor to mainboard
  byte rmpToMainboard;
  // hysteresis * 10°C -> value 10 means +- 1°C
  byte hysteresis;

  void Reset()
  {
    rmpToMainboard = 5;
    hysteresis = 10;
  }
};

EEPROMStore<ControllerConfiguration> ControllerConfiguration;

// sensor to mainboard
byte *rmpToMainboard = &ControllerConfiguration.Data.m_UserData.rmpToMainboard;
byte *hysteresis = &ControllerConfiguration.Data.m_UserData.hysteresis;
 
byte pwm[] = {0, 0, 0, 0, 0, 0};
byte pwmDisabled[] = {0, 0, 0, 0, 0, 0};

unsigned long start;
unsigned long now;
unsigned short zpozdeni;

unsigned long RT0koeficient;
unsigned long RT1koeficient;

// ADC values from mainboard
volatile uint16_t sensorValue0Averaged = 0;
volatile uint16_t sensorValue1Averaged = 0;
volatile uint16_t sensorValue2Averaged = 0;
volatile uint16_t sensorValue3Averaged = 0;
volatile uint16_t sensorValue4Averaged = 0;
// ADC values from thermistors
volatile uint16_t sensorValue6Averaged = 0;
volatile uint16_t sensorValue7Averaged = 0;

boolean T0Connected;
boolean T1Connected;
int T0int;
int T1int;
int T0WithHysteresisInt;
int T1WithHysteresisInt;

#define FAN_RPM_SENSOR_TIMES_FIELD 5
volatile unsigned long fanRpmSensorTimes0[FAN_RPM_SENSOR_TIMES_FIELD];
volatile unsigned long fanRpmSensorTimes1[FAN_RPM_SENSOR_TIMES_FIELD];
volatile unsigned long fanRpmSensorTimes2[FAN_RPM_SENSOR_TIMES_FIELD];
volatile unsigned long fanRpmSensorTimes3[FAN_RPM_SENSOR_TIMES_FIELD];
volatile unsigned long fanRpmSensorTimes4[FAN_RPM_SENSOR_TIMES_FIELD];
volatile unsigned long fanRpmSensorTimes5[FAN_RPM_SENSOR_TIMES_FIELD];
volatile byte lastFanRpmSensorTime0;
volatile byte lastFanRpmSensorTime1;
volatile byte lastFanRpmSensorTime2;
volatile byte lastFanRpmSensorTime3;
volatile byte lastFanRpmSensorTime4;
volatile byte lastFanRpmSensorTime5;
volatile byte lastFanRpmSensorTimeUpdated;
double rpm[6];

// Define Variables PIDs will be connecting to
double outputPid;
double inputPid;
double setpointPid[6];
//Specify the links and initial tuning parameters
PID pid[] = {
  PID(&inputPid, &outputPid, &setpointPid[0], (double)ConfigurationPWM0.Data.m_UserData.kp / 200, (double)ConfigurationPWM0.Data.m_UserData.ki / 200, (double)ConfigurationPWM0.Data.m_UserData.kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[1], (double)ConfigurationPWM1.Data.m_UserData.kp / 200, (double)ConfigurationPWM1.Data.m_UserData.ki / 200, (double)ConfigurationPWM1.Data.m_UserData.kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[2], (double)ConfigurationPWM2.Data.m_UserData.kp / 200, (double)ConfigurationPWM2.Data.m_UserData.ki / 200, (double)ConfigurationPWM2.Data.m_UserData.kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[3], (double)ConfigurationPWM3.Data.m_UserData.kp / 200, (double)ConfigurationPWM3.Data.m_UserData.ki / 200, (double)ConfigurationPWM3.Data.m_UserData.kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[4], (double)ConfigurationPWM4.Data.m_UserData.kp / 200, (double)ConfigurationPWM4.Data.m_UserData.ki / 200, (double)ConfigurationPWM4.Data.m_UserData.kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[5], (double)ConfigurationPWM5.Data.m_UserData.kp / 200, (double)ConfigurationPWM5.Data.m_UserData.ki / 200, (double)ConfigurationPWM5.Data.m_UserData.kd / 200, P_ON_E, DIRECT)
};

#ifdef TIMING_DEBUG
CommandHandler<22, 70, 0> SerialCommandHandler; // 21 commands, max length of command 70, 0 variables
#else
CommandHandler<18, 70, 0> SerialCommandHandler; // 17 commands, max length of command 70, 0 variables
#endif

byte i = 0;
byte j = 0;
byte part_32;  // cycles from 0 to 31;
byte gui = 0;  // enable gui
byte updatesRTToSend[] = {0, 0, 0, 0, 0, 0};

void printlnPwmDrive(PWMConfiguration &conf);
void printStatus();
void printFullStatus();
void setPwmConfiguration(CommandParameter &parameters);
void setPidConfiguration(CommandParameter &parameters);
void disableFan(CommandParameter &parameters);
void setRPMToMainboard(CommandParameter &parameters);
void setHysteresis(CommandParameter &parameters);
void sendPidUpates(CommandParameter &parameters);
byte countPWM(PWMConfiguration &conf, unsigned int temperature);
unsigned short countExpectedRPM(PWMConfiguration &conf, unsigned int temperature);
#ifdef USE_PWM_CACHE
byte countPWM(PWMConfiguration &conf, unsigned int temperature, byte fanNumber);
unsigned short countExpectedRPM(PWMConfiguration &conf, unsigned int temperature, byte fanNumber);
#endif
byte getNewPwm(PWMConfiguration &conf, byte pwmOld, unsigned short sensorValueAveraged, byte fanNumber);
byte getNewPwmByPowerCurve(PWMConfiguration &conf, byte pwmOld USE_FAN_NUMBER_DECLARATION);
byte getNewPwmByConstRpm(PWMConfiguration &conf, byte pwmOld, byte fanNumber);
void setpointPidByRpmCurve(PWMConfiguration &conf, byte pwmOld, byte fanNumber);
byte pidUpdate(byte fanNumber, PWMConfiguration &conf);
byte pidUpdateDirect(byte fanNumber, PWMConfiguration &conf);
void readRPMsensors();
void init_pid();
