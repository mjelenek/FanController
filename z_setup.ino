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

  RT0koeficient = RT0 * (1023 * VOLTAGETHERMISTOR) / ANALOGREFERENCEVOLTAGE;
  RT1koeficient = RT1 * (1023 * VOLTAGETHERMISTOR) / ANALOGREFERENCEVOLTAGE;

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

  Serial.println(F("load PWM Configuration"));
  loadConfiguration();
  Serial.println(F("PWM Configuration loaded"));

  setSerialCommandHandler();
  setTimers();
 
  Serial.println(F("Started"));

//  printTempProfile();
//  printPokus();

  pid0.SetMode(AUTOMATIC);

  //start main loop timer
  start = micros();
  
}

void setSerialCommandHandler(){
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
   
  TCCR1B = TCCR1B & B11111000 | B00000001;    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000010;    // set timer 1 divisor to     8 for PWM frequency of  3921.16 Hz
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

  TCCR2A |= B11000000;  //inverted output 2A
  TIMSK2 |= B00000001;  // enable timer2 overflow interrupt
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

void printPokus(){
  byte toSend[2];
  toSend[0] = 58;
  toSend[1] = 58;
  for(int i = 0; i<50; i++){
  delay(1);
  start = micros();
    for(int j = 0; j<i; j++){
//      Serial.print(F("ss"));
      Serial.write(toSend, 2);
    }
  now = micros();
  zpozdeni = now - start;
  Serial.println("");
  Serial.print(" i=");
  Serial.print(i*2);
  Serial.print(" zpozdeni=");
  Serial.println(zpozdeni);
  }
}


