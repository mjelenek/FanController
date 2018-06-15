void loop(){
  part_64 = i & B00111111; // cycles from 0 to 63;

  // tasks executed every iteration (2ms)
  countT();
  countRPMs();
  setPwm();
  checkSerialCommand();
  // one case each 64 iterations (128ms)
  // 1,     9 (... 17,         25,     33)                            is occupied by calculating temperature
  //    6,     14,     22,         30,     38,     46,     54, 62     is occupied by SerialCommandHandler
  // 3, 7, 11, 15, 19, 23 (... 27, 31, 35, 39, 43, 47, 51, 53)        is occupied by calculating PWM
  // 4, 8, 12, 16, 20, 24 (... 28, 32, 36, 40, 44, 48, 52, 54)        is occupied by calculating PID
/*
  switch (part_64) {
    case 10:
    case 18:
    case 26:
    case 34:
    case 42:
    case 50:
    case 61:
      break;
    default:
      ;
  }
*/
  if(i == 0){
    j++;
    if(gui){
      guiUpdate();
    }
    if(j == 2){
      j = 0;
      decrementPwmDisabled();
      #ifdef TIMING_DEBUG
        timingDebugStart();
      #endif
    }
  }

  now = micros();
  zpozdeni = now - start;

#ifdef TIMING_DEBUG
  timingDebug();
#endif

  if(zpozdeni >= WARN_MICROSECONDS){
    printDelay(part_64, zpozdeni);
  }
  if(zpozdeni < ITERATION_MICROSECONDS){
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

#ifdef TIMING_DEBUG
void timingDebugStart(){
  if(timeCounting > 0){
    timeCounting--;
    if(timeCounting == 0){
      timeTotal = micros() - timeTotal;
      printTimingResult();
    }
  }
  if(timeCountingStartFlag > 0){
    timeCounting = 1;
    timeCountingStartFlag = 0;
    timeInCode = 0;
    to400 = 0;
    to600 = 0;
    to800 = 0;
    to1000 = 0;
    to1200 = 0;
    over1200 = 0;
    timeTotal = micros();
  }
}

void timingDebug(){
  if(timeCounting > 0){
    if(zpozdeni < 400){
      to400++;
    } else
    if(zpozdeni < 600){
      to600++;
    } else
    if(zpozdeni < 800){
      to800++;
    } else
    if(zpozdeni < 1000){
      to1000++;
    } else
    if(zpozdeni < 1200){
      to1200++;
    } else {
      over1200++;
    }

    if(zpozdeni >= WARN_MICROSECONDS_DEBUG){
      printDelay(part_64, zpozdeni);
    }

    timeInCode = timeInCode + zpozdeni;
    now = micros();
    zpozdeni = now - start;
  }
}
#endif

