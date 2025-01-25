#include "wiring_private.h"
#if defined(ARDUINO_AVR_NANO)
// settings for Timer1 set to frequency 25000Hz

volatile unsigned long timer1_overflow_count = 0;
volatile unsigned char timer1_overflow_count_extra_byte = 0;

ISR(TIMER1_OVF_vect)
{
  timer1_overflow_count++;
  if (timer1_overflow_count == 0) {
    timer1_overflow_count_extra_byte++;
  }
}

unsigned long micros() {
  unsigned long m;

  uint16_t t;
  uint8_t oldSREG = SREG;
  cli();
  m = timer1_overflow_count;

  t = TCNT1;

  if ((TIFR1 & _BV(TOV1)) && (t < 630)){
    m++;
  }

  SREG = oldSREG;

  return ((m << 5) + (m << 3) + (t >> 4));
}

unsigned long millis() {
  return micros() / 1000;
}

unsigned long seconds() {
  unsigned long m;
  uint8_t oldSREG = SREG;
  
  cli();
  m = (timer1_overflow_count_extra_byte << 24) | (timer1_overflow_count >> 8);

  SREG = oldSREG;
  //     32 * m
  // ~ ---------
  //      3125
  long seconds =  ((m >> 7) + (m >> 9) + (m >> 11) - (m >> 16));
  return seconds + ((seconds >> 6) * microsecondPerSecond) / 15625;
}

// end of settings for Timer set to frequency 25000Hz
#else
// settings for Timer0 set to frequency 1000Hz
volatile unsigned long timer0_millis = 0;
volatile unsigned char timer0_millis_extra_byte = 0;

ISR(TIMER0_COMPA_vect)
{
  timer0_millis++;
  if (timer0_millis == 0) {
    timer0_millis_extra_byte++;
  }
}

unsigned long millis()
{
  unsigned long m;
  uint8_t oldSREG = SREG;

  // disable interrupts while we read timer0_millis or we might get an
  // inconsistent value (e.g. in the middle of a write to timer0_millis)
  cli();
  m = timer0_millis;
  SREG = oldSREG;

  return m;
}

unsigned long micros() {
  unsigned long m;
  uint8_t oldSREG = SREG, t;
  
  cli();
  m = timer0_millis;
  t = TCNT0;

  if ((TIFR0 & _BV(OCF0A)) && (t < 240)) {
    m++;
  }

  SREG = oldSREG;
  
  return m * 1000 + t * 4;
}

unsigned long seconds() {
  unsigned long m;
  unsigned char e;
  uint8_t oldSREG = SREG;

  cli();
  m = timer0_millis;
  e = timer0_millis_extra_byte;
  SREG = oldSREG;

  m = (e << 22) | (m >> 10);
//    128 * m          192 * (m >> 6)                     134 * (m >> 7)                               144 * (m >> 11)                                           152 * (m >> 14)                                                       216 * (m >> 17)                                                                   182 * (m >> 18)
// ~ --------- = m + ----------------- = m + (m >> 6) + ----------------- = m + (m >> 6) + (m >> 7) + ----------------- = m + (m >> 6) + (m >> 7) + (m >> 11) + ----------------- = m + (m >> 6) + (m >> 7) + (m >> 11) + (m >> 14) + ----------------- = m + (m >> 6) + (m >> 7) + (m >> 11) + (m >> 14) + (m >> 17) + -----------------
//     125                 125                                125                                           125                                                        125                                                                   125                                                                              125
  long seconds = m + (m >> 6) + (m >> 7) + (m >> 11) + (m >> 14) + (m >> 17) + (m >> 18);
  return seconds + ((seconds >> 6) * microsecondPerSecond) / 15625;
}

#endif

void delay(unsigned long ms)
{
  uint32_t start = micros();

  while (ms > 0) {
    yield();
    while ( ms > 0 && (micros() - start) >= 1000) {
      ms--;
      start += 1000;
    }
  }
}

/* Delay for the given number of microseconds.  Assumes a 1, 8, 12, 16, 20 or 24 MHz clock. */
void delayMicroseconds(unsigned int us)
{
  // call = 4 cycles + 2 to 4 cycles to init us(2 for constant delay, 4 for variable)

  // calling avrlib's delay_us() function with low values (e.g. 1 or
  // 2 microseconds) gives delays longer than desired.
  //delay_us(us);

  // for the 16 MHz clock on most Arduino boards

  // for a one-microsecond delay, simply return.  the overhead
  // of the function call takes 14 (16) cycles, which is 1us
  if (us <= 1) return; //  = 3 cycles, (4 when true)

  // the following loop takes 1/4 of a microsecond (4 cycles)
  // per iteration, so execute it four times for each microsecond of
  // delay requested.
  us <<= 2; // x4 us, = 4 cycles

  // account for the time taken in the preceeding commands.
  // we just burned 19 (21) cycles above, remove 5, (5*4=20)
  // us is at least 8 so we can substract 5
  us -= 5; // = 2 cycles,

  // busy wait
  __asm__ __volatile__ (
    "1: sbiw %0,1" "\n\t" // 2 cycles
    "brne 1b" : "=w" (us) : "0" (us) // 2 cycles
  );
  // return = 4 cycles
}

void myInit()
{
  // this needs to be called before setup() or some functions won't
  // work there
  sei();

  // the bootloader connects pins 0 and 1 to the USART; disconnect them
  // here so they can be used as normal digital i/o; they will be
  // reconnected in Serial.begin()
#if defined(UCSRB)
  UCSRB = 0;
#elif defined(UCSR0B)
  UCSR0B = 0;
#endif
}

int main(void)
{
  myInit();

#if defined(USBCON)
  USBDevice.attach();
#endif
  
  setup();
    
  for (;;) {
    loop();
    if (serialEventRun) serialEventRun();
  }
        
  return 0;
} 
