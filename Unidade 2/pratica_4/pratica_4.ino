#include <NTPClient.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "time.h"

const char* ssid = "MR Galaxy A12";
const char* password = "23111963";

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, -10800);

const int buttonPin = 4; // Pino do botão
const int ledPin = 22; // Pino do LED
const int ledESP = 2;
int estadoBotao = 0; // Variável para armazenar o estado do botão
int estadoAnterior = 0; // Variável para armazenar o estado anterior do botão
bool ledState = false;
String lastLine;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(ledESP, OUTPUT);

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  digitalWrite(ledESP, HIGH);
  Serial.println("WiFi connected.");

  timeClient.begin();
  
  if (!SPIFFS.begin(true)) {
    Serial.println("Falha ao montar o sistema de arquivos SPIFFS.");
    return;
  }

  if(SPIFFS.exists("/led_state.txt")) {
    File file = SPIFFS.open("/led_state.txt", "r");
    if(file) {
      String currentLine;
      while(file.available()) {
        currentLine = file.readStringUntil('\n');
        if(currentLine.length() > 0) {
          lastLine = currentLine;
        }
      }
      File file = SPIFFS.open("/led_state.txt");
      Serial.println("File Content:");
      while(file.available()){
          Serial.write(file.read());
      }
      Serial.println("End File Content");
      Serial.print("Último estado do led: ");
      Serial.println(lastLine);
      if(lastLine.indexOf("ON") != -1){
        Serial.println("LIGAR LED");
        digitalWrite(ledPin, HIGH);
      } else {
        Serial.println("APAGAR LED");
        digitalWrite(ledPin, LOW);
      }
      file.close();
      linhasArquivo();
    }
  }
}

void loop() {
  timeClient.update();
  
  estadoBotao = digitalRead(buttonPin); // Lê o estado do botão
  //Serial.print("Botão: ");
  //Serial.println(estadoBotao);

  if(estadoBotao != estadoAnterior) {
    if(estadoBotao == HIGH) { // Verifica se o botão foi pressionado
      ledState = !digitalRead(ledPin);
      Serial.println(timeClient.getFormattedTime());
      Serial.print("Led: ");
      Serial.println(ledState ? "ON" : "OFF");
      digitalWrite(ledPin, !digitalRead(ledPin)); // Inverte o estado do LED
      saveLedState();
    }
    delay(50); // Pequena pausa para evitar leituras erráticas do botão
  }

  estadoAnterior = estadoBotao; // Atualiza o estado anterior do botão
}

void saveLedState() {
  File file = SPIFFS.open("/led_state.txt", "a");
  if(!file){
    Serial.println("Falha ao abrir o arquivo para escrita.");
    return;
  }
  file.print(timeClient.getFormattedTime());
  file.println(ledState ? " ON" : " OFF");
  file.close();
  linhasArquivo();
}

void linhasArquivo(){
  const char* fileName = "/led_state.txt";

  File file = SPIFFS.open(fileName, "r");
  
  if (file) {
    int lineCount = 0;

    while (file.available()) {
      String line = file.readStringUntil('\n');
      if (line.length() > 0) {
        lineCount++;
      }
    }

    Serial.print("Número de linhas no arquivo: ");
    Serial.println(lineCount);
    if(lineCount >= 100){
      if (SPIFFS.remove(fileName)) {
        Serial.println("Arquivo com mais de 100 linhas apagado com sucesso.");
        File file = SPIFFS.open("/led_state.txt", "a");
        file.println(lastLine);
      } else {
        Serial.println("Falha ao apagar o arquivo.");
      }  
    }
    file.close();
  } else {
    Serial.println("Falha ao abrir o arquivo.");
  }

}