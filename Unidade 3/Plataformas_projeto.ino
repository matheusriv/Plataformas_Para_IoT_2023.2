#include <ESP32Servo.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "DHT.h"
#include "SPIFFS.h"
#include <FS.h>
#include "time.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

#define LED 2

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

#define BOTtoken "BoT_Token"
// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "chat_id"

WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, -10800);

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const char* wifi_ssid = "wifi_ssid";
const char* wifi_password = "wifi_password";
int wifi_timeout = 100000;

const char* mqtt_broker = "io.adafruit.com";
const int mqtt_port = 1883;
int mqtt_timeout = 10000;

const char* mqtt_usernameAdafruitIO = "username_adafruit";
const char* mqtt_keyAdafruitIO = "key_adafruit";

Servo servo_racao; 
Servo servo_gaiola;  

const int trigPin = 5;
const int echoPin = 18;
int servo_racao_Pin = 13;
int servo_gaiola_Pin = 12;

long duration;
float distanceCm;
unsigned long previousMillis1;  // Variável para armazenar o tempo da última verificação
unsigned long previousMillis2;  // Variável para armazenar o tempo da última ação (para 10 segundos)
const long botRequestDelay = 1000;  
const long adafruitDelay = 10000; // Intervalo de 10 segundos
char buffer[10];
bool gaiolaFechada = false;
bool gaiolaAberta = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);

  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  servo_racao.setPeriodHertz(50);    // standard 50 hz servo
  servo_racao.attach(servo_racao_Pin, 1000, 2000); // attaches the servo on pin to the servo object
  servo_gaiola.setPeriodHertz(50);    // standard 50 hz servo
  servo_gaiola.attach(servo_gaiola_Pin, 1000, 2000); // attaches the servo on pin to the servo object

  WiFi.mode(WIFI_STA); //"station mode": permite o ESP32 ser um cliente da rede WiFi
  WiFi.begin(wifi_ssid, wifi_password);
  connectWiFi();
  openFS();
  timeClient.begin();
  mqtt_client.setServer(mqtt_broker, mqtt_port);
}

void loop() { 
  timeClient.update();

  unsigned long currentMillis = millis();

  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;

  int hora = atoi(timeClient.getFormattedTime().substring(0, 2).c_str());

  if(distanceCm <= 10){
    if(hora >= 22 && hora < 5 && !gaiolaFechada){
      Serial.println("Fecha Gaiola");
      fecharGaiola();
    } else if(hora >= 5 && !gaiolaAberta){
      Serial.println("Abre Gaiola");
      abrirGaiola();
    }
  }

  // Verifica se já passou da meia-noite para redefinir os flags
  if (hora == 0) {
    gaiolaFechada = false; // Pode fechar a gaiola novamente no próximo dia
    gaiolaAberta = false; // Pode abrir a gaiola novamente no próximo dia
  }

  if (shouldTriggerAction(hora)) {
    // Ação a ser executada 3 vezes por dia
    triggerServoMovementFeeding();
  }


  // Verificação a cada 1 segundo
  if (currentMillis - previousMillis1 >= botRequestDelay) {
    previousMillis1 = currentMillis;  // Salva o tempo atual

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("Bot got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

  }

  if(currentMillis - previousMillis2 >= adafruitDelay){
    previousMillis2 = currentMillis;  // Salva o tempo atual

    Serial.println(timeClient.getFormattedTime());

    connectMQTT();
    if(!mqtt_client.connected()) { // Se MQTT não estiver conectado
      digitalWrite(LED,LOW);
    } else {
      digitalWrite(LED, HIGH); // LED indicando conexão

      mqtt_client.publish("matheusriv/feeds/Plataformas_distancia", String(distanceCm).c_str());
      if(gaiolaFechada == true){
        mqtt_client.publish("matheusriv/feeds/Plataformas_gaiola", String(1).c_str());
      } else{
        mqtt_client.publish("matheusriv/feeds/Plataformas_gaiola", String(0).c_str());
      }      

      mqtt_client.loop();

    }
  }
}

void fecharGaiola(){
  gaiolaFechada = true;
  Serial.println("Fechar Gaiola");
  for(int pos = 0; pos <= 180; pos += 1) { 
    // in steps of 1 degree
    servo_racao.write(pos);    // tell servo to go to position in variable 'pos'
    delay(10);             // waits 15ms for the servo to reach the position
  }
  bot.sendMessage(CHAT_ID, "Alerta: gaiola abrindo às: " + timeClient.getFormattedTime(), "");
}

void abrirGaiola(){
  gaiolaAberta = true;
  Serial.println("Abrir Gaiola");
  for(int pos = 180; pos <= 0; pos += 1) {
    // in steps of 1 degree
    servo_racao.write(pos);    // tell servo to go to position in variable 'pos'
    delay(10);             // waits 15ms for the servo to reach the position
  }
  bot.sendMessage(CHAT_ID, "Alerta: pássaro detectado, gaiola fechando às: " + timeClient.getFormattedTime(), "");
}

bool shouldTriggerAction(int currentHour) {
  // Verifica se a hora atual é uma das três vezes por dia desejadas (por exemplo: 8h, 14h e 20h)
  if (currentHour == 8 || currentHour == 14 || currentHour == 20) {
    return true;
  }

  return false;
}

void triggerServoMovementFeeding() {
  Serial.println("Trigger servo movement feeding");
  Serial.println("75 para 150");
  for (int pos = 75; pos <= 150; pos += 1) {
    servo_racao.write(pos);
    delay(10);
  }
  delay(8000);

  Serial.println("150 para 75");
  for (int pos = 150; pos >= 75; pos -= 1) {
    servo_racao.write(pos);
    delay(10);
  }
  delay(10000);
}

void readFile(String path) {
  Serial.printf("Lendo arquivo: %s\n", path);

  File file = SPIFFS.open(path, "r");
  if (!file) {
    Serial.println("Não foi possível abrir o arquivo");
    return;
  }

  Serial.print("---------- Lendo arquivo ");
  Serial.print(path);
  Serial.println("  ---------");

  while (file.available()) {
    Serial.write(file.read());
  }
  Serial.println("\n------ Fim da leitura do arquivo -------");
  file.close();
}

void formatFile() {
  Serial.println("Formantando SPIFFS");
  SPIFFS.format();
  Serial.println("Formatou SPIFFS");
}

void openFS(void) {
  if (!SPIFFS.begin(true)) {
    Serial.println("\nErro ao abrir o sistema de arquivos");
  }
  else {
    Serial.println("\nSistema de arquivos aberto com sucesso!");
  }
}

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    String text = bot.messages[i].text;
    //Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if(text == "/start") {
      String welcome = "Olá, " + from_name + ".\n";
      welcome += "Use os seguintes comandos para receber mensagens.\n\n";
      welcome += "/gaiola_status\n";
      welcome += "/horarios_racao\n";
      welcome += "/distancia\n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if(text == "/gaiola_status") {
      if(gaiolaAberta == true){
        bot.sendMessage(chat_id, "Gaiola aberta", "");
      } else{
        bot.sendMessage(chat_id, "Gaiola fechada", "");
      }
    }

    if(text == "/horarios_racao") {
      bot.sendMessage(chat_id, "Horários de alimentação programados: 8h, 14h, 20h", "");
    }

    if(text == "/distance") {
      bot.sendMessage(chat_id, "Distância atual medida: " + String(distanceCm, 2) + "cm", "");
    }

  }
}

void connectWiFi() {
  Serial.print("Conectando à rede WiFi .. ");

  unsigned long tempoInicial = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - tempoInicial < wifi_timeout)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  #ifdef ESP32
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conexão com WiFi falhou!");
  } else {
    Serial.print("Conectado com o IP: ");
    Serial.println(WiFi.localIP());
  }
}

void connectMQTT() {
  unsigned long tempoInicial = millis();
  while (!mqtt_client.connected() && (millis() - tempoInicial < mqtt_timeout)) {
    if (WiFi.status() != WL_CONNECTED) {
      connectWiFi();
    }
    Serial.print("Conectando ao MQTT Broker..");

    if (mqtt_client.connect("ESP32Client", mqtt_usernameAdafruitIO, mqtt_keyAdafruitIO)) {
      Serial.println(" Conectado!");
    } else {
      Serial.println(" Conexão falhou!");
      delay(200);
    }
  }
  Serial.println();
}