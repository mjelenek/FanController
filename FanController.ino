#include "PID_v1.h"
#include "CommandHandler.h"
#include "EEPROMStoreISR.h"
#include "classes.h"

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

#define NUMBER_OF_THERMISTORS 2
#define NUMBER_OF_FANS 6

//by multimeter
//#define ANALOGREFERENCEVOLTAGE 3.3
// resistance of resistor in series with thermistor(value measured by multimeter)
unsigned short RT[NUMBER_OF_THERMISTORS] ={9990, 9990};

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

typedef struct CEEPROMPWM
{
  uint16_t m_uChecksum;
  PWMConfiguration m_UserData;
};

CEEPROMPWM EEMEM EEPROPWM[NUMBER_OF_FANS];

EEPROMStore<CEEPROMPWM> ConfigurationPWMHolder[] = {
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[0]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[1]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[2]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[3]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[4]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[5])};
#define ConfigurationPWM(i) ConfigurationPWMHolder[i].Data.m_UserData

typedef struct CEEPROMC
{
  uint16_t m_uChecksum;
  ControllerConfiguration<NUMBER_OF_THERMISTORS> m_UserData;
};

CEEPROMC EEMEM EEPROMConf;

EEPROMStore<CEEPROMC> ControllerConfigurationHolder(&EEPROMConf);
// sensor to mainboard
#define rmpToMainboard ControllerConfigurationHolder.Data.m_UserData.rmpToMainboard
#define hysteresis ControllerConfigurationHolder.Data.m_UserData.hysteresis
#define thermistors(i) ControllerConfigurationHolder.Data.m_UserData.thermistors[i]
 
byte pwm[NUMBER_OF_FANS];
byte pwmDisabled[NUMBER_OF_FANS];

unsigned long start;
unsigned long now;
unsigned short zpozdeni;
byte gui = 0;  // enable gui
byte i = 0;
byte j = 0;
byte part_32;  // cycles from 0 to 31;
byte updatesRTToSend[NUMBER_OF_FANS];

unsigned long RTkoeficient[NUMBER_OF_THERMISTORS];
boolean TConnected[NUMBER_OF_THERMISTORS];
int Tint[NUMBER_OF_THERMISTORS];
int TWithHysteresisInt[NUMBER_OF_THERMISTORS];


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
volatile unsigned long fanRpmSensorTimes[NUMBER_OF_FANS][FAN_RPM_SENSOR_TIMES_FIELD];
volatile byte lastFanRpmSensorTime[NUMBER_OF_FANS];
volatile byte lastFanRpmSensorTimeUpdated;
double rpm[NUMBER_OF_FANS];

// Define Variables PIDs will be connecting to
double outputPid;
double inputPid;
double setpointPid[NUMBER_OF_FANS];
//Specify the links and initial tuning parameters
PID pid[] = {
  PID(&inputPid, &outputPid, &setpointPid[0], (double)ConfigurationPWM(0).kp / 200, (double)ConfigurationPWM(0).ki / 200, (double)ConfigurationPWM(0).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[1], (double)ConfigurationPWM(1).kp / 200, (double)ConfigurationPWM(1).ki / 200, (double)ConfigurationPWM(1).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[2], (double)ConfigurationPWM(2).kp / 200, (double)ConfigurationPWM(2).ki / 200, (double)ConfigurationPWM(2).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[3], (double)ConfigurationPWM(3).kp / 200, (double)ConfigurationPWM(3).ki / 200, (double)ConfigurationPWM(3).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[4], (double)ConfigurationPWM(4).kp / 200, (double)ConfigurationPWM(4).ki / 200, (double)ConfigurationPWM(4).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[5], (double)ConfigurationPWM(5).kp / 200, (double)ConfigurationPWM(5).ki / 200, (double)ConfigurationPWM(5).kd / 200, P_ON_E, DIRECT)
};

int countEffectiveTemperature(byte tSelect);
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
byte getTemperaturePartSelect(byte temperatures[], unsigned int temperature);
boolean pidCompute(byte fanNumber);
byte pidUpdate(byte fanNumber, PWMConfiguration &conf);
byte pidUpdateDirect(byte fanNumber, PWMConfiguration &conf);
void readRPMsensors();
void init_pid();
void measureInterrupts();

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

