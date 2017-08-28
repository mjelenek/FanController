#include <PID_v1.h>
#include <avr/interrupt.h>
#include "CommandHandler.h"
#include "EEPROMStore.h"

#define TIMING_DEBUG
#ifdef TIMING_DEBUG
unsigned long timeInCode;
unsigned long timeTotal;
unsigned long to50;
unsigned long to100;
unsigned long to150;
unsigned long to200;
unsigned long to300;
unsigned long to400;
unsigned long to500;
unsigned long to600;
unsigned long over600;
byte timeCounting = 0;
byte timeCountingStartFlag = 0;

void printTimingResult(){
  Serial.println(F("Timing results"));
  Serial.print(F("Time in code: "));
  Serial.print(timeInCode);
  Serial.print(F(" us. "));
  Serial.print(100 * (float)timeInCode / (float)timeTotal, 2);
  Serial.println(F("%"));
  Serial.print(F("Total time: "));
  Serial.print(timeTotal);
  Serial.println(F(" us"));
  Serial.print(F("<50 - "));
  Serial.println(to50);
  Serial.print(F("<100 - "));
  Serial.println(to100);
  Serial.print(F("<150 - "));
  Serial.println(to150);
  Serial.print(F("<200 - "));
  Serial.println(to200);
  Serial.print(F("<300 - "));
  Serial.println(to300);
  Serial.print(F("<400 - "));
  Serial.println(to400);
  Serial.print(F("<500 - "));
  Serial.println(to500);
  Serial.print(F("<600 - "));
  Serial.println(to600);
  Serial.print(F(">600 - "));
  Serial.println(over600);
  delay(2);
}
#endif

//one iteration microseconds
#define ITERATION_MICROSECONDS 1000
//#define ITERATION_MICROSECONDS 2000
#define WARN_MICROSECONDS 500
#define DELAY_THRESHOLD 10000

// voltage to thermistor
#define VOLTAGETHERMISTOR 3.3
// resistance of resistor in series with thermistor(value measured by multimeter)
#define RT0 9960
#define RT1 9960
//by multimeter
#define ANALOGREFERENCEVOLTAGE 3.3

// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950

#define VOLTAGEINPUT0 A4
#define VOLTAGEINPUT1 A3
#define VOLTAGEINPUT2 A2
#define VOLTAGEINPUT3 A1
#define VOLTAGEINPUT4 A0

#define TEMPINPUT0 A6
#define TEMPINPUT1 A7

#define RPMSENSOR0 7
#define RPMSENSOR1 8
#define RPMSENSOR2 2
#define RPMSENSOR3 4
#define RPMSENSOR4 12
#define RPMSENSOR5 19   // A5

#define LED_OUT 13

#define PWM0 3  //OC2B
#define PWM1 5  //OB0B
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

  // pid parameters
  double kp;
  double ki;
  double kd;
  unsigned int constRPM;

  void Reset()
  {
//    Serial.println(F("Reset"));    
    pwmDrive = 0;
    constPwm = 150;
    tSelect = 0;
    minPwm = 100;
    maxPwm = 200;
    tempTarget = 20;
    tempMax = 40;
    kp = 1;
    ki = 1;
    kd = 0.5;
    constRPM = 1000;
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

unsigned long thermistorResistance0 = 0;
unsigned long thermistorResistance1 = 0;

unsigned long start;
unsigned long now;
unsigned long zpozdeni;

unsigned long RT0koeficient;
unsigned long RT1koeficient;

// ADC values from mainboard
short sensorValue0Averaged = 0;
short sensorValue1Averaged = 0;
short sensorValue2Averaged = 0;
short sensorValue3Averaged = 0;
short sensorValue4Averaged = 0;
// ADC values from thermistors
short sensorValue6Averaged = 0;
short sensorValue7Averaged = 0;

unsigned int T0int;
unsigned int T1int;
boolean T0Connected;
boolean T1Connected;

#define FANSENSOR_HISTORY_SIZE 1838
//#define FANSENSOR_HISTORY_SIZE 919
#define FANSENSOR_SHIFT_MULTIPLIER 3
#define FANSENSOR_SUMS_FIELD 8
volatile byte fanSensorSums[6][FANSENSOR_SUMS_FIELD];

unsigned int rpm0 = 0;
unsigned int rpm1 = 0;
unsigned int rpm2 = 0;
unsigned int rpm3 = 0;
unsigned int rpm4 = 0;
unsigned int rpm5 = 0;

volatile byte fanSensor5Value = 0;

// sensor to mainboard
volatile byte rmpToMainboard = 5;

//Define Variables we'll be connecting to
double Setpoint = 10;
double Input = 10;
double Output = 10;

//Specify the links and initial tuning parameters
//PID pid0(&Input, &Output, &Setpoint, 2, 5, 1, DIRECT);
/*
PID pid1(&Input, &Output, &Setpoint, 2, 5, 1, DIRECT);
PID pid2(&Input, &Output, &Setpoint, 2, 5, 1, DIRECT);
PID pid3(&Input, &Output, &Setpoint, 2, 5, 1, DIRECT);
PID pid4(&Input, &Output, &Setpoint, 2, 5, 1, DIRECT);
PID pid5(&Input, &Output, &Setpoint, 2, 5, 1, DIRECT);
*/
CommandHandler<14, 35, 0> SerialCommandHandler; // 14 commands, max length of command 35, 0 variables

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

