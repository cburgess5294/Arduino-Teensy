This sketch tests the LEDs and ESP8266 on the Arduino-Teensy board.
The LEDs connected to the serial ports are active-low.  This means 
that you need to set the pinmode to OUTPUT and then use digitalwrite
to set the pin LOW in order to turn them on.  To turn them off, use
digitalwrite to set the pin HIGH, or use pinmode to set the pin to INPUT mode.
Pins D13 (built-in LED pin) and D24 (bluetooth module STATE pin) are
active-high and are turned on and off like standard LEDs.
The setup routine runs once when you press reset
*/
void setup() {                
  // initialize the digital pin as an output.
  
  
  Serial.begin(115200);
  Serial.println("Started");
  pinMode(13, OUTPUT);  
  pinMode(24, OUTPUT);     
}

// the loop routine runs over and over again forever:
void loop() {
  Serial.println("Checking LEDs");
  pinMode(1, OUTPUT); // set the pinmode to OUTPUT
  Serial.println("TX1");
  digitalWrite(1, LOW); //TX1 - set the pin LOW to turn on the LED
  delay(2000);  //wait 2 seconds
  digitalWrite(1, HIGH); //set the pin HIGH to turn off the LED
  pinMode(1, INPUT);  //set pinmode back to INPUT so the LED will go out without the pin being set HIGH
  
  pinMode(0, OUTPUT);
  Serial.println("RX1");
  digitalWrite(0, LOW); //RX1
  delay(2000);
  digitalWrite(0, HIGH);
  pinMode(0, INPUT);

  Serial.println("TX2");
  pinMode(10, OUTPUT);
  digitalWrite(10, LOW); //TX2
  delay(2000);
  digitalWrite(10, HIGH);
  pinMode(10, INPUT);

  Serial.println("RX2");
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW); //RX2
  delay(2000);
  digitalWrite(9, HIGH);
  pinMode(9, INPUT);

  Serial.println("TX3");
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW); //TX3
  delay(2000);
  digitalWrite(8, HIGH);
  pinMode(8, INPUT);

  Serial.println("RX3");
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW); //RX3
  delay(2000);
  digitalWrite(7, HIGH);
  pinMode(7, INPUT);

  Serial.println("D13 - built-in LED");
  digitalWrite(13, HIGH); //D13 built-in LED - Set the pin HIGH turn on the LED
  delay(2000);
  digitalWrite(13, LOW); //Set the pin LOW to turn off the LED

  Serial.println("Bluetooth STATE pin");
  digitalWrite(24, HIGH); //D24 bluetooth STATE pin
  delay(2000);
  digitalWrite(24, LOW);
  
  // Check communications with the ESP8266
  Serial.println();Serial.print("Checking ESP8266...");
  Serial1.begin(115200);
  Serial1.println("AT+GMR");
  if(recFind("OK", 5000)){
    Serial.println("....Success");
  }
  else{
    Serial.println("Failed");
  }
  Serial.println("Test Complete");Serial.println();
  Serial1.end(); //Turn serial port 1 off so we can test the LEDs again
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
        Serial.write(rdChar);
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
