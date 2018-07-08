byte countPWM(PWMConfiguration &conf, unsigned int temperature){
  byte tPartSelect = getTemperaturePartSelect(conf.tPwm, temperature);
  unsigned int temperatureTarget = conf.tPwm[tPartSelect] * 10;
  if(temperature <= temperatureTarget){
    return conf.pwm[tPartSelect];
  }
  unsigned int temperatureMax = conf.tPwm[tPartSelect + 1] * 10;
  if(temperature >= temperatureMax){
    return conf.pwm[tPartSelect + 1];
  }
  return ((long)(temperature - temperatureTarget) * ((int)conf.pwm[tPartSelect + 1] - (int)conf.pwm[tPartSelect])) / (temperatureMax - temperatureTarget) + conf.pwm[tPartSelect];
}

#ifdef USE_PWM_CACHE
byte countPWM(PWMConfiguration &conf, unsigned int temperature, byte fanNumber){
    byte pwm = cacheRMPbyTemp[fanNumber].get(temperature);
    if(pwm == 0){
      pwm = countPWM(conf, temperature);
      cacheRMPbyTemp[fanNumber].put(temperature, pwm);
    }
  return pwm;
}
#endif

unsigned short countExpectedRPM(PWMConfiguration &conf, unsigned int temperature){
  byte tPartSelect = getTemperaturePartSelect(conf.tRpm, temperature);
  unsigned short temperatureTarget = conf.tRpm[tPartSelect] * 10;
  if(temperature <= temperatureTarget){
    return conf.rpm[tPartSelect];
  }
  unsigned short temperatureMax = conf.tRpm[tPartSelect + 1] * 10;
  if(temperature >= temperatureMax){
    return conf.rpm[tPartSelect + 1];
  }
  return (((long)(temperature - temperatureTarget)) * ((int)conf.rpm[tPartSelect + 1] - (int)conf.rpm[tPartSelect])) / (temperatureMax - temperatureTarget) + conf.rpm[tPartSelect];
}

#ifdef USE_PWM_CACHE
unsigned short countExpectedRPM(PWMConfiguration &conf, unsigned int temperature, byte fanNumber){
    unsigned short rpm = cacheRMPbyTemp[fanNumber].get(temperature);
    if(rpm == 0){
      rpm = countExpectedRPM(conf, temperature);
      cacheRMPbyTemp[fanNumber].put(temperature, rpm);
    }
  return rpm;
}
#endif

// 3, 7, 11, 15, 19, 23 (... 27, 31, 35, 39, 43, 47, 51, 53)
#define TIME_TO_COMPUTE_PWM part_64 == ((fanNumber << 2) + 3)
// 4, 8, 12, 16, 20, 24 (... 28, 32, 36, 40, 44, 48, 52, 54)
#define TIME_TO_COMPUTE_PWM_BY_PID part_64 == ((fanNumber << 2) + 4)

void setPwm(byte fanNumber){
  unsigned short sensorValue;
  byte pwmOld = pwm[fanNumber];
  if(pwmDisabled[fanNumber] == 0){
    PWMConfiguration &conf = ConfigurationPWM(fanNumber);
    // pwmDrive: 0 - analogInput, 1 - constPWM, 2 - PWM by temperatures, 3 - constRPM, 4 - RPM by temperatures
    switch (conf.pwmDrive) {
      case 0:
        ADCSRA &= ~(1 << ADIE);               // Disable ADC conversion complete interrupt
        sensorValue = powerInADCAveraged[conf.powerInNumber];
        ADCSRA |= (1 << ADIE);                // Enable ADC conversion complete interrupt
        pwm[fanNumber] = sensorValue >> 2;    // map 0-1023 to 0-255
        break;
      case 1:
        pwm[fanNumber] = conf.constPwm;
        break;
      case 2:
        if(TIME_TO_COMPUTE_PWM){
          pwm[fanNumber] = countPWM(conf, countEffectiveTemperature(conf.tSelect) USE_FAN_NUMBER);  
        }
        break;
      case 3:
        if(TIME_TO_COMPUTE_PWM_BY_PID){
          pwm[fanNumber] = getNewPwmByConstRpm(conf, pwmOld, fanNumber);
        }
        break;
      case 4:
        if(TIME_TO_COMPUTE_PWM){
          setpointPid[fanNumber] = countExpectedRPM(conf, countEffectiveTemperature(conf.tSelect) USE_FAN_NUMBER);
        }
        if(TIME_TO_COMPUTE_PWM_BY_PID){
          if(pidCompute(fanNumber)){
             pwm[fanNumber] = (byte)outputPid;
          }
        }
        break;
    }
  } else {
    pwm[fanNumber] = 0;
  }

  if(pwmOld != pwm[fanNumber]){
    writeAnalogValue(PWMOUT(fanNumber), 255 - pwm[fanNumber]);
  }
  pidUpdate(fanNumber, ConfigurationPWM(fanNumber));
}

byte getNewPwmByConstRpm(PWMConfiguration &conf, byte pwmOld, byte fanNumber){
  setpointPid[fanNumber] = conf.constRpm;
  if(pidCompute(fanNumber)){
    return (byte)outputPid;
  }
  return pwmOld;
}

byte getTemperaturePartSelect(byte temperatures[], unsigned int temperature){
  byte i = 0;
  while (i < 3 && temperatures[i+2] != 0 && (temperatures[i+1] * 10) <= temperature){
    i++;
  }
  return i;
}

void setPwm(){
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    setPwm(i);
  }
}

void decrementPwmDisabled(){
  for(byte i = 0; i < NUMBER_OF_FANS; i++){
    if(pwmDisabled[i] > 0){
      pwmDisabled[i]--;
    }
  }
}

boolean pidCompute(byte fanNumber){
  inputPid = rpm[fanNumber];
  return pid[fanNumber].Compute();
}


