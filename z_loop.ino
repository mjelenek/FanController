void loop(){
  byte part_16 = i & B00001111; // cycles from 0 to 15;
  byte part_8 = i & B00000111; // cycles from 0 to 7;
  byte part_4 = i & B00000011; // cycles from 0 to 3;

  // tasks executed every iteration (2ms)
  countRPMs();
  setPwm();

  // one case each 16 iterations (32ms)
  switch (part_16) {
    case 0:
      countT0();  
      break;
    case 2:
      countT1();  
      break;
    case 4:
    case 12:
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
        to400 = 0;
        to600 = 0;
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
  } else
  if(zpozdeni >= 1200){
    over1200++;
  }

  if(timeCounting > 0){
    if(zpozdeni >= WARN_MICROSECONDS_DEBUG){
      Serial.print(F("!"));
      Serial.print(i);
      Serial.print(F("-"));
      Serial.println(zpozdeni);
    }
  }

  timeInCode = timeInCode + zpozdeni;
  now = micros();
  zpozdeni = now - start;
#endif

  if(zpozdeni >= WARN_MICROSECONDS){
    printDelay(i, zpozdeni);
  }
/*
  if(zpozdeni >= ITERATION_MICROSECONDS){
    printDelay(i, zpozdeni);
  } else {
    delayMicroseconds(ITERATION_MICROSECONDS - zpozdeni);  //wait for next iteration
  }
*/
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

