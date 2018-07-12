#define writeLastFanRpmSensorTimeMacro(i) writeLastFanRpmSensorTime(&lastFanRpmSensorTime[i], fanRpmSensorTimes[i], now);

#if HWversion == 1
  #include "ISR_Nano.h"
#endif
#if HWversion == 2
  #include "ISR_Meduino2560.h"
#endif

inline __attribute__((always_inline)) void writeLastFanRpmSensorTime(byte *lastFanRpmSensorTime, unsigned long fanRpmSensorTimes[], unsigned long now){
  byte lastFanRpmSensorTimeValue = *lastFanRpmSensorTime;
  lastFanRpmSensorTimeValue++;
  if(lastFanRpmSensorTimeValue >= FAN_RPM_SENSOR_TIMES_FIELD){
    lastFanRpmSensorTimeValue = 0;
  }
  fanRpmSensorTimes[lastFanRpmSensorTimeValue] = now;
  *lastFanRpmSensorTime = lastFanRpmSensorTimeValue;
}

double countRPM(byte lastFanRpmSensorTimeIndex, unsigned long fanRpmSensorTimes[]){
  disableRpmIRS();
  byte time1Pointer = lastFanRpmSensorTimeIndex + 1;
  if(time1Pointer >= FAN_RPM_SENSOR_TIMES_FIELD){
    time1Pointer = 0;
  }
  unsigned long time0 = fanRpmSensorTimes[lastFanRpmSensorTimeIndex];
  unsigned long time1 = fanRpmSensorTimes[time1Pointer];
  enableRpmIRS();
  unsigned long now = micros();
  if((now - time0) > 240000 || (now - time1) > 840000){
    return 0;
  }
  return (60000000 / (time0 - time1));
}

void countRPMs(){
  for(byte x = 0; x < NUMBER_OF_FANS; x++){
    rpm[x] = countRPM(lastFanRpmSensorTime[x], fanRpmSensorTimes[x]);
  }
}

