#ifdef FREE_MEMORY_DEBUG
__attribute__((noinline)) int getPointerToStartOfFreeMemory(){
  extern int  __bss_end;
  extern int* __brkval;
  if (reinterpret_cast<int>(__brkval) == 0) {
    // if no heap use from end of bss section
    if(!gui){
      Serial.println(F("from end of bss"));
    }
    return reinterpret_cast<int>(&__bss_end);
  } else {
    // use from top of stack to heap
    if(!gui){
      Serial.println(F("from top of stack to heap"));
    }
    return reinterpret_cast<int>(__brkval);
  }
}

__attribute__((noinline)) void fillFreeMemoryByZeroes(){
  char cSREG;
  int memPointer = getPointerToStartOfFreeMemory();
  int free_memory;
  free_memory = reinterpret_cast<int>(&free_memory) - memPointer;
  cSREG = SREG;             //store SREG value
  cli();                    //disable interrupts during timed sequence
  while(memPointer < reinterpret_cast<int>(&free_memory)) {
    (*(byte*)(memPointer++)) = 0;
  }
  SREG = cSREG;
}

void freeMem(CommandParameter &parameters)
{
  int f = parameters.NextParameterAsInteger( 0 );
  if(f > 0){
    Serial.print(F("fibbonacci: "));
    Serial.println(fibbonacci(f));
  }
  freemem();  
}

__attribute__((noinline)) void freemem(){
  int notUsedMemory = 0;
  int memPointer = getPointerToStartOfFreeMemory();
  int free_memory;
  free_memory = reinterpret_cast<int>(&free_memory) - memPointer;

  if(!gui){
    Serial.print(F("memPointer address: "));
    Serial.println(memPointer);
    Serial.print(F("free memory address: "));
    Serial.println(reinterpret_cast<int>(&free_memory));
  }
  while(memPointer < reinterpret_cast<int>(&free_memory)) {
    if((*(byte*)(memPointer++)) == 0){
      notUsedMemory++;
    } else {
      break;
    }
  }
  if(!gui){
    Serial.print(F("actual free memory: "));
    Serial.print(free_memory);
    Serial.println(F(" Bytes"));
    Serial.print(F("not used memory: "));
    Serial.print(notUsedMemory);
    Serial.println(F(" Bytes"));
  } else {
    Serial.print(F("!"));
    Serial.write(11);
    Serial.print(F("freemem"));
    serialWriteInt(free_memory);
    serialWriteInt(notUsedMemory);
    Serial.println(F("#"));
  }
}

// function for increase stack test
int fibbonacci(unsigned long input){
  if(input <= 0)
    return 0;
  if(input == 1)
    return 1;
  return (fibbonacci(input - 1) + fibbonacci(input - 2));
}
#endif

