#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> 
#include <SPI.h>
#include <MFRC522.h>

//-------- Customise these values -----------
const char* ssid = "JioFi3_";
const char* password = "5jyzt";

#define ORG "z693gh"
#define DEVICE_TYPE "nodemcu"
#define DEVICE_ID "nodemcu"
#define TOKEN "nodemcu123"
//-------- Customise the above values --------
#define RST_PIN         D2          // Configurable, see typical pin layout above
#define SS_1_PIN        D3        // Configurable, take a unused pin, only HIGH/LOW required, must be diffrent to SS 2
#define SS_2_PIN        D4 // Configurable, take a unused pin, only HIGH/LOW required, must be diffrent to SS 1


char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

const char publishTopic[] = "iot-2/evt/status/fmt/json";
const char responseTopic[] = "iotdm-1/response";
const char manageTopic[] = "iotdevice-1/mgmt/manage";
const char updateTopic[] = "iotdm-1/device/update";
const char rebootTopic[] = "iotdm-1/mgmt/initiate/device/reboot";

void callback(char* topic, byte* payload, unsigned int payloadLength);

WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

int publishInterval = 30000; // 30 seconds
long lastPublishMillis;
int count1=0;
int count2=0;
int count3=0;
int count4=0;


#define NR_OF_READERS   2

byte ssPins[] = {SS_1_PIN, SS_2_PIN};

MFRC522 mfrc522[NR_OF_READERS];   // Create MFRC522 instance.


void setup() {
 Serial.begin(115200); Serial.println();

 wifiConnect();
 mqttConnect();
 initManagedDevice();
 SPI.begin();        // Init SPI bus

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
    count1=0;
    count2=0;
    count3=0;
    count4=0;
 
  }
}

void loop() {
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    // Look for new cards

    if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
    
      if(reader==0){
count1++;
 Serial.print(count1);
 }
      else
      if(reader==1){
      count1--;
Serial.print(count1);
}

      if(reader==2){
count2++;
 Serial.print(count2);
 }
      else
      if(reader==3){
      count2--;
Serial.print(count2);
}
      if(reader==4){
count3++;
 Serial.print(count3);
 }
      else
      if(reader==5){
      count3--;
Serial.print(count3);
}
      if(reader==6){
count4++;
 Serial.print(count4);
 }
      else
      if(reader==7){
      count4--;
Serial.print(count4);
}

      Serial.print(F("Reader "));
      Serial.print(reader);
      // Show some details of the PICC (that is: the tag/card)
      Serial.print(F(": Card UID:"));
      dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
      Serial.println();
      Serial.print(F("PICC type: "));
      MFRC522::PICC_Type piccType = mfrc522[reader].PICC_GetType(mfrc522[reader].uid.sak);
      Serial.println(mfrc522[reader].PICC_GetTypeName(piccType));
      Serial.println("count is : ");
      Serial.print(count1);
      // Halt PICC
      mfrc522[reader].PICC_HaltA();
      // Stop encryption on PCD
      mfrc522[reader].PCD_StopCrypto1();
    } //if (mfrc522[reader].PICC_IsNewC
  } //for(uint8_t reader 
 if (millis() - lastPublishMillis > publishInterval) {
   publishData();
   lastPublishMillis = millis();
 }

 if (!client.loop()) {
   mqttConnect();
   initManagedDevice();
 }
}

void wifiConnect() {
 Serial.print("Connecting to "); Serial.print(ssid);
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   Serial.print(".");
 }
 Serial.print("nWiFi connected, IP address: "); Serial.println(WiFi.localIP());
}

void mqttConnect() {
 if (!!!client.connected()) {
   Serial.print("Reconnecting MQTT client to "); Serial.println(server);
   while (!!!client.connect(clientId, authMethod, token)) {
     Serial.print(".");
     delay(500);
   }
   Serial.println();
 }
}

void initManagedDevice() {
 if (client.subscribe("iotdm-1/response")) {
   Serial.println("subscribe to responses OK");
 } else {
   Serial.println("subscribe to responses FAILED");
 }

 if (client.subscribe(rebootTopic)) {
   Serial.println("subscribe to reboot OK");
 } else {
   Serial.println("subscribe to reboot FAILED");
 }

 if (client.subscribe("iotdm-1/device/update")) {
   Serial.println("subscribe to update OK");
 } else {
   Serial.println("subscribe to update FAILED");
 }

 StaticJsonBuffer<300> jsonBuffer;
 JsonObject& root = jsonBuffer.createObject();
 JsonObject& d = root.createNestedObject("d");
 JsonObject& metadata = d.createNestedObject("metadata");
 metadata["publishInterval"] = publishInterval;
 JsonObject& supports = d.createNestedObject("supports");
 supports["deviceActions"] = true;

 char buff[300];
 root.printTo(buff, sizeof(buff));
 Serial.println("publishing device metadata:"); Serial.println(buff);
 if (client.publish(manageTopic, buff)) {
   Serial.println("device Publish ok");
 } else {
   Serial.print("device Publish failed:");
 }
}

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
void publishData() {
 /*
 String payload = String(count1);
 payload += millis() / 1000;
 payload += "}}";

 Serial.print("Sending payload: "); Serial.println(payload);

 if (client.publish(publishTopic, (char*) payload.c_str())) {
   Serial.println("Publish OK");
 } else {
   Serial.println("Publish FAILED");
 }
}*/
 StaticJsonBuffer<500> jsonBuffer;
 JsonObject& root = jsonBuffer.createObject();
 JsonObject& data = root.createNestedObject("data");
 data["count1"] = count1;
 data["count2"] = count2;
 data["count3"] = count3;
 data["count4"] = count4;

 char bufferer[500];
 root.printTo(bufferer, sizeof(bufferer));
 Serial.println("Sending data:"); Serial.println(bufferer);
 if (client.publish(publishTopic, bufferer)) {
   Serial.println("Publish OK");
 } else {
   Serial.print("Publish FAILED");
 }
}

void callback(char* topic, byte* payload, unsigned int payloadLength) {
 Serial.print("callback invoked for topic: "); Serial.println(topic);

 if (strcmp (responseTopic, topic) == 0) {
   return; // just print of response for now
 }

 if (strcmp (rebootTopic, topic) == 0) {
   Serial.println("Rebooting...");
   ESP.restart();
 }

 if (strcmp (updateTopic, topic) == 0) {
   handleUpdate(payload);
 }
}

void handleUpdate(byte* payload) {
 StaticJsonBuffer<300> jsonBuffer;
 JsonObject& root = jsonBuffer.parseObject((char*)payload);
 if (!root.success()) {
   Serial.println("handleUpdate: payload parse FAILED");
   return;
 }
 Serial.println("handleUpdate payload:"); root.prettyPrintTo(Serial); Serial.println();

 JsonObject& d = root["d"];
 JsonArray& fields = d["fields"];
 for (JsonArray::iterator it = fields.begin(); it != fields.end(); ++it) {
   JsonObject& field = *it;
   const char* fieldName = field["field"];
   if (strcmp (fieldName, "metadata") == 0) {
     JsonObject& fieldValue = field["value"];
     if (fieldValue.containsKey("publishInterval")) {
       publishInterval = fieldValue["publishInterval"];
       Serial.print("publishInterval:"); Serial.println(publishInterval);
     }
   }
 }
}
