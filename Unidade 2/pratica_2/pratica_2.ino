#define LED_PIN 2
#define BLINK_INTERVAL 250

#define SENSORINICIO_PIN 15
#define SENSOR1_PIN 27
#define SENSOR2_PIN 4
#define LEDINICIO_PIN 13
#define LED1_PIN 12
#define LED2_PIN 14

bool gameStarted = false; 
unsigned long startTime; 
int player1 = 0; 
int player2 = 0;

void setup() {
  Serial.begin(115200); 
  pinMode(SENSORINICIO_PIN, INPUT);
  pinMode(SENSOR1_PIN, INPUT);
  pinMode(SENSOR2_PIN, INPUT);
  pinMode(LEDINICIO_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
}

void loop() {
  if(!gameStarted) {
    if(touchRead(SENSORINICIO_PIN) < 20) {
      startGame();
    }
  } 
  if(gameStarted){
    Serial.print("Sensor 1: ");
    int touch1 = touchRead(SENSOR1_PIN);
    Serial.println(touch1);
    Serial.print("Sensor 2: ");
    int touch2 = touchRead(SENSOR2_PIN);
    Serial.println(touch2);
    if(touch1 < 13 & touch1 > 9){
      Serial.print("1 Tocou: ");
      Serial.println(touch1);
      playerWins(1, playerTime());
    }
    if(touch2 < 13 & touch2 > 9){
      Serial.print("2 Tocou: ");
      Serial.println(touch2);
      playerWins(2, playerTime());
    }
    delay(200);
   
  }
}

void startGame() {
  gameStarted = true;
  for(int i = 0; i < 3; i++) {
    digitalWrite(LEDINICIO_PIN, HIGH);
    delay(500);
    digitalWrite(LEDINICIO_PIN, LOW);
    delay(500);
  }
  digitalWrite(LEDINICIO_PIN, HIGH);
  Serial.println("Jogo começou");
  startTime = millis(); // Registra o tempo de início do jogo
}

unsigned long playerTime(){
  unsigned long endTime = millis(); // Hora de término do jogo
  unsigned long elapsedTime = endTime - startTime; // Calcula o tempo decorrido
  return elapsedTime;
}

void playerWins(int player, unsigned long elapsedT){
  unsigned long elapsedTime = elapsedT;
  
  Serial.print("Jogador ");
  Serial.print(player);
  Serial.println(" venceu!");
  Serial.print("Tempo: ");
  Serial.print(elapsedTime/1000);
  Serial.println(" s");
  
  if(player == 1) {
    player1++;
    digitalWrite(LED1_PIN, HIGH);
  } else {
    player2++;
    digitalWrite(LED2_PIN, HIGH);
  } 

  Serial.println("========");
  Serial.println("Placar:");
  Serial.print("Jogador 1: ");
  Serial.println(player1);
  Serial.print("Jogador 2: ");
  Serial.println(player2);
  Serial.println("========");
  
  delay(4000);
  
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LEDINICIO_PIN, LOW);
  
  gameStarted = false;
  Serial.println("");
  Serial.println("jogo recomeçou");
}