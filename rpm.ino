#define writeLastFanRpmSensorTimeMacro(i) writeLastFanRpmSensorTime(&lastFanRpmSensorTime[i], fanRpmSensorTimes[i], now);
// max 15000rpm
#define MINIMAL_DELAY_BETWEEN_RPM_SIGNAL_CHANGES 1000

#if HWversion == 1
  #include "ISR_Nano.h"
#endif
#if HWversion == 2
  #include "ISR_Meduino2560.h"
#endif

inline __attribute__((always_inline)) void writeLastFanRpmSensorTime(byte *lastFanRpmSensorTime, unsigned long fanRpmSensorTimes[], unsigned long now){
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

double countRPM(byte fanNumber){
  disableRpmIRS();
  byte lastFanRpmSensorTimeIndex = lastFanRpmSensorTime[fanNumber];
  byte time1Pointer = lastFanRpmSensorTimeIndex + 1;
  if(time1Pointer >= FAN_RPM_SENSOR_TIMES_FIELD){
    time1Pointer = 0;
  }
  unsigned long time0 = fanRpmSensorTimes[fanNumber][lastFanRpmSensorTimeIndex];
  unsigned long time1 = fanRpmSensorTimes[fanNumber][time1Pointer];
  enableRpmIRS();
  unsigned long now = micros();
  if((now - time0) > 240000 || (now - time1) > 840000){
    rpm[fanNumber] = 0;
  } else {
    rpm[fanNumber] = (60000000 / (time0 - time1));
  }
}

void countRPMs(){
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    countRPM(i);
  }
}

