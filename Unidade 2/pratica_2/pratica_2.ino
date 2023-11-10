#include <SPI.h>
#include <MFRC522.h>

//Definição dos pinos de conexão do projeto
#define PINO_SS 9
#define PINO_RST 8
#define pinoLED_VERMELHO 6
#define pinoLED_VERDE 5
#define pinoBUZZ 13
int tentativas = 0;

//63 87 FC 94      23 8A B6 94

//Cria o item para configurar o módulo RC522
MFRC522 mfrc522(PINO_SS, PINO_RST);

void setup(){
  Serial.begin(9600); // Inicializa a serial
  SPI.begin();// Inicializa a comunicação SPI
  mfrc522.PCD_Init(); // Inicializa o módulo MFRC522
  pinMode(pinoLED_VERMELHO, OUTPUT);
  pinMode(pinoLED_VERDE, OUTPUT);
  pinMode(pinoBUZZ, OUTPUT);
  digitalWrite(pinoLED_VERMELHO, HIGH);
}

void loop(){
  String tagLida= ""; //Cria uma variável vazia, do tipo string
  if(mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.println("UID da tag: "); // Mostra UID do token na serial
    for(byte i = 0; i < mfrc522.uid.size; i++){
      //tagLida.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      tagLida.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    tagLida.toUpperCase();
    Serial.println(tagLida);// Printa a mensagem convertida em hexadecimal
    if(tagLida == "6387FC94"){  // Libera o acesso
      tentativas = 0;
      digitalWrite(pinoLED_VERMELHO, LOW);
      delay(200);
      Serial.println("Acesso liberado");
      digitalWrite(pinoLED_VERDE, HIGH);
      delay(10000);
      digitalWrite(pinoLED_VERDE, LOW);
      digitalWrite(pinoLED_VERMELHO, HIGH);
    } else{  
      Serial.println("Cartão Inválido");
        tentativas += 1;
        if(tentativas >= 5){
          Serial.println("SISTEMA BLOQUEADO");
          digitalWrite(pinoBUZZ, HIGH); 
          for(int i=0; i<30; i++){ // led piscando por 30 segundos
            digitalWrite(pinoLED_VERMELHO, LOW);
            delay(500);
            digitalWrite(pinoLED_VERMELHO, HIGH);
            delay(500);
          }
          digitalWrite(pinoBUZZ, LOW);
          tentativas=0;
        } else{
          for(int i=0; i<3; i++){
            digitalWrite(pinoLED_VERMELHO, LOW);
            delay(500);
            digitalWrite(pinoLED_VERMELHO, HIGH);
            delay(500);
          }
        }
    }
  }

  delay(500);
}