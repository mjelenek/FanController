void loop(){
  byte part_4 = i & B00000011; // cycles from 0 to 3;
  byte part_2 = i & B00000001; // cycles from 0 to 3;

  // one case each 8 iterations (8ms)
  switch (part_4) {
    case 0:
      countT0();  
      countT1();  
      break;
    case 1:
      rpm0 = countRPM(lastFanRpmSensorTime0, fanRpmSensorTimes0) / 10;
      rpm0 = rpm0 * 10;
      rpm1 = countRPM(lastFanRpmSensorTime1, fanRpmSensorTimes1) / 10;
      rpm1 = rpm1 * 10;
      rpm2 = countRPM(lastFanRpmSensorTime2, fanRpmSensorTimes2) / 10;
      rpm2 = rpm2 * 10;
      break;
    case 2:
      rpm3 = countRPM(lastFanRpmSensorTime3, fanRpmSensorTimes3) / 10;
      rpm3 = rpm3 * 10;
      rpm4 = countRPM(lastFanRpmSensorTime4, fanRpmSensorTimes4) / 10;
      rpm4 = rpm4 * 10;
      rpm5 = countRPM(lastFanRpmSensorTime5, fanRpmSensorTimes5) / 10;
      rpm5 = rpm5 * 10;
    case 3:
      SerialCommandHandler.Process();
  }
//  pid0.Compute();
  setPwm();

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
        to900 = 0;
        to1000 = 0;
        over1000 = 0;
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
  if(zpozdeni < 900){
    to900++;
  } else
  if(zpozdeni < 1000){
    to1000++;
  } else
  if(zpozdeni >= 1000){
    over1000++;
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
  if(i == 100){
    i = 0;
  }
}

