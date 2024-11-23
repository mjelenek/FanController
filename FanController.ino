#define VERSION "1.1"

#if defined(ARDUINO_AVR_NANO)
#define HWversion 1
#endif
#if defined(ARDUINO_AVR_MEGA2560)
#define HWversion 2
#endif

#if HWversion == 1
  #include "Definition_NanoV1.h"
#endif
#if HWversion == 2
//  #include "Definition_Meduino2560.h"
  #include "Definition_Meduino2560v2.h"
#endif

#include "lib/PID_v1.cpp"
#include "lib/EEPROMStoreISR.h"
#include "CommandHandler.h"
#include <avr/wdt.h>

//#define PID_UPDATE_WHEN_RPM_COUNT

#define TIMING_DEBUG
#define FREE_MEMORY_DEBUG

#define COUNT_MILLLIS_BY_DIVIDE_MICROS
#define USE_TEMP_CACHE
#define USE_PWM_CACHE
#define CALIBRATE_THERMISTORS

#ifdef USE_PWM_CACHE
#define USE_FAN_NUMBER_DECLARATION ,byte fanNumber
#define USE_FAN_NUMBER ,fanNumber
#else
#define USE_FAN_NUMBER_DECLARATION
#define USE_FAN_NUMBER
#endif

#ifdef TIMING_DEBUG
#define WARN_MICROSECONDS_DEBUG 3000
unsigned long timeInCode;
unsigned long timeTotal;
unsigned int to1000;
unsigned int to1500;
unsigned int to2000;
unsigned int to2500;
unsigned int to3000;
unsigned int over3000;
unsigned short timeCounting = 0;
#endif

//one iteration microseconds
#define ITERATION_MICROSECONDS 5000
#define WARN_MICROSECONDS 4000
#define DELAY_THRESHOLD 10000

#define MAX_ALLOWED_TEMP 60

#include "classes.h"

typedef struct
{
  uint16_t m_uChecksum;
  PWMConfiguration m_UserData;
} CEEPROMPWM;

CEEPROMPWM EEMEM EEPROPWM[NUMBER_OF_FANS];

#if HWversion == 1
EEPROMStore<CEEPROMPWM> ConfigurationPWMHolder[] = {
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[0]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[1]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[2]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[3]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[4]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[5])};
#endif
#if HWversion == 2
EEPROMStore<CEEPROMPWM> ConfigurationPWMHolder[] = {
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[0]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[1]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[2]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[3]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[4]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[5]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[6]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[7]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[8]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[9]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[10]),
  EEPROMStore<CEEPROMPWM>(&EEPROPWM[11])};
#endif
#define ConfigurationPWM(i) ConfigurationPWMHolder[i].Data.m_UserData

typedef struct
{
  uint16_t m_uChecksum;
  ControllerConfiguration<NUMBER_OF_THERMISTORS, NUMBER_OF_RPM_TO_MAINBOARD> m_UserData;
} CEEPROMC;

CEEPROMC EEMEM EEPROMConf;

EEPROMStore<CEEPROMC> ControllerConfigurationHolder(&EEPROMConf);
// sensor to mainboard
#define rmpToMainboard(i) ControllerConfigurationHolder.Data.m_UserData.rmpToMainboard[i]
#define hysteresis ControllerConfigurationHolder.Data.m_UserData.hysteresis
#define profile ControllerConfigurationHolder.Data.m_UserData.profile
#define microsecondPerSecond ControllerConfigurationHolder.Data.m_UserData.microsecondPerSecond
#define thermistors(i) ControllerConfigurationHolder.Data.m_UserData.thermistors[i]

byte pwm[NUMBER_OF_FANS];
byte pwmDisabled[NUMBER_OF_FANS];

byte configurationToSave = 255;
unsigned long start;
unsigned long now;
unsigned long zpozdeni;
byte gui = 0;  // enable gui
byte i = 0;
byte j = 0;
byte updatesRTToSend[NUMBER_OF_FANS];
byte watchdogCounter;

// ADC values from mainboard
volatile uint16_t powerInADCAveraged[NUMBER_OF_MAINBOARD_CONNECTORS];
// ADC values from thermistors
volatile uint16_t thermistorADCAveraged[NUMBER_OF_THERMISTORS];

boolean TConnected[NUMBER_OF_THERMISTORS];
int Tint[NUMBER_OF_THERMISTORS];
int TWithHysteresisInt[NUMBER_OF_THERMISTORS];
unsigned char fakeTemp[NUMBER_OF_THERMISTORS];

#ifdef CALIBRATE_THERMISTORS
byte calibrateR[NUMBER_OF_THERMISTORS];
byte calibrateBeta[NUMBER_OF_THERMISTORS];
int tempExpectedInt;
#endif

#define FAN_RPM_SENSOR_TIMES_FIELD 5
volatile unsigned long fanRpmSensorTimes[NUMBER_OF_FANS][FAN_RPM_SENSOR_TIMES_FIELD];
volatile byte lastFanRpmSensorTimeIndexes[NUMBER_OF_FANS];
byte lastFanRpmSensorTimeCounted[NUMBER_OF_FANS];
double rpm[NUMBER_OF_FANS];

// Define Variables PIDs will be connecting to
double outputPid;
double inputPid;
double setpointPid[NUMBER_OF_FANS];
//Specify the links and initial tuning parameters
#if HWversion == 1
PID pid[] = {
  PID(&inputPid, &outputPid, &setpointPid[0], (double)ConfigurationPWM(0).kp / 200, (double)ConfigurationPWM(0).ki / 200, (double)ConfigurationPWM(0).kd / 200),
  PID(&inputPid, &outputPid, &setpointPid[1], (double)ConfigurationPWM(1).kp / 200, (double)ConfigurationPWM(1).ki / 200, (double)ConfigurationPWM(1).kd / 200),
  PID(&inputPid, &outputPid, &setpointPid[2], (double)ConfigurationPWM(2).kp / 200, (double)ConfigurationPWM(2).ki / 200, (double)ConfigurationPWM(2).kd / 200),
  PID(&inputPid, &outputPid, &setpointPid[3], (double)ConfigurationPWM(3).kp / 200, (double)ConfigurationPWM(3).ki / 200, (double)ConfigurationPWM(3).kd / 200),
  PID(&inputPid, &outputPid, &setpointPid[4], (double)ConfigurationPWM(4).kp / 200, (double)ConfigurationPWM(4).ki / 200, (double)ConfigurationPWM(4).kd / 200),
  PID(&inputPid, &outputPid, &setpointPid[5], (double)ConfigurationPWM(5).kp / 200, (double)ConfigurationPWM(5).ki / 200, (double)ConfigurationPWM(5).kd / 200)
};
#endif
#if HWversion == 2
PID pid[] = {
  PID(&inputPid, &outputPid, &setpointPid[0], (double)ConfigurationPWM(0).kp / 200, (double)ConfigurationPWM(0).ki / 200, (double)ConfigurationPWM(0).kd / 200),
  PID(&inputPid, &outputPid, &setpointPid[1], (double)ConfigurationPWM(1).kp / 200, (double)ConfigurationPWM(1).ki / 200, (double)ConfigurationPWM(1).kd / 200),
  PID(&inputPid, &outputPid, &setpointPid[2], (double)ConfigurationPWM(2).kp / 200, (double)ConfigurationPWM(2).ki / 200, (double)ConfigurationPWM(2).kd / 200),
  PID(&inputPid, &outputPid, &setpointPid[3], (double)ConfigurationPWM(3).kp / 200, (double)ConfigurationPWM(3).ki / 200, (double)ConfigurationPWM(3).kd / 200),
  PID(&inputPid, &outputPid, &setpointPid[4], (double)ConfigurationPWM(4).kp / 200, (double)ConfigurationPWM(4).ki / 200, (double)ConfigurationPWM(4).kd / 200),
  PID(&inputPid, &outputPid, &setpointPid[5], (double)ConfigurationPWM(5).kp / 200, (double)ConfigurationPWM(5).ki / 200, (double)ConfigurationPWM(5).kd / 200),
  PID(&inputPid, &outputPid, &setpointPid[6], (double)ConfigurationPWM(6).kp / 200, (double)ConfigurationPWM(6).ki / 200, (double)ConfigurationPWM(6).kd / 200),
  PID(&inputPid, &outputPid, &setpointPid[7], (double)ConfigurationPWM(7).kp / 200, (double)ConfigurationPWM(7).ki / 200, (double)ConfigurationPWM(7).kd / 200),
  PID(&inputPid, &outputPid, &setpointPid[8], (double)ConfigurationPWM(8).kp / 200, (double)ConfigurationPWM(8).ki / 200, (double)ConfigurationPWM(8).kd / 200),
  PID(&inputPid, &outputPid, &setpointPid[9], (double)ConfigurationPWM(9).kp / 200, (double)ConfigurationPWM(9).ki / 200, (double)ConfigurationPWM(9).kd / 200),
  PID(&inputPid, &outputPid, &setpointPid[10], (double)ConfigurationPWM(10).kp / 200, (double)ConfigurationPWM(10).ki / 200, (double)ConfigurationPWM(10).kd / 200),
  PID(&inputPid, &outputPid, &setpointPid[11], (double)ConfigurationPWM(11).kp / 200, (double)ConfigurationPWM(11).ki / 200, (double)ConfigurationPWM(11).kd / 200)
};
#endif

void countT(byte tNumber);
int countEffectiveTemperature(byte tSelect);
void printlnPwmDrive(PWMConfiguration &conf);
void printStatus();
void printFullStatus();
void setPwmConfiguration(CommandParameter &parameters);
void setPidConfiguration(CommandParameter &parameters);
void disableFan(CommandParameter &parameters);
void configure(CommandParameter &parameters);
void sendPidUpates(CommandParameter &parameters);
byte countPWM(PWMConfiguration &conf, unsigned int input);
unsigned short countExpectedRPM(PWMConfiguration &conf, unsigned int temperature);
#ifdef USE_PWM_CACHE
byte countPWM(PWMConfiguration &conf, unsigned int input, byte fanNumber);
unsigned short countExpectedRPM(PWMConfiguration &conf, unsigned int temperature, byte fanNumber);
#endif
byte getNewPwm(PWMConfiguration &conf, byte pwmOld, unsigned short sensorValueAveraged, byte fanNumber);
byte getNewPwmByPowerCurve(PWMConfiguration &conf, byte pwmOld USE_FAN_NUMBER_DECLARATION);
byte getNewPwmByConstRpm(PWMConfiguration &conf, byte pwmOld, byte fanNumber);
void setpointPidByRpmCurve(PWMConfiguration &conf, byte pwmOld, byte fanNumber);
byte getTemperaturePartSelect(byte temperatures[], unsigned int temperature, byte len);
boolean pidCompute(byte fanNumber);
void pidUpdate(byte fanNumber, PWMConfiguration &conf);
void pidUpdateDirect(byte fanNumber, PWMConfiguration &conf);
void readRPMsensors();
void init_pid();
void measureInterrupts();
#if HWversion == 2
void setICRn(CommandParameter &parameters);
#endif

CommandHandler<29, 10 + CURVE_RPM_POINTS * 8, 0> SerialCommandHandler; // 29 commands, 0 variables

void setSerialCommandHandler(){
  SerialCommandHandler.AddCommand(F("version"), (void (*)(CommandParameter&))printVersionNumber);
  SerialCommandHandler.AddCommand(F("help"), (void (*)(CommandParameter&))printHelp);
  SerialCommandHandler.AddCommand(F("guiE"), (void (*)(CommandParameter&))guiEnable);
  SerialCommandHandler.AddCommand(F("guiD"), (void (*)(CommandParameter&))guiDisable);
  SerialCommandHandler.AddCommand(F("setFan"), (void (*)(CommandParameter&))setFanConfiguration);
  SerialCommandHandler.AddCommand(F("setPIn"), (void (*)(CommandParameter&))setPowerInCurve);
  SerialCommandHandler.AddCommand(F("setPwm"), (void (*)(CommandParameter&))setPwmCurve);
  SerialCommandHandler.AddCommand(F("setRpm"), (void (*)(CommandParameter&))setRpmCurve);
  SerialCommandHandler.AddCommand(F("setConf"), (void (*)(CommandParameter&))setConfiguration);
  SerialCommandHandler.AddCommand(F("setThermistor"), (void (*)(CommandParameter&))setThermistor);
  SerialCommandHandler.AddCommand(F("setTemp"), (void (*)(CommandParameter&))setTemp);
  SerialCommandHandler.AddCommand(F("s"), (void (*)(CommandParameter&))printStatus);
  SerialCommandHandler.AddCommand(F("fs"), (void (*)(CommandParameter&))printFullStatus);
  SerialCommandHandler.AddCommand(F("conf"), (void (*)(CommandParameter&))sendConfiguration);
  SerialCommandHandler.AddCommand(F("guistat"), (void (*)(CommandParameter&))guistat);
  SerialCommandHandler.AddCommand(F("guiUpdate"), (void (*)(CommandParameter&))guiUpdate);
  SerialCommandHandler.AddCommand(F("load"), (void (*)(CommandParameter&))loadConfiguration);
  SerialCommandHandler.AddCommand(F("save"), (void (*)(CommandParameter&))saveConfiguration);
  SerialCommandHandler.AddCommand(F("disableFan"), (void (*)(CommandParameter&))disableFan);
  SerialCommandHandler.AddCommand(F("pidU"), (void (*)(CommandParameter&))sendPidUpdates);
  SerialCommandHandler.AddCommand(F("cacheStatus"), (void (*)(CommandParameter&))cacheStatus);
  SerialCommandHandler.AddCommand(F("wd"), (void (*)(CommandParameter&))wd);
#ifdef TIMING_DEBUG
  SerialCommandHandler.AddCommand(F("time"), (void (*)(CommandParameter&))sendTime);
  SerialCommandHandler.AddCommand(F("timing"), (void (*)(CommandParameter&))timing);
  SerialCommandHandler.AddCommand(F("mi"), (void (*)(CommandParameter&))measureInterrupts);
#endif
#ifdef FREE_MEMORY_DEBUG
  SerialCommandHandler.AddCommand(F("freemem"), (void (*)(CommandParameter&))freeMem);
#endif
#ifdef CALIBRATE_THERMISTORS
  SerialCommandHandler.AddCommand(F("calibrateRNominal"), (void (*)(CommandParameter&))setCalibrateRNominal);
  SerialCommandHandler.AddCommand(F("calibrateB"), (void (*)(CommandParameter&))setCalibrateB);
#endif
#if HWversion == 2
  SerialCommandHandler.AddCommand(F("setICRn"), (void (*)(CommandParameter&))setICRn);
#endif

  SerialCommandHandler.SetDefaultHandler(Cmd_Unknown);
}
