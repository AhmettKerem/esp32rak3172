#include <SoftwareSerial.h>

// ESP32'de 1. donanım seri portunu kullanıyoruz
SoftwareSerial mySerial(16, 17); // RX, TX

#define RED_LED_PIN 32
#define YELLOW_LED_PIN 33
#define GREEN_LED_PIN 27

float temperature;
float humidity;
bool validIDFlag = false; 

void setup() {
  Serial.begin(115200);

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  mySerial.begin(9600);

  sendATCommand("AT+NWM=0");
  sendATCommand("AT+P2P=868000000:12:125:0:8:22");
  sendATCommand("AT+PRECV=65535");
}

void sendATCommand(String command) {
  mySerial.println(command);
  while (mySerial.available()) {
    mySerial.readString();
  }
}

void sendData(String dataToSend) {
  String data = String("AT+PSEND=") + String(dataToSend);
  mySerial.println(data);
  delay(1000);
}

void loop() {
  if (mySerial.available()) {
    String receivedData = mySerial.readString();
    Serial.println(receivedData);

    if (receivedData.startsWith("+EVT:")) {
      
      char id = receivedData[receivedData.indexOf('C') - 1];
      if (id == 'A') {
        validIDFlag = true;
        Serial.println("ID 'A' tespit edildi, veri işleme başlıyor...");

        processData(receivedData);

        
        validIDFlag = false;
      } else {
        Serial.println("ID 'A' tespit edilmedi. Dinlemeye devam ediliyor...");
        mySerial.println("AT+PRECV=65535");
      }
    }
  }
}

void processData(String data) {
  // Sıcaklık değerini ayırıp işleyeceğiz
  int startTempIndex = data.indexOf('C') + 1;
  int endTempIndex = data.indexOf('F');
  String tempString = data.substring(startTempIndex, endTempIndex);
  tempString.replace('B', '.');
  temperature = tempString.toFloat();

  // Nem değerini ayırıp işleyeceğiz
  int startHumidityIndex = endTempIndex + 1;
  String humidityString = data.substring(startHumidityIndex);
  humidityString.replace('B', '.');
  humidity = humidityString.toFloat();

  // Kimlik numarası, sıcaklık ve nem değerlerini ekrana yazdırıyoruz
  Serial.print("Kimlik: ");
  Serial.println(data[data.indexOf('C') - 1]);
  Serial.print("Sıcaklık: ");
  Serial.println(temperature);
  Serial.print("Nem: ");
  Serial.println(humidity);

  // LED kontrolü
  controlLEDs();

  // Gönderilecek data stringini oluşturma
  String tempData = String(temperature);
  tempData.replace('.', 'D');
  String sendDataString = "B" + String("A") + tempData + "F";

  // Gönderilecek veriyi ekrana yazdırma
  Serial.print("Gönderilecek data: ");
  Serial.println(sendDataString);

  // Veriyi sürekli gönderme
  for (int i = 0; i < 500; i++) { 
    sendData(sendDataString);
    Serial.println("Gönderim yapıldı:  ");
    delay(2000);
  }
}

void controlLEDs() {
  if (temperature < 15.0) {
    // Yeşil LED yanar
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
  } else if (temperature >= 15.0 && temperature < 20.0) {
    // Sarı LED yanar
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, HIGH);
  } else {
    // Kırmızı LED yanar
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, HIGH);
  }
}
