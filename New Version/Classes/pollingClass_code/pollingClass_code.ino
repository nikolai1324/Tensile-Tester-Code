#include <Arduino.h>

class Polling {
  private:
  int rate;
  int timeoutDel; //ms duration to make sure all is gotten from serial input (if str funct added)
  char res;
  char target;
  String str;
  public:
  Polling(int rate, int timeoutDel) {
    this->rate = rate;
    this->timeoutDel = timeoutDel;
    target = 'p';
    res = target;
    str = "p";
  }
  int getRate() {
    return rate;
  }
  int getTimeoutDel() {
    return timeoutDel;
  }
  char poll() { //must be run in loop()
    while(Serial.available() == 0) {
    }
    res = Serial.read();
    return res;
  }
  bool confirmAns(String str, char target) { //broken behavior, returns in groups of 3, must be run in loop()
    Serial.println(str);
    char temp;
    bool b = 0;
    while(!b) {
      while(Serial.available() > 0) {
       temp = Serial.read();
       b = (temp == target);
       if(b) {
        return b;
       } 
      }
    }
  }
};

Polling poller1(9600,10);

void setup() {
  Serial.begin(poller1.getRate());
  Serial.setTimeout(poller1.getTimeoutDel());
}

void loop() {
}