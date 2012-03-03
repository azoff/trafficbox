/*
 * TrafficBox
 */
 
#include <SD.h>
#include <SPI.h>
#include <Ethernet.h>
 
// constants
#define ETHERNET_SERVER_PORT 80
 
// ethernet
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x03, 0x6F };
EthernetServer server(ETHERNET_SERVER_PORT);
 
// lights
int lightPins[]            = { 5, 6, 7 };
const String lightLabels[] = { "RED", "YELLOW", "GREEN" };
 
// sd card
const int inputChipSelect  = 4;
const int outputChipSelect = 10;
 
void setup() {
   
  Serial.begin(9600);
  
  Serial.println("\nInitializing SD card...");
  pinMode(outputChipSelect, OUTPUT);
  if (!SD.begin(inputChipSelect)) {
    Serial.println("ERROR: SD card is missing or invalid.");
    return;
  }
  
  Serial.println("Setting mode for light pins...");
  for (int i=0; i<3; i++)
    pinMode(lightPins[i], OUTPUT);
  
  Serial.println("Initializing ethernet server...");
  if (!Ethernet.begin(mac)) {
    Serial.println("ERROR: Unable to assign IP Address.");
    return;
  } 
  server.begin();
  Serial.print("Server Running: ");
  Serial.print(Ethernet.localIP());
  Serial.print(":");
  Serial.println(ETHERNET_SERVER_PORT);
     
}
 
// the main program loop
void loop() {  
  if (EthernetClient client = server.available()) {
    String method = readRequestPart(client);
    String action = readRequestPart(client);
    if (method.equalsIgnoreCase("GET"))
      serveFileStream(action);
    else if (method.equalsIgnoreCase("POST"))
      toggleLight(action);
    else
      writeStatusLine(400, "Bad Request");
    client.flush();
    client.stop();    
  }
}

// parses the current request from the TCP stream
String readRequestPart(EthernetClient client) {
  String part = ""; char chunk;
  while (client.available()) {
    chunk = client.read();
    if (isWhiteSpace(chunk)) break;
    else part.concat(chunk);
  }
  return part;
}

// does what it says
boolean isWhiteSpace(char toCheck) {
  return (toCheck == ' ' || toCheck == '\n');
}

// attempts to server a static file from the SD card
void serveFileStream(String filePath) {
  const char *dos83Path = getDos83Path(filePath);
  if (File fileStream = SD.open(dos83Path)) {
    writeStatusLine(200, "OK");
    writeContentType(filePath);
    writeContentFile(fileStream);
    fileStream.close();
  } else {
    writeStatusLine(404, "Not Found");
  }  
}

const char *getDos83Path(String filePath) {  
  if (filePath.equalsIgnoreCase("/") || filePath.equalsIgnoreCase("/index.html")) 
    return ;
  else if (filePath.equalsIgnoreCase("/rootdump.txt")) 
    return "/";
  else {
    filePath.toUpperCase();
    int length = filePath.length();
    char dos83Path[length];
    filePath.toCharArray(dos83Path, length);
    return dos83Path;
  }
}

// write out an HTTP status line
void writeStatusLine(int code, String message) {
  server.print("HTTP/1.0 ");
  server.print(code);
  server.print(" ");
  server.println(message);
}

// write out the content type of a particular file
void writeContentType(String filePath) {
  server.print("Content-Type: ");
  String ext = filePath.substring(filePath.lastIndexOf('.')+1);    
  if (ext.equalsIgnoreCase("js"))   server.println("application/javascript");
  if (ext.equalsIgnoreCase("json")) server.println("application/json");
  if (ext.equalsIgnoreCase("ico"))  server.println("image/vnd.microsoft.icon");
  if (ext.equalsIgnoreCase("css"))  server.println("text/css");  
  if (ext.equalsIgnoreCase("txt"))  server.println("text/plain");
  else                              server.println("text/html");
}

// write out an HTTP content from a string
void writeContentString(String content) { 
  server.print("Content-Length: ");
  server.println(content.length());
  server.println();
  server.println(content);
}

// write out an HTTP content from a string
void writeContentFile(File content) { 
  server.print("Content-Length: ");
  server.println(content.available());
  server.println();
  while (content.available())
    server.write((char) content.read());
}

// toggles a light on and off
void toggleLight(String command) {
  int index = command.charAt(1) - '0';
  int setter = command.charAt(2) - '0';
  if (index < 3 && setter >=0) {
    boolean level = setter > 0 ? HIGH : LOW;
    digitalWrite(lightPins[index], level);
    writeStatusLine(200, "OK");
    writeContentType(".txt");
    writeContentString(lightLabels[index] + (level ? " ON" : " OFF"));
  } else {
    writeStatusLine(400, "Bad Request");
  }
}
