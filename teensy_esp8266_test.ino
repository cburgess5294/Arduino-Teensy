/*  This example sketch is designed for use with Basic and Advanced Sensor Boards sold by BurgessWorld Custom Electronics
 *  The sketch uses an ESP8266 wifi module and a DHT temp/humidity sensor to upload temp/humidity data to a 
 *  thingspeak.com channel every 15 minutes.  
 *  It assumes a GREEN LED is attached to pin 5, and a RED LED is attached to pin 13.
 *  The LEDs can be disabled by commenting out "#define EnableLEDs" in the sketch below
 *  It is based on the ESP8266 wifi library available here:  https://github.com/itead/ITEADLIB_Arduino_WeeESP8266
 *  The DHT library available here:  https://github.com/adafruit/DHT-sensor-library/archive/master.zip
 *  And the Adadruit sensor library available here:  https://github.com/adafruit/Adafruit_Sensor/archive/master.zip
 *  The sketch takes a sensor reading every 15 minutes and uploads it to the defined thingspeek channel
 *  After uploading the data, the ESP8266 module is put to sleep until it is time to read the sensor again
 *  You should define your local WiFi settings, and your thingspeak channel write API key.
 *  You can add additional sensors and upload additional data by including the appropriate library
 *  and tweeking the "Build Data Package" section in the sendMessage function
 *  This sketch is provided AS-IS with no implied warranty for fitness of purpose
 *  You may modify this sketch to meet your needs
 */

//comment out the following line to disable status LEDS when running on batteries

//define wifi SSID and password
#define SSID        "SSID"
#define PASSWORD    "PASSWORD"


//initialize other global variables
bool connectStat;
bool espPresent = false;

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial.println(F("Started"));

     
  //wait for ESP status
  delay(5000);
  flushSerial();
  Serial1.println("AT+RST");
  if (recFind("ready", 5000)){
    Serial.println(F("Ready"));
    //set ESP8266 to station mode
    Serial1.println("AT+CWMODE=1");
    if (recFind("OK", 5000)){
      Serial.println(F("Station mode enabled"));
      Serial.println(F("ESP initialized"));
      initEsp();
      espPresent = true;
    }
  }
  else{
    Serial.println(F("ESP not found"));
    Serial.println(F("...check ESP UART Speed"));
    Serial.println(F("...verify ESP LED activity"));
  }

}

void loop() {
}

//function to flush software serial read buffer
void flushSerial(){
    while (Serial1.available() > 0){
      Serial1.read();
    }
}



//Bool function to seach Serial RX buffer for string value
bool recFind(String target, uint32_t timeout)
{
  char rdChar;
  String rdBuff = "";
  unsigned long startMillis = millis();
    while (millis() - startMillis < timeout){
      while (Serial1.available() >1){
        rdChar = Serial1.read();
        rdBuff += rdChar;
        if (rdBuff.indexOf(target) != -1) {
          break;
        }
     }
     if (rdBuff.indexOf(target) != -1) {
          break;
     }
   }
   if (rdBuff.indexOf(target) != -1){
    return true;
   }
   return false;
}

//function to reset ESP
void initEsp(){
  bool espReady = false;
  bool resetStatus = false;
  while (!resetStatus){
    flushSerial();
    //wait for esp ready
    Serial1.println("AT+RST");
    if (recFind("ready", 5000)){
      Serial.println(F("ESP Ready"));
      espReady = true;
      }
    else {
      Serial.println(F("ESP not ready"));
      }
    //check wifi connection
    if (espReady){
      //wait for wifi connection
      bool wifiJoined = false;
      if (recFind("GOT IP", 5000)){
        wifiJoined = true;
        } 
      if (wifiJoined){
        Serial.println(F("Wifi connected"));
        resetStatus = true;
        }
      //join wifi if not connected
      else {
        Serial.println(F("wifi not joined")); 
        //check to see if esp is responding
        Serial1.println("AT");
        if (recFind("OK", 5000)){
        Serial.println(F("ESP is up"));
        Serial1.println(F("AT+CWJAP?"));
          if (recFind("No AP", 5000)){
            Serial.println(F("No AP configured"));
            flushSerial();
            //join wifi
            Serial1.print("AT+CWJAP=\"");
            Serial1.print(SSID);
            Serial1.print("\",\"");
            Serial1.print(PASSWORD);
            Serial1.println("\"");
            if (recFind("GOT IP", 7000)){
              Serial.println(F("wifi joined"));
              resetStatus = true;
            }
            else {
              Serial.println(F("failed to join wifi"));
              }
          }
          else {
            Serial.println(F("no response to AT+CWJAP command"));
            }
        }
        else {
          Serial.println(F("ESP not responding"));
          }
        }
      }
    flushSerial();
 }
}



