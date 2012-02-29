/*
 * TrafficBox
 */

#include <SPI.h>
#include <Ethernet.h>
 
// constants
#define LIGHT_COUNT  3
#define SERVER_PORT 80
 
// define ethernet parameters
byte mac[]     = { 0x90, 0xA2, 0xDA, 0x0D, 0x03, 0x6F };
EthernetServer server(SERVER_PORT);
 
// define light pins
int lightPins[LIGHT_COUNT]      = {5, 6, 7};
String lightLabels[LIGHT_COUNT] = {"red", "yellow", "green"};
 
void setup() {
   
  Serial.begin(9600);
  Serial.println("Opening serial port @ 9600 baud...");
  
  Serial.println("Setting pin modes for output pins...");
  for (int i = 0; i < LIGHT_COUNT; i++)
    pinMode(lightPins[i], OUTPUT);    
  
  Serial.println("Initializing ethernet server...");
  Ethernet.begin(mac);
  server.begin();
  Serial.print("Server Running: ");
  Serial.print(Ethernet.localIP());
  Serial.print(":");
  Serial.println(SERVER_PORT);
     
}
 
// the main program loop
void loop() {
  EthernetClient client = server.available();
  if (client == true) {
    char out = client.read();
    Serial.print(out);
  }
}

// toggles a light on and off
void toggleLight(int index, boolean level) {
  Serial.print("switch ");
  Serial.print(lightLabels[index]);
  Serial.print(" light o");
  Serial.println(level ? "n" : "ff");
  digitalWrite(lightPins[index], level);
}
