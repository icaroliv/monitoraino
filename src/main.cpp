#include <Adafruit_Si7021.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <WiFi.h> 
HTTPClient http;
WiFiClient clientWiFi;
PubSubClient client(clientWiFi);
Adafruit_Si7021 sensor = Adafruit_Si7021();
const size_t capacity = JSON_OBJECT_SIZE(5) + 30;
DynamicJsonDocument doc(capacity);
String output;
const char* stateLight="";
char* msg="";
int ultimaLeitura = 0; 
int last =0;
int now = 0;
#define mS_TO_S_FACTOR 100000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SEND  36000
WiFiUDP ntpUDP;
NTPClient timeClient (ntpUDP);

void callback(char* topic, byte* payload, unsigned int length){
     //Serial.print("chegou");
    // digitalWrite(2,!digitalRead(2));

    
      Serial.println(topic);

   //  char* msg = (char*)payload;
      // Serial.println(topic);
   //   Serial.println((char*)payload);
     if(strcmp(topic,"monitoraino/light/conn")==0){
      //   digitalWrite(2,!digitalRead(2));
     }else{
       // Serial.println((char*)payload);
        digitalWrite(2,!digitalRead(2));
     }
     
     if(digitalRead(2)){
        stateLight="L";
     }else{
        stateLight="D";
     }
     client.publish("monitoraino/light/status",stateLight);

}

void connectWifi(){
  while(WiFi.status() != WL_CONNECTED){
     Serial.print(".");
     delay(500);
   }
}

void connectBroker(){
  
  if(client.connect("client912")){
     Serial.println("Conectado ao Broker");
     digitalWrite(2,HIGH);
     client.subscribe("monitoraino/light");
     client.subscribe("monitoraino/light/conn");
   }
}

void setup() {
   Serial.begin(115200);
   pinMode(2,OUTPUT);
   WiFi.begin("INTELBRAS V","veraluciasantos5843");
   client.setServer("iot.eclipse.org",1883);
   client.setCallback(callback);
   timeClient.begin();
   
   connectWifi();

   connectBroker();

   timeClient.setTimeOffset(-10800);

   while(!timeClient.update()){
      timeClient.forceUpdate();
    }
   
   if(!sensor.begin()){
      Serial.println("Erro na inicialização do sensor");
   }

  

}



void loop() {
  if(!WiFi.status() != WL_CONNECTED){
     connectWifi();
  }
  if(!client.connected()){
    connectBroker();
  }
  client.loop();
  int atual = millis();
  int diferenca = atual - ultimaLeitura;
  
  if(diferenca >= 5000){
      ultimaLeitura = atual;
      
      doc["umidade"] = sensor.readHumidity();
      doc["temperatura"] = sensor.readTemperature();
      serializeJson(doc,output);
      Serial.println(output);
      char buff[output.length()+1];
      output.toCharArray(buff,output.length()+1);
      client.publish("clima/sd",buff);
      
      int timepost = millis();
      int diff = timepost - last;
      
      if(diff>=3600000){
         last = timepost;
         doc["time"] = timeClient.getFormattedDate();
         http.begin("https://icapiweather.herokuapp.com/weather");
         http.addHeader("Content-Type","application/json");
         String out = "";
         serializeJson(doc,out);
         http.POST(out);
         teste = "";
         http.end();
        // Serial.println("oi");
      }

      
      output = "";
      doc.clear();
  }
  

  //client.loop();

}