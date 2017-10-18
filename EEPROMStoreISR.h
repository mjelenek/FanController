#define __PROG_TYPES_COMPAT__

// Only supported for AVR micros because we use the special EEMEM directive
// to automatically allocated memory in the eeprom. 
#if defined(__AVR__)

#include <avr/eeprom.h>
#include <avr/crc16.h>

template <class TData> class EEPROMStore
{
  // The data stored in the eprom. The EEMEM complier attribute instructs
  // the complier to locate this variable in the eeprom. 
  struct EEMEM CEEPROMData
  {
    uint16_t m_uChecksum;
    TData m_UserData;

  } m_EEPROMData;

  uint16_t uChecksum;

public:

  TData Data;

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
      memcpy(&Data, &WorkingCopy.m_UserData, sizeof(TData));
      return true;
    }

    return false;
  }

  byte Save()
  {
    // We only save if the current version in the eeprom doesn't match the data we plan to save. 
    // This helps protect the eeprom against save called many times within the arduino loop,
    // though it makes things a little slower. 
    uChecksum = CalculateChecksum(Data);
    CEEPROMData StoredVersion;
    if (!Load(StoredVersion) || StoredVersion.m_uChecksum != uChecksum || memcmp(&StoredVersion.m_UserData, &Data, sizeof(Data)) != 0)
    {
      int i = eeprom_interrupt_write_block(&uChecksum, &m_EEPROMData.m_uChecksum, sizeof(uChecksum));
      if(i != 0) return i;
      i = eeprom_interrupt_write_block(&Data, &m_EEPROMData.m_UserData, sizeof(Data));
      if(i != 0) return i;
      return 0; 
    }
    return 0; 
  }

  void Reset()
  {
    Data.Reset();
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

    while (szData--)
    {
      uChecksum = _crc16_update(uChecksum, *pRawData++);
    }

    return uChecksum;
  }
};


#define BUFF_STORE_SIZE 12
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


//example test program
/*
unsigned char data_buffer[32];    //arbitrary size is 32 bytes of data.  This ought to do for this purpose
unsigned char e_log[50] [32]__attribute__((section(".eeprom"))) ; //no initialization,  size is 32*50 = 1600 bytes out of 4096 in the atmega128
#include <avr/eeprom.h>

void test_eeprom_write(void)
{
  unsigned char counter;
  
  //fill the data buffer with an arbitrary number
  for (counter = 0; counter<sizeof(data_buffer); counter++) //fill the buffer with an arbitrary number
  {
    data_buffer[counter] = counter+1;
  }
  
  //write to the eeprom
  
  
  //write the 32 bytes to the first block in the eeprom array
  while(eeprom_busy||(EECR & (1<<EEWE)));   //explicitly wait for eeprom to be free & idle (hopefully rarely necessary)
  eeprom_interrupt_write_block(data_buffer, &e_log[0], sizeof(data_buffer));
  while(eeprom_busy); //wait for the eeprom to not be busy
  
  //refill the buffer with something else
  for (counter = 0; counter<sizeof(data_buffer); counter++) //fill the buffer with zeros
  {
    data_buffer[counter] = 0;
  }
  
  // read back the data
  eeprom_read_block(data_buffer, &e_log[0], sizeof(data_buffer)); //read back the data
  // that is it
*/





//this initializes the eeprom writing data, and enables the eeprom interrupt.
//the eeprom interrupt fires continuously while it is ready to accept data
// after that the eeprom interrupt clicks thru the data as fast as the HW allows, then shuts off

//"source" is a pointer to the data buffer in SRAM
//"dest" is a pointer to the destination in EEPROM
//"num_bytes" is sizeof() the data buffer

//returns a (-1) if the buffer is full
//returns a (-2) if the eeprom is busy with a write function (e.g. something else is using the eprom,
//     or the last write command is still in process just after the "eeprom_busy" flag is cleared


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
//  if (EECR & (1<<EEPE)) return (-2);    //check if something else might have the eeprom busy
  
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

// start ISR and writing itself
void startWritingBufferByISR(){
  EECR |= (1<<EERIE);   //enable the eeprom ready interrupt.  It will now take over all the data shifting
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
