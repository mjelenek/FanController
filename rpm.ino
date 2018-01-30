#ifdef USE_TIMER1_OVF

#define fanSensor5FilterDefinition B00000111
#define CNT2_MIN_VALUE_FOR_READ_RPM_SENSOR5 230

// overflow on timer1 interrupt handler
ISR(TIMER1_OVF_vect){
  static byte lastState;
  static byte fanSensor5Filter = 0;
  byte cnt2 = TCNT2;
  byte fanSensor5 = (PINC >> 5) & 1;
  if(cnt2 < CNT2_MIN_VALUE_FOR_READ_RPM_SENSOR5){
    return;
  }
  fanSensor5Filter = ((fanSensor5Filter << 1) | fanSensor5) & fanSensor5FilterDefinition;
  byte fanSensor5Value = lastState;
  if(fanSensor5Filter == 0){
    fanSensor5Value = 0;
  }
  if(fanSensor5Filter == fanSensor5FilterDefinition){
    fanSensor5Value = 1;
  }

  if(lastState ^ fanSensor5Value){
    if(*rmpToMainboard == 5){
      if(fanSensor5Value) LED_OUT_SET;
    }
    if(fanSensor5Value){
      writeLastFanRpmSensorTime(&lastFanRpmSensorTime5, fanRpmSensorTimes5,  micros());
      lastFanRpmSensorTimeUpdated |= B00100000;
    }
    lastState = fanSensor5Value;
  }
}
#endif USE_TIMER1_OVF

// change pin PB0, PB4
ISR(PCINT0_vect){
  static byte lastState;
  unsigned long now = micros();
  byte state = PINB;
  byte changed = state ^ lastState;

  if(changed & (1 << PINB0)){
    if(*rmpToMainboard == 3){
      if(state & (1 << PINB0)) LED_OUT_SET;
    }
    if(state & (1 << PINB0)){
      writeLastFanRpmSensorTime(&lastFanRpmSensorTime3, fanRpmSensorTimes3, now);
      lastFanRpmSensorTimeUpdated |= B00001000;
    }
  }

  if(changed & (1 << PINB4)){
    if(*rmpToMainboard == 4){
      if(state & (1 << PINB4)) LED_OUT_SET;
    }
    if(state & (1 << PINB4)){
      writeLastFanRpmSensorTime(&lastFanRpmSensorTime4, fanRpmSensorTimes4, now);
      lastFanRpmSensorTimeUpdated |= B00010000;
    }
  }

  lastState = state;
}

#ifndef USE_TIMER1_OVF
// change pin PC5
ISR(PCINT1_vect){
  static byte lastState;
  unsigned long now = micros();
  byte state = PINC & (1 << PINC5);
  if(*rmpToMainboard == 5){
    if(state) LED_OUT_SET;
  }
  if(state){
    writeLastFanRpmSensorTime(&lastFanRpmSensorTime5, fanRpmSensorTimes5,  now);
    lastFanRpmSensorTimeUpdated |= B00100000;
  }
  lastState = state;
}
#endif USE_TIMER1_OVF

// change pin PD2, PD4, PD7
ISR(PCINT2_vect){
  static byte lastState;
  unsigned long now = micros();
  byte state = PIND;
  byte changed = state ^ lastState;

  if(changed & (1 << PIND2)){
    if(*rmpToMainboard == 0){
      if(state & (1 << PIND2)) LED_OUT_SET;
    }
    if(state & (1 << PIND2)){
      writeLastFanRpmSensorTime(&lastFanRpmSensorTime0, fanRpmSensorTimes0, now);
      lastFanRpmSensorTimeUpdated |= B00000001;
    }
  }

  if(changed & (1 << PIND4)){
    if(*rmpToMainboard == 1){
      if(state & (1 << PIND4)) LED_OUT_SET;
    }
    if(state & (1 << PIND4)){
      writeLastFanRpmSensorTime(&lastFanRpmSensorTime1, fanRpmSensorTimes1, now);
      lastFanRpmSensorTimeUpdated |= B00000010;
    }
  }

  if(changed & (1 << PIND7)){
    if(*rmpToMainboard == 2){
      if(state & (1 << PIND7)) LED_OUT_SET;
    }
    if(state & (1 << PIND7)){
      writeLastFanRpmSensorTime(&lastFanRpmSensorTime2, fanRpmSensorTimes2, now);
      lastFanRpmSensorTimeUpdated |= B00000100;
    }
  }

  lastState = state;
}

inline __attribute__((always_inline)) void writeLastFanRpmSensorTime(byte *lastFanRpmSensorTime, unsigned long fanRpmSensorTimes[], unsigned long now){
  byte lastFanRpmSensorTimeValue = *lastFanRpmSensorTime;
  lastFanRpmSensorTimeValue++;
  if(lastFanRpmSensorTimeValue >= FAN_RPM_SENSOR_TIMES_FIELD){
    lastFanRpmSensorTimeValue = 0;
  }
  fanRpmSensorTimes[lastFanRpmSensorTimeValue] = now;
  *lastFanRpmSensorTime = lastFanRpmSensorTimeValue;
}

short countRPM(byte lastFanRpmSensorTimeIndex, unsigned long fanRpmSensorTimes[]){
  PCICR = 0;                            // disable pin change interrupts
  #ifdef USE_TIMER1_OVF
  TIMSK1 &= B11111110;                  // disable timer1 overflow interrupt
  #endif USE_TIMER1_OVF
  byte time1Pointer = lastFanRpmSensorTimeIndex + 1;
  if(time1Pointer >= FAN_RPM_SENSOR_TIMES_FIELD){
    time1Pointer = 0;
  }
  unsigned long time0 = fanRpmSensorTimes[lastFanRpmSensorTimeIndex];
  unsigned long time1 = fanRpmSensorTimes[time1Pointer];
  #ifdef USE_TIMER1_OVF
  TIMSK1 |= B00000001;                  // enable timer1 overflow interrupt
  #endif USE_TIMER1_OVF
  PCICR = (1 << PCIE0) | (1 << PCIE1) | (1 << PCIE2); // enable pin change interrupts
  unsigned long now = micros();
  if((now - time0) > 240000 || (now - time1) > 840000){
    return 0;
  }
  return 60000000 / (time0 - time1);
}

void countRPMs(){
  PCICR = 0;                            // disable pin change interrupts
  #ifdef USE_TIMER1_OVF
  TIMSK1 &= B11111110;                  // disable timer1 overflow interrupt
  #endif USE_TIMER1_OVF
  byte lastFanRpmSensorTimeUpdatedLocal = lastFanRpmSensorTimeUpdated;
  lastFanRpmSensorTimeUpdated = 0;
  #ifdef USE_TIMER1_OVF
  TIMSK1 |= B00000001;                  // enable timer1 overflow interrupt
  #endif USE_TIMER1_OVF
  PCICR = (1 << PCIE0) | (1 << PCIE1) | (1 << PCIE2); // enable pin change interrupts

  if(((lastFanRpmSensorTimeUpdatedLocal & B00000001) > 0) || (i == (254 - 64))){
    rpm[0] = countRPM(lastFanRpmSensorTime0, fanRpmSensorTimes0);
  }
  if(((lastFanRpmSensorTimeUpdatedLocal & B00000010) > 0) || (i == (255 - 64))){
    rpm[1] = countRPM(lastFanRpmSensorTime1, fanRpmSensorTimes1);
  }
  if(((lastFanRpmSensorTimeUpdatedLocal & B00000100) > 0) || (i == (254 - 32))){
    rpm[2] = countRPM(lastFanRpmSensorTime2, fanRpmSensorTimes2);
  }
  if(((lastFanRpmSensorTimeUpdatedLocal & B00001000) > 0) || (i == (255 - 32))){
    rpm[3] = countRPM(lastFanRpmSensorTime3, fanRpmSensorTimes3);
  }
  if(((lastFanRpmSensorTimeUpdatedLocal & B00010000) > 0) || (i == 254)){
    rpm[4] = countRPM(lastFanRpmSensorTime4, fanRpmSensorTimes4);
  }
  if(((lastFanRpmSensorTimeUpdatedLocal & B00100000) > 0) || (i == 255)){
    rpm[5] = countRPM(lastFanRpmSensorTime5, fanRpmSensorTimes5);
  }
}

