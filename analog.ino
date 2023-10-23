// ADC conversion complete interrupt handler
// One ADC conversion takes 128*25 = 3200 cycles.
static volatile char adcIndexStatic = 0;

void init_adc()
{
#if HWversion == 1
  DIDR0 = B11011111;                                   // Disable digital input buffer for ADC pins
#endif

#if HWversion == 2
  DIDR0 = B00111111;                                   // Disable digital input buffer for ADC pins
  DIDR2 = B11111100;                                   // Disable digital input buffer for ADC pins
#endif

  
  ADMUX = 0;                                          // VREF is EXTERNAL, channel 0
  ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) // prescaler to 128
         | (1 << ADIE)                                // Enable ADC conversion complete interrupt
         | (1 << ADEN)                                // Enable ADC
         | (1 << ADSC);                               // Start conversion
}

void startNextADC()
{
  ADCSRA |= (1 << ADSC);             // Start next ADC conversion
}

ISR(ADC_vect)
{
  // copy these to local variables so they can be stored in registers
  // (volatile variables must be read from memory on every access)
  char adcIndex = adcIndexStatic;
 
  // Read the AD conversion result
  uint16_t sensorValue = ADC;
  
#if HWversion == 1
  // Process value and select next ADC input
  switch (adcIndex) {
    case 0:
      powerInADCAveraged[4] = ((powerInADCAveraged[4] + sensorValue) >> 1);
      ADMUX = 1;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 1:
      powerInADCAveraged[3] = ((powerInADCAveraged[3] + sensorValue) >> 1);
      ADMUX = 2;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 2:
      powerInADCAveraged[2] = ((powerInADCAveraged[2] + sensorValue) >> 1);
      ADMUX = 3;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 3:
      powerInADCAveraged[1] = ((powerInADCAveraged[1] + sensorValue) >> 1);
      ADMUX = 4;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 4:
      powerInADCAveraged[0] = ((powerInADCAveraged[0] + sensorValue) >> 1);
      ADMUX = 6;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 5:
      thermistorADCAveraged[0] = ((thermistorADCAveraged[0] + sensorValue) >> 1);
      ADMUX = 7;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 6:
      thermistorADCAveraged[1] = ((thermistorADCAveraged[1] + sensorValue) >> 1);
      adcIndex = -1;
      ADMUX = 0;                    // select next ADC channel, VREF is EXTERNAL
      break;
    default:
      adcIndex = -1;
  }
#endif

#if HWversion == 2
  // Process value and select next ADC input
  switch (adcIndex) {
    case 0:
      thermistorADCAveraged[5] = ((thermistorADCAveraged[5] + sensorValue) >> 1);
      ADMUX = 1;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 1:
      thermistorADCAveraged[4] = ((thermistorADCAveraged[4] + sensorValue) >> 1);
      ADMUX = 2;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 2:
      thermistorADCAveraged[3] = ((thermistorADCAveraged[3] + sensorValue) >> 1);
      ADMUX = 3;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 3:
      thermistorADCAveraged[2] = ((thermistorADCAveraged[2] + sensorValue) >> 1);
      ADMUX = 4;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 4:
      thermistorADCAveraged[1] = ((thermistorADCAveraged[1] + sensorValue) >> 1);
      ADMUX = 5;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 5:
      thermistorADCAveraged[0] = ((thermistorADCAveraged[0] + sensorValue) >> 1);
      ADCSRB = (1 << MUX5);
      ADMUX = 6;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 6:
      powerInADCAveraged[0] = ((powerInADCAveraged[0] + sensorValue) >> 1);
      ADMUX = 7;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 7:
      powerInADCAveraged[1] = ((powerInADCAveraged[1] + sensorValue) >> 1);
      ADMUX = 4;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 8:
      powerInADCAveraged[2] = ((powerInADCAveraged[2] + sensorValue) >> 1);
      ADMUX = 5;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 9:
      powerInADCAveraged[3] = ((powerInADCAveraged[3] + sensorValue) >> 1);
      ADMUX = 3;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 10:
      powerInADCAveraged[4] = ((powerInADCAveraged[4] + sensorValue) >> 1);
      ADMUX = 2;                    // select next ADC channel, VREF is EXTERNAL
      break;
    case 11:
      powerInADCAveraged[5] = ((powerInADCAveraged[5] + sensorValue) >> 1);
      adcIndex = -1;
      ADCSRB = 0;
      ADMUX = 0;                    // select next ADC channel, VREF is EXTERNAL
      break;
    default:
      adcIndex = -1;
  }
#endif
  adcIndex++;
  adcIndexStatic = adcIndex;

// slow down ADC conversions
//  ADCSRA &= ~(1 << ADEN);            // Disable ADC
//  ADCSRA |= (1 << ADEN);             // Enable ADC

//  ADCSRA |= (1 << ADSC);             // Start next conversion
}
