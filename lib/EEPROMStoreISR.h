#define __PROG_TYPES_COMPAT__

// Only supported for AVR micros because we use the special EEMEM directive
// to automatically allocated memory in the eeprom. 
#if defined(__AVR__)

#include <avr/eeprom.h>
#include <util/crc16.h>

template <class CEEPROMData> class EEPROMStore
{
  // The Data stored in the eprom. The EEMEM complier attribute instructs
  // the complier to locate this variable in the eeprom. 
  CEEPROMData* m_EEPROMData;

public:

  CEEPROMData Data;

  EEPROMStore(CEEPROMData* eepromPointer)
  {
    m_EEPROMData = eepromPointer;
    Reset();
    if (!Load())
      Reset();
  }

  bool Load()
  {
    CEEPROMData WorkingCopy;
    if (Load(WorkingCopy))
    {
      memcpy(&Data, &WorkingCopy, sizeof(CEEPROMData));
      return true;
    }

    return false;
  }

  byte Save()
  {
    // We only save if the current version in the eeprom doesn't match the Data we plan to save. 
    // This helps protect the eeprom against save called many times within the arduino loop,
    // though it makes things a little slower. 
    Data.m_uChecksum = CalculateChecksum(reinterpret_cast<const uint8_t *>(&(Data.m_UserData)));
    CEEPROMData StoredVersion;
    if (!Load(StoredVersion) || StoredVersion.m_uChecksum != Data.m_uChecksum || memcmp(&StoredVersion.m_UserData, &(Data.m_UserData), sizeof(CEEPROMData) - sizeof(uint16_t)) != 0)
    {
      int i = eeprom_interrupt_write_block(&Data, m_EEPROMData, sizeof(CEEPROMData));
      if(i != 0) return i;
      return 0; 
    }
    return 0; 
  }

  void Reset()
  {
    Data.m_UserData.Reset();
  }

private:
  bool Load(CEEPROMData &Result)
  {
    eeprom_read_block(&Result, (const void *)m_EEPROMData, sizeof(CEEPROMData));
    uint16_t uChecksum = CalculateChecksum(reinterpret_cast<const uint8_t *>(&(Result.m_UserData)));
    return uChecksum == Result.m_uChecksum;
  }

  uint16_t CalculateChecksum(const uint8_t *pRawData) const
  {
    uint16_t uChecksum = 0;
    size_t szData = sizeof(CEEPROMData) - sizeof(uint16_t);

    while (szData--){
      uChecksum = _crc16_update(uChecksum, *pRawData++);
    }

    return uChecksum;
  }
};


#define BUFF_STORE_SIZE NUMBER_OF_FANS + 1
struct DataToStore
  {
    volatile void* eeprom_data;
    volatile void* eeprom_dest;
    volatile byte eeprom_data_size;
  };

volatile DataToStore bufferToStore[BUFF_STORE_SIZE];
volatile byte bufferToStoreActual = 0;
volatile byte bufferToStoreLast = 0;
volatile byte eeprom_buffer_full = 0;
volatile unsigned char* eeprom_dest;
volatile unsigned char* eeprom_data;
volatile byte eeprom_data_size;
volatile char eeprom_busy = 0;

// this initializes the eeprom writing data, and enables the eeprom interrupt.
// the eeprom interrupt fires continuously while it is ready to accept data
// after that the eeprom interrupt clicks thru the data as fast as the HW allows, then shuts off

//"source" is a pointer to the data buffer in SRAM
//"dest" is a pointer to the destination in EEPROM
//"num_bytes" is sizeof() the data buffer

//returns a (-1) if the buffer is full

//another potential usage of this command is:
//while(eeprom_interrupt_write_block (void* source, void* dest, char num_bytes));
//this will keep retrying the function until it returns a (0) which indicates success
//this avoids explicitly testing that the eeprom is ready for the next operation.


byte nextIndexInBuffer(byte index){
  index++;
  if(index >= BUFF_STORE_SIZE){
    index = 0;
  }
  return index;
}

// enable interrupt and start writing bufferToStore
void startWritingBufferByISR(){
  EECR |= (1<<EERIE);   //enable the eeprom ready interrupt.  It will now take over all the data
}

// actually write pointers to bufferToStore
char eeprom_interrupt_write_block (void* source, void* dest, byte num_bytes)
{

  if(eeprom_buffer_full) return (-1);     //check bufferToStore is not full
  
  bufferToStore[bufferToStoreLast].eeprom_data = source;
  bufferToStore[bufferToStoreLast].eeprom_dest = dest;
  bufferToStore[bufferToStoreLast].eeprom_data_size = num_bytes;
  bufferToStoreLast = nextIndexInBuffer(bufferToStoreLast);
  if(bufferToStoreActual == bufferToStoreLast){
    eeprom_buffer_full = 1;
  }

  if(eeprom_busy){
    //eeprom is still working on a previous request. It will continue with new data added to buffer
    return(0);
  }
  eeprom_busy = 1; 
  
  //now the interrupt will do the rest of the job.
  //    startWritingBufferByISR();
  return(0);
}

//-------------------------------------------------------------------------------------------------------------------------


ISR(EE_READY_vect){

  if (eeprom_data_size == 0){
    //if no bytes left...
    if(bufferToStoreActual == bufferToStoreLast && eeprom_buffer_full == 0){
      // this was the last data from buffer, no other data ready to store
      EECR &= ~(1<<EERIE);    //clear the eeprom ready interrupt
      eeprom_busy = 0;        //release control of the eeprom
      return;
    } else {
      // next data to store are ready in buffer
      eeprom_dest = (unsigned char*)bufferToStore[bufferToStoreActual].eeprom_dest;             //point to the first location of data destination
      eeprom_data = (unsigned char*)bufferToStore[bufferToStoreActual].eeprom_data;             //point to the first location of data source
      eeprom_data_size = bufferToStore[bufferToStoreActual].eeprom_data_size;   //total bytes
      bufferToStoreActual = nextIndexInBuffer(bufferToStoreActual);
      eeprom_buffer_full = 0;
    }
  } else {
    eeprom_data++;          //point to the next location 
    eeprom_dest++;          //point to the next location 
  } 
  
  //process the next byte and move the pointers for the next, if any
  EEAR = (unsigned int)eeprom_dest;           //set the eeprom address
  EEDR = ((unsigned char*)eeprom_data)[0];    //enter the data byte

  char cSREG = SREG;        //store SREG value
  cli();                    //disable interrupts during timed sequence
  EECR |= (1<<EEMPE);       //first enable master write
  EECR |= (1<<EEPE);        //now enable the actual write.  This sequence is required by the HW.
  SREG = cSREG;             //restore SREG value (I-bit)
  
  eeprom_data_size--;       //decrement the number of bytes left
}


#endif
