/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <PulseSensorPlayground.h> 

//  Variables
const int PulseWire = A0;       // PulseSensor PURPLE WIRE connected to ANALOG PIN 0
//const int LED = LED_BUILTIN;          // The on-board Arduino LED, close to PIN 13.
int Threshold = 337;           // Determine which Signal to "count as a beat" and which to ignore.
                               // Use the "Gettting Started Project" to fine-tune Threshold Value beyond default setting.
                               // Otherwise leave the default "550" value. 
                               
PulseSensorPlayground pulseSensor;  // Creates an instance of the PulseSensorPlayground object called "pulseSensor"

// Update these with values suitable for your network.

const char* ssid = "KOO";
const char* password = "11111111";
const char* mqtt_server = "121.137.153.145";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
int msgIndex = 0; 

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  randomSeed(micros());

}

void callback(char* topic, byte* payload, unsigned int length) {

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {

    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      client.subscribe("setting");
    } 
  }
}

void setup() {

  pinMode(33, OUTPUT);
  digitalWrite(33, HIGH);
  Serial.begin(9600);

  pulseSensor.analogInput(PulseWire);   
  pulseSensor.setThreshold(Threshold);    

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

void loop() {
  char receivedData;
  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  digitalWrite(33, LOW);
  while (Serial.available() > 0) {
    receivedData = Serial.read();   // read one byte from serial buffer and save to receivedData
    //Serial.print(receivedData);
    snprintf(msg, MSG_BUFFER_SIZE, "%c", receivedData);
    //Serial.println(receivedData);
    Serial.write(receivedData);
    client.publish("Control_num", msg);
  }

  //  while (Serial.available()) {
  //   char receivedChar = Serial.read(); // 한 문자 읽기
  //   if (receivedChar == '\n') { // 메시지의 끝을 확인
  //     msg[msgIndex] = '\0'; // 문자열의 끝을 나타내는 NULL 문자 추가
  //     client.publish("Control_num", msg); // 완성된 메시지 전송
  //     msgIndex = 0; // 버퍼 인덱스 초기화
  //   } else {
  //     msg[msgIndex] = receivedChar; // 버퍼에 문자 추가
  //     msgIndex++;
  //   }
  // }

  if (pulseSensor.sawStartOfBeat()) {            // Constantly test to see if "a beat happened".
  int myBPM = pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an "int".
                                                // "myBPM" hold this BPM value now. 
  Serial.println("♥  A HeartBeat Happened ! "); // If test is "true", print a message "a heartbeat happened".

  snprintf(msg, MSG_BUFFER_SIZE, "%d", myBPM);
  Serial.print("Publish BPM message: ");
  Serial.println(msg);
  client.publish("HB", msg);
  }
}
