void print(){
  if(s == 1){
    if(i == 51){

      Serial.print(F(" T0:"));
      if (T0Connected){
        Serial.print(T0int / 10, 1);
      } else {
        Serial.print(F("N/A"));
      }
    }
    if(i == 59){
      Serial.print(F(" T1:"));
      if (T1Connected){
        Serial.println(T1int / 10, 1);
      } else {
        Serial.println(F("N/A"));
      }
    }
    if(fs == 1){
      if(i == 66){
        Serial.print(F(" pwm0Drive: "));
      }
      if(i == 67){
        printlnPwmDrive(ConfigurationPWM0.Data);
      }
      if(i == 74){
        Serial.print(F(" pwm1Drive: "));
      }
      if(i == 75){
        printlnPwmDrive(ConfigurationPWM1.Data);
      }
      if(i == 82){
        Serial.print(F(" pwm2Drive: "));
      }
      if(i == 83){
        printlnPwmDrive(ConfigurationPWM2.Data);
      }
      if(i == 90){
        Serial.print(F(" pwm3Drive: "));
      }
      if(i == 91){
        printlnPwmDrive(ConfigurationPWM3.Data);
      }
      if(i == 98){
        Serial.print(F(" pwm4Drive: "));
      }
      if(i == 99){
        printlnPwmDrive(ConfigurationPWM4.Data);
      }
      if(i == 106){
        Serial.print(F(" pwm5Drive: "));
      }
      if(i == 107){
        printlnPwmDrive(ConfigurationPWM5.Data);
      }
      if(i == 114){
        Serial.print(F(" PWM0:"));
        Serial.print(pwm0);
      }
      if(i == 115){
        Serial.print(F(" PWM1:"));
        Serial.print(pwm1);
      }
      if(i == 138){
        Serial.print(F(" PWM2:"));
        Serial.print(pwm2);
      }
      if(i == 139){
        Serial.print(F(" PWM3:"));
        Serial.print(pwm3);
      }
      if(i == 154){
        Serial.print(F(" PWM4:"));
        Serial.print(pwm4);
      }
      if(i == 155){
        Serial.print(F(" PWM5:"));
        Serial.println(pwm5);
      }
    }
    if(i == 170){
      Serial.print(F(" RPM0:"));
      Serial.print(rpm0);
    }
    if(i == 171){
      Serial.print(F(" RPM1:"));
      Serial.print(rpm1);
    }
    if(i == 178){
      Serial.print(F(" RPM2:"));
      Serial.print(rpm2);
    }
    if(i == 179){
      Serial.print(F(" RPM3:"));
      Serial.print(rpm3);
    }
    if(i == 194){
      Serial.print(F(" RPM4:"));
      Serial.print(rpm4);
    }
    if(i == 195){
      Serial.print(F(" RPM5:"));
      Serial.println(rpm5);
      s = 0;
      fs = 0;
    }
  }  
	zpozdeni = micros() - start;
	if(zpozdeni > 500){
    if(!gui){
      Serial.print(i);
      Serial.print(F("-"));
      Serial.println(zpozdeni);
    } else {
      Serial.write(6);
      Serial.print(F("!"));
      Serial.write(i);
      serialWriteLong(zpozdeni);
    }
	}
}

void printlnPwmDrive(PWMConfiguration &conf){
  // pwmDrive: 0 - const, 1 - analogInput, 2 - T0, 3 - T1, 4  - (T1+T2)/2
  switch (conf.pwmDrive) {
    case 0:
      Serial.println(F("constant speed"));
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

void printStatus(CommandParameter &parameters)
{
  s = 1;
}

void printFullStatus(CommandParameter &parameters)
{
  s = 1;
  fs = 1;
}

void printDelay(byte i, unsigned long d){
  if(!gui){
    Serial.print(F("!"));
    Serial.println(d);
  } else {
    Serial.print(F("#"));
    Serial.write(5);
    Serial.write(i);
    serialWriteLong(d);
  }
}

