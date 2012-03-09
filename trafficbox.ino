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
const int lightPins[] = { 5, 6, 7 };
 
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
      serveFileStream(action);
    else
      writeStatusLine(400, "Bad Request");
    delay(1000);
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
 
void serveFileStream(char *action) {
  char *dos83Path = getDos83Path(action);
  PgmPrint("OPEN "); Serial.println(dos83Path);  
  File fileStream = SD.open(dos83Path);
  if(fileStream) {
    writeStatusLine(200, "OK");
    writeContentType(action);
    writeFileStream(fileStream);
  } else {
    writeStatusLine(404, "Not Found");
  }
}

/************************************************
 * PATH MAPPINGS FROM LONG TO SHORT (8.3) NAMES *
 ************************************************/

char *getDos83Path(char *path) {
  if (strcmp("/", path) == 0)                                        return "INDEX~1.HTM";
  if (strcmp("/styles/normalize.css", path) == 0)                    return "STYLES/NORMAL~1.CSS";
  if (strcmp("/styles/jquery.mobile-1.1.0-rc.1.min.css", path) == 0) return "STYLES/JQUERY~1.CSS";
  if (strcmp("/styles/screen.css", path) == 0)                       return "STYLES/SCREEN~1.CSS";
  if (strcmp("/scripts/jquery-1.7.1.min.js", path) == 0)             return "SCRIPTS/JQUERY~1.JS";
  if (strcmp("/scripts/jquery.mobile-1.1.0-rc.1.min.js", path) == 0) return "SCRIPTS/JQUERY~2.JS";
  if (strcmp("/scripts/trafficbox.js", path) == 0)                   return "SCRIPTS/TRAFFI~1.JS";
  if (strcmp("/images/ajax-loader.gif", path) == 0)                  return "IMAGES/AJAX-L~1.GIF";  
  return "";
}

/********************
 * HTTP STATUS LINE *
 ********************/
 
void writeStatusLine(int code, const char *message) {
  server.print("HTTP/1.0 ");
  server.print(code);
  server.print(" ");
  server.println(message);
}

/*********************
 * HTTP CONTENT TYPE *
 *********************/

void writeContentType(char *path) {
  server.print("Content-Type: ");
  if (strstr("js", path) != 0)   server.println("application/javascript");
  if (strstr("json", path) != 0) server.println("application/json");
  if (strstr("ico", path) != 0)  server.println("image/vnd.microsoft.icon");
  if (strstr("gif", path) != 0)  server.println("image/gif");
  if (strstr("css", path) != 0)  server.println("text/css");  
  if (strstr("txt", path) != 0)  server.println("text/plain");
  else                           server.println("text/html");
}

/*************************
 * STREAM FILE OVER HTTP *
 *************************/

void writeFileStream(File fileStream) { 
  server.print("Content-Length: ");
  server.println(fileStream.available());
  server.println();
  while (fileStream.available())
    server.write((char)fileStream.read());
  fileStream.close();
}

/*

// write out an HTTP content from a string
void writeContentString(String content) { 
  server.print("Content-Length: ");
  server.println(content.length());
  server.println();
  server.println(content);
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
}*/
