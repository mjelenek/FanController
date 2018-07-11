#if defined(ARDUINO_AVR_NANO)
#define HWversion 1
#endif
#if defined(ARDUINO_AVR_MEGA2560)
#define HWversion 2
#endif

#include "PID_v1.h"
#include "CommandHandler.h"

#if HWversion == 1
  #include "Definition_NanoV1.h"
#endif
#if HWversion == 2
  #include "Definition_Meduino2560.h"
#endif

#include "EEPROMStoreISR.h"

#define TIMING_DEBUG
//#define SAVE_DEBUG
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
#define WARN_MICROSECONDS_DEBUG 1200
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

#define MAX_ALLOWED_TEMP 60

#include "classes.h"

typedef struct CEEPROMPWM
{
  uint16_t m_uChecksum;
  PWMConfiguration m_UserData;
};

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

typedef struct CEEPROMC
{
  uint16_t m_uChecksum;
  ControllerConfiguration<NUMBER_OF_THERMISTORS, NUMBER_OF_RPM_TO_MAINBOARD> m_UserData;
};

CEEPROMC EEMEM EEPROMConf;

EEPROMStore<CEEPROMC> ControllerConfigurationHolder(&EEPROMConf);
// sensor to mainboard
#define rmpToMainboard(i) ControllerConfigurationHolder.Data.m_UserData.rmpToMainboard[i]
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
byte part_64;  // cycles from 0 to 63;
byte updatesRTToSend[NUMBER_OF_FANS];

// ADC values from mainboard
volatile uint16_t powerInADCAveraged[NUMBER_OF_MAINBOARD_CONNECTORS];
// ADC values from thermistors
volatile uint16_t thermistorADCAveraged[NUMBER_OF_THERMISTORS];

boolean TConnected[NUMBER_OF_THERMISTORS];
int Tint[NUMBER_OF_THERMISTORS];
int TWithHysteresisInt[NUMBER_OF_THERMISTORS];
unsigned char fakeTemp[NUMBER_OF_THERMISTORS];

#define FAN_RPM_SENSOR_TIMES_FIELD 5
volatile unsigned long fanRpmSensorTimes[NUMBER_OF_FANS][FAN_RPM_SENSOR_TIMES_FIELD];
volatile byte lastFanRpmSensorTime[NUMBER_OF_FANS];
volatile boolean lastFanRpmSensorTimeUpdated[NUMBER_OF_FANS];
double rpm[NUMBER_OF_FANS];

#define writeLastFanRpmSensorTimeMacro(i) writeLastFanRpmSensorTime(&lastFanRpmSensorTime[i], fanRpmSensorTimes[i], now);\
  lastFanRpmSensorTimeUpdated[i] = true;

// Define Variables PIDs will be connecting to
double outputPid;
double inputPid;
double setpointPid[NUMBER_OF_FANS];
//Specify the links and initial tuning parameters
#if HWversion == 1
PID pid[] = {
  PID(&inputPid, &outputPid, &setpointPid[0], (double)ConfigurationPWM(0).kp / 200, (double)ConfigurationPWM(0).ki / 200, (double)ConfigurationPWM(0).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[1], (double)ConfigurationPWM(1).kp / 200, (double)ConfigurationPWM(1).ki / 200, (double)ConfigurationPWM(1).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[2], (double)ConfigurationPWM(2).kp / 200, (double)ConfigurationPWM(2).ki / 200, (double)ConfigurationPWM(2).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[3], (double)ConfigurationPWM(3).kp / 200, (double)ConfigurationPWM(3).ki / 200, (double)ConfigurationPWM(3).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[4], (double)ConfigurationPWM(4).kp / 200, (double)ConfigurationPWM(4).ki / 200, (double)ConfigurationPWM(4).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[5], (double)ConfigurationPWM(5).kp / 200, (double)ConfigurationPWM(5).ki / 200, (double)ConfigurationPWM(5).kd / 200, P_ON_E, DIRECT)
};
#endif
#if HWversion == 2
PID pid[] = {
  PID(&inputPid, &outputPid, &setpointPid[0], (double)ConfigurationPWM(0).kp / 200, (double)ConfigurationPWM(0).ki / 200, (double)ConfigurationPWM(0).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[1], (double)ConfigurationPWM(1).kp / 200, (double)ConfigurationPWM(1).ki / 200, (double)ConfigurationPWM(1).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[2], (double)ConfigurationPWM(2).kp / 200, (double)ConfigurationPWM(2).ki / 200, (double)ConfigurationPWM(2).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[3], (double)ConfigurationPWM(3).kp / 200, (double)ConfigurationPWM(3).ki / 200, (double)ConfigurationPWM(3).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[4], (double)ConfigurationPWM(4).kp / 200, (double)ConfigurationPWM(4).ki / 200, (double)ConfigurationPWM(4).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[5], (double)ConfigurationPWM(5).kp / 200, (double)ConfigurationPWM(5).ki / 200, (double)ConfigurationPWM(5).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[6], (double)ConfigurationPWM(6).kp / 200, (double)ConfigurationPWM(6).ki / 200, (double)ConfigurationPWM(6).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[7], (double)ConfigurationPWM(7).kp / 200, (double)ConfigurationPWM(7).ki / 200, (double)ConfigurationPWM(7).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[8], (double)ConfigurationPWM(8).kp / 200, (double)ConfigurationPWM(8).ki / 200, (double)ConfigurationPWM(8).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[9], (double)ConfigurationPWM(9).kp / 200, (double)ConfigurationPWM(9).ki / 200, (double)ConfigurationPWM(9).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[10], (double)ConfigurationPWM(10).kp / 200, (double)ConfigurationPWM(10).ki / 200, (double)ConfigurationPWM(10).kd / 200, P_ON_E, DIRECT),
  PID(&inputPid, &outputPid, &setpointPid[11], (double)ConfigurationPWM(11).kp / 200, (double)ConfigurationPWM(11).ki / 200, (double)ConfigurationPWM(11).kd / 200, P_ON_E, DIRECT)
};
#endif

void countT();
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
byte getTemperaturePartSelect(byte temperatures[], unsigned int temperature, byte len);
boolean pidCompute(byte fanNumber);
byte pidUpdate(byte fanNumber, PWMConfiguration &conf);
byte pidUpdateDirect(byte fanNumber, PWMConfiguration &conf);
void readRPMsensors();
void init_pid();
void measureInterrupts();

CommandHandler<23, 10 + CURVE_RPM_POINTS * 8, 0> SerialCommandHandler; // 23 commands, 0 variables

void setSerialCommandHandler(){
  SerialCommandHandler.AddCommand(F("help"), printHelp);
  SerialCommandHandler.AddCommand(F("guiE"), guiEnable);
  SerialCommandHandler.AddCommand(F("guiD"), guiDisable);
  SerialCommandHandler.AddCommand(F("setFan"), setFanConfiguration);
  SerialCommandHandler.AddCommand(F("setPwm"), setPwmCurve);
  SerialCommandHandler.AddCommand(F("setRpm"), setRpmCurve);
  SerialCommandHandler.AddCommand(F("setConf"), setConfiguration);
  SerialCommandHandler.AddCommand(F("setThermistor"), setThermistor);
  SerialCommandHandler.AddCommand(F("setTemp"), setTemp);
  SerialCommandHandler.AddCommand(F("s"), printStatus);
  SerialCommandHandler.AddCommand(F("fs"), printFullStatus);
  SerialCommandHandler.AddCommand(F("conf"), configuration);
  SerialCommandHandler.AddCommand(F("guistat"), guistat);
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

