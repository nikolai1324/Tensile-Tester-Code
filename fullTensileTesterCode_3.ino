//libraries to include
#include <arduino.h>
#include <HX711.h>
#include <stepper.h>
//written for arduino nano

//General variables
  //motor variables
int enable = 8; //D8
int direction = 9; //D9
int pul = 10; //D10

  //load cell/stress variables/obj
#define DOUT 11 //D11
#define CLK 12 //D12
HX711 scale;
float calibration_factor = 1;
int intialArea = -1; //change for sample, possibly add command prompt input
double compensationAngle = -1;
double stressVal = -1;

  //button varibles
int upButton = 2; //D2
int downButton = 3; //D3
int runTest = 4; //D4
int emerStop = 5; //D5
//typedef enum {down, up, press, release} ButtonState_t; //states of pushbuttons
//ButtonState_t upButton, downButton, runTest, emerStop;
bool isRunning = false;
bool buttonResult[] = {false, true, false};

  //digital linear slide variable
int dro_bits[24];        // For storing the data bit. bit_array[0] = data bit 1 (LSB), bit_array[23] = data bit 24 (MSB).
char str[7]; //Array for the char "string"
unsigned long droTimer = 0; //update frequency (the DRO is checked this often)
unsigned long buttonTimer = 0; //debouncing
int clk = 6; //Blue, D6
int data = 7; //Red, D7
int button = 13; //button pin, D13
float convertedValue = 0.0; //raw conversion value (bit to mms)
float resultValue = 0.0; //final result value: conversion value - tare value (if there is any taring)
float previousresultValue = 0.0; //temporary storage to be able to register a change in the value
float tareValue = 0.0; //tare value to set a new zero point anywhere (using a button)
//float maxDispl = 50; //for now assume max displacement 50mm

//helper functions
  //motor obj
Stepper myStepper = Stepper(50, 5, 6);
  //motor running function
void motorRun(bool running, bool dir, int pulse){
  if(running) {
    digitalWrite(enable, HIGH);
    if(dir) {
      digitalWrite(direction, HIGH);
      delayMicroseconds(10);
      digitalWrite(pul, LOW);
      delay(1);
      while(true) {
        delayMicroseconds(pulse);
        digitalWrite(pul, HIGH);
        delayMicroseconds(pulse);
        digitalWrite(pul, LOW);
     }
    }
    else {
      digitalWrite(direction, LOW);
      delayMicroseconds(10);
      digitalWrite(pul, LOW);
      delay(1);
      while(true) {
        delayMicroseconds(pulse);
        digitalWrite(pul, HIGH);
        delayMicroseconds(pulse);
        digitalWrite(pul, LOW);
      }
     }
    }
    else {
      digitalWrite(enable, LOW);
    }
}
  //calibration const. finder
double calibFinder() {
  Serial.begin(9600);
  Serial.println("HX711 calibration sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press + or a to increase calibration factor");
  Serial.println("Press - or z to decrease calibration factor");

  scale.begin(DOUT, CLK);
  scale.set_scale();
  scale.tare(); //Reset the scale to 0

  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);

  int endNum = 1;
  while (endNum > 0){
    scale.set_scale(calibration_factor); //Adjust to this calibration factor
    //Serial.print("Reading: ");
    Serial.print(scale.get_units());
    //Serial.print(" lbs"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
    //Serial.print(" calibration_factor: ");
    Serial.print(calibration_factor);
    Serial.println();
      
    if(Serial.available())
    {
      char temp = Serial.read();
      if(temp == '+' || temp == 'a')
        calibration_factor += 10;
      else if(temp == '-' || temp == 'z')
        calibration_factor -= 10;
      else if(temp == 'f')
        endNum == -1;
    }
  }
  return calibration_factor;
}
  //engineering stress reader
void stressReader(double calibration_factor) {
  double scReading = 0.0;
  Serial.begin(9600);
  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor);
  scReading = scale.get_units(), 1;
  Serial.print(scReading);
}
  //button read function
void buttonReader() {
  if(digitalRead(upButton)==HIGH || digitalRead(runTest)==HIGH) {
    buttonResult[0] = true;
    buttonResult[1] = true;
    buttonResult[2] = false;
  }
  else if(digitalRead(downButton)==HIGH) {
    buttonResult[0] = true;
    buttonResult[1] = false;
    buttonResult[2] = false;
  }
  else if(digitalRead(emerStop)==HIGH) {
    buttonResult[0] = false;
    buttonResult[1] = true;
    buttonResult[2] = false;
  }
}

void buttonReader_SM() {
  if(Serial.available()) {
    char temp = Serial.read();
    if(temp == 'u' || temp = 'r') {
      buttonResult[0] = true;
      buttonResult[1] = true;
      buttonResult[2] = false;
  }
    else if(temp == 'd') {
      buttonResult[0] = true;
      buttonResult[1] = false;
      buttonResult[2] = false;
  }
    else if(temp == 's') {
      buttonResult[0] = false;
      buttonResult[1] = true;
      buttonResult[2] = false;
  }

  }
}

    //digital linear slide stuff
  //button reader for linear slide
void readButton()
{
  if (digitalRead(button) == 0) //The button should pull the circuit to GND! (It is pulled up by default)
  {
    if (millis() - buttonTimer > 300) //software "debounce"
    {
      tareValue = convertedValue; //use the most recent conversion value as the tare value
      buttonTimer = millis();
    }
  }
}

  //read the encoder
void readEncoder()
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
      while (digitalRead(clk) == HIGH) {} //wait until the clk goes low
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
      while (digitalRead(clk) == LOW) {} // wait for the rising edge

      dro_bits[i] = digitalRead(data);
      //Print the data on the serial
      Serial.print(dro_bits[i]); //kept for testing
      Serial.print(" ");

      while (digitalRead(clk) == HIGH) {} // wait for the falling edge
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
      //don't touch the value (stays positive)
      Serial.println("Positive ");
    }
    else
    {
      convertedValue = -1 * convertedValue; // convert to negative
      Serial.println("Negative ");
    }

    convertedValue = (convertedValue / 100.0); //conversion to mm
    //Division by 100 comes from the fact that the produced number is still an integer (e.g. 9435) and we want a float
    //The 100 is because of the resolution (0.01 mm). x/100 is the same as x*0.01.

    //The final result is stored in a separate variable where the tare is subtracted
    //We need a separate variable because the converted value changes "on its own scale".
    resultValue = convertedValue - tareValue;

    //Dump everything on the serial
    Serial.print("Raw reading: "); //kept here for testing right now
    Serial.println(convertedValue);
    Serial.print("Tare value: ");
    Serial.println(tareValue);
    Serial.print("Result after taring: ");
    Serial.println(resultValue); //only necessary value
    Serial.println(" ");

    droTimer = millis();
  }
}


void setup() {
  //pin setup
    //motor pins
      //a+ red
      //a- green
      //b+ yellow
      //b- blue
  pinMode(pul, OUTPUT);
  pinMode(enable, OUTPUT);
  delayMicroseconds(10);
  pinMode(direction, OUTPUT);
  pinMode(upButton, INPUT);
  pinMode(downButton, INPUT);
  pinMode(runTest, INPUT);
  pinMode(emerStop, INPUT);
    //digital linear slide
  Serial.begin(9600);
  Serial.println("DRO reading Arduino"); //test message to see if the serial works
  pinMode(clk, INPUT_PULLUP);
  pinMode(data, INPUT_PULLUP);
  pinMode(button, INPUT_PULLUP);
}

void loop() {
  double calconst = calibFinder();
  scale.tare();
  buttonReader_SM();
  motorRun(buttonResult[0],buttonResult[1],buttonResult[2]);
  //while(resultValue<maxDispl && !buttonResult[0]) {
  while(!buttonResult[0]) {
    buttonReader_SM();
    motorRun(buttonResult[0],buttonResult[1],buttonResult[2]);
    stressReader(calibration_factor);
    readEncoder();
    readButton();
  }
  buttonResult[0] = false;
}
