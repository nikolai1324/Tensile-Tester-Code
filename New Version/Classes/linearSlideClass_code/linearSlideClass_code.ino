#include <Arduino.h>

class Slide {
private:
int dro_bits[24];// For storing the data bit. bit_array[0] = data bit 1 (LSB), bit_array[23] = data bit 24 (MSB).
char str[7]; //Array for the char "string"
unsigned long droTimer; //update frequency (the DRO is checked this often), usually 0
unsigned long buttonTimer; //debouncing, likely unused as checkTaring doesn't reference it
int clkPin; //Blue, D6
int dataPin; //Red, D7
int buttonPin; //button pin, A2 or 23, likely unused as checkTaring doesn't reference it
float convertedValue; //raw conversion value (bit to mms)
float resultValue; //final result value: conversion value - tare value (if there is any taring)
float previousresultValue; //temporary storage to be able to register a change in the value
float tareValue; //tare value to set a new zero point anywhere (using a button)
public:
Slide(int clkPin, int dataPin, int buttonPin, unsigned long droTimer) {
  this->clkPin = clkPin;
  this->dataPin = dataPin;
  this->buttonPin = buttonPin;
  this->droTimer = droTimer;
  convertedValue = 0.0;
  resultValue = 0.0;
  previousresultValue = 0.0;
  tareValue = 0.0;
  dro_bits[24];
  str[7];
  buttonTimer = 0;
}
void checkTaring(bool readButton) {//button result bool is used instead of "readButton" method
  tareValue = convertedValue;
}
int readEncoder()
{
  //This function reads the encoder
  //I added a timer if you don't need high update frequency

  if (millis() - droTimer > 1000) //if 1 s passed we start to wait for the incoming reading
  {
    convertedValue = 0; //set it to zero, so no garbage will be included in the final conversion value

    //This part was not shown in the video because I added it later based on a viewer's idea
    unsigned long syncTimer = 0; //timer for syncing the readout process with the DRO's clock signal
    bool synchronized = false; // Flag that let's the code know if the clk is synced
    while (synchronized == false)
    {
      syncTimer = millis(); //start timer
      while (digitalRead(clkPin) == HIGH) {} //wait until the clk goes low
      //Time between the last rising edge of CLK and the first falling edge is 115.7 ms
      //Time of the "wide high part" that separates the 4-bit parts: 410 us

      if (millis() - syncTimer > 5) //if the signal has been high for more than 5 ms, we know that it has been synced
      { //with 5 ms delay, the code can re-check the condition ~23 times so it can hit the 115.7 ms window
        synchronized = true;
      }
      else
      {
        synchronized = false;
      }
    }

    for (int i = 0; i < 23; i++) //We read the whole data block - just for consistency
    {
      while (digitalRead(clkPin) == LOW) {} // wait for the rising edge
      dro_bits[i] = digitalRead(dataPin);
      while (digitalRead(clkPin) == HIGH) {} // wait for the falling edge
    }
    Serial.println(" ");

    //Reconstructing the real value
    for (int i = 0; i < 20; i++) //we don't process the whole array, it is not necessary for our purpose
    {
      convertedValue = convertedValue + (pow(2, i) * dro_bits[i]);
      //Summing up all the 19 bits.
      //Essentially: 1*[i] + 2*[i] + 4*[i] + 8*[i] + 16 * [i] + ....
    }

    if (dro_bits[20] == 1)
    {
       //val is pos
    }
    else
    {
      convertedValue = -1 * convertedValue; // convert to negative
    }

    convertedValue = (convertedValue / 100.0); //conversion to mm
    //Division by 100 comes from the fact that the produced number is still an integer (e.g. 9435) and we want a float
    //The 100 is because of the resolution (0.01 mm). x/100 is the same as x*0.01.

    //The final result is stored in a separate variable where the tare is subtracted
    //We need a separate variable because the converted value changes "on its own scale".
    resultValue = convertedValue - tareValue;

    //Dump everything on the serial if testing here, but removed since in a class 
    return resultValue; //only necessary value
    droTimer = millis();
  }
}
};
void setup() {
}

void loop() {
}
