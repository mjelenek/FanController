//#include <PID_v1.h>
#include <avr/interrupt.h>
#include "CommandHandler.h"
#include "EEPROMStore.h"

#define TIMING_DEBUG
#ifdef TIMING_DEBUG
unsigned long timeInCode;
unsigned long timeTotal;
unsigned int to500;
unsigned int to800;
unsigned int to900;
unsigned int to1000;
unsigned int over1000;
byte timeCounting = 0;
byte timeCountingStartFlag = 0;

void printTimingResult(){
  Serial.println(F("Timing results"));
  Serial.print(F("Time in code: "));
  Serial.print(100 * (float)timeInCode / (float)timeTotal, 2);
  Serial.println(F("%"));
  Serial.print(F("<500 - "));
  Serial.println(to500);
  Serial.print(F("<800 - "));
  Serial.println(to800);
  Serial.print(F("<900 - "));
  Serial.println(to900);
  Serial.print(F("<1000 - "));
  Serial.println(to1000);
  Serial.print(F(">1000 - "));
  Serial.println(over1000);
}
#endif

//one iteration microseconds
#define ITERATION_MICROSECONDS 5000
#define WARN_MICROSECONDS 4000
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

#define LED_OUT 13

#define PWM0 3  //OC2B
#define PWM1 5  //OC0B
#define PWM2 6  //OC0A
#define PWM3 9  //OC1A
#define PWM4 10 //OC1B
#define PWM5 11 //OC2A

class PWMConfiguration
{
public:

  // 0 - const, 1 - analogInput, 2 - temperatures, 4 - constRPM
  byte pwmDrive;
  // pwm when pwmDrive == 0
  byte constPwm;
  // settings when pwmDrive == 2
  byte tSelect; // select temperature sensor 0 - T0, 1 - T1, 2  - (T1+T2)/2
  byte minPwm;
  byte maxPwm;
  byte tempTarget;
  byte tempMax;
/*
  // pid parameters
  double kp;
  double ki;
  double kd;
  unsigned int constRPM;
*/
  void Reset()
  {
//    Serial.println(F("Reset"));    
    pwmDrive = 0;
    constPwm = 120;
    tSelect = 0;
    minPwm = 100;
    maxPwm = 200;
    tempTarget = 30;
    tempMax = 50;
    /*
    kp = 1;
    ki = 1;
    kd = 0.5;
    constRPM = 1000;
  */
  }

  void Set(byte pwmDrive1, byte constPwm1, byte tSelect1, byte minPwm1, byte maxPwm1, byte tempTarget1, byte tempMax1)
  {
    if(pwmDrive1 >= 0 && pwmDrive1 <= 2){
      pwmDrive = pwmDrive1;
    }
    constPwm = constPwm1;
    if(tSelect1 >= 0 && tSelect1 <= 2){
      tSelect = tSelect1;
    }
    minPwm = minPwm1;
    maxPwm = maxPwm1;
    if(tempTarget1 <= 60){
      tempTarget = tempTarget1;
    }
    if(tempMax1 <= 60){
      tempMax = tempMax1;
    }
  }

  void guiStat(){
    Serial.write(pwmDrive);
    Serial.write(constPwm);
    Serial.write(tSelect);
    Serial.write(minPwm);
    Serial.write(maxPwm);
    Serial.write(tempTarget);
    Serial.write(tempMax);
  }
};

EEPROMStore<PWMConfiguration> ConfigurationPWM0;
EEPROMStore<PWMConfiguration> ConfigurationPWM1;
EEPROMStore<PWMConfiguration> ConfigurationPWM2;
EEPROMStore<PWMConfiguration> ConfigurationPWM3;
EEPROMStore<PWMConfiguration> ConfigurationPWM4;
EEPROMStore<PWMConfiguration> ConfigurationPWM5;

byte pwm0 = 0;
byte pwm1 = 0;
byte pwm2 = 0;
byte pwm3 = 0;
byte pwm4 = 0;
byte pwm5 = 0;

byte pwm0Disabled = 0;
byte pwm1Disabled = 0;
byte pwm2Disabled = 0;
byte pwm3Disabled = 0;
byte pwm4Disabled = 0;
byte pwm5Disabled = 0;

//unsigned long thermistorResistance0 = 0;
//unsigned long thermistorResistance1 = 0;

unsigned long start;
unsigned long now;
unsigned long zpozdeni;

unsigned long RT0koeficient;
unsigned long RT1koeficient;

// ADC values from mainboard
volatile unsigned short sensorValue0Averaged = 0;
volatile unsigned short sensorValue1Averaged = 0;
volatile unsigned short sensorValue2Averaged = 0;
volatile unsigned short sensorValue3Averaged = 0;
volatile unsigned short sensorValue4Averaged = 0;
// ADC values from thermistors
volatile unsigned short sensorValue6Averaged = 0;
volatile unsigned short sensorValue7Averaged = 0;

boolean T0Connected;
boolean T1Connected;
int T0int;
int T1int;
// hysteresis * 10°C -> value 10 means +- 1°C
#define HYSTERESIS 10
int T0WithHysteresisInt;
int T1WithHysteresisInt;

#define FAN_RPM_SENSOR_TIMES_FIELD 3
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

unsigned int rpm0 = 0;
unsigned int rpm1 = 0;
unsigned int rpm2 = 0;
unsigned int rpm3 = 0;
unsigned int rpm4 = 0;
unsigned int rpm5 = 0;

volatile byte cnt2;
volatile byte cnt2Fail;

// sensor to mainboard
volatile byte rmpToMainboard = 5;
/*
//Define Variables we'll be connecting to
double Setpoint = 10;
double Input = 10;
double Output = 10;
*/
//Specify the links and initial tuning parameters
//PID pid0(&Input, &Output, &Setpoint, 2, 5, 1, DIRECT);
/*
PID pid1(&Input, &Output, &Setpoint, 2, 5, 1, DIRECT);
PID pid2(&Input, &Output, &Setpoint, 2, 5, 1, DIRECT);
PID pid3(&Input, &Output, &Setpoint, 2, 5, 1, DIRECT);
PID pid4(&Input, &Output, &Setpoint, 2, 5, 1, DIRECT);
PID pid5(&Input, &Output, &Setpoint, 2, 5, 1, DIRECT);
*/
#ifdef TIMING_DEBUG
CommandHandler<16, 35, 0> SerialCommandHandler; // 16 commands, max length of command 35, 0 variables
#else
CommandHandler<16, 35, 0> SerialCommandHandler; // 14 commands, max length of command 35, 0 variables
#endif

byte i = 0;
byte j = 0;
byte gui = 0;  // enable gui

void printlnPwmDrive(PWMConfiguration &conf);
void printStatus();
void printFullStatus();
void setPwmConfiguration(CommandParameter &parameters);
void disableFan(CommandParameter &parameters);
void setRPMToMainboard(CommandParameter &parameters);
byte getNewPwm(PWMConfiguration &conf, byte pwm, unsigned int sensorValueAveraged);
void readRPMsensors();

