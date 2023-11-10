//codigo mestre
#include <SoftwareSerial.h>

SoftwareSerial ArduinoSlave(10, 11);

#define PIN_BOTAO 6
String answer;
String msg;
int val=0, oldVal=0;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_BOTAO, INPUT);
  ArduinoSlave.begin(9600);
}

void loop() {
  int sensorValue = analogRead(A0);
  int buttonState = digitalRead(PIN_BOTAO);
  //Read answer from slave
  readSlavePort();

  String dataToSend = String(buttonState) + "*" + String(sensorValue);

  ArduinoSlave.print(dataToSend);
  Serial.println("Enviado string para slave");
  Serial.println(dataToSend);

  delay(3000);
}

void readSlavePort(){
    if (ArduinoSlave.available() >0) {
      char c = ArduinoSlave.read(); //gets one byt
      answer += c; //makes the string readString
    } 
}