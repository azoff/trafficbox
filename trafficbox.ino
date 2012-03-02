/*
 * TrafficBox
 */
 
#include <SPI.h>
#include <Ethernet.h>
 
// constants
#define LIGHT_COUNT  3
#define SERVER_PORT  80
#define REQUEST_SIZE 20
 
// define ethernet parameters
byte mac[]     = { 0x90, 0xA2, 0xDA, 0x0D, 0x03, 0x6F };
EthernetServer server(SERVER_PORT);
String request[REQUEST_SIZE];
 
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
  if (EthernetClient client = server.available()) {    
    int count = readRequest(client);
    if (count > 2) {
      writeResponse("200 OK", "text/plain", request[0] + " " + request[1]);
    } else {
      writeResponse("400 Bad Request", "text/plain", "Bad Request");
    }
    closeResponse(client);
  }
}

void closeResponse(EthernetClient client) {
  for (int i=0; i<REQUEST_SIZE; i++)
    request[i] = "";
  client.flush();
  client.stop();
}

// reads the current request into a string array
int readRequest(EthernetClient client) {
  int index = 0; char last, chunk;
  while(client.available() && index < REQUEST_SIZE) {
    chunk = client.read();
    if (index < 2 && chunk == ' ' || index > 1 && (chunk == '\n' || chunk == ':')) {
      if (last != chunk) 
            request[index++].trim();
    } else  request[index] += chunk;   
    last = chunk;
  }
  return index > 0 ? (index+1) : 0; 
}

// writes a simple HTTP 1.0 response back to the client
void writeResponse(String statusMessage, String contentType, String content) {
  server.println("HTTP/1.0 "+statusMessage);
  if (contentType.length() > 0)
    server.println("Content-Type: " + contentType);
  server.println("Content-Length: " + String(content.length()));
  server.println("\n" + content);
}

// toggles a light on and off
void toggleLight(int index, boolean level) {
  Serial.print("switch ");
  Serial.print(lightLabels[index]);
  Serial.print(" light o");
  Serial.println(level ? "n" : "ff");
  digitalWrite(lightPins[index], level);
}
