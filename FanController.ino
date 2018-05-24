#include "PID_v1.h"
#include "CommandHandler.h"
#include "EEPROMStoreISR.h"

#define HWversion 1.0

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
#endif

//one iteration microseconds
#define ITERATION_MICROSECONDS 2000
#define WARN_MICROSECONDS 1600
#define DELAY_THRESHOLD 10000

//by multimeter
//#define ANALOGREFERENCEVOLTAGE 3.3
// resistance of resistor in series with thermistor(value measured by multimeter)
#define RT0 9990
#define RT1 9990

#define NUMBER_OF_THERMISTORS 2

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

void serialWriteInt(unsigned int i);

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

typedef struct CEEPROMPWM
{
  uint16_t m_uChecksum;
  PWMConfiguration m_UserData;
};

CEEPROMPWM EEMEM EEPROPWM0, EEPROPWM1, EEPROPWM2, EEPROPWM3, EEPROPWM4, EEPROPWM5;

EEPROMStore<CEEPROMPWM> ConfigurationPWMHolder[] = {
  EEPROMStore<CEEPROMPWM>(&EEPROPWM0),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM1),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM2),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM3),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM4),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM5)};
/*
PWMConfiguration *ConfigurationPWM[] = {&ConfigurationPWMHolder[0].Data.m_UserData, &ConfigurationPWMHolder[1].Data.m_UserData,
                                        &ConfigurationPWMHolder[2].Data.m_UserData, &ConfigurationPWMHolder[3].Data.m_UserData,
                                        &ConfigurationPWMHolder[4].Data.m_UserData, &ConfigurationPWMHolder[5].Data.m_UserData};
*/
#define ConfigurationPWM(i) ConfigurationPWMHolder[i].Data.m_UserData

class ThermistorDefinition
{
  public:
  // temp. for nominal resistance (almost always 25 C)
  unsigned char tempNominal;
  // resistance at nominal temperature
  unsigned short resistanceNominal;
  // The beta coefficient of the thermistor (usually 3000-4000)
  unsigned short bCoefficient;

  void Reset()
  {
    tempNominal = 25;
    resistanceNominal = 10000;
    bCoefficient = 3950;
  }

  void Set(unsigned char t, unsigned short r, unsigned short b){
    if(t < 0 || t >= 100) return;
    if(r < 3000 || r >= 50000) return;
    if(b < 1000 || b >= 10000) return;
    tempNominal = t;
    resistanceNominal = r;
    bCoefficient = b;
  }

  void sendDefinition(){
    Serial.write(tempNominal);
    serialWriteInt(resistanceNominal);
    serialWriteInt(bCoefficient);
  }
};

class ControllerConfiguration
{
public:
  // sensor to mainboard
  byte rmpToMainboard;
  // hysteresis * 10°C -> value 10 means +- 1°C
  byte hysteresis;
  //Thermistor definitions
  ThermistorDefinition thermistors[NUMBER_OF_THERMISTORS];
  
  void Reset()
  {
    rmpToMainboard = 5;
    hysteresis = 10;
    for(int i = 0; i < NUMBER_OF_THERMISTORS; i++){
      thermistors[i].Reset();
    }
  }
};

  typedef struct CEEPROMC
  {
    uint16_t m_uChecksum;
    ControllerConfiguration m_UserData;
  };
CEEPROMC EEMEM EEPROMConf;

EEPROMStore<CEEPROMC> ControllerConfigurationHolder(&EEPROMConf);

// sensor to mainboard
byte *rmpToMainboard = &ControllerConfigurationHolder.Data.m_UserData.rmpToMainboard;
byte *hysteresis = &ControllerConfigurationHolder.Data.m_UserData.hysteresis;
ThermistorDefinition *thermistors = ControllerConfigurationHolder.Data.m_UserData.thermistors;
 
byte pwm[] = {0, 0, 0, 0, 0, 0};
byte pwmDisabled[] = {0, 0, 0, 0, 0, 0};

unsigned long start;
unsigned long now;
unsigned short zpozdeni;
byte gui = 0;  // enable gui
byte i = 0;
byte j = 0;
byte part_32;  // cycles from 0 to 31;
byte updatesRTToSend[] = {0, 0, 0, 0, 0, 0};

unsigned long RT0koeficient;
unsigned long RT1koeficient;
boolean T0Connected;
boolean T1Connected;
int T0int;
int T1int;
int T0WithHysteresisInt;
int T1WithHysteresisInt;

// ADC values from mainboard
volatile uint16_t sensorValue0Averaged = 0;
volatile uint16_t sensorValue1Averaged = 0;
volatile uint16_t sensorValue2Averaged = 0;
volatile uint16_t sensorValue3Averaged = 0;
volatile uint16_t sensorValue4Averaged = 0;
// ADC values from thermistors
volatile uint16_t sensorValue6Averaged = 0;
volatile uint16_t sensorValue7Averaged = 0;

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
  PID(&inputPid, &outputPid, &setpointPid[0], (double)ConfigurationPWM(0).kp / 200, (double)ConfigurationPWM(0).ki / 200, (double)ConfigurationPWM(0).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[1], (double)ConfigurationPWM(1).kp / 200, (double)ConfigurationPWM(1).ki / 200, (double)ConfigurationPWM(1).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[2], (double)ConfigurationPWM(2).kp / 200, (double)ConfigurationPWM(2).ki / 200, (double)ConfigurationPWM(2).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[3], (double)ConfigurationPWM(3).kp / 200, (double)ConfigurationPWM(3).ki / 200, (double)ConfigurationPWM(3).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[4], (double)ConfigurationPWM(4).kp / 200, (double)ConfigurationPWM(4).ki / 200, (double)ConfigurationPWM(4).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[5], (double)ConfigurationPWM(5).kp / 200, (double)ConfigurationPWM(5).ki / 200, (double)ConfigurationPWM(5).kd / 200, P_ON_E, DIRECT)
};

void printlnPwmDrive(PWMConfiguration &conf);
void printStatus();
void printFullStatus();
void setPwmConfiguration(CommandParameter &parameters);
void setPidConfiguration(CommandParameter &parameters);
void disableFan(CommandParameter &parameters);
void configure(CommandParameter &parameters);
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

#ifdef TIMING_DEBUG
CommandHandler<22, 70, 0> SerialCommandHandler; // 22 commands, max length of command 70, 0 variables
#else
CommandHandler<18, 70, 0> SerialCommandHandler; // 18 commands, max length of command 70, 0 variables
#endif

void setSerialCommandHandler(){
  SerialCommandHandler.AddCommand(F("help"), printHelp);
  SerialCommandHandler.AddCommand(F("guiE"), guiEnable);
  SerialCommandHandler.AddCommand(F("guiD"), guiDisable);
  SerialCommandHandler.AddCommand(F("setFan"), setPwmConfiguration);
  SerialCommandHandler.AddCommand(F("setPid"), setPidConfiguration);
  SerialCommandHandler.AddCommand(F("setConf"), setConfiguration);
  SerialCommandHandler.AddCommand(F("setThermistor"), setThermistor);
  SerialCommandHandler.AddCommand(F("s"), printStatus);
  SerialCommandHandler.AddCommand(F("fs"), printFullStatus);
  SerialCommandHandler.AddCommand(F("conf"), configuration);
  SerialCommandHandler.AddCommand(F("guistat1"), guistat1);
  SerialCommandHandler.AddCommand(F("guistat2"), guistat2);
  SerialCommandHandler.AddCommand(F("guiUpdate"), guiUpdate);
  SerialCommandHandler.AddCommand(F("load"), loadConfiguration);
  SerialCommandHandler.AddCommand(F("save"), saveConfiguration);
  SerialCommandHandler.AddCommand(F("disableFan"), disableFan);
  SerialCommandHandler.AddCommand(F("pidU"), sendPidUpdates);
  SerialCommandHandler.AddCommand(F("cacheStatus"), cacheStatus);
#ifdef TIMING_DEBUG
  SerialCommandHandler.AddCommand(F("time"), sendTime);
  SerialCommandHandler.AddCommand(F("timing"), timing);
  SerialCommandHandler.AddCommand(F("mi"), measureInterrupts);
#endif
#ifdef FREE_MEMORY_DEBUG
  SerialCommandHandler.AddCommand(F("freemem"), freeMem);
#endif
  SerialCommandHandler.SetDefaultHandler(Cmd_Unknown);
}

