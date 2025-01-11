void loop(){
  // tasks executed every iteration (5ms)
  wdt_reset();  // reset the WDT timer
  countT(i % NUMBER_OF_THERMISTORS);
  calculateRPMs();
  setPwm();
  checkSave();
  startNextADC();

  if(i == 0 || i == 128){
    if(gui){
      guiUpdate();
    }
    decrementPwmDisabled();
  }

  #ifdef CALIBRATE_THERMISTORS
  if((i & 15) == 0){ // every 80ms
    calibrateRNominal();
    calibrateB();
  }
  #endif

  SerialCommandHandler.Process();

  now = micros();
  zpozdeni = now - start;

#ifdef TIMING_DEBUG
  timingDebug();
#endif

  if(zpozdeni >= WARN_MICROSECONDS){
    printDelay(i, zpozdeni);
  }
  if(zpozdeni < ITERATION_MICROSECONDS){
    delayMicrosecondsIncludingInterrupts(ITERATION_MICROSECONDS - zpozdeni);
  }

  if(zpozdeni < DELAY_THRESHOLD){
    start = start + ITERATION_MICROSECONDS;
  } else {
    printDelayThreshold();
    delay(100);       //wait for empty serial buffer and next pid iteration
    start = micros(); //too big delay, restart main loop timer
  }

  i++;
  if(i == 0){
    j++;
  }
}

void delayMicrosecondsIncludingInterrupts(unsigned long us)
{
  unsigned long startWaitingTime = micros();
  unsigned long stopWaitingTime = startWaitingTime + us;
  unsigned long now;

  if (us < 20) return;

  if (startWaitingTime < stopWaitingTime) {
    do{
      now = micros();
    } while(startWaitingTime <= now && now < stopWaitingTime);
  } else {
    do{
      now = micros();
    } while(startWaitingTime <= now || now < stopWaitingTime);
  }
}

#ifdef TIMING_DEBUG
void timing(){
    timeCounting = 256;
    timeInCode = 0;
    to1000 = 0;
    to1500 = 0;
    to2000 = 0;
    to2500 = 0;
    to3000 = 0;
    over3000 = 0;
    timeTotal = start;
}

void timingDebug(){
  if(timeCounting > 0){
    timeCounting--;
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

    if(timeCounting == 0){
      timeTotal = micros() - timeTotal;
      printTimingResult();
    }

    now = micros();
    zpozdeni = now - start;
  }
}
#endif
