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

void guistat(){
  Serial.write(49);
  Serial.print(F("guistat"));
  ConfigurationPWM0.Data.guiStat();
  ConfigurationPWM1.Data.guiStat();
  ConfigurationPWM2.Data.guiStat();
  ConfigurationPWM3.Data.guiStat();
  ConfigurationPWM4.Data.guiStat();
  ConfigurationPWM5.Data.guiStat();
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
#endif

void guiUpdate(){
  Serial.write(25);
  Serial.print(F("guiUpdate"));
  serialWriteInt(rpm0);
  serialWriteInt(rpm1);
  serialWriteInt(rpm2);
  serialWriteInt(rpm3);
  serialWriteInt(rpm4);
  serialWriteInt(rpm5);
  serialWriteInt(T0int);
  serialWriteInt(T1int);
}

void setPwmConfiguration(CommandParameter &parameters){
  
//  Serial.println(F("setPwmConfiguration"));

  byte pwmChannel = parameters.NextParameterAsInteger();
  byte pwmDrive = parameters.NextParameterAsInteger();
  byte constPwm = parameters.NextParameterAsInteger();
  byte tSelect =  parameters.NextParameterAsInteger();
  byte minPwm = parameters.NextParameterAsInteger();
  byte maxPwm = parameters.NextParameterAsInteger();
  byte tempTarget = parameters.NextParameterAsInteger();
  byte tempMax = parameters.NextParameterAsInteger();
/*
  Serial.print(F("channel: "));
  Serial.println(pwmChannel);
  Serial.print(F("drive: "));
  Serial.println(pwmDrive);
  Serial.print(F("const: "));
  Serial.println(constPwm);
  Serial.print(F("min: "));
  Serial.println(minPwm);
  Serial.print(F("max: "));
  Serial.println(maxPwm);
  Serial.print(F("tTarget: "));
  Serial.println(tempTarget);
  Serial.print(F("tMax: "));
  Serial.println(tempMax);
*/
  switch (pwmChannel) {
  case 0:
    ConfigurationPWM0.Data.Set(pwmDrive, constPwm, tSelect, minPwm, maxPwm, tempTarget, tempMax);
    break;
  case 1:
    ConfigurationPWM1.Data.Set(pwmDrive, constPwm, tSelect, minPwm, maxPwm, tempTarget, tempMax);
    break;
  case 2:
    ConfigurationPWM2.Data.Set(pwmDrive, constPwm, tSelect, minPwm, maxPwm, tempTarget, tempMax);
    break;
  case 3:
    ConfigurationPWM3.Data.Set(pwmDrive, constPwm, tSelect, minPwm, maxPwm, tempTarget, tempMax);
    break;
  case 4:
    ConfigurationPWM4.Data.Set(pwmDrive, constPwm, tSelect, minPwm, maxPwm, tempTarget, tempMax);
    break;
  case 5:
    ConfigurationPWM5.Data.Set(pwmDrive, constPwm, tSelect, minPwm, maxPwm, tempTarget, tempMax);
  }

}

void disableFan(CommandParameter &parameters){
  while(1){
    byte pwmChannel = parameters.NextParameterAsInteger( 255 );
    byte delayParam = parameters.NextParameterAsInteger( 255 );

    if(pwmChannel == 255 || delayParam == 255)
      return;
    
    switch (pwmChannel) {
    case 0:
      pwm0Disabled = delayParam;
      break;
    case 1:
      pwm1Disabled = delayParam;
      break;
    case 2:
      pwm2Disabled = delayParam;
      break;
    case 3:
      pwm3Disabled = delayParam;
      break;
    case 4:
      pwm4Disabled = delayParam;
      break;
    case 5:
      pwm5Disabled = delayParam;
    }
  }
}

void setRPMToMainboard(CommandParameter &parameters){
  // 0 - from sensor4, 1 - from sensor5
  byte select = parameters.NextParameterAsInteger();
  if(select == 0){
    rmpToMainboard = 0;
  } else {
    rmpToMainboard = 1;
  }
}

void tempCacheStatus(){
  cacheT0.printStatus();
  cacheT1.printStatus();
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

