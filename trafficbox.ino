/*
 * TrafficBox
 */
 
// constants
#define LIGHT_COUNT  3
#define COMMAND_SIZE 2
 
// define light pins
int lightPins[LIGHT_COUNT]      = {11, 12, 13};
String lightLabels[LIGHT_COUNT] = {"red", "yellow", "green"};
 
// define command buffer
int commandIndex = 0;
int commandBuffer[COMMAND_SIZE];
 
// program initialization
void setup() {
   
  // open serial port
  Serial.begin(9600);
   
  // set pins to output mode
  for (int i = 0; i < LIGHT_COUNT; i++)
    pinMode(lightPins[i], OUTPUT);  
     
}
 
// the main program loop
void loop() {
  if (commandIndex < COMMAND_SIZE) {
    readInput();
    commandIndex++;
  } 
  if (commandIndex >= COMMAND_SIZE) {
    processCommand();
    commandIndex = 0;    
  }
}

// reads input into the command buffer
void readInput() {
  while (Serial.available() <= 0);
  commandBuffer[commandIndex] = Serial.read() - '0';
}

// runs the command currently in the buffer
void processCommand() {
  int lightIndex     = commandBuffer[0];
  boolean lightLevel = commandBuffer[1] > 0;
  if (lightIndex < LIGHT_COUNT)
    toggleLight(lightIndex, lightLevel);
  else
    Serial.println("Invalid Command");
}

// toggles a light on and off
void toggleLight(int index, boolean level) {
  Serial.print("switch ");
  Serial.print(lightLabels[index]);
  Serial.print(" light o");
  Serial.println(level ? "n" : "ff");
  digitalWrite(lightPins[index], level);
}
