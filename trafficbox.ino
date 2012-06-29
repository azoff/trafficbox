/*
 * TrafficBox
 */
 
#include <SD.h>
#include <Ethernet.h>
#include <SPI.h>
 
// ethernet
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x03, 0x6F };
EthernetServer server(80);
 
// lights
const int lightPins[] = { 7, 6, 5 };
 
/*************************
 * ERROR LOGGING UTILITY *
 *************************/
#define error(s) error_P(PSTR(s))
void error_P(const char* str) {
  PgmPrint("ERROR: ");
  SerialPrintln_P(str);
  while(1);
}
 
/************************
 * INITIALIZE LIBRARIES *
 ************************/ 
void setup() {
   
  Serial.begin(9600);

  PgmPrint("\nFree RAM: ");
  Serial.println(FreeRam());
 
  for (int i=0; i<3; i++) pinMode(lightPins[i], OUTPUT);
  pinMode(10, OUTPUT); digitalWrite(10, HIGH);
  PgmPrintln("Pin modes set");
   
  SD.begin(4);
  PgmPrintln("SD card initialized");
  
  if (!Ethernet.begin(mac))
    error("Unable to assign IP Address");
  PgmPrint("DHCP assigned IP ");
  Serial.println(Ethernet.localIP());
  
  server.begin();
  PgmPrintln("HTTP/1.1 via port 80");
  
  //SD.ls(LS_R);
     
}
 
/*********************
 * MAIN PROGRAM LOOP *
 *********************/ 
void loop() {
  char method[10], action[50];
  if (EthernetClient client = server.available()) {
    readRequestLine(client, method, action);    
    Serial.print(method); PgmPrint(" ");
    Serial.println(action);
    if (strcmp(method, "GET") == 0)
      serveIndexFile(action);
    if (strcmp(method, "OPTIONS") == 0)
      serveLightLevels();
    if (strcmp(method, "POST") == 0) {
      toggleLight(action);
      serveLightLevels();
    } else
      writeStatusLine(400, "Bad Request");
    delay(1);
    client.stop();    
  }
}

/****************************
 * READS HTTP REQUEST PARTS *
 ****************************/
void readRequestLine(EthernetClient client, char *method, char *action) {
  int part=0,i=0,j=0; char chunk;
  while (client.connected()) {
    if (client.available()) {
      chunk = client.read();
      if (chunk == '\n' || chunk == ' ')
        part++;
      else if (part == 0)
        method[i++] = chunk;
      else if (part == 1)
        action[j++] = chunk;
      else break;
    } else break;
  }
  client.flush();
  method[i] = action[j] = '\0';
}

/************************************************
 * SERVES A FILE FROM THE SD CARD OVER ETHERNET *
 ************************************************/ 
void serveIndexFile(char *path) {
  PgmPrint("SERVE INDEX");
  if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
      File fileStream = SD.open("index~1.htm");
      if(fileStream) {
        writeStatusLine(200, "OK");
        writeContentType(".htm");
        writeCacheControl(31556926);
        writeFileStream(fileStream);
      } else {
        writeStatusLine(500, "Internal Server Error");
      }
  } else {
    writeStatusLine(404, "Not Found");
  }
}

/*******************************************
 * READ THE LIGHT LEVELS OF ALL THE LIGHTS *
 *******************************************/
void serveLightLevels() {
  char lights[4], mode;
  for (int i=0; i<3; i++)
    lights[i] = digitalRead(lightPins[i]) ? '1' : '0';
  lights[3] = 0;
  PgmPrint("LIGHTS ");
  Serial.println(lights);
  writeStatusLine(200, "OK");
  writeContentType("");  
  writeStringContent(lights);
}

/********************
 * HTTP STATUS LINE *
 ********************/
 
void writeStatusLine(int code, const char *message) {
  server.print("HTTP/1.1 ");
  server.print(code);
  server.print(" ");
  server.println(message);
}

/**********************
 * HTTP CACHE CONTROL *
 **********************/
void writeCacheControl(long maxAge) {
  server.print("Cache-Control: max-age=");
  server.println(maxAge);
}

/*********************
 * HTTP CONTENT TYPE *
 *********************/
void writeContentType(char *path) {
  server.print("Content-Type: ");
  if (strstr(path, ".js")  != 0)      server.println("application/javascript");
  else if (strstr(path, ".ico") != 0) server.println("image/x-icon");
  else if (strstr(path, ".css") != 0) server.println("text/css");  
  else if (strstr(path, ".htm") != 0) server.println("text/html");
  else                                server.println("text/plain");
}

/*************************
 * STREAM FILE OVER HTTP *
 *************************/
void writeFileStream(File fileStream) { 
  server.print("Content-Length: ");
  server.println(fileStream.available());
  server.println();
  while (fileStream.available())
    server.print((char)fileStream.read());
  fileStream.close();
}

/*************************
 * STREAM FILE OVER HTTP *
 *************************/
void writeStringContent(char *content) { 
  server.print("Content-Length: ");
  server.println(strlen(content));
  server.println();
  server.println(content);
}

/***********************
 * TOGGLES A LIGHT PIN *
 ***********************/
boolean toggleLight(char *command) {
  int index  = command[1] - '0';
  int setter = command[2] - '0';
  if (index < 3 && setter >=0) {
    boolean level = setter > 0 ? HIGH : LOW;
    digitalWrite(lightPins[index], level);
    return true;
  } else {
    return false;
  }
}
