#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <SoftwareSerial.h>

#define AP_SSID "zombie"
#define AP_PWD "12345678"

#define port 80
#define host "xyz.azurewebsites.net"

// sha1 hash your apisecret at www.sha1-online.com
#define apisecret "123"

SoftwareSerial wixel(4,5);    // nodemcu+wixel
//SoftwareSerial wixel(12,14);  // huzzah+wixel

int ledpin=16; // nodemcu
//int ledpin=0; // huzzah

String wixelread(){
  String content="";
  char ch;
  Serial.write("reading from wixel .. ");
  while (wixel.available()) {
    ch = wixel.read();
    content.concat(ch);
  }

  Serial.print("returning '");
  Serial.print(content);
  Serial.println("'");
  return content;
  
}

void upload(String now, String content){

// light up led while we upload

 digitalWrite(ledpin, HIGH);
 
Serial.println("uploading..");
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = port;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  int ix0 = 0;
  int ix1 = content.indexOf(" ");

  String sender  = content.substring(ix0, ix1); ix0=ix1+1; ix1 = content.indexOf(" ", ix0);
  String raw     = content.substring(ix0, ix1); ix0=ix1+1; ix1 = content.indexOf(" ", ix0);
  String filt    = content.substring(ix0, ix1); ix0=ix1+1; ix1 = content.indexOf(" ", ix0);
  String batt    = content.substring(ix0, ix1); ix0=ix1+1; ix1 = content.indexOf(" ", ix0);
  String rssi    = content.substring(ix0, ix1); ix0=ix1+1; ix1 = content.indexOf(" ", ix0);
  String packet  = content.substring(ix0, ix1); ix0=ix1+1; ix1 = content.indexOf(" ", ix0);
  String wbatt   = content.substring(ix0, ix1); ix0=ix1+1; ix1 = content.indexOf(" ", ix0);

  int ix2=now.indexOf(" ");
  String date = now.substring(1, ix2);
  String datestring = now.substring(ix2+1, now.length());

  int sgv = filt.toInt()/1000; // !

  String body = "{\"unfiltered\":" + raw + "," +
               "\"filtered\":" + filt + "," +
               "\"device\":\"xDrip-ESP8266\"," +
               "\"rssi\":" + rssi + "," + 
               "\"sgv\":" + sgv + "," +
               "\"dateString\":\"" + datestring + "\"," +
               "\"type\":\"sgv\", " +
               "\"date\":" + date + "}";

  Serial.println(body);
  
  String url = "/api/v1/entries";

  String request = String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Content-Type: application/json\r\n" + 
               "Accept: application/json\r\n" + 
               "api-secret: " + apisecret + "\r\n" +
               "Content-Length: " + body.length() + "\r\n" + 
               "Connection: close\r\n\r\n" +
               body;

  
  Serial.println("sending post");

  Serial.println(request);
 
  // This will send the request to the server
  client.print(request);
  delay(1000);
  Serial.println("reading from client");
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.println(line.c_str());
  }
  
  Serial.println();
  Serial.println("closing connection");

// flash led a few times to show we finished upload

 delay(1000);
 digitalWrite(ledpin, LOW); delay(1000); digitalWrite(ledpin, HIGH); delay(1000);
 digitalWrite(ledpin, LOW); delay(1000); digitalWrite(ledpin, HIGH); delay(1000);
 digitalWrite(ledpin, LOW); delay(1000); digitalWrite(ledpin, HIGH); delay(1000);
 
}

String getnow(){

 Serial.println("getnow");
 
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect("orcasoft.no", 80)) {
    Serial.println("connection failed");
    return "0 0";
  }

  Serial.println("connected ok");
  
  String url = "/now.php";

  String request = String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: orcasoft.no\r\n" + 
               "Connection: close\r\n\r\n";
  
  Serial.println("sending get");

  Serial.println(request);
 
  // This will send the request to the server
  client.print(request);
  delay(1000);
  Serial.println("reading from client");
  // Read all the lines of the reply from server and print them to Serial
  String line;
  while(client.available()){
    line = client.readStringUntil('\r');
    Serial.println(line);
  }

  Serial.print("got current now '");
  Serial.print(line);

  Serial.println("'");
  Serial.println("closing connection");

  return(line);
}

void setup() {  

  Serial.begin(9600);
  wixel.begin(9600);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(AP_SSID, AP_PWD);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("waiting for wifi");
    delay(1000);
  }
  
  pinMode(ledpin, OUTPUT);

  // flash led to show we are connected to wifi

  digitalWrite(ledpin, LOW); delay(1000); digitalWrite(ledpin, HIGH); delay(1000);
  digitalWrite(ledpin, LOW); delay(1000); digitalWrite(ledpin, HIGH); delay(1000);

}

void loop() {
 
  String content = wixelread();

//content="6600358 281536 314624 216 -91 132 0";
  
  if(content.length()>0 && content.indexOf("AT NAMExDrip") == -1)
    upload(getnow(), content);
    
  delay(60000); // check every minute
  
}
