void setup() {

//  wdt_disable();
 //serial port configuration
  Serial.begin(115200);
//  Serial.begin(9600);


   pinMode(RPMSENSOR0, INPUT);
   pinMode(RPMSENSOR1, INPUT);
   pinMode(RPMSENSOR2, INPUT);
   pinMode(RPMSENSOR3, INPUT);
   pinMode(RPMSENSOR4, INPUT);
   pinMode(RPMSENSOR5, INPUT);

  //welcome
  Serial.println(F("Starting..."));

  //set reference voltage (external 3.3V)
  analogReferenceAsynchronous(EXTERNAL);
  analogRead(TEMPINPUT0); //must read once before any other calls.
  //adc_init(ADC, 16000000, 10000*2, 3);

//  RT0koeficient = RT0 * (1023 * VOLTAGETHERMISTOR) / ANALOGREFERENCEVOLTAGE;
//  RT1koeficient = RT1 * (1023 * VOLTAGETHERMISTOR) / ANALOGREFERENCEVOLTAGE;

  // VOLTAGETHERMISTOR == ANALOGREFERENCEVOLTAGE
  RT0koeficient = (unsigned long)RT0 * 1023;
  RT1koeficient = (unsigned long)RT1 * 1023;

  Serial.print(F("koeficientT0:"));
  Serial.print(RT0koeficient);
  Serial.print(F(", koeficientT1:"));
  Serial.println(RT1koeficient);

  readTemperaturesInitial();
  if(T0Connected){
    Serial.print(F("T0 connected"));
  } else {
    Serial.print(F("T0 not connected"));
  }
  if(T1Connected){
    Serial.println(F(", T1 connected"));
  } else {
    Serial.println(F(", T1 not connected"));
  }

//  wdt_enable(WDTO_4S);

  loadConfiguration();

  setSerialCommandHandler();
  setTimers();
  init_pcint();
 
  Serial.println(F("System started. Type !help for more informations."));
  delay(10);

//  printPokus();
    measureInterrupts();

//  pid0.SetMode(AUTOMATIC);

  start = micros();

#ifdef TIMING_DEBUG
  timeTotal = micros();
  timeCounting = 1;
#endif  
}

void setSerialCommandHandler(){
  SerialCommandHandler.AddCommand(F("help"), printHelp);
  SerialCommandHandler.AddCommand(F("guiE"), guiEnable);
  SerialCommandHandler.AddCommand(F("guiD"), guiDisable);
  SerialCommandHandler.AddCommand(F("setFan"), setPwmConfiguration);
  SerialCommandHandler.AddCommand(F("s"), printStatus);
  SerialCommandHandler.AddCommand(F("fs"), printFullStatus);
  SerialCommandHandler.AddCommand(F("guistat"), guistat);
  SerialCommandHandler.AddCommand(F("guiUpdate"), guiUpdate);
  SerialCommandHandler.AddCommand(F("load"), loadConfiguration);
  SerialCommandHandler.AddCommand(F("save"), saveConfiguration);
  SerialCommandHandler.AddCommand(F("rpm"), setRPMToMainboard);
  SerialCommandHandler.AddCommand(F("disableFan"), disableFan);
  SerialCommandHandler.AddCommand(F("tempCacheStatus"), tempCacheStatus);
#ifdef TIMING_DEBUG
  SerialCommandHandler.AddCommand(F("timing"), timing);
  SerialCommandHandler.AddCommand(F("mi"), measureInterrupts);
#endif
  SerialCommandHandler.SetDefaultHandler(Cmd_Unknown);
}

void setTimers(){

  //---------------------------------------------- Set PWM frequency for D5 & D6 -------------------------------
  //TCCR0B = TCCR0B & B11111000 | B00000001;    // set timer 0 divisor to     1 for PWM frequency of 62500.00 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000010;    // set timer 0 divisor to     8 for PWM frequency of  7812.50 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000011;    // set timer 0 divisor to    64 for PWM frequency of   976.56 Hz (The DEFAULT)
  //TCCR0B = TCCR0B & B11111000 | B00000100;    // set timer 0 divisor to   256 for PWM frequency of   244.14 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000101;    // set timer 0 divisor to  1024 for PWM frequency of    61.04 Hz
   
   
  //---------------------------------------------- Set PWM frequency for D9 & D10 ------------------------------
   
  //TCCR1B = TCCR1B & B11111000 | B00000001;    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
  TCCR1B = TCCR1B & B11111000 | B00000010;    // set timer 1 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000011;    // set timer 1 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  //TCCR1B = TCCR1B & B11111000 | B00000100;    // set timer 1 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000101;    // set timer 1 divisor to  1024 for PWM frequency of    30.64 Hz
   
  //---------------------------------------------- Set PWM frequency for D3 & D11 ------------------------------
   
  TCCR2B = TCCR2B & B11111000 | B00000001;    // set timer 2 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000010;    // set timer 2 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000011;    // set timer 2 divisor to    32 for PWM frequency of   980.39 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000100;    // set timer 2 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  //TCCR2B = TCCR2B & B11111000 | B00000101;    // set timer 2 divisor to   128 for PWM frequency of   245.10 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000110;    // set timer 2 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000111;    // set timer 2 divisor to  1024 for PWM frequency of    30.64 Hz

  cli();
  // synchronize timers 1 and 2
  GTCCR = (1<<TSM)|(1<<PSRASY)|(1<<PSRSYNC); // stop timers
  TCNT1 = 0;
  TIMSK1 |= B00000001;                       // enable timer1 overflow interrupt
  GTCCR = 0;                                 // start timers
  delayMicroseconds(5);
  GTCCR = (1<<TSM)|(1<<PSRASY)|(1<<PSRSYNC);  // stop timers
  TCNT2 = 255;
  GTCCR = 0;                                  // start timers
  sei();

  Serial.println(F("Timers configuration"));
  Serial.print(F("TCCR0A: "));
  Serial.println(TCCR0A, BIN);
  Serial.print(F("TCCR0B: "));
  Serial.println(TCCR0B, BIN);
  Serial.print(F("TIMSK0: "));
  Serial.println(TIMSK0, BIN);
  Serial.print(F("TCCR1A: "));
  Serial.println(TCCR1A, BIN);
  Serial.print(F("TCCR1B: "));
  Serial.println(TCCR1B, BIN);
  Serial.print(F("TIMSK1: "));
  Serial.println(TIMSK1, BIN);
  Serial.print(F("TCCR2A: "));
  Serial.println(TCCR2A, BIN);
  Serial.print(F("TCCR2B: "));
  Serial.println(TCCR2B, BIN);
  Serial.print(F("TIMSK2: "));
  Serial.println(TIMSK2, BIN);

}

void init_pcint()
{
    // PB0, PB4
    PCMSK0 = (1 << PCINT0) | (1 << PCINT4);

    // PD2, PD4, PD7
    PCMSK2 = (1 << PCINT18) | (1 << PCINT20) | (1 << PCINT23);

    // PORTB, PORTD
    PCICR = (1 << PCIE0) | (1 << PCIE2);
}

void readTemperaturesInitial(){
    sensorValue6Averaged = analogRead(A6);
    if(sensorValue6Averaged > 10){
      T0Connected = true;
      thermistorResistance0 = RT0koeficient / sensorValue6Averaged - RT0;
    } else {
      T0Connected = false;
    }
    if(T0Connected == true){
      T0int = countTemperature(thermistorResistance0);
    }

    sensorValue7Averaged = analogRead(A7);
    if(sensorValue7Averaged > 10){
      T1Connected = true;
      thermistorResistance1 = RT1koeficient / sensorValue7Averaged - RT1;
    } else {
      T1Connected = false;
    }
    if(T1Connected == true){
      T1int = countTemperature(thermistorResistance1);
    }
}

void printTempProfile(){
  for(int i = 0; i <= 1024; i++){
      unsigned long thermistorResistance = RT0koeficient / i - RT0;
      unsigned int t = countTemperature(thermistorResistance);
      short tShort = (short)t;
      Serial.print(i);
      Serial.print(" - ");
      Serial.println(tShort);
  }
}

void measureInterrupts(){
  delay(20);
  TIMSK1 &= B11111110;  // disable timer1 overflow interrupt
  PCICR = 0;  // disable pin change interrupts
  start = micros();
  unsigned int a = doSomeMath(100);
  now = micros();
  unsigned long m1 = now - start;

  TIMSK1 |= B00000001;  // enable timer1 overflow interrupt
  PCICR = (1 << PCIE0) | (1 << PCIE2);  // enable pin change interrupts
  delay(10);
  start = micros();
  unsigned int b = doSomeMath(200);
  now = micros();
  unsigned long m2 = now - start;
  float percentage = (1 - (float)m1 / (float)m2 ) * 100;
  
  unsigned int c = a + b;
  Serial.print(F("c="));
  Serial.println(c);
  Serial.print(F("m1:"));
  Serial.print(m1);
  Serial.println(F("us"));
  Serial.print(F("m2:"));
  Serial.print(m2);
  Serial.println(F("us"));
  Serial.print(F("Interrupts occupy "));
  Serial.print(percentage, 1);
  Serial.println(F("% of processor time"));
}

unsigned int doSomeMath(unsigned int x){
  unsigned int result = 0;  
  for(unsigned int i = x; i <= (x + 2000); i++){
    result = result + countTemperature(i);
  }
  return result;
}

