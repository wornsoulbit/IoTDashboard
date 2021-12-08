#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include "DHTesp.h"

DHTesp dht;
//const char * ssid = "TP-Link_8856";
//const char * password = "87973365";

const char * ssid = "Helix_2010";
const char * password = "Boutinerie";

//const char * ssid = "Please_Let_Me_See_My_Kids";
//const char * password = "sussybaka";

const char * mqtt_server = "10.0.0.46";

//const char * mqtt_server = "192.168.1.104";

#define D3 0
#define D4 2
constexpr uint8_t RST_PIN = D3;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = D4;     // Configurable, see typical pin layout above

// FIX THESE PINS, THEY ARE WRONG, CHANGED TO ALLOW FOR RFID READER
int buzzer = 16;
int light_pin = 5;
int fan_pinA = 1;
int fan_pinB = 10;
int fan_pinC = 9;
int photoResist = 15;

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

WiFiClient vanieriot;
PubSubClient client(vanieriot);

void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("WiFi connected - ESP-8266 IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(String topic, byte * message, unsigned int length) {
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    String messagein;

    for (int i = 0; i < length; i++) {
        Serial.print((char) message[i]);
        messagein += (char) message[i];
    }

    if (topic == "room/light") {
        if (messagein == "true") {
            Serial.println(" Light is ON");
            digitalWrite(light_pin, HIGH);
        } else {
            Serial.println(" Light is OFF");
            digitalWrite(light_pin, LOW);
        }
    }

    if (topic == "room/fan") {
        if (messagein == "true") {
            Serial.println(" Fan is ON");
            digitalWrite(fan_pinA, HIGH);
            digitalWrite(fan_pinB, LOW);
            digitalWrite(fan_pinC, HIGH);
        } else {
            Serial.println(" Fan is OFF");
            digitalWrite(fan_pinA, LOW);
            digitalWrite(fan_pinB, LOW);
            digitalWrite(fan_pinC, LOW);
        }
    }

    if (topic == "IoTlab/IntruderBuzzer") {
        if (messagein == "Invalid ID") {
            Serial.println(" Intruder Alert!");
            digitalWrite(buzzer, HIGH);
            delay(5000);
            digitalWrite(buzzer, LOW);
        } else {
            digitalWrite(buzzer, LOW);
        }
    }
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");

        //  String clientId = "ESP8266Client-";
        // clientId += String(random(0xffff), HEX);
        // Attempt to connect
        // if (client.connect(clientId.c_str())) {
        if (client.connect("vanieriot")) {

            Serial.println("connected");
            client.subscribe("room/light");
            client.subscribe("room/fan");
            client.subscribe("room/photoresistor");
            client.subscribe("IoTlab/IntruderBuzzer");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

String tag;
void setup() {

    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    dht.setup(4, DHTesp::DHT11);
    pinMode(light_pin, OUTPUT);
    pinMode(fan_pinA, OUTPUT);
    pinMode(fan_pinB, OUTPUT);
    pinMode(fan_pinC, OUTPUT);
    pinMode(photoResist, INPUT);
    pinMode(buzzer, OUTPUT);
    SPI.begin(); // Init SPI bus
    rfid.PCD_Init(); // Init MFRC522
}

void general_publish(){
  float temp = dht.getTemperature();
   float hum = dht.getHumidity();
   float photoResistor = analogRead(photoResist);
    
   char tempArr[8];
   dtostrf(temp, 6, 2, tempArr);
   char humArr[8];
   dtostrf(hum, 6, 2, humArr);
   char resistArr[8];
   dtostrf(photoResistor, 6, 2, resistArr);

   client.publish("IoTlab/temperature", tempArr);
   client.publish("IoTlab/humidity", humArr);
   client.publish("IoTlab/photoResistor", resistArr);
}

void publish_rfid(){
  if (rfid.PICC_ReadCardSerial()) {
    for (byte i = 0; i < 4; i++) {
      tag += rfid.uid.uidByte[i];
    }
    //Serial.println(tag);

    int str_len = tag.length()+1;
    char tag_array[str_len];
    tag.toCharArray(tag_array, str_len); 
    
    client.publish("IoTlab/RFID", tag_array);                                                                             
    tag = "";
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    if (!client.loop())
        client.connect("vanieriot");

    if(!rfid.PICC_IsNewCardPresent()){
      float temp = dht.getTemperature();
      float hum = dht.getHumidity();
      float photoResistor = analogRead(photoResist);
    
      char tempArr[8];
      dtostrf(temp, 6, 2, tempArr);
      char humArr[8];
      dtostrf(hum, 6, 2, humArr);
      char resistArr[8];
       dtostrf(photoResistor, 6, 2, resistArr);

       client.publish("IoTlab/temperature", tempArr);
       client.publish("IoTlab/humidity", humArr);
       client.publish("IoTlab/photoResistor", resistArr);

       delay(2000);
    }
    else if (!rfid.PICC_ReadCardSerial()) {return;}
    else{
      for (byte i = 0; i < 4; i++) {
        tag += rfid.uid.uidByte[i];
      }
    //Serial.println(tag);

      int str_len = tag.length()+1;
      char tag_array[str_len];
      tag.toCharArray(tag_array, str_len); 
      client.publish("IoTlab/RFID", tag_array);                                                                             
     tag = "";
     rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }
        
    //general_publish();
    //publish_rfid();
    
}
