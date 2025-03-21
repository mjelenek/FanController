// Arduino Meduino 2560
#define NUMBER_OF_THERMISTORS 6
#define NUMBER_OF_FANS 12
#define NUMBER_OF_MAINBOARD_CONNECTORS 6
#define NUMBER_OF_RPM_TO_MAINBOARD 6

#define CURVE_ANALOG_POINTS 6
#define CURVE_PWM_POINTS 10
#define CURVE_RPM_POINTS 10

// size of temperatures cache. Size is 2^CACHE_T_SIZE - value 5 means 32 records ~ 3°C
#define CACHE_T_SIZE 5
// size of PWM by temperature or RPM by temperature cache. Size is 2^CACHE_PWM_SIZE - value 2 means 4 records
#define CACHE_PWM_SIZE 2

//define input pin
#define RPMSENSOR0 18  //INT5/PE5
#define RPMSENSOR1 14  //INT4/PE4
#define RPMSENSOR2 15  //PCINT10/PJ1
#define RPMSENSOR3 2   //PCINT9/PJ0
#define RPMSENSOR4 3   //INT2/PD2
#define RPMSENSOR5 21  //INT3/PD3
#define RPMSENSOR6 20  //INT0/PD0
#define RPMSENSOR7 19  //INT1/PD1
#define RPMSENSOR8 51  //PCINT2/PB2
#define RPMSENSOR9 50  //PCINT3/PB3
#define RPMSENSOR10 53 //PCINT0/PB0
#define RPMSENSOR11 52 //PCINT1/PB1
//dalsi pouzitelne PCINT16/PK0, PCINT17/PK1, pri uvolneni OC2A PCINT4/PB4

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

#define TACH1_1 PORTG |= _BV(PG0)
#define TACH1_0 PORTG &= ~_BV(PG0)

#define TACH2_1 PORTG |= _BV(PG2)
#define TACH2_0 PORTG &= ~_BV(PG2)

#define TACH3_1 PORTC |= _BV(PC0)
#define TACH3_0 PORTC &= ~_BV(PC0)

#define TACH4_1 PORTC |= _BV(PC2)
#define TACH4_0 PORTC &= ~_BV(PC2)

#define TACH5_1 PORTC |= _BV(PC4)
#define TACH5_0 PORTC &= ~_BV(PC4)

//PWM frequency of 25000Hz (16000000 / (ICRn*2))
word ICRn = 320;
float RATIO = ((float)ICRn)/255;

void setRatio(word parameter){
  ICRn = parameter;
  RATIO = ((float)ICRn)/255;
  ICR1 = ICRn;
  ICR3 = ICRn;
  ICR4 = ICRn;
  ICR5 = ICRn;
}

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

void writePwmValue(byte fanNumber, byte val);

void init_extint()
{
  EICRA = (1 << ISC00) | (1 << ISC10) | (1 << ISC20) | (1 << ISC30);
  EICRB = (1 << ISC40) | (1 << ISC50);
  EIMSK = (1 << INT0) | (1 << INT1) | (1 << INT2) | (1 << INT3) | (1 << INT4) | (1 << INT5);
}

void init_pcint()
{
  // PB0, PB1, PB2, PB3
  PCMSK0 = (1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2) | (1 << PCINT3);

  // PJ0, PJ1
  PCMSK1 = (1 << PCINT9) | (1 << PCINT10);

  // PORTB, PORTJ
  PCICR = (1 << PCIE0) | (1 << PCIE1);  // enable pin change interrupts

}

void disableRpmIRS(){
  EIMSK = 0;                            // disable external interrupt
  PCICR = 0;                            // disable pin change interrupts
}

void enableRpmIRS(){
  EIMSK = (1 << INT0) | (1 << INT1) | (1 << INT2) | (1 << INT3) | (1 << INT4) | (1 << INT5);
  PCICR = (1 << PCIE0) | (1 << PCIE1);  // enable pin change interrupts
}

void disableRpmIRS(byte fanNumber){
switch (fanNumber) {
    case 0:
      EIMSK &= ~(1 << INT5);
      break;
    case 1:
      EIMSK &= ~(1 << INT4);
      break;
    case 2:
      PCICR &= ~(1 << PCIE1);
      break;
    case 3:
      PCICR &= ~(1 << PCIE1);
      break;
    case 4:
      EIMSK &= ~(1 << INT3);
      break;
    case 5:
      EIMSK &= ~(1 << INT4);
      break;
    case 6:
      EIMSK &= ~(1 << INT0);
      break;
    case 7:
      EIMSK &= ~(1 << INT1);
      break;
    case 8:
      PCICR &= ~(1 << PCIE0);
      break;
    case 9:
      PCICR &= ~(1 << PCIE0);
      break;
    case 10:
      PCICR &= ~(1 << PCIE0);
      break;
    case 11:
      PCICR &= ~(1 << PCIE0);
      break;
    default:
    ;
  }
}

void enableRpmIRS(byte fanNumber){
switch (fanNumber) {
    case 0:
      EIMSK |= (1 << INT5);
      break;
    case 1:
      EIMSK |= (1 << INT4);
      break;
    case 2:
      PCICR |= (1 << PCIE1);
      break;
    case 3:
      PCICR |= (1 << PCIE1);
      break;
    case 4:
      EIMSK |= (1 << INT3);
      break;
    case 5:
      EIMSK |= (1 << INT4);
      break;
    case 6:
      EIMSK |= (1 << INT0);
      break;
    case 7:
      EIMSK |= (1 << INT1);
      break;
    case 8:
      PCICR |= (1 << PCIE0);
      break;
    case 9:
      PCICR |= (1 << PCIE0);
      break;
    case 10:
      PCICR |= (1 << PCIE0);
      break;
    case 11:
      PCICR |= (1 << PCIE0);
      break;
    default:
    ;
  }
}

void setTimers(){

  TCCR0A = TCCR1A = TCCR2A = TCCR3A = TCCR4A = TCCR5A = 0;
  TCCR0B = TCCR1B = TCCR2B = TCCR3B = TCCR4B = TCCR5B = 0;
  TCNT1 = TCNT2 = TCNT3 = TCNT4 = TCNT5 = 0;
  
  //---------------------------------------------- Set PWM frequency for T0 -------------------------------
  TCCR0A = (1 << WGM01);                        // put timer 0 in CTC mode

  OCR0A = 249;
  TCCR0B = B00000011;                           // set timer 0 divisor to    64 for PWM frequency of 1000 Hz (16000000 / (64 * (OCR0A+1)))
  TIMSK0 = (1 << OCIE0A);                       // enable TIMER0_COMPA interrupt
  // put timer 0 in 8-bit fast hardware pwm mode
  //TCCR0A = (1 << WGM00) | (1 << WGM01);
  // timer 0 is in 8-bit fast hardware pwm mode
  //TCCR0B = TCCR0B & B11111000 | B00000001;    // set timer 0 divisor to     1 for PWM frequency of 62500.00 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000010;    // set timer 0 divisor to     8 for PWM frequency of  7812.50 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000011;    // set timer 0 divisor to    64 for PWM frequency of   976.56 Hz (The DEFAULT)
  //TCCR0B = TCCR0B & B11111000 | B00000100;    // set timer 0 divisor to   256 for PWM frequency of   244.14 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000101;    // set timer 0 divisor to  1024 for PWM frequency of    61.04 Hz
  // timer 0 is in 8-bit phase correct pwm mode
  //TCCR0B = TCCR0B & B11111000 | B00000001;    // set timer 0 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000010;    // set timer 0 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000011;    // set timer 0 divisor to    64 for PWM frequency of   490.20 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000100;    // set timer 0 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000101;    // set timer 0 divisor to  1024 for PWM frequency of    30.64 Hz
  //---------------------------------------------- Set PWM frequency for T1 ------------------------------------
  ICR1 = ICRn;
  TCCR1B = B00010001;    // set timer 1 divisor to 1 for PWM, Phase and Frequency Correct, frequency 25000Hz (16000000 / (ICRn*2))
  delayMicroseconds(8);
  //---------------------------------------------- Set PWM frequency for T2 ------------------------------------
  TCCR2A = (1 << WGM20);                        // put timer 2 in 8-bit phase correct pwm mode
  TCCR2B = TCCR2B & B11111000 | B00000001;      // set timer 2 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000010;    // set timer 2 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000011;    // set timer 2 divisor to    32 for PWM frequency of   980.39 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000100;    // set timer 2 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  //TCCR2B = TCCR2B & B11111000 | B00000101;    // set timer 2 divisor to   128 for PWM frequency of   245.10 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000110;    // set timer 2 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000111;    // set timer 2 divisor to  1024 for PWM frequency of    30.64 Hz
  //---------------------------------------------- Set PWM frequency for T3 ------------------------------------
  ICR3 = ICRn;
  TCCR3B = B00010001;    // set timer 3 divisor to 1 for PWM, Phase and Frequency Correct, frequency 25000Hz (16000000 / (ICRn*2))
  delayMicroseconds(8);
  //---------------------------------------------- Set PWM frequency for T4 ------------------------------------
  ICR4 = ICRn;
  TCCR4B = B00010001;    // set timer 4 divisor to 1 for PWM, Phase and Frequency Correct, frequency 25000Hz (16000000 / (ICRn*2))
  delayMicroseconds(8);
  //---------------------------------------------- Set PWM frequency for T5 ------------------------------------
  ICR5 = ICRn;
  TCCR5B = B00010001;    // set timer 5 divisor to 1 for PWM, Phase and Frequency Correct, frequency 25000Hz (16000000 / (ICRn*2))

  // set pwm outputs to zero
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    writePwmValue(i, 0);
  }

  // connect timers to output pins
  TCCR1A = (1 << COM1A0) | (1 << COM1A1) | (1 << COM1B0) | (1 << COM1B1) | (1 << COM1C0) | (1 << COM1C1);
  TCCR2A |= (1 << COM2A0) | (1 << COM2A1) | (1 << COM2B0) | (1 << COM2B1);
  TCCR3A = (1 << COM3A0) | (1 << COM3A1);
  TCCR4A = (1 << COM4A0) | (1 << COM4A1) | (1 << COM4B0) | (1 << COM4B1) | (1 << COM4C0) | (1 << COM4C1);
  TCCR5A = (1 << COM5A0) | (1 << COM5A1) | (1 << COM5B0) | (1 << COM5B1) | (1 << COM5C0)| (1 << COM5C1);
}
