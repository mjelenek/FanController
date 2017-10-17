void loop(){
  byte part_8 = i & B00001111; // cycles from 0 to 7;
  byte part_4 = i & B00000011; // cycles from 0 to 3;
  byte part_2 = i & B00000001; // cycles from 0 to 2;

  // one case each 8 iterations (16ms)
  switch (part_8) {
    case 0:
      countT0();  
      break;
    case 1:
      countT1();  
      break;
    case 2:
      rpm[0] = countRPM(lastFanRpmSensorTime0, fanRpmSensorTimes0);
      break;
    case 3:
      rpm[1] = countRPM(lastFanRpmSensorTime1, fanRpmSensorTimes1);
      break;
    case 4:
      rpm[2] = countRPM(lastFanRpmSensorTime2, fanRpmSensorTimes2);
      break;
    case 5:
      rpm[3] = countRPM(lastFanRpmSensorTime3, fanRpmSensorTimes3);
      break;
    case 6:
      rpm[4] = countRPM(lastFanRpmSensorTime4, fanRpmSensorTimes4);
      break;
    case 7:
      rpm[5] = countRPM(lastFanRpmSensorTime5, fanRpmSensorTimes5);
  }
  
  setPwm();

  SerialCommandHandler.Process();

  if(i == 50){
    if(gui){
      guiUpdate();
    }
  }

  if(i == 0){
    j++;
    if(gui){
      guiUpdate();
    }
    if(j == 2){
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
        to500 = 0;
        to800 = 0;
        to1000 = 0;
        to1200 = 0;
        over1200 = 0;
        timeTotal = micros();
      }
      #endif
       
    }
  }


  now = micros();
  zpozdeni = now - start;

#ifdef TIMING_DEBUG
  if(zpozdeni < 500){
    to500++;
  } else
  if(zpozdeni < 800){
    to800++;
  } else
  if(zpozdeni < 1000){
    to1000++;
  } else
  if(zpozdeni < 1200){
    to1200++;
  } else
  if(zpozdeni >= 1200){
    over1200++;
  }

  timeInCode = timeInCode + zpozdeni;
#endif

  if(zpozdeni >= WARN_MICROSECONDS){
    printDelay(i, zpozdeni);
  }
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
//  if(i == 100){
//    i = 0;
//  }
}

