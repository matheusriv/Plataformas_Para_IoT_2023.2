//codigo escravo
#include <SoftwareSerial.h>
SoftwareSerial ArduinoMaster (10, 11);
#define ledPin 5
String msg="";
int intVal=0, oldIntVal=0;
int buttonState, sensorValue;

void setup() {
  Serial.begin(9600);
  ArduinoMaster.begin(9600);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  readMasterPort();
  convertMsgToCmd();
  // Send answer to master

  if (buttonState == 1) {
    analogWrite(ledPin, 255); // Ligar o LED na alta intensidade
  } else {
    int mp = map(sensorValue, 0, 1023, 255, 0);
    analogWrite(ledPin, mp);
    Serial.println(mp);
    if (mp < 40) {
      Serial.println("Baixa luminosidade!");
    }
  }
}

void readMasterPort(){
  while (ArduinoMaster.available()) { 
    delay(10);
    if (ArduinoMaster.available() >0) { 
      char c = ArduinoMaster.read();

      msg += c; //makes the string readString
    }
  }
  ArduinoMaster.flush();
}

void convertMsgToCmd(){
  if (msg.length() >0) { 

    char carrayl[6]; //magic needed to convert stri

    msg.toCharArray(carrayl, sizeof(carrayl));
    
    int separatorIndex = msg.indexOf('*');
    if (separatorIndex != -1) {
      buttonState = msg.substring(0, separatorIndex).toInt();
      sensorValue = msg.substring(separatorIndex + 1).toInt();
    }

    msg="";
  }

}