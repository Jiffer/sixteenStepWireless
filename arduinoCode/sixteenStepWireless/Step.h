#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <WiFiUdp.h>
#include <OSCMessage.h>

class Step {
    // object varibles
    int index;
    int panelNumber;
    int pin;
    unsigned long lastStepTime = 0;
    int debounceTime = 50;
    unsigned long lastAvg = 0;
    int avgTime = 10;

    int threshold = 600;
    int lastValue = 0;
    int history[5];
    int hPtr = 0;
    float movingAverage;

    int lHistory[50];
    int lPtr = 0;
    float lMovingAverage;

    const IPAddress * sendIp;
    const unsigned int * sendPort;

  public:
    int velocity;
    // constructors
    Step() {
      // no Pin!
    }

    // Constructor passes in pin number to check, panel number and step index
    Step(int stepPin, int _panel, int i) {
      pin = stepPin;
      panelNumber = _panel;
      index = i;
    }

    void init(const IPAddress *ip, const unsigned int *port) {
      sendIp = ip;
      sendPort = port;
      pinMode(pin, INPUT);
    }

    bool check() {
      velocity = analogRead(pin);
      if (millis() > avgTime + lastAvg) {
        lastAvg = millis();
        history[hPtr++] = velocity;
        int arraySize = sizeof(history) / sizeof(int);
        hPtr %= arraySize;
        int accum = 0;
        for (int i = 0; i < arraySize; i++) {
          accum += history[i];
        }
        movingAverage = accum / arraySize;

        // calculate long term moving average:
        lHistory[lPtr++] = movingAverage;
        int larraySize = sizeof(lHistory) / sizeof(int);
        lPtr %= larraySize;
        int ac = 0;
        for (int i = 0; i < larraySize; i++) {
          ac += lHistory[i];
        }
        lMovingAverage = ac / larraySize;
      }
      if (millis() > lastStepTime + debounceTime) {
        if (velocity > movingAverage + threshold && velocity > lMovingAverage + threshold) {
          // reset timer for debounce
          lastStepTime = millis();

          return true;
        }
        else {
          return false;
        }
      }
    }

    // send OSC message
    void sendMessages(WiFiUDP * Udp) {
      
      // construct address 
      // by concatenating values to string // then convert to Char array for transmission
      String addy = "/panel/";
      addy += panelNumber;
      addy += "/step/";
      addy += index;
      //      Serial.println(addy);
      
      // Convert string to character arrray
      char cAddy[addy.length() + 1];
      addy.toCharArray(cAddy, addy.length() + 1);

      OSCMessage msg(cAddy);
      msg.add(velocity);
      Udp->beginPacket(*sendIp, *sendPort);
      msg.send(*Udp);
      Udp->endPacket();
      msg.empty();

      Serial.print("pin: "); Serial.print(pin);Serial.print(" a: "); Serial.print(movingAverage);
      // Serial.print("la: "); Serial.print(lMovingAverage);Serial.print(" v- a: ");Serial.println(velocity - lMovingAverage);
      return;
    }

};
