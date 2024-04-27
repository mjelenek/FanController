#define TACH0_SET(PIN_NUM) if(state & (1 << PIN_NUM)) {TACH0_1;} else {TACH0_0;}
#define TACH1_SET(PIN_NUM) if(state & (1 << PIN_NUM)) {TACH1_1;} else {TACH1_0;}
#define TACH2_SET(PIN_NUM) if(state & (1 << PIN_NUM)) {TACH2_1;} else {TACH2_0;}
#define TACH3_SET(PIN_NUM) if(state & (1 << PIN_NUM)) {TACH3_1;} else {TACH3_0;}
#define TACH4_SET(PIN_NUM) if(state & (1 << PIN_NUM)) {TACH4_1;} else {TACH4_0;}
#define TACH5_SET(PIN_NUM) if(state & (1 << PIN_NUM)) {TACH5_1;} else {TACH5_0;}

#define TACH_SET(RPMSENSOR, PIN, PIN_NUM, RPM_NUM, TACH_NUM)   if(rmpToMainboard(RPM_NUM) == RPMSENSOR){\
    byte state = PIN;\
    TACH_NUM(PIN_NUM)\
  }

#define TACH_SET_STATE(RPMSENSOR, PIN_NUM, RPM_NUM, TACH_NUM)   if(rmpToMainboard(RPM_NUM) == RPMSENSOR){\
    TACH_NUM(PIN_NUM)\
  }

#define writeTachMacro(RPMSENSOR, PIN, PIN_NUM)  TACH_SET(RPMSENSOR, PIN, PIN_NUM, 0, TACH0_SET)\
  TACH_SET(RPMSENSOR, PIN, PIN_NUM, 1, TACH1_SET)\
  TACH_SET(RPMSENSOR, PIN, PIN_NUM, 2, TACH2_SET)\
  TACH_SET(RPMSENSOR, PIN, PIN_NUM, 3, TACH3_SET)\
  TACH_SET(RPMSENSOR, PIN, PIN_NUM, 4, TACH4_SET)\
  TACH_SET(RPMSENSOR, PIN, PIN_NUM, 5, TACH5_SET)

#define writeTachMacroState(RPMSENSOR, PIN_NUM)  TACH_SET_STATE(RPMSENSOR, PIN_NUM, 0, TACH0_SET)\
  TACH_SET_STATE(RPMSENSOR, PIN_NUM, 1, TACH1_SET)\
  TACH_SET_STATE(RPMSENSOR, PIN_NUM, 2, TACH2_SET)\
  TACH_SET_STATE(RPMSENSOR, PIN_NUM, 3, TACH3_SET)\
  TACH_SET_STATE(RPMSENSOR, PIN_NUM, 4, TACH4_SET)\
  TACH_SET_STATE(RPMSENSOR, PIN_NUM, 5, TACH5_SET)

#define handleExtIntMacro(RPMSENSOR, PIN, PIN_NUM)  unsigned long now = micros();\
  writeTachMacro(RPMSENSOR, PIN, PIN_NUM)\
  writeLastFanRpmSensorTimeMacro(RPMSENSOR);

#define handleChangePinMacro(RPMSENSOR, PIN)    if(changed & (1 << PIN)){\
    writeTachMacroState(RPMSENSOR, PIN)\
    writeLastFanRpmSensorTimeMacro(RPMSENSOR);\
  }

// external interrupt on pin PD0
ISR(INT0_vect){
  handleExtIntMacro(RPMSENSOR_INT0, PIND, PIND0)
}

// external interrupt on pin PD1
ISR(INT1_vect){
  handleExtIntMacro(RPMSENSOR_INT1, PIND, PIND1)
}

// external interrupt on pin PD2
ISR(INT2_vect){
  handleExtIntMacro(RPMSENSOR_INT2, PIND, PIND2)
}

// external interrupt on pin PD3
ISR(INT3_vect){
  handleExtIntMacro(RPMSENSOR_INT3, PIND, PIND3)
}

// external interrupt on pin PE4
ISR(INT4_vect){
  handleExtIntMacro(RPMSENSOR_INT4, PINE, PINE4)
}

// external interrupt on pin PE5
ISR(INT5_vect){
  handleExtIntMacro(RPMSENSOR_INT5, PINE, PINE5)
}

// change pin PB0, PB1, PB2, PB3
ISR(PCINT0_vect){
  static byte lastState;
  unsigned long now = micros();
  byte state = PINB;
  byte changed = state ^ lastState;

  handleChangePinMacro(RPMSENSOR_PCINT0, PINB0)

  handleChangePinMacro(RPMSENSOR_PCINT1, PINB1)
 
  handleChangePinMacro(RPMSENSOR_PCINT2, PINB2)
  
  handleChangePinMacro(RPMSENSOR_PCINT3, PINB3)
 
  lastState = state;
}

// change pin PJ0, PJ1
ISR(PCINT1_vect){
  static byte lastState;
  unsigned long now = micros();
  byte state = PINJ;
  byte changed = state ^ lastState;

  handleChangePinMacro(RPMSENSOR_PCINT9, PINJ0)

  handleChangePinMacro(RPMSENSOR_PCINT10, PINJ1)

  lastState = state;
}
