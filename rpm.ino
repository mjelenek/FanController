#define fanSensor5FilterDefinition B0000011

volatile byte fanSensor5Filter = 0;

// overflow on timer2 interrupt handler
ISR(TIMER2_OVF_vect){
//  byte fanSensor5 = digitalRead(RPMSENSOR5);
  byte fanSensor5 = (PINC >> 5) & 1;
  fanSensor5Filter = ((fanSensor5Filter << 1) | fanSensor5) & fanSensor5FilterDefinition;
  if(fanSensor5Filter == 0){
    fanSensor5Value = 0;
    TIMSK2 &= B11111110;    // disable timer2 overflow interrupt
  }
  if(fanSensor5Filter == fanSensor5FilterDefinition){
    fanSensor5Value = 1;
    TIMSK2 &= B11111110;    // disable timer2 overflow interrupt
  }
}

// overflow on timer1 interrupt handler
ISR(TIMER1_OVF_vect){
  static byte interruptCounter = 0;
  static unsigned int fanSensorPosition = 0;

  switch (interruptCounter) {
    case 0:
      readRPMsensor(0, fanSensorPosition, PIND >> 7);
      readRPMsensor(1, fanSensorPosition, PINB & 1);
      break;
    case 1:
      readRPMsensor(2, fanSensorPosition, (PIND >> 2) & 1);
      readRPMsensor(3, fanSensorPosition, (PIND >> 4) & 1);
      break;
    case 2:
      readRPMsensor(4, fanSensorPosition, (PINB >> 4) & 1);
      readRPMsensor(5, fanSensorPosition, fanSensor5Value);
      break;
    case 3:
      TIMSK2 |= B00000001;    // enable timer2 overflow interrupt
      fanSensorPosition++;
      if(fanSensorPosition >= FANSENSOR_HISTORY_SIZE){
        fanSensorPosition = 0;
      }
      break;
  }
  interruptCounter = (interruptCounter + 1) & B00000011;
}

void readRPMsensor(byte fanNumber, unsigned int fanSensorPosition, byte sensorValue){
  #define sensorFilterMax B00000011
  static byte sensorFilters[6];
  static byte lastSensors[6];

  sensorFilters[fanNumber] = ((sensorFilters[fanNumber] << 1) + sensorValue) & sensorFilterMax;
  sensorValue = lastSensors[fanNumber];
  if(sensorFilters[fanNumber] == 0) sensorValue = 0;
  if(sensorFilters[fanNumber] == sensorFilterMax) sensorValue = 1;

//  sensorValue = filterSensor(fanNumber, sensorValue, lastSensors[fanNumber]);
  writeFanSensorHistory(fanNumber, fanSensorSums[fanNumber], fanSensorPosition, lastSensors[fanNumber] ^ sensorValue);
  if(rmpToMainboard == fanNumber){
    digitalWrite(LED_OUT, sensorValue);
  }
  lastSensors[fanNumber] = sensorValue;
}

/*
byte filterSensor(byte fanNumber, byte sensorValue, byte lastSensorValue){
  #define sensorFilterMax B00000011
  static byte sensorFilters[6];
  sensorFilters[fanNumber] = ((sensorFilters[fanNumber] << 1) + sensorValue) & sensorFilterMax;
  if(sensorFilters[fanNumber] == 0) return 0;
  if(sensorFilters[fanNumber] == sensorFilterMax) return 1;
  return lastSensorValue;
}
*/

void writeFanSensorHistory(byte fanSensorNumber, byte fanSensorSums[], unsigned int position, byte value){
  static byte fanSensorCounter[6];
  byte sensorCounter = fanSensorCounter[fanSensorNumber];
  sensorCounter = sensorCounter + value;
  byte positionInByte = position & B11111111;
  if((position == (FANSENSOR_HISTORY_SIZE - 1)) || (positionInByte == B11111111)){
    fanSensorSums[position >> 8] = sensorCounter;
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
  rpm0 = countRPM(fanSensorSums[0]);
  rpm1 = countRPM(fanSensorSums[1]);
  rpm2 = countRPM(fanSensorSums[2]);
  rpm3 = countRPM(fanSensorSums[3]);
  rpm4 = countRPM(fanSensorSums[4]);
  rpm5 = countRPM(fanSensorSums[5]);
}

