byte countPWM(unsigned int temperature, unsigned int temperatureTarget, unsigned int temperatureMax, byte pwmMin, byte pwmMax){
  temperatureTarget = temperatureTarget  * 10;
  if(temperature <= temperatureTarget){
    return pwmMin;
  }
  temperatureMax = temperatureMax * 10;
  if(temperature >= temperatureMax){
    return pwmMax;
  }
  return (temperature - temperatureTarget) * (pwmMax - pwmMin) / (temperatureMax - temperatureTarget) + pwmMin;
}

unsigned short countExpectedRPM(unsigned short temperature, unsigned short temperatureTarget, unsigned short temperatureMax, unsigned short rpmMin, unsigned short rpmMax){
  temperatureTarget = temperatureTarget  * 10;
  if(temperature <= temperatureTarget){
    return rpmMin;
  }
  temperatureMax = temperatureMax * 10;
  if(temperature >= temperatureMax){
    return rpmMax;
  }
  return (((unsigned long)(temperature - temperatureTarget)) * (rpmMax - rpmMin)) / (temperatureMax - temperatureTarget) + rpmMin;
}

byte getNewPwm(PWMConfiguration &conf, byte pwm, unsigned short sensorValueAveraged, byte fanNumber){
  if(pwmDisabled[fanNumber] > 0){
    return 0;
  }
  // pwmDrive: 0 - const PWM, 1 - analogInput, 2 - PWM by temperatures, 3 - constRPM, 4 - RPM by temperatures
  switch (conf.pwmDrive) {
    case 0:
      return map(sensorValueAveraged, 0, 1023, 0, 255);
    case 1:
      return conf.constPwm;
    case 2:
      // compute once every 8 cycles is enough
      if(((i + fanNumber) & B00000111) == 0){
    
      // tSelect: 0 - T0, 1 - T1, 2  - average value of T0 and T1
        switch (conf.tSelect) {
          case 0:
            if(T0Connected){
              return countPWM(T0WithHysteresisInt, conf.tempTarget, conf.tempMax, conf.minPwm, conf.maxPwm);
            }
            return conf.maxPwm;
          case 1:
            if(T1Connected){
              return countPWM(T1WithHysteresisInt, conf.tempTarget, conf.tempMax, conf.minPwm, conf.maxPwm);
            }
            return conf.maxPwm;
          case 2:
            if(T0Connected && T1Connected){
              return countPWM(T0WithHysteresisInt + T1WithHysteresisInt, conf.tempTarget << 1, conf.tempMax << 1, conf.minPwm, conf.maxPwm);
            }
            if(T0Connected){
              return countPWM(T0WithHysteresisInt, conf.tempTarget, conf.tempMax, conf.minPwm, conf.maxPwm);
            }
            if(T1Connected){
              return countPWM(T1WithHysteresisInt, conf.tempTarget, conf.tempMax, conf.minPwm, conf.maxPwm);
            }
          return conf.maxPwm;
        }
      } else {
        return pwm;
      }
    case 3:
      setpointPid[fanNumber] = conf.constRpm;
      if(pidCompute(fanNumber)){
        return (byte)outputPid;
      } else {
        return pwm;
      }
    case 4:
      // compute once every 64 cycles (128ms)
      if(((i + fanNumber) & B00111111) == 0){
  
        // tSelect: 0 - T0, 1 - T1, 2  - average value of T0 and T1
        switch (conf.tSelect) {
          case 0:
            if(T0Connected){
              setpointPid[fanNumber] = countExpectedRPM(T0WithHysteresisInt, conf.tempTargetRpm, conf.tempMaxRpm, conf.minRpm, conf.maxRpm);
            } else {
              setpointPid[fanNumber] = conf.tempMaxRpm;
            }
            break;
          case 1:
            if(T1Connected){
              setpointPid[fanNumber] = countExpectedRPM(T1WithHysteresisInt, conf.tempTargetRpm, conf.tempMaxRpm, conf.minRpm, conf.maxRpm);
            } else {
              setpointPid[fanNumber] = conf.tempMaxRpm;
            }
            break;
          case 2:
            if(T0Connected && T1Connected){
              setpointPid[fanNumber] = countExpectedRPM(T0WithHysteresisInt + T1WithHysteresisInt, conf.tempTargetRpm << 1, conf.tempMaxRpm << 1, conf.minRpm, conf.maxRpm);
              break;
            }
            if(T0Connected){
              setpointPid[fanNumber] = countExpectedRPM(T0WithHysteresisInt, conf.tempTargetRpm, conf.tempMaxRpm, conf.minRpm, conf.maxRpm);
              break;
            }
            if(T1Connected){
              setpointPid[fanNumber] = countExpectedRPM(T1WithHysteresisInt, conf.tempTargetRpm, conf.tempMaxRpm, conf.minRpm, conf.maxRpm);
              break;
            }
            setpointPid[fanNumber] = conf.tempMaxRpm;
            break;
          default:
            setpointPid[fanNumber] = conf.tempMaxRpm;
        }
      }
      if(pidCompute(fanNumber)){
        return (byte)outputPid;
      }
    default:
      return pwm;
  }
}

void setPwm0(){
  ADCSRA &= ~(1 << ADIE);  // Disable ADC conversion complete interrupt
  unsigned short sensorValue = sensorValue4Averaged;
  ADCSRA |= (1 << ADIE);  // Enable ADC conversion complete interrupt
  byte pwmOld = pwm[0];
  pwm[0] = getNewPwm(ConfigurationPWM0.Data.m_UserData, pwm[0], sensorValue, 0);
  if(pwmOld != pwm[0]){
    analogWrite(PWM0, 255 - pwm[0]);
  }
  pidUpdate(0, ConfigurationPWM0.Data.m_UserData);
}

void setPwm1(){
  ADCSRA &= ~(1 << ADIE);  // Disable ADC conversion complete interrupt
  unsigned short sensorValue = sensorValue3Averaged;
  ADCSRA |= (1 << ADIE);  // Enable ADC conversion complete interrupt
  byte pwmOld = pwm[1];
  pwm[1] = getNewPwm(ConfigurationPWM1.Data.m_UserData, pwm[1], sensorValue, 1);
  if(pwmOld != pwm[1]){
    analogWrite(PWM1, 255 - pwm[1]);
  }
  pidUpdate(1, ConfigurationPWM1.Data.m_UserData);
}

void setPwm2(){
  ADCSRA &= ~(1 << ADIE);  // Disable ADC conversion complete interrupt
  unsigned short sensorValue = sensorValue2Averaged;
  ADCSRA |= (1 << ADIE);  // Enable ADC conversion complete interrupt
  byte pwmOld = pwm[2];
  pwm[2] = getNewPwm(ConfigurationPWM2.Data.m_UserData, pwm[2], sensorValue, 2);
  if(pwmOld != pwm[2]){
    analogWrite(PWM2, 255 - pwm[2]);
  }
  pidUpdate(2, ConfigurationPWM2.Data.m_UserData);
}

void setPwm3(){
  ADCSRA &= ~(1 << ADIE);  // Disable ADC conversion complete interrupt
  unsigned short sensorValue = sensorValue1Averaged;
  ADCSRA |= (1 << ADIE);  // Enable ADC conversion complete interrupt
  byte pwmOld = pwm[3];
  pwm[3] = getNewPwm(ConfigurationPWM3.Data.m_UserData, pwm[3], sensorValue, 3);
  if(pwmOld != pwm[3]){
    analogWrite(PWM3, 255 - pwm[3]);
  }
  pidUpdate(3, ConfigurationPWM3.Data.m_UserData);
}

void setPwm4(){
  ADCSRA &= ~(1 << ADIE);  // Disable ADC conversion complete interrupt
  unsigned short sensorValue = sensorValue0Averaged;
  ADCSRA |= (1 << ADIE);  // Enable ADC conversion complete interrupt
  byte pwmOld = pwm[4];
  pwm[4] = getNewPwm(ConfigurationPWM4.Data.m_UserData, pwm[4], sensorValue, 4);
  if(pwmOld != pwm[4]){
    analogWrite(PWM4, 255 - pwm[4]);
  }
  pidUpdate(4, ConfigurationPWM4.Data.m_UserData);
}

void setPwm5(){
  ADCSRA &= ~(1 << ADIE);  // Disable ADC conversion complete interrupt
  unsigned short sensorValue = sensorValue0Averaged;
  ADCSRA |= (1 << ADIE);  // Enable ADC conversion complete interrupt
  byte pwmOld = pwm[5];
  pwm[5] = getNewPwm(ConfigurationPWM5.Data.m_UserData, pwm[5], sensorValue, 5);
  if(pwmOld != pwm[5]){
    analogWrite(PWM5, 255 - pwm[5]);
  }
  pidUpdate(5, ConfigurationPWM5.Data.m_UserData);
}

void setPwm(){
  setPwm0();
  setPwm1();
  setPwm2();
  setPwm3();
  setPwm4();
  setPwm5();
}

void decrementPwmDisabled(){
  for(int i = 0; i <= 5; i++){
    if(pwmDisabled[i] > 0){
      pwmDisabled[i]--;
    }
  }
}

boolean pidCompute(byte fanNumber){
  inputPid = rpm[fanNumber];
  return pid[fanNumber].Compute();
}


