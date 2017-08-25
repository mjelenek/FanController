void loop(){
  byte part_8 = i & B00000111; // cycles from 0 to 7;
  byte part_64 = (i & B00111111) >> 3; // cycles from 0 to 8 with step i = 8;

  // one case each 8 iterations (8ms)
  switch (part_8) {
    case 0:
      if(ConfigurationPWM0.Data.pwmDrive == 1){
        sensorValue0Averaged = readAnalogValueAndSmooth(sensorValue0Averaged, VOLTAGEINPUT0);
        setPwm0();
      }
      break;
    case 1:
      if(ConfigurationPWM1.Data.pwmDrive == 1){
        sensorValue1Averaged = readAnalogValueAndSmooth(sensorValue1Averaged, VOLTAGEINPUT1);
        setPwm1();
      }
      break;
    case 2:
      if(ConfigurationPWM2.Data.pwmDrive == 1){
        sensorValue2Averaged = readAnalogValueAndSmooth(sensorValue2Averaged, VOLTAGEINPUT2);
        setPwm2();
      }
      break;
    case 3:
      if(ConfigurationPWM3.Data.pwmDrive == 1){
        sensorValue3Averaged = readAnalogValueAndSmooth(sensorValue3Averaged, VOLTAGEINPUT3);
        setPwm3();
      }
      break;
    case 4:
      if(ConfigurationPWM4.Data.pwmDrive == 1){
        sensorValue4Averaged = readAnalogValueAndSmooth(sensorValue4Averaged, VOLTAGEINPUT4);
        setPwm4();
      }
      if(ConfigurationPWM5.Data.pwmDrive == 1){
        setPwm5();
      }
      break;
    case 5:
      break;
    case 6:
      // one case each 64 iterations (64ms)
      switch (part_64) {
        case 0:
          readT0();  
          break;
        case 1:
          countT0();  
          break;
        case 2:
          readT1();  
          break;
        case 3:
          countT1();  
          break;
        case 4:
          rpm0 = countRPM(fanSensorSums0);
          rpm1 = countRPM(fanSensorSums1);
          rpm2 = countRPM(fanSensorSums2);
          break;
        case 5:
          rpm3 = countRPM(fanSensorSums3);
          rpm4 = countRPM(fanSensorSums4);
          rpm5 = countRPM(fanSensorSums5);
          break;
        case 6:
          setPwm();
          break;
        case 7:
//          pid0.Compute();
        ;
      }
      break;
    case 7:
      SerialCommandHandler.Process();
  }


  if(i == 255){
    j++;
    if(j == 2 && gui){
      guiUpdate();
    }
    if(j == 4){
      decrementPwmDisabled();
      j = 0;
      
      #ifdef TIMING_DEBUG
      if(timeCounting > 0){
        timeCounting--;
        if(timeCounting == 0){
          now = micros();
          timeTotal = now - timeTotal;
          printTimingResult();
        }
      }
      if(timeCountingStartFlag > 0){
        timeCounting = 1;
        timeCountingStartFlag = 0;
        timeInCode = 0;
        timeInInterrupt = 0;
        to50 = 0;
        to100 = 0;
        to150 = 0;
        to200 = 0;
        to300 = 0;
        to400 = 0;
        to500 = 0;
        to600 = 0;
        over600 = 0;
        readRPM = 0;
        timeTotal = micros();
      }
      #endif
       
    }
  }


  now = micros();
  zpozdeni = now - start;

#ifdef TIMING_DEBUG
  if(zpozdeni < 50){
    to50++;
  } else
  if(zpozdeni < 100){
    to100++;
  } else
  if(zpozdeni < 150){
    to150++;
  } else
  if(zpozdeni < 200){
    to200++;
  } else
  if(zpozdeni < 300){
    to300++;
  } else
  if(zpozdeni < 400){
    to400++;
  } else
  if(zpozdeni < 500){
    to500++;
  } else
  if(zpozdeni < 600){
    to600++;
  } else
  if(zpozdeni >= 600){
    over600++;
  }

  timeInCode = timeInCode + zpozdeni;
#endif

  // enable timer2 overflow interrupt
//  TIMSK2 |= B00000001;
  
/*
  if(zpozdeni >= WARN_MICROSECONDS){
    printDelay(i, zpozdeni);
  }
*/
  if(zpozdeni >= ITERATION_MICROSECONDS){
    printDelay(i, zpozdeni);
  } else {
    delayMicroseconds(ITERATION_MICROSECONDS - zpozdeni);  //wait for next iteration
  }

  if(zpozdeni < DELAY_THRESHOLD){
    start = start + ITERATION_MICROSECONDS;
  } else {
    printDelayThreshold();
    delay(6);  //wait for empty serial buffer, for 115200 b/s delay 6ms is enough
    start = micros(); //too big delay, restart main loop timer
  }

  i++;
}

