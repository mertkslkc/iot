/*

BulutBilisimciler.com IoT Sıcaklık ve Nem uygulaması kodlarıdır....
Bu uygulamada DHT11 sıcaklık ve nem sensörü ile NodeMCU (ESP8266) kullanılmıştır.

mertkislakci@gmail.com
 
*/

#include "DHT.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

const char* ssid = "Wifi Adını Buraya Giriniz";  //WiFi adı
const char* password = "Wifi Şifresini Buraya Giriniz"; //WiFi şifresi

//BulutBilişimciler üzerinden ayağa kaldırdığınız NodeRed sunucunun adresi /iot öncesini silip kendi adresinizle değiştirin.
const char* url = "https://ip10-3-104-4-ch6fb09od340kjsj0hlg-1880-nemrut00oap0.direct.bulutbilisimciler.com/iot"; 

std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
HTTPClient https;


#define DHTPIN D4  // DHT'nin dijital pini NodeMCU (ESP8266) D4 pinine bağlanmıştır.
#define DHTTYPE DHT11 // Bu uygulamada DHT11 kullanılmıştır.
 
DHT dht(DHTPIN, DHTTYPE);
float h, t, c;

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("WiFi Bağlanılıyor");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  client->setInsecure();

}

void loop() {

  if ((WiFi.status() == WL_CONNECTED)) {
    Serial.print("İstek Oluşturuluyor. \n");
    if (https.begin(*client, url)) {

      Serial.print("POST Atıldı !\n");

      float h = dht.readHumidity(); //Ölcülen nem
      float t = dht.readTemperature(); //Ölcülen sıcaklık
      float c = dht.computeHeatIndex(t, h, false); //Hissedilen sıcaklık

      if (isnan(h) || isnan(t) || isnan(c)) {
        Serial.println("DHT Ölçüm Yapamadı!");
        return;
      }
      
      StaticJsonDocument<200> doc;
      String requestBody;

      doc["sicaklik"] = t;
      doc["nem"] = h;
      doc["hissedilen"] = c;

      serializeJson(doc, requestBody);

      https.addHeader("Content-Type", "application/json");
      int httpCode = https.POST(requestBody);

      if (httpCode > 0) {

        Serial.printf("Dönen Yanıt: %d\n", httpCode);

        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.print("Gönderilen:");
          Serial.println(payload);
        }
      } else {
        Serial.printf("İstek Başarısız Oldu: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("Sunucu ile bağlantı kurulamıyor. \n");
    }
  }
   Serial.println("----------------------------------------------------");
  Serial.println("5 Saniye sonra yeni istek iletilecek.");
  Serial.println("----------------------------------------------------");
  delay(5000);
}
