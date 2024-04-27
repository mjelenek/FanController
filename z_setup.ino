void setup() {

if(MCUSR & (1<<WDRF)){
   // a watchdog reset occurred
  watchdogCounter++;
  MCUSR = 0;
}

//  wdt_disable();
 //serial port configuration
  Serial.begin(115200);
//  Serial.begin(19200);

//welcome
  Serial.println(F("Starting..."));

#ifdef FREE_MEMORY_DEBUG
  fillFreeMemoryByZeroes();
#endif

  setPinsIO();

  watchdogSetup();
  
  setTimers();

  init_adc();

  init_thermistors();
  
  loadConfiguration();

  init_extint();
  
  init_pcint();

  setSerialCommandHandler();

  init_pid();

//  printTempProfile();

  Serial.println(F("System started. Type !help for more informations."));
  delay(1);

  start = micros();
}

void init_thermistors(){
  for(byte i = 0; i <= NUMBER_OF_THERMISTORS + NUMBER_OF_MAINBOARD_CONNECTORS; i++){
    delay(1);  //wait for read values from ADC;
    startNextADC();
  }
  for(byte i = 0; i < NUMBER_OF_THERMISTORS; i++){
    countT(i);  
    Serial.print(F("T"));
    Serial.print(i);
    if(! TConnected[i]){
      Serial.print(F(" not"));
    }
    Serial.println(F(" connected"));
  }
}

void init_pid(){
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    pid[i].SetOutputLimits(ConfigurationPWM(i).minPidPwm, 255);
    pid[i].SetSampleTime(75000);                     // will be computed every 80ms (see isTimeToComputePID() function)

    switch (ConfigurationPWM(i).getPwmDrive()) {
    case 0:
    case 1:
    case 2:
      pid[i].SetMode(MANUAL);
      break;
    case 3:
    case 4:
      pid[i].SetMode(AUTOMATIC);
    }
    pid[i].Compute(false);    
  }
}

void watchdogSetup() {
  cli();        // disable all interrupts
  wdt_reset();  // reset the WDT timer
  /*
    WDTCSR configuration:
    WDIE = 1: Interrupt Enable
    WDE = 1 :Reset Enable
    Timeout
    WDP3 WDP2 WDP1 WDP0 Timeout(ms)
     0    0    0    0    16
     0    0    0    1    32
     0    0    1    0    64
     0    0    1    1   128
     0    1    0    0   256
     0    1    0    1   512
     0    1    1    0  1024
     0    1    1    1  2048
     1    0    0    0  4096
     1    0    0    1  8192
  */
  // Enter Watchdog Configuration mode:
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  // Set Watchdog settings:
  WDTCSR = (1<<WDE) | (1<<WDP3) | (0<<WDP2) | (0<<WDP1) | (0<<WDP0);
  sei();
}

void printTempProfile(){
  for(int i = 0; i <= 1024; i++){
      unsigned long RT = RT(0);
      unsigned long RTkoeficient = (RT << 10) - RT;
      unsigned int t = countTemperature(RTkoeficient / i - RT, thermistors(0));
      short tShort = (short)t;
      Serial.print(i);
      Serial.print(" - ");
      Serial.println(tShort);
  }
}

void measureInterrupts(){
  delay(20);
  disableRpmIRS();                      // disable pin change interrupts
  ADCSRA &= ~(1 << ADIE);               // disable ADC conversion complete interrupt
  unsigned long start = micros();
  unsigned int a = doSomeMath(100);
  now = micros();
  unsigned long m1 = now - start;

  enableRpmIRS();                       // enable pin change interrupts
  ADCSRA |= (1 << ADIE);                // enable ADC conversion complete interrupt
  delay(10);
  start = micros();
  unsigned int b = doSomeMath(101);
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
  Serial.print(F("Interrupts(except TIMER0_OVF) occupy "));
  Serial.print(percentage, 2);
  Serial.println(F("% of processor time"));
}

unsigned int doSomeMath(unsigned int x){
  unsigned int result = 0;  
  for(unsigned int i = x; i <= (x + 2000); i++){
    result = result + countTemperature(i, thermistors(0));
  }
  return result;
}
