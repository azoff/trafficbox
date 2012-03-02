/*
 * TrafficBox
 */
 
#include <SD.h>
#include <SPI.h>
#include <Ethernet.h>
 
// constants
#define LIGHT_COUNT  3
#define SERVER_PORT  80
#define REQUEST_SIZE 20
 
// ethernet
byte mac[]     = { 0x90, 0xA2, 0xDA, 0x0D, 0x03, 0x6F };
EthernetServer server(SERVER_PORT);
String action = "", method = "";
 
// lights
int lightPins[LIGHT_COUNT]      = {5, 6, 7};
String lightLabels[LIGHT_COUNT] = {"red", "yellow", "green"};
 
// sd card
const int inputChipSelect  = 4;
const int outputChipSelect = 10;
 
void setup() {
   
  Serial.begin(9600);
  
  Serial.println("Initializing SD card...");
  pinMode(outputChipSelect, OUTPUT);
  if (!SD.begin(inputChipSelect)) {
    Serial.println("ERROR: SD card is missing or invalid.");
    return;
  }
  
  Serial.println("Setting mode for light pins...");
  for (int i = 0; i < LIGHT_COUNT; i++)
    pinMode(lightPins[i], OUTPUT);
  
  Serial.println("Initializing ethernet server...");
  if (!Ethernet.begin(mac)) {
    Serial.println("ERROR: Unable to assign IP Address.");
    return;
  } server.begin();
  Serial.print("Server Running: ");
  Serial.print(Ethernet.localIP());
  Serial.print(":");
  Serial.println(SERVER_PORT);
     
}
 
// the main program loop
void loop() {  
  if (EthernetClient client = server.available()) {    
    readRequest(client);
    if (method == "GET") {      
      serveStaticFile();
    } else {
      serveErrorMessage();
    }
    endRequest(client);
  }
}

// serves an error message for a bad request
void serveErrorMessage() {
  writeResponse("400 Bad Request", "text/plain", "Bad Request");
}

// attempts to server a static file from the SD card
void serveStaticFile() {
  String filepath    = inferPath(action); 
  String content     = readFileContents(filepath);
  if (content.length() > 0) {    
    String contentType = inferContentType(filepath);
    writeResponse("200 OK", contentType, content);
  } else {
    writeResponse("404 Not Found", "text/plain", filepath + " not found");
  }
}

// infers the content type of a given file path
String readFileContents(String path) {
  int length = path.length()+1;
  char c_path[length];
  String contents = "";
  path.toCharArray(c_path, length);
  File staticFile = SD.open(c_path, FILE_READ);
  if (staticFile) {
    while (staticFile.available())
      contents += (char) staticFile.read();
    staticFile.close();
  } 
  return contents;
}

// infers the content type of a given file path
String inferContentType(String path) {
  String extension = path.substring(path.lastIndexOf('.')+1);
  if (extension == "html") return "text/html";
  if (extension == "js")   return "application/javascript";
  if (extension == "css")  return "text/css";
  if (extension == "ico")  return "image/vnd.microsoft.icon";
  return "text/plain";
}

// infers the implicit path on HTML GET requests
String inferPath(String path) {
  String correctPath = path; 
  correctPath.trim();
  if (correctPath.lastIndexOf('.') < 0) {
    if (!correctPath.endsWith("/"))
      correctPath += '/';
    correctPath += "index.html";
  }
  if (correctPath.startsWith("/")) {
    correctPath = correctPath.substring(1);
  }
  return correctPath;
}

// resets the request to its original parameters
void endRequest(EthernetClient client) {
  action = ""; 
  method = "";
  client.flush();
  client.stop();
}

// parses the current request from the TCP stream
void readRequest(EthernetClient client) {
  int index = 0;   
  char last, chunk;
  while(client.available() && index < REQUEST_SIZE) {
    chunk = client.read();
    if (chunk == ' ' && last != chunk) index++;
    else if (index == 0)  method += chunk;
    else if (index == 1)  action += chunk;
    else {
      Serial.println(method + " " + action);
      break;
    }
    last = chunk;
  }
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
