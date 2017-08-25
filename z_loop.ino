void loop(){
  byte part_8 = i & B00000111; // cycles from 0 to 7;
  byte part_64 = (i & B00111111) >> 3; // cycles from 0 to 8 with step i = 8;

  // one case each 8 iterations (8ms)
  switch (part_8) {
    case 0:
      if(ConfigurationPWM0.Data.pwmDrive == 1){
//        sensorValue0Averaged = readAnalogValueAndSmooth(sensorValue0Averaged, VOLTAGEINPUT0, readRPMsensors);
        setPwm0();
      } else {
        readRPMsensors();
      }
      break;
    case 1:
      if(ConfigurationPWM1.Data.pwmDrive == 1){
//        sensorValue1Averaged = readAnalogValueAndSmooth(sensorValue1Averaged, VOLTAGEINPUT1, readRPMsensors);
  	    setPwm1();
      } else {
        readRPMsensors();
      }
      break;
    case 2:
      if(ConfigurationPWM2.Data.pwmDrive == 1){
//        sensorValue2Averaged = readAnalogValueAndSmooth(sensorValue2Averaged, VOLTAGEINPUT2, readRPMsensors);
  	    setPwm2();
      } else {
        readRPMsensors();
      }
      break;
    case 3:
      if(ConfigurationPWM3.Data.pwmDrive == 1){
//        sensorValue3Averaged = readAnalogValueAndSmooth(sensorValue3Averaged, VOLTAGEINPUT3, readRPMsensors);
  	    setPwm3();
      } else {
        readRPMsensors();
      }
      break;
    case 4:
      if(ConfigurationPWM4.Data.pwmDrive == 1){
//        sensorValue4Averaged = readAnalogValueAndSmooth(sensorValue4Averaged, VOLTAGEINPUT4, readRPMsensors);
  	    setPwm4();
      } else {
        readRPMsensors();
      }
      break;
    case 5:
      if(ConfigurationPWM5.Data.pwmDrive == 1){
        readRPMsensors();
//        sensorValue4Averaged = readAnalogValueAndSmooth(sensorValue4Averaged, VOLTAGEINPUT4, readRPMsensors);
  	    setPwm5();
      } else {
        readRPMsensors();
      }
      break;
    case 6:
      // one case each 64 iterations (64ms)
      switch (part_64) {
        case 0:
          readT0();  
          break;
        case 1:
          readRPMsensors();  
          countT0();  
          break;
        case 2:
          readT1();  
          break;
        case 3:
          readRPMsensors();  
          countT1();  
          break;
        case 4:
          readRPMsensors();  
          rpm0 = countRPM(fanSensorSums0);
          rpm1 = countRPM(fanSensorSums1);
          rpm2 = countRPM(fanSensorSums2);
          break;
        case 5:
          readRPMsensors();  
          rpm3 = countRPM(fanSensorSums3);
          rpm4 = countRPM(fanSensorSums4);
          rpm5 = countRPM(fanSensorSums5);
          break;
        case 6:
          readRPMsensors();  
          setPwm();
          break;
        case 7:
          readRPMsensors();
          pid0.Compute();
      }
      break;
    case 7:
      readRPMsensors();  
      SerialCommandHandler.Process();
  }

  print();

  if(i == 255){
/*    
    if(gui){
      guiUpdate();
    }
*/
    j++;
    if(j == 2 && gui){

      guiUpdate();
    }
    if(j == 4){
      decrementPwmDisabled();
      j = 0;
    }
  }

  i++;

  // enable timer2 overflow interrupt
  TIMSK2 |= B00000001;

  now = micros();
  zpozdeni = now - start;
  if(zpozdeni >= ITERATION_MICROSECONDS){

    if(!gui){
      Serial.print(F("!"));
      Serial.println(zpozdeni);
    } else {
      Serial.write(6);
      Serial.print(F("!"));
      Serial.write(i - 1);
      serialWriteLong(zpozdeni);
    }
  } else {
    delayMicroseconds(ITERATION_MICROSECONDS - zpozdeni);  //wait for next iteration
  }

  if(zpozdeni < DELAY_THRESHOLD){
    start = start + ITERATION_MICROSECONDS;
  } else {  //too big delay, restart main loop timer
    if(!gui){
      Serial.println(F("!delayed"));
    } else {
      Serial.write(7);
      Serial.print(F("delayed"));
    }
    delay(6);  //wait for empty serial buffer, for 115200 b/s delay 6ms is enough
    start = micros();
  }
}

