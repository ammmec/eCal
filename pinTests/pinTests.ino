#define PWR D11
#define CS 5
#define DC D3
#define RST D6
#define BUSY D7

void setup() {
  Serial.begin(115200);
  pinMode(PWR, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(DC, OUTPUT);
  pinMode(RST, OUTPUT);
  pinMode(BUSY, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(SCK, OUTPUT);
}

void loop() {
  delay(5000);
  Serial.println("High");
  digitalWrite(PWR, HIGH);  // 7
  digitalWrite(CS, HIGH);   // 5
  digitalWrite(DC, HIGH);   // 14
  digitalWrite(RST, HIGH);  // 1
  digitalWrite(BUSY, HIGH); // 18
  digitalWrite(SCK, HIGH);  // 22
  digitalWrite(MOSI, HIGH); // 23
  delay(5000);
  Serial.println("Low");
  digitalWrite(PWR, LOW);
  digitalWrite(CS, LOW);
  digitalWrite(DC, LOW);
  digitalWrite(RST, LOW);
  digitalWrite(BUSY, LOW);
  digitalWrite(SCK, LOW);
  digitalWrite(MOSI, LOW);
}
