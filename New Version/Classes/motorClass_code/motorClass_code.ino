#include <Arduino.h>
//#define MOTORDIR_PIN 4
//#define MOTORDR_PIN 5
//#define MOTORENA_PIN 6
class Motor {
  private:
  byte dirPin; //direction pin
  byte drPin; //drive pin, also called pulse
  byte enaPin; //enable pin
  byte period;
  bool state[3]; //[dir pulse enable]
  public:
  Motor(byte dirPin, byte drPin, byte enPin, byte steps) {
    this->dirPin = dirPin;
    this->drPin = drPin;
    this->enaPin = enaPin;
    state[1] = 1;
    state[2] = 0;
    state[3] = 0;
    period = 10*10*10; //1 sec
    init();
  }
  void init() {
    pinMode(dirPin, INPUT);
    pinMode(drPin, INPUT);
    pinMode(enaPin, INPUT);
    resetState();
  }
  void resetState() {
    digitalWrite(dirPin, HIGH);
    digitalWrite(drPin, LOW);
    //digitalWrite(enaPin, LOW); //can be left off
  }
  bool getState() {
    return state;
  }
  bool checkStoppingCondit(bool otherFunctionRes) { //incomplete, needs to call the other function here and get bool result
    return otherFunctionRes;
  }
  void motorRun(bool state[]) {
    resetState();
    int timeReserve = 5*10*10*10;//currently running of timer, ms, and checking
    while(timeReserve > 1*10*10*10) { 
      digitalWrite(dirPin, state[1]);
      digitalWrite(drPin, HIGH);
      state[1] = 1;
      delayMicroseconds(period);
      digitalWrite(drPin, LOW);
      state[1] = 0;
      delayMicroseconds(period);
      timeReserve = timeReserve - 2*period;
      if(timeReserve < 3*10*10*10) {
        timeReserve = timeReserve + 5*10*10*10*checkStoppingCondit(1); //incomplete, needs to call the other function here and get bool result
      }
    }
  }
  void motorUp() {
    resetState();
    motorRun(state);
  }
  void motorDown() {
    resetState();
    state[0] = 0;
    motorRun(state);
  }
};
void setup() {
}
void loop() {
}
