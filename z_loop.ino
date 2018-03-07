void loop(){
  part_32 = i & B00011111; // cycles from 0 to 31;

  // tasks executed every iteration (2ms)
  countRPMs();
  setPwm();

  // one case each 32 iterations (64ms)
  // 3, 7, 11, 15, 19, 23 is occupied by calculating PWM
  // 4, 8, 12, 16, 20, 24 is occupied by calculating PID
  switch (part_32) {
    case 1:
      countT0();  
      break;
    case 2:
      countT1();  
      break;
    case 5:
    case 13:
    case 21:
    case 29:
      SerialCommandHandler.Process();
      break;
    default:
      ;
  }

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
    printDelay(i, zpozdeni);
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
      Serial.print(F("!"));
      Serial.print(part_32);
      Serial.print(F("-"));
      Serial.println(zpozdeni);
    }

    timeInCode = timeInCode + zpozdeni;
    now = micros();
    zpozdeni = now - start;
  }
}
#endif

