unsigned int binaryToBCD(unsigned int input){    //enter any number from 0 to 9999
  if(input > 99999999){
      return 0x9999;
  }
    unsigned int output = 0;
    signed char a;
    for(a = 13; a >= 0; a--){
        if((output & 0xF) >= 5)
            output += 3;
        if(((output & 0xF0) >> 4) >= 5)
            output += (3 << 4);
        if(((output & 0xF00) >> 8) >= 5)
            output += (3 << 8);
        output = (output << 1) | ((input >> a) & 1);
    }
    return output;
}

unsigned long binaryToBCDLong(unsigned long input){    //enter any number from 0 to 99999999
  if(input > 99999999){
      return 0x99999999;
  }
  if(input <= 9999){
    return binaryToBCD((unsigned int)input);
  } else {
    unsigned long output = 0;
    signed char a;
    for(a = 25; a >= 0; a--){
        if((output & 0xF) >= 5)
            output += 3;
        if(((output >> 4) & 0xF) >= 5)
            output += (3 << 4);
        if(((output >> 8) & 0xF) >= 5)
            output += (3 << 8);
        if(((output >> 12) & 0xF) >= 5)
            output += (3 << 12);
        if(((output >> 16) & 0xF) >= 5)
            output += (3 << 16);
        if(((output >> 20) & 0xF) >= 5)
            output += (3 << 20);
        if(((output >> 24) & 0xF) >= 5)
            output += (3 << 24);
        if(((output >> 28) & 0xF) >= 5)
            output += (3 << 28);
        output = (output << 1) | ((input >> a) & 1);
    }
    return output;
  }
}

void serialWriteInt(unsigned int i){
  Serial.write(lowByte(i));
  Serial.write(highByte(i));
}

void serialWriteLong(unsigned long l){
  Serial.write(lowByte(l));
  Serial.write(lowByte(l >> 8));
  Serial.write(lowByte(l >> 16));
  Serial.write(lowByte(l >> 32));
}

void guistat1(){
  Serial.print(F("!!"));
  Serial.write(118);
  Serial.print(F("gui1"));
  ConfigurationPWM[0] -> guiStat();
  ConfigurationPWM[1] -> guiStat();
  ConfigurationPWM[2] -> guiStat();
  ConfigurationPWM[3] -> guiStat();
  ConfigurationPWM[4] -> guiStat();
  ConfigurationPWM[5] -> guiStat();
}

void guiEnable(){
  gui = 1;  
}

void guiDisable(){
  gui = 0;  
}

#ifdef TIMING_DEBUG
  void timing(){
    timeCountingStartFlag = 1;
  }

  void sendMillis(){
    unsigned long x1 = millis();
    unsigned long x2 = millis();
    unsigned long x3 = millis();
    unsigned long x4 = millis();
    unsigned long x5 = millis();
    Serial.println("");    
    Serial.println(x1);    
    Serial.println(x2);    
    Serial.println(x3);    
    Serial.println(x4);    
    Serial.println(x5);    
  }
  
  void sendMicros(){
    unsigned long x1 = micros();
    unsigned long x2 = micros();
    unsigned long x3 = micros();
    unsigned long x4 = micros();
    unsigned long x5 = micros();
    Serial.println("");    
    Serial.println(x1);    
    Serial.println(x2);    
    Serial.println(x3);    
    Serial.println(x4);    
    Serial.println(x5);    
  }
  
#endif

void guiUpdate(){
  Serial.print(F("!!"));
  Serial.write(29);
  Serial.print(F("guiUpdate"));

  serialWriteInt(roundRPM(rpm[0]));
  serialWriteInt(roundRPM(rpm[1]));
  serialWriteInt(roundRPM(rpm[2]));
  serialWriteInt(roundRPM(rpm[3]));
  serialWriteInt(roundRPM(rpm[4]));
  serialWriteInt(roundRPM(rpm[5]));
  serialWriteInt(T0int);
  serialWriteInt(T1int);
  serialWriteInt(T0WithHysteresisInt);
  serialWriteInt(T1WithHysteresisInt);
//  serialWriteInt(sensorValue6Averaged);
//  serialWriteInt(sensorValue7Averaged);
}

byte pidUpdate(byte fanNumber, PWMConfiguration &conf){
  if(updatesRTToSend[fanNumber] > 0 && (((fanNumber << 2) + i) & B00011111) == 0){
    unsigned short expectedRpm = rpm[fanNumber];
    if(pwmDisabled[fanNumber] == 0){
      if(conf.pwmDrive == 3){
        expectedRpm = conf.constRpm;
      }
      if(conf.pwmDrive == 4){
        expectedRpm = setpointPid[fanNumber];
      }
    }
    Serial.print(F("!!"));
    Serial.write(8);
    Serial.print(F("pU"));
    Serial.write(fanNumber);
    serialWriteInt(expectedRpm);
    serialWriteInt((unsigned int) (rpm[fanNumber] + 0.5));
    Serial.write(pwm[fanNumber]);
    updatesRTToSend[fanNumber]--;
  }
}

byte pidUpdateDirect(byte fanNumber, PWMConfiguration &conf){
  if(updatesRTToSend[fanNumber] > 0){
    unsigned short expectedRpm = rpm[fanNumber];
    if(pwmDisabled[fanNumber] == 0){
      if(conf.pwmDrive == 3){
        expectedRpm = conf.constRpm;
      }
      if(conf.pwmDrive == 4){
        expectedRpm = setpointPid[fanNumber];
      }
    }
    Serial.print(F("!!"));
    Serial.write(8);
    Serial.print(F("pU"));
    Serial.write(fanNumber);
    serialWriteInt(expectedRpm);
    serialWriteInt((unsigned int) (rpm[fanNumber] + 0.5));
    Serial.write(pwm[fanNumber]);
    updatesRTToSend[fanNumber]--;
  }
}

unsigned short roundRPM(double rpm){
  unsigned short rpmRounded = (rpm + 5) / 10;
  return rpmRounded * 10;
}

void setPwmConfiguration(CommandParameter &parameters){
  if(eeprom_busy) return;   //update not allowed during save configuration to EEPROM
  
  byte pwmChannel = parameters.NextParameterAsInteger();
  byte pwmDrive = parameters.NextParameterAsInteger();
  byte constPwm = parameters.NextParameterAsInteger();
  byte tSelect =  parameters.NextParameterAsInteger();
  byte minPwm = parameters.NextParameterAsInteger();
  byte maxPwm = parameters.NextParameterAsInteger();
  byte tempTarget = parameters.NextParameterAsInteger();
  byte tempMax = parameters.NextParameterAsInteger();

  if(pwmChannel >= 0 && pwmChannel <= 5){
    ConfigurationPWM[pwmChannel] -> set(pwmDrive, constPwm, tSelect, minPwm, maxPwm, tempTarget, tempMax);
#ifdef USE_PWM_CACHE
    cacheRMPbyTemp[pwmChannel].clear();
#endif    
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
  }
}

void setPidConfiguration(CommandParameter &parameters){
  if(eeprom_busy) return;   //update not allowed during save configuration to EEPROM
  
  byte pwmChannel = parameters.NextParameterAsInteger();
  unsigned short constRPM = parameters.NextParameterAsInteger();
  unsigned short minRPM = parameters.NextParameterAsInteger();
  unsigned short maxRPM = parameters.NextParameterAsInteger();
  byte tempTargetRPM = parameters.NextParameterAsInteger();
  byte tempMaxRPM = parameters.NextParameterAsInteger();
  byte kp = parameters.NextParameterAsInteger();
  byte ki = parameters.NextParameterAsInteger();
  byte kd = parameters.NextParameterAsInteger();
  byte minPidPwm = parameters.NextParameterAsInteger();

  if(pwmChannel >= 0 && pwmChannel <= 5){
    ConfigurationPWM[pwmChannel] -> setPid(constRPM, minRPM, maxRPM, tempTargetRPM, tempMaxRPM, kp, ki, kd, minPidPwm);
    pid[pwmChannel].SetTunings((double) kp / 200, (double) ki / 200, (double) kd / 200);
    pid[pwmChannel].SetOutputLimits(ConfigurationPWM[pwmChannel] -> minPidPwm, 255);
#ifdef USE_PWM_CACHE
    cacheRMPbyTemp[pwmChannel].clear();
#endif    
  }
}

void disableFan(CommandParameter &parameters){
  while(1){
    byte pwmChannel = parameters.NextParameterAsInteger( 255 );
    byte delayParam = parameters.NextParameterAsInteger( 255 );

    if(pwmChannel == 255 || delayParam == 255)
      return;

    if(pwmChannel >= 0 && pwmChannel <= 5){
      pwmDisabled[pwmChannel] = delayParam;
    }
  }
}

void sendPidUpates(CommandParameter &parameters){
  while(1){
    byte pwmChannel = parameters.NextParameterAsInteger( 255 );
    byte numberOfUpdates = parameters.NextParameterAsInteger( 255 );

    if(pwmChannel == 255 || numberOfUpdates == 255)
      return;

    if(pwmChannel >= 0 && pwmChannel <= 5){
      updatesRTToSend[pwmChannel] = numberOfUpdates;
    }
  }
}

void setRPMToMainboard(CommandParameter &parameters){
  if(eeprom_busy) return;   //update not allowed during save configuration to EEPROM

  byte select = parameters.NextParameterAsInteger();
  if(select >= 0 && select <= 5){
    *rmpToMainboard = select;
  }
}

void setHysteresis(CommandParameter &parameters){
  unsigned short h = parameters.NextParameterAsInteger();
  if(h >= 0 && h <= 100){
    *hysteresis = h;
  }
}

void cacheStatus(){
#ifdef USE_TEMP_CACHE
  Serial.println(F("cache T0"));
  cacheT0.printStatus();
  Serial.println(F("cache T1"));
  cacheT1.printStatus();
#endif
#ifdef USE_PWM_CACHE
  Serial.println(F("cache RPMbyTemp[0]"));
  cacheRMPbyTemp[0].printStatus();
  Serial.println(F("cache RPMbyTemp[1]"));
  cacheRMPbyTemp[1].printStatus();
  Serial.println(F("cache RPMbyTemp[2]"));
  cacheRMPbyTemp[2].printStatus();
  Serial.println(F("cache RPMbyTemp[3]"));
  cacheRMPbyTemp[3].printStatus();
  Serial.println(F("cache RPMbyTemp[4]"));
  cacheRMPbyTemp[4].printStatus();
  Serial.println(F("cache RPMbyTemp[5]"));
  cacheRMPbyTemp[5].printStatus();
#endif
}

void Cmd_Unknown()
{
  if(gui){
    Serial.write(15);
    Serial.print(F("Unknown command"));
  } else {
    Serial.println(F("Unknown command"));
  }
}

