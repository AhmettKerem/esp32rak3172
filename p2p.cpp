#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <SoftwareSerial.h>

#define TRIGGER_PIN 26
#define ECHO_PIN 27

Adafruit_SHT31 sht30 = Adafruit_SHT31();
SoftwareSerial rakSerial(16, 17); 

void setup() {
  Serial.begin(115200);
  rakSerial.begin(9600);

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  if (!sht30.begin(0x44)) {
    Serial.println("SHT30 sensör başlatılamadı!");
    while (1) delay(1);
  }

  initializeRAK3172();
}

void loop() {
  float distance = getDistance();
  Serial.print("Mesafe: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance < 100) { 
    float temperature = sht30.readTemperature();
    float humidity = sht30.readHumidity();

    Serial.print("Sıcaklık: ");
    Serial.print(temperature);
    Serial.println(" *C");
    
    Serial.print("Nem: ");
    Serial.print(humidity);
    Serial.println(" %");

    String formattedData = formatData(temperature, humidity);

    sendData(formattedData);
  }

  delay(100); 
}

float getDistance() {

  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2;

  return distance;
}

void initializeRAK3172() {
  rakSerial.println("AT+MODE=TEST");
  delay(1000);
  rakSerial.println("AT+P2P=868000000:12:125:0:8:22"); 
}

void sendData(String data) {
  rakSerial.print("AT+PSEND=");
  rakSerial.println(data);
  delay(5000);
}

String formatData(float temperature, float humidity) {
  int tempInt = (int)temperature;
  int tempFrac = (int)((temperature - tempInt) * 100);
  int humInt = (int)humidity;
  int humFrac = (int)((humidity - humInt) * 100);

  char c[40];
  sprintf(c, "AC%dB%dF%dB%d", tempInt, tempFrac, humInt, humFrac);

  return String(c);
}
