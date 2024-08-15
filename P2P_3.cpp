#include <SoftwareSerial.h>
#include <atomic>  // std::atomic kullanmak için

/////////////////////////////////////////////
#define NJM "0"
#define DEV_ADDR  "0017a969"
#define APPS_KEY  "3030f340cefc7d7b6a1fd2288784d4b5"
#define NWKS_KEY  "3030f340cefc7d7b6a1fd2288784d4b5"
#define DEV_EUI "af38b281c152b52d"
#define APP_KEY  "5ef53fce707e999f21331a440b704eeb"
#define CLASS  "A"
#define NWM   "1"
#define NJM   "0"
#define BAND "4"
#define JOIN_REQ "1:0:8:10"
#define P2P_FREQ      "868000000"
#define P2P_SF        "12"
#define P2P_BANDWITH  "125"
#define P2P_CR        "0"
#define P2P_PREAMBLE  "8"
#define P2P_POWER     "22"
#define P2P_OPS P2P_FREQ":"P2P_SF":"P2P_BANDWITH":"P2P_CR":"P2P_PREAMBLE":"P2P_POWER
///////////////////////////////////////////////////


int recent_mode = 0;

SoftwareSerial mySerial(16, 17);
String temp;
String humidity;
String value;
uint8_t lflag = 0;


std::atomic<bool> rxPending{false};
void IRAM_ATTR receiveHandler() {
  rxPending.store(true);
}

/////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);
  mySerial.setTimeout(1000);  //30 mmfph! //100 //1000ms changed to 10ms because of when data transmission if device gets a p2p data when it already has, connection cuts off. I don't know why. Also 10 default. but 8 and 5 needs to be explored.
  mySerial.onReceive(receiveHandler);
  set_conf(0);
}

void loop() {
  if(!lflag){
    openP2pListen();
    lflag = 1;
  }

  value = listenRes();

  if(value.indexOf("+EVT") != -1)
  {
    
    Serial.println(value);
    bool control1 = control(value);
    if(control1){
      String B = portSend("AT+NWM=1");
    
      while (B.indexOf("OK") == -1) {
        B = portSend("AT+NWM=1");
      }
      set_keys(0);
      set_conf(1);
      
      //portSend("AT+JOIN=" JOIN_REQ);
      B = portSend("AT+SEND=1:" + String(temp));

      while (B.indexOf("+EVT:SEND CONFIRMED") == -1) {
        
        B = portSend("AT+SEND=1:" + String(temp));
      }
      
      set_conf(0);
      set_conf(0);
      lflag = 0;
    }
    
  }
  
}

//////////////////////////////////////////////////////////////////////////////

[[nodiscard]] String listenRes() {
    String holdStr{};
    if (rxPending.load() && mySerial.available()) {
      holdStr = mySerial.readString();
      rxPending.store(false);
    }
    holdStr.trim();
    
    return holdStr;
}

/////////////////////////////////////////////////////////////////////////////////////

void openP2pListen() {
  String checkReturn = portSend("AT+PRECV=65535");
checkTemp:
  if (checkReturn.indexOf("OK") != -1){
    Serial.println("P2P Listen has opened.");
    
  }
  else if (checkReturn.indexOf("AT_ERROR") != -1) {
    checkReturn = portSend("AT+PRECV=65535");
    Serial.println("hata");
    goto checkTemp;
  }
  
  rxPending.store(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

bool control(String data){
  
  if(data.indexOf("BA")!=-1){
    int index_split = data.indexOf('\n');
    data = data.substring(index_split+6);
    int index_A = data.indexOf('A');
    int index_D = data.indexOf('D');
    int index_F = data.indexOf('F');
    temp = String(data.substring(index_A+1,index_D))+String('B')+String('B')+String(data.substring(index_D+1,index_F));
    
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println();
    return true;
  }
  else{
    return false;
  }
}

/////////////////////////////////////////////////////////////////////////////////

String portSend(String pdata) {
  mySerial.println(pdata);
  delay(10);
  
listenme:
  String resValue = listenRes();
  
  if (resValue.isEmpty())
    goto listenme;
  else
    return resValue;
}

/////////////////////////////////////////////////////////////////////////////
void set_keys(int choice) {
  String B = "";
  if (0 == choice) {
    if (NJM == "0") {

      Serial.println("Recent join mode is ABP.");
      B = portSend("AT+DEVADDR=" DEV_ADDR);
      

      while (B.indexOf("OK") == -1) {
        B = portSend("AT+DEVADDR=" DEV_ADDR);
      }
      
      B = portSend("AT+APPSKEY=" APPS_KEY);

      while (B.indexOf("OK") == -1) {
        B = portSend("AT+APPSKEY=" APPS_KEY);
      }
      
      B = portSend("AT+NWKSKEY=" NWKS_KEY);

      while (B.indexOf("OK") == -1) {
        B = portSend("AT+NWKSKEY=" NWKS_KEY);
      }
      
    } else {
      Serial.println("Recent join mode is OTAA.");

      B = portSend("AT+DEVEUI=" DEV_EUI);

      while (B.indexOf("OK") == -1) {
        B = portSend("AT+DEVEUI=" DEV_EUI);
      }

      B = portSend("AT+APPKEY=" APP_KEY);

      while (B.indexOf("OK") == -1) {
        B = portSend("AT+APPKEY=" APP_KEY);
      }
    }

    Serial.println("Auto Register (choice = 0) completed");
  }
}

////////////////////////////////////////////////////////////////////////////

void set_conf(int mode_selector) { 
  recent_mode = mode_selector;
  if (1 == mode_selector) {  // NWM = LORAWAN

    String B = portSend("AT+CLASS=" CLASS);

    while (B.indexOf("OK") == -1) {
      B = portSend("AT+CLASS=" CLASS);
    }
    B = portSend("AT+BAND=" BAND);

    while (B.indexOf("OK") == -1) {
      B = portSend("AT+BAND=" BAND);
    }
    B = portSend("AT+NJM=" NJM);

    while (B.indexOf("OK") == -1) {
      B = portSend("AT+NJM=" NJM);
    }

    B = portSend("AT+JOIN=" JOIN_REQ);

    while (B.indexOf("JOINED") == -1) {
      
      B = listenRes();
      if (B.indexOf("JOIN FAILED") != -1) {
        Serial.println("Network Join Session has been failed. The response is = " + B + " Trying to join again..");
        B = portSend("AT+JOIN=" JOIN_REQ);
        
      }
    }

    Serial.println("Network Join Session has been COMPLETE. The response is = " + B);
  } else if (0 == mode_selector) {

    // NWM = P2P
    //<Freq>,<SF>,<Bandwidth>,<CR>,<Preamble>,<Power>
    //AT+P2P=868000000:7:125:0:8:15
    String B = portSend("AT+NWM=0");

    while (B.indexOf("OK") == -1) {
      B = portSend("AT+NWM=0");
    }
    B = portSend("AT+P2P=" P2P_OPS);

    while (B.indexOf("OK") == -1) {
      B = portSend("AT+P2P=" P2P_OPS);
    }
  }
  
}