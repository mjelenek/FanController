void serialWriteShort(unsigned short i){
  Serial.write(lowByte(i));
  Serial.write(highByte(i));
}

void serialWriteLong(unsigned long l){
  Serial.write(lowByte(l));
  Serial.write(lowByte(l >> 8));
  Serial.write(lowByte(l >> 16));
  Serial.write(lowByte(l >> 24));
}

void sendConfiguration(){
  Serial.print(F("!"));
  Serial.write(NUMBER_OF_RPM_TO_MAINBOARD + NUMBER_OF_THERMISTORS * 5 + 16);
  Serial.print(F("conf"));
  Serial.write((byte)HWversion);
  Serial.write(NUMBER_OF_FANS);
  Serial.write(NUMBER_OF_THERMISTORS);
  Serial.write(NUMBER_OF_MAINBOARD_CONNECTORS);
  Serial.write(NUMBER_OF_RPM_TO_MAINBOARD);
  Serial.write(CURVE_ANALOG_POINTS);
  Serial.write(CURVE_PWM_POINTS);
  Serial.write(CURVE_RPM_POINTS);
  for(byte i = 0; i < NUMBER_OF_RPM_TO_MAINBOARD; i++){
    Serial.write(rmpToMainboard(i));
  }
  Serial.write(hysteresis);
  Serial.write(lowByte(microsecondPerSecond));
  Serial.write(highByte(microsecondPerSecond));
  for(byte i = 0; i < NUMBER_OF_THERMISTORS; i++){
    thermistors(i).sendDefinition();
  }
  Serial.write(profile);
  Serial.print(F("#"));
}

void guistat(CommandParameter &parameters){
  byte confToSend[NUMBER_OF_FANS];
  byte numberOfBytesToSend = 4;
  byte numberOfConfigurationsToSend = 0;
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    byte confNumber = parameters.NextParameterAsInteger( 255 );
    if(confNumber >= NUMBER_OF_FANS){
      break;
    }
    confToSend[i] = confNumber;
    numberOfConfigurationsToSend++;
    numberOfBytesToSend++;
    numberOfBytesToSend += sizeof(PWMConfiguration) + 1;
  }

  Serial.print(F("!"));
  Serial.write(numberOfBytesToSend);
  Serial.print(F("gui"));
  Serial.write(numberOfConfigurationsToSend);
  for(byte i = 0; i < numberOfConfigurationsToSend; i++){
    Serial.write(confToSend[i]);
    ConfigurationPWM(confToSend[i]).guiStat();
  }
  Serial.print(F("#"));
}

void guiEnable(){
  gui = 1;  
}

void guiDisable(){
  gui = 0;  
}

#ifdef TIMING_DEBUG
  void sendTime(){
    unsigned long us = micros();
    unsigned long ms = millis();
    unsigned long s = seconds();
    Serial.print(us);    
    Serial.println(F("us"));    
    Serial.print(ms);    
    Serial.println(F("ms"));    
    Serial.print(s);    
    Serial.println(F("s"));    
    s = s / 60;
    Serial.print(s);    
    Serial.println(F("min"));    
    s = s / 60;
    Serial.print(s);    
    Serial.println(F("hod"));    
  }
#endif

void guiUpdate(){
  Serial.print(F("!"));
  Serial.write(4 + 3 + NUMBER_OF_MAINBOARD_CONNECTORS * 2 + NUMBER_OF_THERMISTORS * 4 + NUMBER_OF_FANS * 2);
  Serial.print(F("guiU"));

  Serial.write(NUMBER_OF_FANS);
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    serialWriteShort(rpm[i]);
  }
  Serial.write(NUMBER_OF_THERMISTORS);
  for(byte i = 0; i < NUMBER_OF_THERMISTORS; i++){
    if(fakeTemp[i] > 0){
      serialWriteShort(Tint[i] + 10000);
    } else {
      serialWriteShort(Tint[i]);
    }
    serialWriteShort(TWithHysteresisInt[i]);
  }
  Serial.write(NUMBER_OF_MAINBOARD_CONNECTORS);
  for(byte i = 0; i < NUMBER_OF_MAINBOARD_CONNECTORS; i++){
    serialWriteShort(powerInADCAveraged[i]);
  }
  Serial.print(F("#"));
}

void pidUpdate(byte fanNumber, PWMConfiguration &conf){
#ifdef PID_UPDATE_WHEN_RPM_COUNT
  if((((fanNumber << 1) + i) & B00111111) == 0){
#else
  if((((fanNumber << 1) + i) & B00001111) == 0){
#endif
    pidUpdateDirect(fanNumber, conf);
  }
}

void pidUpdateDirect(byte fanNumber, PWMConfiguration &conf){
  if(updatesRTToSend[fanNumber] > 0){
    unsigned short expectedRpm = rpm[fanNumber];
    if(pwmDisabled[fanNumber] == 0){
      if(conf.getPwmDrive() == 3){
        expectedRpm = conf.constRpm;
      }
      if(conf.getPwmDrive() == 4){
        expectedRpm = setpointPid[fanNumber];
      }
    }
    Serial.print(F("!"));
    Serial.write(9);
    Serial.print(F("pU"));
    Serial.write(fanNumber);
    Serial.write(updatesRTToSend[fanNumber]);
    serialWriteShort(expectedRpm);
    serialWriteShort((unsigned int) (rpm[fanNumber] + 0.5));
    Serial.write(pwm[fanNumber]);
    Serial.print(F("#"));
    updatesRTToSend[fanNumber]--;
  }
}

unsigned short roundRPM(double rpm){
  unsigned short rpmRounded = (rpm + 5) / 10;
  return rpmRounded * 10;
}

void setFanConfiguration(CommandParameter &parameters){
  if(eeprom_busy) return;   //update not allowed during save configuration to EEPROM
  
  byte pwmChannel = parameters.NextParameterAsInteger();
  byte pwmDrive = parameters.NextParameterAsInteger();
  byte powerInNumber = parameters.NextParameterAsInteger();
  byte constPwm = parameters.NextParameterAsInteger();
  unsigned short constRPM = parameters.NextParameterAsInteger();
  byte tSelect =  parameters.NextParameterAsInteger();
  byte kp = parameters.NextParameterAsInteger();
  byte ki = parameters.NextParameterAsInteger();
  byte kd = parameters.NextParameterAsInteger();
  byte minPidPwm = parameters.NextParameterAsInteger();
  if(pwmChannel >= 0 && pwmChannel < NUMBER_OF_FANS){
    ConfigurationPWM(pwmChannel).set(pwmDrive, powerInNumber, constPwm, tSelect, constRPM, kp, ki, kd, minPidPwm);
    pid[pwmChannel].SetTunings((double) kp / 200, (double) ki / 200, (double) kd / 200);
    pid[pwmChannel].SetOutputLimits(ConfigurationPWM(pwmChannel).minPidPwm, 255);
    pwmDisabled[pwmChannel] = 0;
    switch (pwmDrive) {
    case 0:
    case 1:
    case 2:
      pid[pwmChannel].SetMode(MANUAL);
      break;
    case 3:
    case 4:
      pid[pwmChannel].SetMode(AUTOMATIC);
    }
#ifdef USE_PWM_CACHE
    cacheFan[pwmChannel].clear();
#endif    
  }
}

void setPowerInCurve(CommandParameter &parameters){
  if(eeprom_busy) return;   //update not allowed during save configuration to EEPROM
  
  byte pwmChannel = parameters.NextParameterAsInteger();
  unsigned short powerIn[CURVE_ANALOG_POINTS];
  byte pwm[CURVE_ANALOG_POINTS];
  for(byte i = 0; i < CURVE_ANALOG_POINTS; i++){
    powerIn[i] = parameters.NextParameterAsInteger( 0 );
    pwm[i] = parameters.NextParameterAsInteger( 0 );
  }
  if(pwmChannel >= 0 && pwmChannel < NUMBER_OF_FANS){
    ConfigurationPWM(pwmChannel).setPowerInCurve(CURVE_ANALOG_POINTS, powerIn, pwm);
#ifdef USE_PWM_CACHE
    cacheFan[pwmChannel].clear();
#endif    
  }
}

void setPwmCurve(CommandParameter &parameters){
  if(eeprom_busy) return;   //update not allowed during save configuration to EEPROM
  
  byte pwmChannel = parameters.NextParameterAsInteger();
  byte t[CURVE_PWM_POINTS];
  byte pwm[CURVE_PWM_POINTS];
  for(byte i = 0; i < CURVE_PWM_POINTS; i++){
    t[i] = parameters.NextParameterAsInteger( 0 );
    pwm[i] = parameters.NextParameterAsInteger( 0 );
  }
  if(pwmChannel >= 0 && pwmChannel < NUMBER_OF_FANS){
    ConfigurationPWM(pwmChannel).setPWMCurve(CURVE_PWM_POINTS, t, pwm);
#ifdef USE_PWM_CACHE
    cacheFan[pwmChannel].clear();
#endif    
  }
}

void setRpmCurve(CommandParameter &parameters){
  if(eeprom_busy) return;   //update not allowed during save configuration to EEPROM
  
  byte pwmChannel = parameters.NextParameterAsInteger();
  byte t[CURVE_RPM_POINTS];
  unsigned short rpm[CURVE_RPM_POINTS];
  for(byte i = 0; i < CURVE_RPM_POINTS; i++){
    t[i] = parameters.NextParameterAsInteger( 0 );
    rpm[i] = parameters.NextParameterAsInteger( 0 );
  }
  if(pwmChannel >= 0 && pwmChannel < NUMBER_OF_FANS){
    ConfigurationPWM(pwmChannel).setRPMCurve(CURVE_RPM_POINTS, t, rpm);
#ifdef USE_PWM_CACHE
    cacheFan[pwmChannel].clear();
#endif    
  }
}

void disableFan(CommandParameter &parameters){
  while(1){
    byte pwmChannel = parameters.NextParameterAsInteger( 255 );
    byte delayParam = parameters.NextParameterAsInteger( 255 );

    if(pwmChannel == 255 || delayParam == 255)
      return;

    if(pwmChannel >= 0 && pwmChannel < NUMBER_OF_FANS){
      pwmDisabled[pwmChannel] = delayParam;
    }
  }
}

void sendPidUpdates(CommandParameter &parameters){
  while(1){
    byte pwmChannel = parameters.NextParameterAsInteger( 255 );
    byte numberOfUpdates = parameters.NextParameterAsInteger( 255 );

    if(pwmChannel == 255 || numberOfUpdates == 255)
      return;

    if(pwmChannel >= 0 && pwmChannel < NUMBER_OF_FANS){
      updatesRTToSend[pwmChannel] = numberOfUpdates;
    }
  }
}

void setConfiguration(CommandParameter &parameters){
  if(eeprom_busy) return;   //update not allowed during save configuration to EEPROM

  profile = parameters.NextParameterAsInteger( 0 );

  byte h = parameters.NextParameterAsInteger( 10 );

  short ups = parameters.NextParameterAsInteger( 0 );

  if(h >= 0 && h <= 100){
    hysteresis = h;
  }

  if(ups >= -10000 && ups <= 10000){
    microsecondPerSecond = ups;
  }

  byte i = 0;
  while(1){
    byte select = parameters.NextParameterAsInteger( 255 );
    if(select >= 0 && select < NUMBER_OF_FANS && i < NUMBER_OF_RPM_TO_MAINBOARD){
      rmpToMainboard(i++) = select;
    } else {
      return;
    }
  }
}

#if HWversion == 2
void setICRn(CommandParameter &parameters)
{
  //default PWM frequency of 25000Hz (16000000 / (ICRn*2-2))
  unsigned int parameter = parameters.NextParameterAsInteger( 321 );
  // 800Hz - 80kHz
  if(parameter >= 101 && parameter <= 10001){
    setRatio((word)parameter);
    for(byte i = 0; i < NUMBER_OF_FANS; i++){
      #ifdef USE_PWM_CACHE
      cacheFan[i].clear();
      #endif
      writePwmValue(i, pwm[i]);
    }
  }
}
#endif

void wd(CommandParameter &parameters){
  Serial.print(F("!"));
  Serial.write(22);
  Serial.print(F("WatchDogReset "));
  if (watchdogCounter > 0){
    Serial.print(F("YES "));
  } else {
    Serial.print(F("NO  "));
  }
  serialWriteLong(seconds());
  Serial.print(F("#"));
}

void Cmd_Unknown()
{
  if(gui){
    Serial.print(F("!"));
    Serial.write(15);
    Serial.print(F("Unknown command#"));
  } else {
    Serial.println(F("Unknown command"));
  }
}
