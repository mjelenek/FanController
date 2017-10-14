void printStatus(){
  Serial.print(F("T0:"));
  if (T0Connected){
    Serial.print(T0int / 10, 1);
  } else {
    Serial.print(F("N/A"));
  }
  Serial.print(F(" T1:"));
  if (T1Connected){
    Serial.println(T1int / 10, 1);
  } else {
    Serial.println(F("N/A"));
  }
  Serial.print(F("RPM0:"));
  Serial.print(rpm0);
  Serial.print(F(" RPM1:"));
  Serial.print(rpm1);
  Serial.print(F(" RPM2:"));
  Serial.print(rpm2);
  Serial.print(F(" RPM3:"));
  Serial.print(rpm3);
  Serial.print(F(" RPM4:"));
  Serial.print(rpm4);
  Serial.print(F(" RPM5:"));
  Serial.println(rpm5);
}

void printFullStatus(){
  printStatus();
  Serial.print(F("PWM0:"));
  Serial.print(pwm0);
  Serial.print(F(" PWM1:"));
  Serial.print(pwm1);
  Serial.print(F(" PWM2:"));
  Serial.print(pwm2);
  Serial.print(F(" PWM3:"));
  Serial.print(pwm3);
  Serial.print(F(" PWM4:"));
  Serial.print(pwm4);
  Serial.print(F(" PWM5:"));
  Serial.println(pwm5);
  Serial.print(F(" pwm0Drive: "));
  printlnPwmDrive(ConfigurationPWM0.Data);
  Serial.print(F(" pwm1Drive: "));
  printlnPwmDrive(ConfigurationPWM1.Data);
  Serial.print(F(" pwm2Drive: "));
  printlnPwmDrive(ConfigurationPWM2.Data);
  Serial.print(F(" pwm3Drive: "));
  printlnPwmDrive(ConfigurationPWM3.Data);
  Serial.print(F(" pwm4Drive: "));
  printlnPwmDrive(ConfigurationPWM4.Data);
  Serial.print(F(" pwm5Drive: "));
  printlnPwmDrive(ConfigurationPWM5.Data);
}  
  

void printlnPwmDrive(PWMConfiguration &conf){
  // pwmDrive: 0 - const, 1 - analogInput, 2 - T0, 3 - T1, 4  - (T1+T2)/2
  switch (conf.pwmDrive) {
    case 0:
      Serial.print(F("constant speed, PWM="));
      Serial.println(conf.constPwm);
      break;
    case 1:
      Serial.println(F("analog input"));
      break;
    case 2:
      Serial.print(F("temperature 0, minPWM="));
      Serial.print(conf.minPwm);
      Serial.print(F(", maxPWM="));
      Serial.println(conf.maxPwm);
      break;
    case 3:
      Serial.print(F("temperature 1, minPWM="));
      Serial.print(conf.minPwm);
      Serial.print(F(", maxPWM="));
      Serial.println(conf.maxPwm);
      break;
    case 4:
      Serial.print(F("temperature 0 and 1, minPWM="));
      Serial.print(conf.minPwm);
      Serial.print(F(", maxPWM="));
      Serial.println(conf.maxPwm);
      break;
  }
}

void printDelay(byte i, unsigned long d){
  if(!gui){
    Serial.print(F("!"));
    Serial.println(d);
  } else {
    Serial.write(6);
    Serial.print(F("!"));
    Serial.write(i);
    serialWriteLong(d);
  }
}

void printDelayThreshold(){
    if(!gui){
      Serial.println(F("!delayed"));
    } else {
      Serial.write(7);
      Serial.print(F("delayed"));
    }
}

void printHelp(){
  Serial.println(F("Available commands:"));
  Serial.println(F("s - print status in human readable format"));
  Serial.println(F("fs - print full status in human readable format"));
  Serial.println(F("guiE - enable GUI mode. GUI mode means, that informations about temperatures and fan rotations are sending periodically in binary mode"));
  Serial.println(F("guiD - disable GUI mode"));
  Serial.println(F("guistat - send fan configurations in binary mode"));
  Serial.println(F("guiUpdate - send temperatures and fan rotations in binary mode"));
  Serial.println(F("setFan param1 param2 param3 param4 param5 param6 param7 param8 - set PWM configuration of fan"));
  Serial.println(F("    param1 - fan number. Allowed values 0 - 5"));
  Serial.println(F("    param2 - select the way of driving fan. 0 - constant pwm, 1 - driven by analog input, 2 - driven by temperatures"));
  Serial.println(F("    param3 - constant power in case of param2 == 0. Allowed values 0 - 255"));
  Serial.println(F("  next parameters are used in case of param2 == 2"));
  Serial.println(F("    param4 - select temperature source for driving by temperature. 0 - T0, 1 - T1, 2 - average value of T0 and T1"));
  Serial.println(F("    param5 - fan power at target temperature and below target temperature. Allowed values 0 - 255"));
  Serial.println(F("    param6 - fan power at maximum temperature and above maximal temperature. Allowed values 0 - 255"));
  Serial.println(F("    param7 - target temperature. Allowed values 0 - 60"));
  Serial.println(F("    param8 - maximal temperature. Allowed values 0 - 60. Param8 should be bigger than param7"));
  Serial.println(F("load - load fan configurations from internal memory"));
  Serial.println(F("save - save fan configurations into internal memory"));
  Serial.println(F("rpm - setRPMToMainboard"));
  Serial.println(F("disableFan param1 param2 ... paramN - disable rotation of fan [param1, param3, param5...] for [param2, param4, param6...] seconds"));
  Serial.println(F("tempCacheStatus - print current state of caches with temperature values computed from thermistors"));
#ifdef TIMING_DEBUG
  Serial.println(F("timing -  - print measured processor time occupied by program"));
  Serial.println(F("mi - print measured processor time occupied by interrupt services"));
#endif
  Serial.println(F("help - this help"));
}

