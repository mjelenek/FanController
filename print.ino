void printStatus(){
  for(byte i = 0; i < NUMBER_OF_THERMISTORS; i++){
    Serial.print(F(" T"));
    Serial.print(i);
    Serial.print(F(":"));
    if(fakeTemp[i] == 0){
      if (TConnected[i]){
        Serial.print(Tint[i] / 10);
        Serial.print(F("."));
        Serial.print(Tint[i] % 10);
      } else {
        Serial.print(F("N/A"));
      }
    } else {
        Serial.print(Tint[i] / 10);
        Serial.print(F("."));
        Serial.print(Tint[i] % 10);
        Serial.print(F(" (fake value)"));
    }
  }
  Serial.println(F(""));
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    Serial.print(F(" RPM"));
    Serial.print(i);
    Serial.print(F(":"));
    Serial.print(roundRPM(rpm[i]));
  }
  Serial.println(F(""));
}

void printFullStatus(){
  printStatus();
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    Serial.print(F(" PWM"));
    Serial.print(i);
    Serial.print(F(":"));
    Serial.print(pwm[i]);
  }
  Serial.println(F(""));
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    Serial.print(F(" pwm"));
    Serial.print(i);
    Serial.print(F("Drive: "));
    printlnPwmDrive(ConfigurationPWMHolder[i].Data.m_UserData);
  }
  Serial.print(F(" hysteresis: "));
  Serial.println(hysteresis);
}  
  

void printlnPwmDrive(PWMConfiguration &conf){
  // 0 - analogInput, 1 - constPWM, 2 - PWM by temperatures, 3 - constRPM, 4 - RPM by temperatures
  switch (conf.pwmDrive) {
    case 0:
      Serial.println(F("analog input"));
      break;
    case 1:
      Serial.print(F("constant power, PWM="));
      Serial.println(conf.constPwm);
      break;
    case 2:
      Serial.print(F("PWM by temperature, t0="));
      Serial.print(conf.tPwm[0]);
      Serial.print(F(" pwm0="));
      Serial.print(conf.pwm[0]);
      Serial.print(F(", t1="));
      Serial.print(conf.tPwm[1]);
      Serial.print(F(" pwm1="));
      Serial.print(conf.pwm[1]);
      Serial.print(F(", t2="));
      Serial.print(conf.tPwm[2]);
      Serial.print(F(" pwm2="));
      Serial.print(conf.pwm[2]);
      Serial.print(F(", t3="));
      Serial.print(conf.tPwm[3]);
      Serial.print(F(" pwm3="));
      Serial.print(conf.pwm[3]);
      Serial.print(F(", t4="));
      Serial.print(conf.tPwm[4]);
      Serial.print(F(" pwm4="));
      Serial.println(conf.pwm[4]);
      break;
    case 3:
      Serial.print(F("constant speed, expected RPM="));
      Serial.println(conf.constRpm);
      break;
    case 4:
      Serial.print(F("RPM by temperature, t0="));
      Serial.print(conf.tRpm[0]);
      Serial.print(F(" rpm0="));
      Serial.print(conf.rpm[0]);
      Serial.print(F(", t1="));
      Serial.print(conf.tRpm[1]);
      Serial.print(F(" rpm1="));
      Serial.print(conf.rpm[1]);
      Serial.print(F(", t2="));
      Serial.print(conf.tRpm[2]);
      Serial.print(F(" rpm2="));
      Serial.print(conf.rpm[2]);
      Serial.print(F(", t3="));
      Serial.print(conf.tRpm[3]);
      Serial.print(F(" rpm3="));
      Serial.print(conf.rpm[3]);
      Serial.print(F(", t4="));
      Serial.print(conf.tRpm[4]);
      Serial.print(F(" rpm4="));
      Serial.println(conf.rpm[4]);
      break;
  }
}

void printDelay(byte i, unsigned long d){
  if(!gui){
    Serial.print(F("!"));
    Serial.print(i);
    Serial.print(F("-"));
    Serial.println(d);
  } else {
    Serial.print(F("!"));
    Serial.write(10);
    Serial.print(F("delay"));
    Serial.write(i);
    serialWriteLong(d);
    Serial.print(F("#"));
  }
}

void printDelayThreshold(){
    if(!gui){
      Serial.println(F("!delayed"));
    } else {
    Serial.print(F("!"));
    Serial.write(7);
    Serial.print(F("delayed"));
    Serial.print(F("#"));
    }
}

void printHelp(){
  /*
  Serial.println(F("Available commands:"));
  Serial.println(F("s - print status in human readable format"));
  Serial.println(F("fs - print full status in human readable format"));
  Serial.println(F("guiE - enable GUI mode. GUI mode means, that informations about temperatures and fan rotations are sending periodically in binary mode"));
  Serial.println(F("guiD - disable GUI mode"));
  Serial.println(F("guistat1 - send fan configurations of fan0-fan2 in binary mode"));
  Serial.println(F("guistat2 - send fan configurations of fan3-fan5 in binary mode"));
  Serial.println(F("    message format of guistat1 and guistat2(character \"|\" is just a separator here) - \"!\"(starting char) | (byte)106(length of message) | \"gui1\" | fan0Status | fan1Status | fan2Status | \"#\"(ending char)"));
  Serial.println(F("    ... where fanXStatus is - (byte)driving | (byte)constPwm | (byte)temperature select | (byte)tPwm0 | (byte)pwm0 | (byte)tPwm1 | (byte)pwm1 | (byte)tPwm2 | (byte)pwm2 | (byte)tPwm3 | (byte)pwm3 | (byte)tPwm4 | (byte)pwm4 | (int)constRpm | (byte)tRpm0 | (int)rpm0 | (byte)tRpm1 | (int)rpm1 | (byte)tRpm2 | (int)rpm2 | (byte)tRpm3 | (int)rpm3 | (byte)tRpm4 | (int)rpm4 | (byte)kp | (byte)ki | (byte)kd | (byte)minPidPwm"));
  Serial.println(F("    ... (byte) means value in one byte, (int) means value in two bytes - lowByte | highByte"));
  Serial.println(F("guiUpdate - send temperatures and fan rotations in binary mode"));
  Serial.println(F("    message format of guiUpdate(character \"|\" is just a separator here) - \"!\"(starting char) | (byte)24(length of message) | \"guiU\" | (int)rpm0 | (int)rpm1 | (int)rpm2 | (int)rpm3 | (int)rpm4 | (int)rpm5 | (int)T0*10 | (int)T1*10 | (int)T0WithHysteresis*10 | (int)T1WithHysteresis*10 | \"#\"(ending char)"));
  Serial.println(F("setFan param1 ... param14 - set PWM configuration of fan, parameters param1 - param8 are mandatory"));
  Serial.println(F("    param1 - fan number. Allowed values 0 - 5"));
  Serial.println(F("    param2 - select the way of driving fan. 0 - driven by analog input, 1 - constant pwm, 2 - pwm driven by temperatures, 3 - constant rpm, 4 - rpm driven by temperatures"));
  Serial.println(F("    param3 - constant power in case of param2 == 1. Allowed values 0 - 255"));
  Serial.println(F("  next parameters are used in case of param2 == 2"));
  Serial.println(F("    param4 - select temperature source for driving by temperature. 0 - T0, 1 - T1, 2 - average value of T0 and T1"));
  Serial.println(F("    param5 - temperature t0. Allowed values 0 - 60"));
  Serial.println(F("    param6 - fan power at temperature t0 and below temperature t0. Allowed values 0 - 255"));
  Serial.println(F("    param7 - temperature t1. Allowed values 0 - 60, t1 shall be bigger than t0"));
  Serial.println(F("    param8 - fan power at temperature t1, and above temperature t1 if next temperature is not defined. Allowed values 0 - 255"));
  Serial.println(F("    param9 - temperature t2. Allowed values 0 - 60, t2 shall be bigger than t1"));
  Serial.println(F("    param10 - fan power at temperature t2, and above temperature t2 if next temperature is not defined. Allowed values 0 - 255"));
  Serial.println(F("    param11 - temperature t3. Allowed values 0 - 60, t3 shall be bigger than t2"));
  Serial.println(F("    param12 - fan power at temperature t3, and above temperature t3 if next temperature is not defined. Allowed values 0 - 255"));
  Serial.println(F("    param13 - temperature t4. Allowed values 0 - 60, t4 shall be bigger than t3"));
  Serial.println(F("    param14 - fan power at temperature t4, and above temperature t4 if next temperature is not defined. Allowed values 0 - 255"));
  Serial.println(F("setPid param1 ... param16 - set PID configuration of fan, parameters param1 - param10 are mandatory. Used when fan is driven by PID. Parameter drivingFan is set to 3(constant rpm) or 4 (rpm driven by temperatures)"));
  Serial.println(F("    param1 - fan number. Allowed values 0 - 5"));
  Serial.println(F("    param2 - const rpm value, used when parameter drivingFan is set to 3"));
  Serial.println(F("    param3 - Kp parameter of PID multiplied by 200. Allowed values 0 - 255"));
  Serial.println(F("    param4 - Ki parameter of PID multiplied by 200. Allowed values 0 - 255"));
  Serial.println(F("    param5 - Kd parameter of PID multiplied by 200. Allowed values 0 - 255"));
  Serial.println(F("    param6 - minPidPwm parameter of PID. Allowed values 0 - 255. When fan is driven by PID, pwm value is in range minPidPwm - 255"));
  Serial.println(F("    param7 - temperature t0. Allowed values 0 - 60"));
  Serial.println(F("    param8 - rpm value at temperature t0 and below t0"));
  Serial.println(F("    param9 - temperature t1. Allowed values 0 - 60, t1 shall be bigger than t0"));
  Serial.println(F("    param10 - rpm value at temperature t1, and above temperature t1 if next temperature is not defined"));
  Serial.println(F("    param11 - temperature t2. Allowed values 0 - 60, t2 shall be bigger than t1"));
  Serial.println(F("    param12 - rpm value at temperature t2, and above temperature t2 if next temperature is not defined"));
  Serial.println(F("    param13 - temperature t3. Allowed values 0 - 60, t3 shall be bigger than t2"));
  Serial.println(F("    param14 - rpm value at temperature t3, and above temperature t3 if next temperature is not defined"));
  Serial.println(F("    param15 - temperature t4. Allowed values 0 - 60, t4 shall be bigger than t3"));
  Serial.println(F("    param16 - rpm value at temperature t4, and above temperature t4 if next temperature is not defined"));
  Serial.println(F("load - load fan configurations from internal memory"));
  Serial.println(F("save - save fan configurations into internal memory"));
  Serial.println(F("setConf - set configuration(TODO)"));
  Serial.println(F("setThermistor - set thermistor parameters(TODO)"));
  Serial.println(F("setTemp - set fake temperature of thermistor param1 to value param2. If param2 == 0, unset fake temperature."));
  Serial.println(F("disableFan param1 param2 ... paramN - disable rotation of fan [param1, param3, param5...] for [param2, param4, param6...] seconds"));
  Serial.println(F("cacheStatus - print current state of caches with temperature values computed from thermistors"));
#ifdef TIMING_DEBUG
  Serial.println(F("time -  - print time from start - microsecond, milliseconds, seconds"));
  Serial.println(F("timing -  - print measured processor time occupied by program"));
  Serial.println(F("mi - print measured processor time occupied by interrupt services"));
#endif
#ifdef FREE_MEMORY_DEBUG
  Serial.println(F("freemem - print lowest measured free memory over the stack"));
#endif
  */
  Serial.println(F("help - this help"));
}

#ifdef TIMING_DEBUG
void printTimingResult(){
  if(!gui){
    Serial.print(F("Time in code: "));
    Serial.print(100 * (float)timeInCode / (float)timeTotal, 2);
    Serial.println(F("%"));
    Serial.print(F("Average delay: "));
    Serial.println((unsigned long)timeInCode >> 8);
    Serial.print(F("<1000-"));
    Serial.println(to1000);
    Serial.print(F("<1500-"));
    Serial.println(to1500);
    Serial.print(F("<2000-"));
    Serial.println(to2000);
    if(to2500 > 0){
      Serial.print(F("<2500-"));
      Serial.println(to2500);
    }
    if(to3000 > 0){
      Serial.print(F("<3000-"));
      Serial.println(to3000);
    }
    if(over3000 > 0){
      Serial.print(F(">3000-"));
      Serial.println(over3000);
    }
  } else {
    Serial.print(F("!"));
    Serial.write(24);
    Serial.print(F("timing"));
    serialWriteInt((unsigned int)(10000 * (float)timeInCode / (float)timeTotal));
    serialWriteLong((unsigned long)timeInCode >> 8);
    serialWriteInt(to1000);
    serialWriteInt(to1500);
    serialWriteInt(to2000);
    serialWriteInt(to2500);
    serialWriteInt(to3000);
    serialWriteInt(over3000);
    Serial.print(F("#"));
  }
}
#endif


