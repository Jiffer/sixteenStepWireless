/*---------------------------------------------------------------------------------------------

  sixteenStep wireless version

  --------------------------------------------------------------------------------------------- */
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <WiFiUdp.h>
#include <OSCMessage.h>

#include "Step.h"

// TODO: make pins and panel#, IP address based on hardware switches
#define PANEL 2 // panel 0 has 3, // panel 1 has 5, // panel 2 and 3 have 4 pins.

#if PANEL == 1
int tilePins[3] = {34, 36, 39};
IPAddress ip(10, 0, 1, 101); // my IP address (requested)
#endif
#if PANEL == 2
int tilePins[5] = {34, 36, 39, 35, 32};
IPAddress ip(10, 0, 1, 102); // my IP address (requested)
#endif
#if PANEL == 3
int tilePins[4] = {34, 36, 39, 35};
IPAddress ip(10, 0, 1, 103); // my IP address (requested)
#endif
#if PANEL == 4
int tilePins[4] = {34, 36, 39, 35};
IPAddress ip(10, 0, 1, 104); // my IP address (requested)
#endif

// Tile objects // ADC1 ch 0: 36 (VP), 39 (VN), 34  .. 35, 32 to keep going
// {34, 36, 39};          // panel 0
// {34, 36, 39, 35, 32};  // pannel 1
// {34, 36, 39, 35};      // panels 2, 3

const int numTiles = int(sizeof(tilePins) / sizeof(int));
// pointer to array of steps
Step *tile[numTiles];


// WIFI Credentials
char ssid[] = "things";          // your network SSID (name)
char password[] = "connected";                    // your network password

WiFiUDP Udp;                                // A UDP instance to let us send and receive packets over UDP
const IPAddress outIp(10, 0, 1, 2);  // remote IP of your computer
// use to request a static IP
//IPAddress ip(10, 0, 1, 103); // my IP address (requested)
IPAddress gateway(10, 0, 1, 1);
IPAddress subnet(255, 255, 255, 0);
const unsigned int outPort = 9999;          // remote port to receive OSC
const unsigned int localPort = 8888;        // local port to listen for OSC packets (actually not used for sending)

void setup() {
  Serial.begin(115200);


  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  int i = 0;
  int state = true;
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 10) {
      state = false;
      break;
    }
    i++;
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
#ifdef ESP32
  Serial.println(localPort);
#else
  Serial.println(Udp.localPort());
#endif

  // Construct and initialize Steps:
  for (int i = 0; i < numTiles; i++) {
    tile[i] = new Step(tilePins[i], PANEL,  i);
    tile[i]->init(&outIp, &outPort);
  }

}

void loop() {
  // check each tile and send a message as needed
  for (int i = 0; i < numTiles; i++) {
    if (tile[i]->check()) {
      tile[i]->sendMessages(&Udp);
    }
  }
}
