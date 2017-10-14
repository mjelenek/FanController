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

unsigned short countDesiredRPM(unsigned long temperature, unsigned long temperatureTarget, unsigned long temperatureMax, unsigned long rpmMin, unsigned long rpmMax){
  temperatureTarget = temperatureTarget  * 10;
  if(temperature <= temperatureTarget){
    return rpmMin;
  }
  temperatureMax = temperatureMax * 10;
  if(temperature >= temperatureMax){
    return rpmMax;
  }
  return (temperature - temperatureTarget) * (rpmMax - rpmMin) / (temperatureMax - temperatureTarget) + rpmMin;
}

byte getNewPwm(PWMConfiguration &conf, byte pwm, unsigned int sensorValueAveraged, byte pidIndex){
  // pwmDrive: 0 - const PWM, 1 - analogInput, 2 - PWM by temperatures, 3 - constRPM, 4 - RPM by temperatures
  switch (conf.pwmDrive) {
    case 0:
      return conf.constPwm;
    case 1:
      return map(sensorValueAveraged, 0, 1023, 0, 255);
    case 2:
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
    case 3:
      setpointPid[pidIndex] = conf.constRpm;
      if(pid[pidIndex].Compute()){
//        pidUpdate(pidIndex, setpointPid[pidIndex], inputPid[pidIndex], (byte)outputPid[pidIndex]);
        return (byte)outputPid[pidIndex];
      }
    case 4:
  // tSelect: 0 - T0, 1 - T1, 2  - average value of T0 and T1
      switch (conf.tSelect) {
        case 0:
          if(T0Connected){
            setpointPid[pidIndex] = countDesiredRPM(T0WithHysteresisInt, conf.tempTargetRpm, conf.tempMaxRpm, conf.minRpm, conf.maxRpm);
          } else {
            setpointPid[pidIndex] = conf.tempMaxRpm;
          }
          break;
        case 1:
          if(T1Connected){
            setpointPid[pidIndex] = countDesiredRPM(T1WithHysteresisInt, conf.tempTargetRpm, conf.tempMaxRpm, conf.minRpm, conf.maxRpm);
          } else {
            setpointPid[pidIndex] = conf.tempMaxRpm;
          }
          break;
        case 2:
          if(T0Connected && T1Connected){
            setpointPid[pidIndex] = countDesiredRPM(T0WithHysteresisInt + T1WithHysteresisInt, conf.tempTargetRpm << 1, conf.tempMaxRpm << 1, conf.minRpm, conf.maxRpm);
            break;
          }
          if(T0Connected){
            setpointPid[pidIndex] = countDesiredRPM(T0WithHysteresisInt, conf.tempTargetRpm, conf.tempMaxRpm, conf.minRpm, conf.maxRpm);
            break;
          }
          if(T1Connected){
            setpointPid[pidIndex] = countDesiredRPM(T1WithHysteresisInt, conf.tempTargetRpm, conf.tempMaxRpm, conf.minRpm, conf.maxRpm);
            break;
          }
          setpointPid[pidIndex] = conf.tempMaxRpm;
          break;
        default:
          setpointPid[pidIndex] = conf.tempMaxRpm;
      }
      if(pid[pidIndex].Compute()){
//        pidUpdate(pidIndex, setpointPid[pidIndex], inputPid[pidIndex], (byte)outputPid[pidIndex]);
        return (byte)outputPid[pidIndex];
      }
    default:
      return pwm;
  }
}

void setPwm0(){
  if(pwm0Disabled > 0){
    pwm0 = 0;
  } else {
    ADCSRA &= ~(1 << ADIE);  // Disable ADC conversion complete interrupt
    unsigned short sensorValue = sensorValue4Averaged;
    ADCSRA |= (1 << ADIE);  // Enable ADC conversion complete interrupt
    pwm0 = getNewPwm(ConfigurationPWM0.Data, pwm0, sensorValue, 0);
  }
  analogWrite(PWM0, 255 - pwm0);
}

void setPwm1(){
  if(pwm1Disabled > 0){
    pwm1 = 0;
  } else {
    ADCSRA &= ~(1 << ADIE);  // Disable ADC conversion complete interrupt
    unsigned short sensorValue = sensorValue3Averaged;
    ADCSRA |= (1 << ADIE);  // Enable ADC conversion complete interrupt
    pwm1 = getNewPwm(ConfigurationPWM1.Data, pwm1, sensorValue, 1);
  }
  analogWrite(PWM1, 255 - pwm1);
}

void setPwm2(){
  if(pwm2Disabled > 0){
    pwm2 = 0;
  } else {
    ADCSRA &= ~(1 << ADIE);  // Disable ADC conversion complete interrupt
    unsigned short sensorValue = sensorValue2Averaged;
    ADCSRA |= (1 << ADIE);  // Enable ADC conversion complete interrupt
    pwm2 = getNewPwm(ConfigurationPWM2.Data, pwm2, sensorValue, 2);
  }
  analogWrite(PWM2, 255 - pwm2);
}

void setPwm3(){
  if(pwm3Disabled > 0){
    pwm3 = 0;
  } else {
    ADCSRA &= ~(1 << ADIE);  // Disable ADC conversion complete interrupt
    unsigned short sensorValue = sensorValue1Averaged;
    ADCSRA |= (1 << ADIE);  // Enable ADC conversion complete interrupt
    pwm3 = getNewPwm(ConfigurationPWM3.Data, pwm3, sensorValue, 3);
  }
  analogWrite(PWM3, 255 - pwm3);
}

void setPwm4(){
  if(pwm4Disabled > 0){
    pwm4 = 0;
  } else {
    ADCSRA &= ~(1 << ADIE);  // Disable ADC conversion complete interrupt
    unsigned short sensorValue = sensorValue0Averaged;
    ADCSRA |= (1 << ADIE);  // Enable ADC conversion complete interrupt
    pwm4 = getNewPwm(ConfigurationPWM4.Data, pwm4, sensorValue, 4);
  }
  analogWrite(PWM4, 255 - pwm4);
}

void setPwm5(){
  if(pwm5Disabled > 0){
    pwm5 = 0;
  } else {
    ADCSRA &= ~(1 << ADIE);  // Disable ADC conversion complete interrupt
    unsigned short sensorValue = sensorValue0Averaged;
    ADCSRA |= (1 << ADIE);  // Enable ADC conversion complete interrupt
    pwm5 = getNewPwm(ConfigurationPWM5.Data, pwm5, sensorValue, 5);
  }
  analogWrite(PWM5, 255 - pwm5);
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
  if(pwm0Disabled > 0){
    pwm0Disabled--;
  }
  if(pwm1Disabled > 0){
    pwm1Disabled--;
  }
  if(pwm2Disabled > 0){
    pwm2Disabled--;
  }
  if(pwm3Disabled > 0){
    pwm3Disabled--;
  }
  if(pwm4Disabled > 0){
    pwm4Disabled--;
  }
  if(pwm5Disabled > 0){
    pwm5Disabled--;
  }
}


