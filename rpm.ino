#define fanSensor5FilterDefinition B0000011

volatile byte fanSensor5Filter = 0;
volatile byte interruptFlag = 0;

// overflow on timer2 interrupt handler
ISR(TIMER2_OVF_vect){

#ifdef TIMING_DEBUG
  unsigned long startInterrupt = micros();
#endif

//  byte fanSensor5 = digitalRead(RPMSENSOR5);
  byte fanSensor5 = (PINC >> 5) & 1;
  fanSensor5Filter = ((fanSensor5Filter << 1) | fanSensor5) & fanSensor5FilterDefinition;
  if(fanSensor5Filter == 0){
    fanSensor5Value = 0;
    // disable timer2 overflow interrupt
    TIMSK2 &= B11111110;
  }
  if(fanSensor5Filter == fanSensor5FilterDefinition){
    fanSensor5Value = 1;
    // disable timer2 overflow interrupt
    TIMSK2 &= B11111110;
  }

#ifdef TIMING_DEBUG
  timeInInterrupt = timeInInterrupt + micros() - startInterrupt;
#endif
}

// overflow on timer1 interrupt handler
ISR(TIMER1_OVF_vect){
  // enable timer2 overflow interrupt
  TIMSK2 |= B00000001;
}

void readRPMsensors(){
#ifdef TIMING_DEBUG
  unsigned long startrpm = micros();
#endif
/*
  byte sensor0 = digitalRead(RPMSENSOR0);
  byte sensor1 = digitalRead(RPMSENSOR1);
  byte sensor2 = digitalRead(RPMSENSOR2);
  byte sensor3 = digitalRead(RPMSENSOR3);
  byte sensor4 = digitalRead(RPMSENSOR4);
*/
  byte sensor0 = PIND >> 7;
  byte sensor1 = PINB & 1;
  byte sensor2 = (PIND >> 2) & 1;
  byte sensor3 = (PIND >> 4) & 1;
  byte sensor4 = (PINB >> 4) & 1;
  byte sensor5 = fanSensor5Value;

  sensor0 = filterSensor(&sensor0Filter, sensor0, lastSensor0);
  sensor1 = filterSensor(&sensor1Filter, sensor1, lastSensor1);
  sensor2 = filterSensor(&sensor2Filter, sensor2, lastSensor2);
  sensor3 = filterSensor(&sensor3Filter, sensor3, lastSensor3);
  sensor4 = filterSensor(&sensor4Filter, sensor4, lastSensor4);

//  Serial.println(sensor0);

  writeFanSensorHistory(0, fanSensorSums0, fanSensorPosition, lastSensor0 ^ sensor0);
  writeFanSensorHistory(1, fanSensorSums1, fanSensorPosition, lastSensor1 ^ sensor1);
  writeFanSensorHistory(2, fanSensorSums2, fanSensorPosition, lastSensor2 ^ sensor2);
  writeFanSensorHistory(3, fanSensorSums3, fanSensorPosition, lastSensor3 ^ sensor3);
  writeFanSensorHistory(4, fanSensorSums4, fanSensorPosition, lastSensor4 ^ sensor4);
  writeFanSensorHistory(5, fanSensorSums5, fanSensorPosition, lastSensor5 ^ sensor5);
  fanSensorPosition++;
  if(fanSensorPosition >= FANSENSOR_HISTORY_SIZE){
    fanSensorPosition = 0;
  }

  if(rmpToMainboard){
    digitalWrite(LED_OUT, sensor5);
  } else {
    digitalWrite(LED_OUT, sensor4);
  }

  lastSensor0 = sensor0;
  lastSensor1 = sensor1;
  lastSensor2 = sensor2;
  lastSensor3 = sensor3;
  lastSensor4 = sensor4;
  lastSensor5 = sensor5;

#ifdef TIMING_DEBUG
  readRPM = readRPM + micros() - startrpm;
#endif
}

byte filterSensor(byte *sensorFilter, byte sensorValue, byte lastSensorValue){
  *sensorFilter = ((*sensorFilter << 1) + sensorValue) & sensorFilterMax;
  if(*sensorFilter == 0) return 0;
  if(*sensorFilter == sensorFilterMax) return 1;
  return lastSensorValue;
}


byte fanSensorCounter[6];
void writeFanSensorHistory(byte fanSensorNumber, byte fanSensorSums[], unsigned int position, byte value){
  byte sensorCounter = fanSensorCounter[fanSensorNumber];
  sensorCounter = sensorCounter + value;
  byte positionInByte = position & B00011111;
  if((position == (FANSENSOR_HISTORY_SIZE - 1)) || (positionInByte == B00011111)){
    fanSensorSums[position >> 5] = sensorCounter;
    sensorCounter = 0;
  }
  fanSensorCounter[fanSensorNumber] = sensorCounter;
}

unsigned int countRPM(byte fanSensorSums[]){
  unsigned int result = 0;
  for(byte i = 0; i < FANSENSOR_SUMS_FIELD; i++){
    result = result + fanSensorSums[i];
  }
  result = result << FANSENSOR_SHIFT_MULTIPLIER;
  //  result = result / 10;
  result = ((unsigned long)result * 205) >> 11;  //divide by 10 - accuracy better than 0.1% and 2x faster than x / 10
  result = result * 10;
  return result;
}

word countRPMs(){
  rpm0 = countRPM(fanSensorSums0);
  rpm1 = countRPM(fanSensorSums1);
  rpm2 = countRPM(fanSensorSums2);
  rpm3 = countRPM(fanSensorSums3);
  rpm4 = countRPM(fanSensorSums4);
  rpm5 = countRPM(fanSensorSums5);
}

