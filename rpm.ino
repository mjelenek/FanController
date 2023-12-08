#define writeLastFanRpmSensorTimeMacro(i) writeLastFanRpmSensorTime(&lastFanRpmSensorTime[i], fanRpmSensorTimes[i], now);
// max 10000rpm
#define MINIMAL_DELAY_BETWEEN_RPM_SIGNAL_CHANGES 1500

#if HWversion == 1
  #include "ISR_Nano.h"
#endif
#if HWversion == 2
  #include "ISR_Meduino2560.h"
#endif

inline __attribute__((always_inline)) void writeLastFanRpmSensorTime(volatile byte *lastFanRpmSensorTime, volatile unsigned long fanRpmSensorTimes[], unsigned long now){
  byte lastFanRpmSensorTimeIndex = *lastFanRpmSensorTime;
  unsigned long lastRecord = fanRpmSensorTimes[lastFanRpmSensorTimeIndex];
  if(now - lastRecord >= MINIMAL_DELAY_BETWEEN_RPM_SIGNAL_CHANGES){
    lastFanRpmSensorTimeIndex++;
    if(lastFanRpmSensorTimeIndex >= FAN_RPM_SENSOR_TIMES_FIELD){
      lastFanRpmSensorTimeIndex = 0;
    }
    fanRpmSensorTimes[lastFanRpmSensorTimeIndex] = now;
    *lastFanRpmSensorTime = lastFanRpmSensorTimeIndex;
  }
}

void calculateRPM(byte fanNumber){
  disableRpmIRS(fanNumber);
  byte lastFanRpmSensorTimeIndex = lastFanRpmSensorTime[fanNumber];
  byte time1Pointer = lastFanRpmSensorTimeIndex + 1;
  if(time1Pointer >= FAN_RPM_SENSOR_TIMES_FIELD){
    time1Pointer = 0;
  }
  unsigned long time0 = fanRpmSensorTimes[fanNumber][lastFanRpmSensorTimeIndex];
  unsigned long time1 = fanRpmSensorTimes[fanNumber][time1Pointer];
  enableRpmIRS(fanNumber);
  unsigned long now = micros();
  if((now - time0) > 300000 || (now - time1) > 900000){
    rpm[fanNumber] = 0;
    lastFanRpmSensorTimeCounted[fanNumber] = lastFanRpmSensorTimeIndex;
  } else {
    if(lastFanRpmSensorTimeCounted[fanNumber] != lastFanRpmSensorTimeIndex){
      rpm[fanNumber] = (60000000 / (time0 - time1));
      lastFanRpmSensorTimeCounted[fanNumber] = lastFanRpmSensorTimeIndex;
      pidUpdateDirect(fanNumber, ConfigurationPWM(fanNumber));
    }
  }
}

void calculateRPMs(){
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    calculateRPM(i);
  }
}
