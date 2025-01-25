// Meduino version with MOSFETs driven by MOSFET driver
#include "Definition_Meduino2560.h"

void writePwmValue(byte fanNumber, byte val) {
  switch(fanNumber) {
    case 0:
      OCR3A = (word)(val*RATIO); // set pwm duty
      break;
    case 1:
      OCR4B = (word)(val*RATIO); // set pwm duty
      break;
    case 2:
      OCR4A = (word)(val*RATIO); // set pwm duty
      break;
    case 3:
      OCR2B = val; // set pwm duty
      break;
    case 4:
      OCR4C = (word)(val*RATIO); // set pwm duty
      break;
    case 5:
      OCR1A = (word)(val*RATIO); // set pwm duty
      break;
    case 6:
      OCR2A = val; // set pwm duty
      break;
    case 7:
      OCR1B = (word)(val*RATIO); // set pwm duty
      break;
    case 8:
      OCR1C = (word)(val*RATIO); // set pwm duty
      break;
    case 9:
      OCR5B = (word)(val*RATIO); // set pwm duty
      break;
    case 10:
      OCR5C = (word)(val*RATIO); // set pwm duty
      break;
    case 11:
      OCR5A = (word)(val*RATIO); // set pwm duty
      break;
  }
}
