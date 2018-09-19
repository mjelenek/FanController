#define NUMBER_OF_THERMISTORS 6
#define NUMBER_OF_FANS 12
#define NUMBER_OF_MAINBOARD_CONNECTORS 6
#define NUMBER_OF_RPM_TO_MAINBOARD 6

#define CURVE_ANALOG_POINTS 6
#define CURVE_PWM_POINTS 10
#define CURVE_RPM_POINTS 10

// size of temperatures cache. Size is 2^CACHE_T_SIZE - value 6 means 64 records ~ 6°C
#define CACHE_T_SIZE 6
// size of PWM by temperature or PRM by temperature cache. Size is 2^CACHE_PWM_SIZE - value 2 means 4 records
#define CACHE_PWM_SIZE 2

//by multimeter
//#define ANALOGREFERENCEVOLTAGE 3.3
// resistance of resistor in series with thermistor(value measured by multimeter)
const unsigned short RT_PGM[NUMBER_OF_THERMISTORS] ={9990, 9980, 9970, 9960, 9960, 9980};

//define input pin
#define RPMSENSOR0 18  //INT3/PD3
#define RPMSENSOR1 14  //PCINT10/PJ1
#define RPMSENSOR2 15  //PCINT9/PJ0
#define RPMSENSOR3 2   //INT4/PE4
#define RPMSENSOR4 3   //INT5/PE5
#define RPMSENSOR5 21  //INT0/PD0
#define RPMSENSOR6 20  //INT1/PD1
#define RPMSENSOR7 19  //INT2/PD2
#define RPMSENSOR8 51  //PCINT2/PB2
#define RPMSENSOR9 50  //PCINT3/PB3
#define RPMSENSOR10 53 //PCINT0/PB0
#define RPMSENSOR11 52 //PCINT1/PB1

//define input pin int -> rpm
#define RPMSENSOR_INT0 6
#define RPMSENSOR_INT1 7
#define RPMSENSOR_INT2 4
#define RPMSENSOR_INT3 5
#define RPMSENSOR_INT4 1
#define RPMSENSOR_INT5 0
#define RPMSENSOR_PCINT0 10
#define RPMSENSOR_PCINT1 11
#define RPMSENSOR_PCINT2 8
#define RPMSENSOR_PCINT3 9
#define RPMSENSOR_PCINT9 3
#define RPMSENSOR_PCINT10 2

//PWM output pins
#define PWM0 5    //OC3A
#define PWM1 7    //OC4B
#define PWM2 6    //OC4A
#define PWM3 9    //OC2B
#define PWM4 8    //OC4C
#define PWM5 11   //OC1A
#define PWM6 10   //OC2A
#define PWM7 12   //OC1B
#define PWM8 13   //OC1C
#define PWM9 45   //OC5B
#define PWM10 44  //OC5C
#define PWM11 46  //OC5A

//TACH output pins
#define TACH0 43 //PL6
#define TACH1 41 //PG0
#define TACH2 39 //PG2
#define TACH3 37 //PC0
#define TACH4 35 //PC2
#define TACH5 33 //PC4

#define TACH0_1 PORTL |= _BV(PL6)
#define TACH0_0 PORTL &= ~_BV(PL6)
#define TACH0_SET {TACH0_1;} else {TACH0_0;}

#define TACH1_1 PORTG |= _BV(PG0)
#define TACH1_0 PORTG &= ~_BV(PG0)
#define TACH1_SET {TACH1_1;} else {TACH1_0;}

#define TACH2_1 PORTG |= _BV(PG2)
#define TACH2_0 PORTG &= ~_BV(PG2)
#define TACH2_SET {TACH2_1;} else {TACH2_0;}

#define TACH3_1 PORTC |= _BV(PC0)
#define TACH3_0 PORTC &= ~_BV(PC0)
#define TACH3_SET {TACH3_1;} else {TACH3_0;}

#define TACH4_1 PORTC |= _BV(PC2)
#define TACH4_0 PORTC &= ~_BV(PC2)
#define TACH4_SET {TACH4_1;} else {TACH4_0;}

#define TACH5_1 PORTC |= _BV(PC4)
#define TACH5_0 PORTC &= ~_BV(PC4)
#define TACH5_SET {TACH5_1;} else {TACH5_0;}

void setPinsIO(){
  pinMode(RPMSENSOR0, INPUT);
  pinMode(RPMSENSOR1, INPUT);
  pinMode(RPMSENSOR2, INPUT);
  pinMode(RPMSENSOR3, INPUT);
  pinMode(RPMSENSOR4, INPUT);
  pinMode(RPMSENSOR5, INPUT);
  pinMode(RPMSENSOR6, INPUT);
  pinMode(RPMSENSOR7, INPUT);
  pinMode(RPMSENSOR8, INPUT);
  pinMode(RPMSENSOR9, INPUT);
  pinMode(RPMSENSOR10, INPUT);
  pinMode(RPMSENSOR11, INPUT);

  pinMode(TACH0, OUTPUT);
  pinMode(TACH1, OUTPUT);
  pinMode(TACH2, OUTPUT);
  pinMode(TACH3, OUTPUT);
  pinMode(TACH4, OUTPUT);
  pinMode(TACH5, OUTPUT);

  pinMode(PWM0, OUTPUT);
  pinMode(PWM1, OUTPUT);
  pinMode(PWM2, OUTPUT);
  pinMode(PWM3, OUTPUT);
  pinMode(PWM4, OUTPUT);
  pinMode(PWM5, OUTPUT);
  pinMode(PWM6, OUTPUT);
  pinMode(PWM7, OUTPUT);
  pinMode(PWM8, OUTPUT);
  pinMode(PWM9, OUTPUT);
  pinMode(PWM10, OUTPUT);
  pinMode(PWM11, OUTPUT);
}

#define RT(P) RT_PGM[P]

void init_extint()
{
  EICRA |= (1 << ISC00) | (1 << ISC10) | (1 << ISC20) | (1 << ISC30);
  EICRB |= (1 << ISC40) | (1 << ISC50);
  EIMSK |= (1 << INT0) | (1 << INT1) | (1 << INT2) | (1 << INT3) | (1 << INT4) | (1 << INT5);
}

void init_pcint()
{
  // PB0, PB1, PB2, PB3
  PCMSK0 = (1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2) | (1 << PCINT3);

  // PJ0, PJ1
  PCMSK1 = (1 << PCINT9) | (1 << PCINT10);

  // PORTB, PORTJ
  PCICR = (1 << PCIE0) | (1 << PCIE1); // enable pin change interrupts

}

void disableRpmIRS(){
  EIMSK = 0;                            // disable external interrupt
  PCICR = 0;                            // disable pin change interrupts
}

void enableRpmIRS(){
  EIMSK |= (1 << INT0) | (1 << INT1) | (1 << INT2) | (1 << INT3) | (1 << INT4) | (1 << INT5);
  PCICR = (1 << PCIE0) | (1 << PCIE1) | (1 << PCIE2); // enable pin change interrupts
}

void setTimers(){

  TCCR1B = TCCR1B & B11111000;
  TCCR2B = TCCR2B & B11111000;
  TCCR3B = TCCR3B & B11111000;
  TCCR4B = TCCR4B & B11111000;
  TCCR5B = TCCR5B & B11111000;

  TCNT1 = 0;
  TCNT2 = 0;
  TCNT3 = 0;
  TCNT4 = 0;
  TCNT5 = 0;
  
  //---------------------------------------------- Set PWM frequency for T0 -------------------------------
  // put timer 0 in 8-bit fast hardware pwm mode
  TCCR0A |= (1 << WGM00);
  TCCR0A |= (1 << WGM01);
 
  // timer 0 is in 8-bit fast hardware pwm mode
  //TCCR0B = TCCR0B & B11111000 | B00000001;    // set timer 0 divisor to     1 for PWM frequency of 62500.00 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000010;    // set timer 0 divisor to     8 for PWM frequency of  7812.50 Hz
  TCCR0B = TCCR0B & B11111000 | B00000011;    // set timer 0 divisor to    64 for PWM frequency of   976.56 Hz (The DEFAULT)
  //TCCR0B = TCCR0B & B11111000 | B00000100;    // set timer 0 divisor to   256 for PWM frequency of   244.14 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000101;    // set timer 0 divisor to  1024 for PWM frequency of    61.04 Hz
  // timer 0 is in 8-bit phase correct pwm mode
  //TCCR0B = TCCR0B & B11111000 | B00000001;    // set timer 0 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000010;    // set timer 0 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000011;    // set timer 0 divisor to    64 for PWM frequency of   490.20 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000100;    // set timer 0 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000101;    // set timer 0 divisor to  1024 for PWM frequency of    30.64 Hz
  //---------------------------------------------- Set PWM frequency for T1 ------------------------------------
  TCCR1B = TCCR1B & B11111000 | B00000001;    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000010;    // set timer 1 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000011;    // set timer 1 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  //TCCR1B = TCCR1B & B11111000 | B00000100;    // set timer 1 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000101;    // set timer 1 divisor to  1024 for PWM frequency of    30.64 Hz
  delayMicroseconds(6);
  //---------------------------------------------- Set PWM frequency for T2 ------------------------------------
  TCCR2B = TCCR2B & B11111000 | B00000001;    // set timer 2 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000010;    // set timer 2 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000011;    // set timer 2 divisor to    32 for PWM frequency of   980.39 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000100;    // set timer 2 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  //TCCR2B = TCCR2B & B11111000 | B00000101;    // set timer 2 divisor to   128 for PWM frequency of   245.10 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000110;    // set timer 2 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000111;    // set timer 2 divisor to  1024 for PWM frequency of    30.64 Hz
  delayMicroseconds(6);
  //---------------------------------------------- Set PWM frequency for T3 ------------------------------------
  TCCR3B = TCCR3B & B11111000 | B00000001;    // set timer 3 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR3B = TCCR3B & B11111000 | B00000010;    // set timer 3 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR3B = TCCR3B & B11111000 | B00000011;    // set timer 3 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  //TCCR3B = TCCR3B & B11111000 | B00000100;    // set timer 3 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR3B = TCCR3B & B11111000 | B00000101;    // set timer 3 divisor to  1024 for PWM frequency of    30.64 Hz
  delayMicroseconds(6);
  //---------------------------------------------- Set PWM frequency for T4 ------------------------------------
  TCCR4B = TCCR4B & B11111000 | B00000001;    // set timer 4 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR4B = TCCR4B & B11111000 | B00000010;    // set timer 4 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR4B = TCCR4B & B11111000 | B00000011;    // set timer 4 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  //TCCR4B = TCCR4B & B11111000 | B00000100;    // set timer 4 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR4B = TCCR4B & B11111000 | B00000101;    // set timer 4 divisor to  1024 for PWM frequency of    30.64 Hz
  delayMicroseconds(6);
  //---------------------------------------------- Set PWM frequency for T5 ------------------------------------
  TCCR5B = TCCR5B & B11111000 | B00000001;    // set timer 5 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR5B = TCCR5B & B11111000 | B00000010;    // set timer 5 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR5B = TCCR5B & B11111000 | B00000011;    // set timer 5 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  //TCCR5B = TCCR5B & B11111000 | B00000100;    // set timer 5 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR5B = TCCR5B & B11111000 | B00000101;    // set timer 5 divisor to  1024 for PWM frequency of    30.64 Hz

  // connect timers to output pins
  TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << COM1C1);
  TCCR2A |= (1 << COM2A1) | (1 << COM2B1);
  TCCR3A |= (1 << COM3A1);
  TCCR4A |= (1 << COM4A1) | (1 << COM4B1) | (1 << COM4C1);
  TCCR5A |= (1 << COM5A1) | (1 << COM5B1) | (1 << COM5C1);
}

//compensate non-lienarity of outputs
byte countRealVal(unsigned int val){
  byte realVal = (val * val + 324) / 325;
  if(realVal >= 201){
    return 255;
  } else {
    return realVal;
  }
}

void writePwmValue(byte fanNumber, byte val) {
  switch(fanNumber) {
    case 0:
      OCR3A = 255 - val; // set pwm duty
      break;
    case 1:
      OCR4B = 255 - val; // set pwm duty
      break;
    case 2:
      OCR4A = 255 - val; // set pwm duty
      break;
    case 3:
      OCR2B = 255 - val; // set pwm duty
      break;
    case 4:
      OCR4C = 255 - val; // set pwm duty
      break;
    case 5:
      OCR1A = 255 - val; // set pwm duty
      break;
    case 6:
      OCR2A = 255 - val; // set pwm duty
      break;
    case 7:
      OCR1B = 255 - val; // set pwm duty
      break;
    case 8:
      OCR1C = 255 - val; // set pwm duty
      break;
    case 9:
      OCR5B = 255 - val; // set pwm duty
      break;
    case 10:
//      OCR5C = val; // set pwm duty
      OCR5C = countRealVal(val); // set pwm duty
      break;
    case 11:
//      OCR5A = val; // set pwm duty
      OCR5A = countRealVal(val); // set pwm duty
      break;
  }
}
