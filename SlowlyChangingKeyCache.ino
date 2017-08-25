
template <class TData, byte sizeShift> class SlowlyChangingKeyCache
{
  struct Record
  {
    TData value;
  } record;

  byte mask = (B00000001 << sizeShift) - 1;
  Record buffer[B00000001 << sizeShift];
  short lowLimit = 0;
  short highLimit = 0;

public:
  void put(short key, TData value){
    if(key > highLimit){
      if(key - highLimit >= B00000001 << sizeShift){
        highLimit = key;
        lowLimit = key;
      } else {
        while(highLimit < key){
          highLimit = highLimit + 1;
          buffer[highLimit & mask].value = 0;
          if((highLimit & mask) == (lowLimit & mask)){
            lowLimit = lowLimit + 1;
          }
        }
      }
    }
    if(key < lowLimit){
      if(lowLimit - key >= B00000001 << sizeShift){
        highLimit = key;
        lowLimit = key;
      } else {
        while(lowLimit > key){
          lowLimit = lowLimit - 1;
          buffer[lowLimit & mask].value = 0;
          if(highLimit & mask == lowLimit & mask){
            highLimit = highLimit - 1;
          }
        }
      }
    }
    buffer[key & mask].value = value;
//    printStatus();
  };
  
  TData get(short key){
    if(key >= lowLimit && key <= highLimit){
      return buffer[key & mask].value;
    }
    return 0;
  };

  void printStatus(){
    Serial.print(F("cache"));
    if(gui){
      for(short i = lowLimit; i <= highLimit; i++){
        byte buff[4];
        buff[0] = lowByte(i);
        buff[1] = lowByte(i >> 8);
 
        TData data = get(i);
        buff[2] = lowByte(data);
        buff[3] = lowByte(data >> 8);
        Serial.write(buff, 4);
      }
    } else {
      Serial.println(F(""));
      for(short i = lowLimit; i <= highLimit; i++){
        Serial.print(i);
        Serial.print(F(" - "));
        Serial.println(get(i));
      }
    }
  }
  
};

SlowlyChangingKeyCache<short, 6> cacheT0;
SlowlyChangingKeyCache<short, 5> cacheT1;


