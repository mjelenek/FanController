#define writeTachMacro(RPMSENSOR, PIN, PIN_NUM)  if(rmpToMainboard(0) == RPMSENSOR){\
    byte state = PIN;\
    if(state & (1 << PIN_NUM)) TACH0_SET;\
  }\
  if(rmpToMainboard(1) == RPMSENSOR){\
    byte state = PIN;\
    if(state & (1 << PIN_NUM)) TACH1_SET;\
  }\
  if(rmpToMainboard(2) == RPMSENSOR){\
    byte state = PIN;\
    if(state & (1 << PIN_NUM)) TACH2_SET;\
  }\
  if(rmpToMainboard(3) == RPMSENSOR){\
    byte state = PIN;\
    if(state & (1 << PIN_NUM)) TACH3_SET;\
  }\
  if(rmpToMainboard(4) == RPMSENSOR){\
    byte state = PIN;\
    if(state & (1 << PIN_NUM)) TACH4_SET;\
  }\
  if(rmpToMainboard(5) == RPMSENSOR){\
    byte state = PIN;\
    if(state & (1 << PIN_NUM)) TACH5_SET;\
  }

#define writeTachMacroState(RPMSENSOR, PIN_NUM)  if(rmpToMainboard(0) == RPMSENSOR){\
    if(state & (1 << PIN_NUM)) TACH0_SET;\
  }\
  if(rmpToMainboard(1) == RPMSENSOR){\
    if(state & (1 << PIN_NUM)) TACH1_SET;\
  }\
  if(rmpToMainboard(2) == RPMSENSOR){\
    if(state & (1 << PIN_NUM)) TACH2_SET;\
  }\
  if(rmpToMainboard(3) == RPMSENSOR){\
    if(state & (1 << PIN_NUM)) TACH3_SET;\
  }\
  if(rmpToMainboard(4) == RPMSENSOR){\
    if(state & (1 << PIN_NUM)) TACH4_SET;\
  }\
  if(rmpToMainboard(5) == RPMSENSOR){\
    if(state & (1 << PIN_NUM)) TACH5_SET;\
  }

// external interrupt on pin PD0
ISR(INT0_vect){
  unsigned long now = micros();
  writeTachMacro(RPMSENSOR_INT0, PIND, PIND0)
  writeLastFanRpmSensorTimeMacro(RPMSENSOR_INT0);
}

// external interrupt on pin PD1
ISR(INT1_vect){
  unsigned long now = micros();
  writeTachMacro(RPMSENSOR_INT1, PIND, PIND1)
  writeLastFanRpmSensorTimeMacro(RPMSENSOR_INT1);
}

// external interrupt on pin PD2
ISR(INT2_vect){
  unsigned long now = micros();
  writeTachMacro(RPMSENSOR_INT2, PIND, PIND2)
  writeLastFanRpmSensorTimeMacro(RPMSENSOR_INT2);
}

// external interrupt on pin PD3
ISR(INT3_vect){
  unsigned long now = micros();
  writeTachMacro(RPMSENSOR_INT3, PIND, PIND3)
  writeLastFanRpmSensorTimeMacro(RPMSENSOR_INT3);
}

// external interrupt on pin PE4
ISR(INT4_vect){
  unsigned long now = micros();
  writeTachMacro(RPMSENSOR_INT4, PINE, PINE4)
  writeLastFanRpmSensorTimeMacro(RPMSENSOR_INT4);
}

// external interrupt on pin PE5
ISR(INT5_vect){
  unsigned long now = micros();
  writeTachMacro(RPMSENSOR_INT5, PINE, PINE5)
  writeLastFanRpmSensorTimeMacro(RPMSENSOR_INT5);
}

// change pin PB0, PB1, PB2, PB3
ISR(PCINT0_vect){
  static byte lastState;
  unsigned long now = micros();
  byte state = PINB;
  byte changed = state ^ lastState;

  if(changed & (1 << PINB0)){
    writeTachMacroState(RPMSENSOR_PCINT0, PINB0)
    writeLastFanRpmSensorTimeMacro(RPMSENSOR_PCINT0);
  }

  if(changed & (1 << PINB1)){
    writeTachMacroState(RPMSENSOR_PCINT1, PINB1)
    writeLastFanRpmSensorTimeMacro(RPMSENSOR_PCINT1);
  }

  if(changed & (1 << PINB2)){
    writeTachMacroState(RPMSENSOR_PCINT2, PINB2)
    writeLastFanRpmSensorTimeMacro(RPMSENSOR_PCINT2);
  }

  if(changed & (1 << PINB3)){
    writeTachMacroState(RPMSENSOR_PCINT3, PINB3)
    writeLastFanRpmSensorTimeMacro(RPMSENSOR_PCINT3);
  }

  lastState = state;
}

// change pin PJ0, PJ1
ISR(PCINT1_vect){
  static byte lastState;
  unsigned long now = micros();
  byte state = PINJ;
  byte changed = state ^ lastState;

  if(changed & (1 << PINJ0)){
    writeTachMacroState(RPMSENSOR_PCINT9, PINJ0)
    writeLastFanRpmSensorTimeMacro(RPMSENSOR_PCINT9);
  }

  if(changed & (1 << PINJ1)){
    writeTachMacroState(RPMSENSOR_PCINT10, PINJ1)
    writeLastFanRpmSensorTimeMacro(RPMSENSOR_PCINT10);
  }

  lastState = state;
}

