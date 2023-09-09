#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#else
#error "Board not found"
#endif

#include <WebSocketsServer.h>
#include <ArduinoJson.h>

#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

#define LED1 D0
#define LED2 D1
#define LED3 D2
#define LED4 D3
#define buzz D4 
const int trigPin = 14;
const int echoPin = 12;

float distanceCm;
long duration;

char webpage[] PROGMEM = R"=====(

<!DOCTYPE html>
<html>

<script>

var connection = new WebSocket('ws://'+location.hostname+':81/');

var button_1_status = 0;
var button_2_status = 0;
var button_3_status = 0;
var button_4_status = 0;
var button_5_status = 0;

function button_1_on()
{
   button_1_status = 1; 
  console.log("LED 1 is ON");
  send_data();
}

function button_1_off()
{
  button_1_status = 0;
console.log("LED 1 is OFF");
send_data();
}

function button_2_on()
{
   button_2_status = 1; 
  console.log("LED 2 is ON");
  send_data();
}

function button_2_off()
{
  button_2_status = 0;
console.log("LED 2 is OFF");
send_data();
}
function button_3_on()
{
   button_3_status = 1; 
  console.log("LED 3 is ON");
  send_data();
}

function button_3_off()
{
  button_3_status = 0;
console.log("LED 3 is OFF");
send_data();
}
function button_4_on()
{
   button_4_status = 1; 
  console.log("LED 4 is ON");
  send_data();
}

function button_4_off()
{
  button_4_status = 0;
console.log("LED 4 is OFF");
send_data();
}
function button_5_off()
{
  button_5_status = 0;
console.log("buzz is OFF");
send_data();
}
function send_data()
{
  var full_data = '{"LED1" :'+button_1_status+',"LED2":'+button_2_status+',"LED3":'+button_3_status+',"LED4":'+button_4_status+',"buzz":'+button_5_status+'}';
  connection.send(full_data);
}


</script>
<body>

<center>
<h1>My Home Automation</h1>

<h3> LED 1 </h3>
<button onclick= "button_1_on()" >On</button><button onclick="button_1_off()" >Off</button>
<h3> LED 2 </h3>
<button onclick="button_2_on()">On</button><button onclick="button_2_off()">Off</button>
<h3> LED 3 </h3>
<button onclick= "button_3_on()" >On</button><button onclick="button_3_off()" >Off</button>
<h3> LED 4 </h3>
<button onclick="button_4_on()">On</button><button onclick="button_4_off()">Off</button>

</center>
</body>
</html>

)=====";


#include <ESPAsyncWebServer.h>

AsyncWebServer server(80); // server port 80
WebSocketsServer websockets(81);

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Page Not found");
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) 
  {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = websockets.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        websockets.sendTXT(num, "Connected from server");
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      String message = String((char*)( payload));
      Serial.println(message);
     DynamicJsonDocument doc(200);
    // deserialize the data
    DeserializationError error = deserializeJson(doc, message);
    // parse the parameters we expect to receive (TO-DO: error handling)
      // Test if parsing succeeds.
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  int LED1_status = doc["LED1"];
  int LED2_status = doc["LED2"];
  int LED3_status = doc["LED3"];
  int LED4_status = doc["LED4"];
  int buzz_status = doc["buzz"];
  digitalWrite(LED1,LED1_status);
  digitalWrite(LED2,LED2_status);
  digitalWrite(LED3,LED3_status);
  digitalWrite(LED4,LED4_status);  
  digitalWrite(buzz,buzz_status);
  }
}

void setup(void)
{
  
  Serial.begin(115200);
  pinMode(trigPin,OUTPUT);
  pinMode(echoPin,INPUT);
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);
  pinMode(LED4,OUTPUT);
  pinMode(buzz,OUTPUT);
  
  
  WiFi.softAP("halum automation", "");
  Serial.println("halum");
  Serial.println("");
  Serial.println(WiFi.softAPIP());


  if (MDNS.begin("ESP")) { //esp.local/
    Serial.println("MDNS responder started");
  }
  server.on("/", [](AsyncWebServerRequest * request)
  { 
  request->send_P(200, "text/html", webpage);
  });
  server.on("/led1/on", HTTP_GET, [](AsyncWebServerRequest * request)
  { 
    digitalWrite(LED1,HIGH);
  request->send_P(200, "text/html", webpage);
  });

  server.onNotFound(notFound);
  server.begin();  // it will start webserver
  websockets.begin();
  websockets.onEvent(webSocketEvent);

}
void loop()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_VELOCITY/2;
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);

  if(distanceCm<30)
 {
   digitalWrite(buzz,HIGH);
 }
 else
 {
   digitalWrite(buzz,LOW);
 }
 websockets.loop();

}