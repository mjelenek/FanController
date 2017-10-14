#define fanSensor5FilterDefinition B0000111

// overflow on timer1 interrupt handler
ISR(TIMER1_OVF_vect){
  static byte lastState;
  static byte fanSensor5Filter = 0;
  cnt2 = TCNT2;
  if(cnt2 < CNT2_MIN_VALUE_FOR_READ_RPM_SENSOR5){
    return;
  }
  byte fanSensor5 = (PINC >> 5) & 1;
  unsigned long now = micros();
  fanSensor5Filter = ((fanSensor5Filter << 1) | fanSensor5) & fanSensor5FilterDefinition;
  byte fanSensor5Value = lastState;
  if(fanSensor5Filter == 0){
    fanSensor5Value = 0;
  }
  if(fanSensor5Filter == fanSensor5FilterDefinition){
    fanSensor5Value = 1;
  }

  if(lastState ^ fanSensor5Value){
    if(rmpToMainboard == 5){
      digitalWrite(LED_OUT, fanSensor5Value);
    }
    if(fanSensor5Value){
      writeLastFanRpmSensorTime(&lastFanRpmSensorTime5, fanRpmSensorTimes5, now);
    }
    lastState = fanSensor5Value;
  }
}

// change pin PB0, PB4
ISR(PCINT0_vect){
  static byte lastState;
  unsigned long now = micros();
  byte state = PINB;
  byte changed = state ^ lastState;

  if(changed & (1 << PINB0)){
    if(rmpToMainboard == 3){
      digitalWrite(LED_OUT, state & (1 << PINB0));
    }
    if(state & (1 << PINB0)){
      writeLastFanRpmSensorTime(&lastFanRpmSensorTime3, fanRpmSensorTimes3, now);
    }
  }

  if(changed & (1 << PINB4)){
    if(rmpToMainboard == 4){
      digitalWrite(LED_OUT, state & (1 << PINB4));
    }
    if(state & (1 << PINB4)){
      writeLastFanRpmSensorTime(&lastFanRpmSensorTime4, fanRpmSensorTimes4, now);
    }
  }

  lastState = state;
}

// change pin PD2, PD4, PD7
ISR(PCINT2_vect){
  static byte lastState;
  unsigned long now = micros();
  byte state = PIND;
  byte changed = state ^ lastState;

  if(changed & (1 << PIND2)){
    if(rmpToMainboard == 0){
      digitalWrite(LED_OUT, state & (1 << PIND2));
    }
    if(state & (1 << PIND2)){
      writeLastFanRpmSensorTime(&lastFanRpmSensorTime0, fanRpmSensorTimes0, now);
    }
  }

  if(changed & (1 << PIND4)){
    if(rmpToMainboard == 1){
      digitalWrite(LED_OUT, state & (1 << PIND4));
    }
    if(state & (1 << PIND4)){
      writeLastFanRpmSensorTime(&lastFanRpmSensorTime1, fanRpmSensorTimes1, now);
    }
  }

  if(changed & (1 << PIND7)){
    if(rmpToMainboard == 2){
      digitalWrite(LED_OUT, state & (1 << PIND7));
    }
    if(state & (1 << PIND7)){
      writeLastFanRpmSensorTime(&lastFanRpmSensorTime2, fanRpmSensorTimes2, now);
    }
  }

  lastState = state;
}

inline __attribute__((always_inline)) void writeLastFanRpmSensorTime(byte *lastFanRpmSensorTime, unsigned long fanRpmSensorTimes[], unsigned long now){
//void writeLastFanRpmSensorTime(byte fanNumber, unsigned long now){
  (*lastFanRpmSensorTime)++;
  if(*lastFanRpmSensorTime >= FAN_RPM_SENSOR_TIMES_FIELD){
    *lastFanRpmSensorTime = 0;
  }
  fanRpmSensorTimes[*lastFanRpmSensorTime] = now;
}

double countRPM(byte lastFanRpmSensorTimeIndex, unsigned long fanRpmSensorTimes[]){
  PCICR = 0;  // disable pin change interrupts
  byte time1Pointer = lastFanRpmSensorTimeIndex + 1;
  if(time1Pointer >= FAN_RPM_SENSOR_TIMES_FIELD){
    time1Pointer = 0;
  }
  unsigned long time0 = fanRpmSensorTimes[lastFanRpmSensorTimeIndex];
  unsigned long time1 = fanRpmSensorTimes[time1Pointer];
  PCICR = (1 << PCIE0) | (1 << PCIE2);  // enable pin change interrupts
  if((micros() - time0) > 240000 || (micros() - time1) > 840000){
    return 0;
  }
  return 60000000 / (time0 - time1);
}

void countRPMs(){
  rpm0 = countRPM(lastFanRpmSensorTime0, fanRpmSensorTimes0);
  rpm1 = countRPM(lastFanRpmSensorTime1, fanRpmSensorTimes1);
  rpm2 = countRPM(lastFanRpmSensorTime2, fanRpmSensorTimes2);
  rpm3 = countRPM(lastFanRpmSensorTime3, fanRpmSensorTimes3);
  rpm4 = countRPM(lastFanRpmSensorTime4, fanRpmSensorTimes4);
  rpm5 = countRPM(lastFanRpmSensorTime5, fanRpmSensorTimes5);
}

