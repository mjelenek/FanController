#define __PROG_TYPES_COMPAT__

// Only supported for AVR micros because we use the special EEMEM directive
// to automatically allocated memory in the eeprom. 
#if defined(__AVR__)

#include <avr/eeprom.h>
#include <avr/crc16.h>

template <class TData> class EEPROMStore
{
  // The Data stored in the eprom. The EEMEM complier attribute instructs
  // the complier to locate this variable in the eeprom. 
  struct EEMEM CEEPROMData
  {
    uint16_t m_uChecksum;
    TData m_UserData;

  } m_EEPROMData;

public:

  CEEPROMData Data;
//  TData Data;

  EEPROMStore()
  {
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
    Data.m_uChecksum = CalculateChecksum(Data.m_UserData);
    CEEPROMData StoredVersion;
    if (!Load(StoredVersion) || StoredVersion.m_uChecksum != Data.m_uChecksum || memcmp(&StoredVersion.m_UserData, &Data.m_UserData, sizeof(TData)) != 0)
    {
//      int i = eeprom_interrupt_write_block(&Data.m_uChecksum, &m_EEPROMData.m_uChecksum, sizeof(Data.m_uChecksum));
//      if(i != 0) return i;
      int i = eeprom_interrupt_write_block(&Data, &m_EEPROMData, sizeof(CEEPROMData));
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
    eeprom_read_block(&Result, (const void *)&m_EEPROMData, sizeof(CEEPROMData));
    uint16_t uChecksum = CalculateChecksum(Result.m_UserData);
    return uChecksum == Result.m_uChecksum;
  }

  uint16_t CalculateChecksum(const TData &TestData) const
  {
    uint16_t uChecksum = 0;
    const uint8_t *pRawData = reinterpret_cast<const uint8_t *>(&TestData);
    size_t szData = sizeof(TestData);

    while (szData--){
      uChecksum = _crc16_update(uChecksum, *pRawData++);
    }

    return uChecksum;
  }
};


#define BUFF_STORE_SIZE 6
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
volatile void* eeprom_dest;
volatile void* eeprom_data;
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
  return(0);
}

// enable interrupt and start writing bufferToStore
void startWritingBufferByISR(){
  EECR |= (1<<EERIE);   //enable the eeprom ready interrupt.  It will now take over all the data
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
      eeprom_dest = bufferToStore[bufferToStoreActual].eeprom_dest;             //point to the first location of data destination
      eeprom_data = bufferToStore[bufferToStoreActual].eeprom_data;             //point to the first location of data source
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
    
  EECR |= (1<<EEMPE);       //first enable master write
  EECR |= (1<<EEPE);        //now enable the actual write.  This sequence is required by the HW.
  
  eeprom_data_size--;       //decrement the number of bytes left
}


#endif
