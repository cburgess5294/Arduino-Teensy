
/*  This example sketch is designed for use with the Arduino-Teensy r4.0 and higher boards sold by BurgessWorld Custom Electronics
 *  The sketch uses an ESP8266 wifi module and a DHT temp/humidity sensor to upload temp/humidity data to a 
 *  thingspeak.com channel every 15 minutes.  
 *  It uses the STATE LED next to the HC05/06 header in place of a GREEN LED, and the LED on pin D13 as the RED LED
 *  The LEDs can be disabled by commenting out "#define EnableLEDs" at the beginning of the sketch
 *  It assumes the DHT module is connected to digital pin 16 but you can configure this to your liking
 *  You should place a jummper between the ESP ENA pin on the 38-pin end connector and digital pin 2 to allow the sketch to reset the ESP8266 module
 *  You should modify the BAUD rate for Serial1 to match the BAUD rate of your ESP8266 module (default is 115200)
 *  The functions used are based on a modified version of the ESP8266 wifi library available here:  https://github.com/itead/ITEADLIB_Arduino_WeeESP8266
 *  The modified version is available in my github account here:  https://github.com/cburgess5294/Arduino-Teensy/blob/master/ITEADLIB_Arduino_WeeESP8266-master_TEENSY.zip
 *  The library was modified to make it compatible with the Teensy serial ports, as the original is not
 *  It also used The DHT library available here:  https://github.com/adafruit/DHT-sensor-library/archive/master.zip
 *  And the Adadruit sensor library available here:  https://github.com/adafruit/Adafruit_Sensor/archive/master.zip
 *  The sketch takes a sensor reading every 15 minutes and uploads it to the defined thingspeek channel
 *  After uploading the data, the ESP8266 module is put to sleep until it is time to read the sensor again
 *  You should define your local WiFi settings, and your thingspeak channel write API key.
 *  You can add additional sensors and upload additional data by including the appropriate library
 *  and tweeking the "Build Data Package" section in the sendMessage function
 *  This sketch is provided AS-IS with no implied warranty for fitness of purpose
 *  You may modify this sketch to meet your needs
 */
#include <ESP8266TEENSY.h>
#include <DHT.h>

//comment out the following line to disable status LEDS when running on batteries
#define EnableLEDs

//Define status indicator LED pins
#define GREEN 24
#define RED 13

//define wifi SSID and password
#define SSID        "BURGESSWORLD1"
#define PASSWORD    "thisistherouterpassword"

//define thingspeak address and port
#define HOST_NAME   "api.thingspeak.com"
#define HOST_PORT   (80)

//define thingspeek channel write key
#define channelKey "921QR0W64PY0N9BY"

// Define DHT Sensor type - uncomment whatever type you're using
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

//define DHT sensor pin
#define DHTPIN 16

// Define ESP8266 RESET pin
#define ESPRESETPIN 2

//define structure for returning sensor readings
struct reading {
    int h;
    int f;
} reading_r ;

//create ESP8266 instance
ESP8266 wifi(Serial1, 115200);

//initialize DHT
DHT dht(DHTPIN, DHTTYPE);

//initialize other global variables
bool connectStat;
int failCount;
bool espPresent = false;

void setup() {
  Serial.begin(57600);
  Serial.println(F("Started"));
  //set pin mode for status indicator LED pins
  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);
   
  //set pin mode for DHT22 output pin
  pinMode(DHTPIN, INPUT_PULLUP);

  //SET ESP RESET PIN TO OUTPUT
  pinMode(ESPRESETPIN, OUTPUT);

  //reset ESP8266
  digitalWrite(ESPRESETPIN, LOW);
  delay(20);
  digitalWrite(ESPRESETPIN, HIGH);

  //wait for ESP status
  if (recFind("ready", 5000)){
    Serial.println(F("Ready"));
    //set ESP8266 to station mode
    if (wifi.setOprToStation()){
      Serial.println(F("Station mode enabled"));
      Serial.println(F("ESP initialized"));
      flushSerial();
      flashLED(GREEN, 1);
      initEsp();
      espPresent = true;
    }
  }
  else{
    Serial.println(F("ESP not found"));
    Serial.println(F("...check ESP UART Speed"));
    Serial.println(F("...verify ESP LED activity"));
    flashLED(RED, 5);
  }

}

void loop() {
  if (espPresent){
    //read sensor
    reading_r = readSensor();
    //send message
    sendMessage();
    //put ESP to sleep
    Serial.println(F("Put ESP to sleep"));
    bool espAsleep = false;
    while (!espAsleep){
      Serial1.println(F("AT+SLEEP=2")); //put the ESP8266 to sleep
      if (recFind("OK", 5000)){
        Serial.println(F("ESP sleeping"));
        espAsleep = true;
      }
      else {
        Serial.println(F("ESP failed to sleep"));
        flushSerial();
      }
    }
    delay(900000); //delay 15 minutes
    Serial.println(F("Initialize ESP"));
    initEsp();
  }
}

//function to flush software serial read buffer
void flushSerial(){
    while (Serial1.available() > 0){
      Serial1.read();
    }
}

//function to flash LEDs a specified number of times
void  flashLED(uint8_t color, uint8_t flashCount){
  #ifdef EnableLEDs
    for (uint8_t flsh = 0; flsh < flashCount; flsh++ ){
      digitalWrite(color, HIGH);
      delay(150);
      digitalWrite(color, LOW);
      delay(100);
    }
  #endif
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
    //reset ESP
    digitalWrite(ESPRESETPIN, LOW);
    delay(300);
    digitalWrite(ESPRESETPIN, HIGH);
    //wait for esp ready
    if (recFind("ready", 5000)){
      Serial.println(F("ESP Ready"));
      espReady = true;
      flashLED(GREEN, 2);
      }
    else {
      Serial.println(F("ESP not ready"));
      flashLED(RED, 2);
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
        flashLED(GREEN, 3);
        resetStatus = true;
        Serial.print("IP:");
        Serial.println(wifi.getLocalIP().c_str());
        }
      //join wifi if not connected
      else {
        Serial.println(F("wifi not joined")); 
        //check to see if esp is responding
        if (wifi.kick()){
        Serial.println(F("ESP is up"));
        Serial1.println(F("AT+CWJAP?"));
          if (recFind("No AP", 5000)){
            Serial.println(F("No AP configured"));
            flushSerial();
            //join wifi
            if (wifi.joinAP(SSID, PASSWORD)){
              Serial.println(F("wifi joined"));
              flashLED(GREEN, 3);
              resetStatus = true;
              Serial.print("IP:");
              Serial.println( wifi.getLocalIP().c_str());
            }
            else {
              Serial.println(F("failed to join wifi"));
              flashLED(RED, 3);
              }
          }
          else {
            Serial.println(F("no response to AT+CWJAP command"));
            flashLED(RED, 3);
            }
        }
        else {
          Serial.println(F("ESP didn't respond to kick"));
          flashLED(RED, 3);
          }
        }
      }
    flushSerial();
 }
}

struct reading readSensor(){
  uint8_t sumReadHUM = 100;
  uint8_t sumReadTMP = 100;
  uint8_t trashReadHUM = 0;
  uint8_t trashReadTMP = 0;
  //Enable DHT22 VCC pin
  pinMode(DHTPIN, INPUT);
  delay(100);
  Serial.println(F("read sensor"));
  //read sensor and discard first reading
  trashReadTMP += dht.readTemperature(true);
  trashReadHUM += dht.readHumidity();
  delay(2200);
  //read sensor 2 more times and average
  sumReadTMP += dht.readTemperature(true);
  sumReadHUM += dht.readHumidity();
  delay(2200);
  sumReadTMP += dht.readTemperature(true);
  sumReadHUM += dht.readHumidity();
  pinMode(DHTPIN, INPUT_PULLUP);
  //add readings to struct and return
  reading_r.h = (sumReadHUM / 2);
  reading_r.f = (sumReadTMP / 2);
  Serial.println(reading_r.h);
  Serial.println(reading_r.f);
  return reading_r;
}

void sendMessage(){
  connectStat = 0;
  uint8_t disableCount = 0;
  while (!connectStat){

    //open tcp conncection to thingspeek.com
    if (wifi.createTCP(HOST_NAME, HOST_PORT)){
      Serial.println(F("TCP connected"));
      flashLED(GREEN, 4);
    
      //build data package
      String outmessage = "GET /update?key=";
      outmessage+=channelKey;
      outmessage+="&field1=";
      outmessage+=reading_r.h;
      /* you can create addditional field definitions to upload more data fields by duplicating the following 
      *  two lines of code and incrementing the 'field#' number in the first line and specifying another variable 
      *  to upload in the second line */
      outmessage+="&field2=";
      outmessage+=reading_r.f;
      //add the rest of the data packet
      outmessage+=" HTTP/1.0\r\n\r\n";

      //find length of outmessage and add 1
      int outmessage_len = outmessage.length() + 1; 
      // Prepare the character array (the buffer) 
      char outChar[outmessage_len];
      // Copy string message to character array for sending
      outmessage.toCharArray(outChar, outmessage_len);
  
      //Send data package
      if (wifi.send((const uint8_t*)outChar, outmessage_len)){
        Serial.println(F("send success!"));
        flashLED(GREEN, 5);
        flushSerial();
        connectStat = 1;
      }
      else {
        Serial.println(F("send failed"));
        Serial1.println(F("AT+CIPCLOSE"));
        flashLED(RED, 5);
     }
   }
   else {
      Serial.println(F("socket error"));
      Serial1.println(F("AT+CIPCLOSE"));
      flashLED(RED, 4);
  }
  if (connectStat){
    failCount = 0;
    disableCount = 0;
  }
  else {
    if (disableCount == 3){
      failCount = 0;
      disableCount = 0;
      connectStat = 1;
    }
    else {
      if (failCount == 3){
        flushSerial();
        initEsp();
        failCount = 0;
        disableCount++;
      }
      else {
        failCount++;
       }
     }
    }
   flushSerial();
  }
}

