/* 
  This is the EMMMA-K-v3 Slave Processor code
*/

#include <Arduino.h>
#include "driver/touch_pad.h"

// For some reason Serial1 doesn't work on the slave and not sure why...
#define SERIAL2MASTER Serial0 // Serial0 (rx and tx pins) or Serial1 (rx = 39, tx = 38)

const uint8_t numPins = 14;  // The MCU has 14 touch pins

// Note that the pin arrangement is slightly different from the master
static const touch_pad_t pins[numPins] = 
{
    TOUCH_PAD_NUM5,
    TOUCH_PAD_NUM6,
    TOUCH_PAD_NUM9,
    TOUCH_PAD_NUM10,
    TOUCH_PAD_NUM11,
    TOUCH_PAD_NUM12,
    TOUCH_PAD_NUM13,
    TOUCH_PAD_NUM4,
    TOUCH_PAD_NUM14,
    TOUCH_PAD_NUM7,
    TOUCH_PAD_NUM8,
    TOUCH_PAD_NUM3,
    TOUCH_PAD_NUM1,
    TOUCH_PAD_NUM2
};

static uint32_t benchmark[numPins]; // to store the initial touch values of the pins
static bool pinsTouched[numPins] = {};

void setup() 
{
  Serial.begin(115200);
  SERIAL2MASTER.begin(2000000); // rx = rx, tx = tx
  Serial0.setDebugOutput(false); // Without this system error messages seem to get sent to this port!

  touch_pad_init();
  
  touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);

  touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
  touch_pad_fsm_start();

  for(int i = 0; i < numPins; i++) 
  {
      touch_pad_config(pins[i]);
  }

  delay(2000);

  uint32_t touch_value;
  for (int i = 0; i < numPins; i++) 
  {
      //read benchmark value
      touch_pad_read_benchmark(pins[i], &touch_value);
      Serial.print(touch_value);
      Serial.print(" ");
      benchmark[i] = touch_value;
  }

  Serial.println("Ready...");
}

#if 0
void loop()
{
  static uint8_t n = 0;

  SERIAL2MASTER.write(n++);

  while(!SERIAL2MASTER.available())
    ;

  uint8_t c = SERIAL2MASTER.read();

  Serial.println(c, 16);

  //delay(100);
}
#else

void loop() 
{
  uint32_t touch_value;

  while(!SERIAL2MASTER.available())
    ;

  uint8_t c = SERIAL2MASTER.read();

  //Serial.println(c, 16);

  if(c == 0xA5)
  {
    //Serial.println("read touch pins");

    for(int i = 0; i < numPins; i++)
    {
      touch_pad_read_raw_data(pins[i], &touch_value);

      //Serial.println(touch_value);

      if(touch_value > benchmark[i] + (0.3 * benchmark[i]))
      {
        pinsTouched[i] = true;
      }
      else if(touch_value < benchmark[i] + (0.2 * benchmark[i])) // note a bit of hysterisis
      {
        pinsTouched[i] = false;
      }
    }

    uint8_t p = 0x80;
    //static uint8_t lastp = 0;

    // The slave has 14 touch pins and we send the values in two bytes of 7
    for(int i = 0; i < 7; i++)
    {
      if(pinsTouched[i])
      {
        p |= (1 << i);
      }
    }

    SERIAL2MASTER.write(p);

    //if(p != lastp)
    //{
    //  lastp = p;
    //  Serial.println(p);
    //}

    p = 0;

    // Send the remaining 6 pin touched values
    for(int i = 0; i < 7; i++)
    {
      if(pinsTouched[i + 7])
      {
        p |= (1 << i);
      }
    }

    SERIAL2MASTER.write(p);
  }
}

#endif