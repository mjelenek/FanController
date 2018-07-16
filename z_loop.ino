void loop(){
  // tasks executed every iteration (5ms)
  countT(i % NUMBER_OF_THERMISTORS);
  countRPMs();
  setPwm();
  SerialCommandHandler.Process();

  if(i == 0 || i == 128){
    if(gui){
      guiUpdate();
    }
    decrementPwmDisabled();
  }

  if(i == 0){
    #ifdef TIMING_DEBUG
      timingDebugStart();
    #endif
  }

  now = micros();
  zpozdeni = now - start;

#ifdef TIMING_DEBUG
  timingDebug();
#endif

  if(zpozdeni >= WARN_MICROSECONDS){
    printDelay(i, zpozdeni);
  }
  if(zpozdeni < ITERATION_MICROSECONDS){
    delayMicroseconds(ITERATION_MICROSECONDS - zpozdeni);  //wait for next iteration
  }

  if(zpozdeni < DELAY_THRESHOLD){
    start = start + ITERATION_MICROSECONDS;
  } else {
    printDelayThreshold();
    delay(100);       //wait for empty serial buffer and next pid iteration
    init_pid();
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
    to1000 = 0;
    to1500 = 0;
    to2000 = 0;
    to2500 = 0;
    to3000 = 0;
    over3000 = 0;
    timeTotal = micros();
  }
}

void timingDebug(){
  if(timeCounting > 0){
    if(zpozdeni < 1000){
      to1000++;
    } else
    if(zpozdeni < 1500){
      to1500++;
    } else
    if(zpozdeni < 2000){
      to2000++;
    } else
    if(zpozdeni < 2500){
      to2500++;
    } else
    if(zpozdeni < 3000){
      to3000++;
    } else {
      over3000++;
    }

    if(zpozdeni >= WARN_MICROSECONDS_DEBUG){
      printDelay(i, zpozdeni);
    }

    timeInCode = timeInCode + zpozdeni;
    now = micros();
    zpozdeni = now - start;
  }
}
#endif

