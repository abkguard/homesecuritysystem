#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Arthur";
const char* password = "0978457739";
const char* mqttServer = "140.118.25.64";  // MQTT伺服器位址
const char* mqttUserName = "";  // 使用者名稱，隨意設定。
const char* mqttPwd = "";  // MQTT密碼
const char* clientID = "Arthur_880416";      // 用戶端ID，隨意設定。
const char* topic = "Myhome/guard";
const char* mqtt_subscribe_topic = "Myhome/status/set";
const int   mqtt_qos = 1;

unsigned long prevMillis = 0;  // 暫存經過時間（毫秒）
const long interval = 1000;  // 上傳資料的間隔時間，20秒。
String msgStr = "";      // 暫存MQTT訊息字串

int door_sensor_Reed = D7;
bool alarm = true;
bool doorState = false; //true: open

//vibration sensor variable
#define ANALOG_IN_PIN A0
#define VIBRATION_SENSOR_PIN D5
int motionDetected = LOW;
int sensorVal = 0;
int maxVal = 0;
int waitTime = 0;
//int vibTimes = 0;
int alarmState = 1;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect(clientID, mqttUserName, mqttPwd)) {
      Serial.println("MQTT connected");
      client.subscribe(mqtt_subscribe_topic, mqtt_qos);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(1000);  // 等5秒之後再重試
    }
  }
}

void callback(char* topicS, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topicS);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
//    Serial.println(int(payload[i]));
//    Serial.println((int)payload[i]);
    Serial.println(int(payload[i]-48));
  }
  alarmState = int(payload[0]-48);
}

void pubStr() {
  if (alarmState == 1) {
    // 宣告字元陣列
    byte arrSize = msgStr.length() + 1;
    char msg[arrSize];

    Serial.print("Publish message: ");
    Serial.println(msgStr);
    msgStr.toCharArray(msg, arrSize); // 把String字串轉換成字元陣列格式
    client.publish(topic, msg);       // 發布MQTT主題與訊息
    msgStr = "";
  }
}

void setup() {
  pinMode(door_sensor_Reed, INPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);
}

void loop() {
  //vib sensor
  motionDetected = digitalRead(VIBRATION_SENSOR_PIN);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  //  sensorVal = analogRead(ANALOG_IN_PIN);
  //  Serial.println("0");
  //  if (!client.connected()) {
  //    reconnect();
  //  }
  //  client.loop();

  if (motionDetected == LOW && analogRead(ANALOG_IN_PIN) < 1014 ) //&& waitTime == 0)
  {
    maxVal = 1024 - analogRead(ANALOG_IN_PIN);
    for (int i = 0; i < 2000; ++i) {
      if (maxVal < 1024 - analogRead(ANALOG_IN_PIN)) {
        maxVal = 1024 - analogRead(ANALOG_IN_PIN);
      }
      delay(0.1);
    }
    if (maxVal < 90) {
      Serial.println("----------detect vibration----------");
    }
    else {
      Serial.println("----------warn!!!!!----------");

      msgStr = "window vibration alarm";
      //      byte msgSize = msgStr.length() + 1;
      pubStr();
    }
    //    Serial.print(vibTimes);
    Serial.print(". Anaologue: ");
    Serial.println(maxVal);
  }

  if (digitalRead(door_sensor_Reed) == HIGH && doorState) {
    doorState = false;
    Serial.println("Door Closed");
  }
  else if (digitalRead(door_sensor_Reed) == LOW && doorState == false) {
    doorState = true;
    msgStr = "Door Open";
    Serial.println("Door Open!!!");
    //    byte msgSize = msgStr.length() + 1;
    pubStr();
    //    byte arrSize = msgStr.length() + 1;
    //    char msg[arrSize];
    //
    //    Serial.print("Publish message: ");
    //    Serial.println(msgStr);
    //    msgStr.toCharArray(msg, arrSize); // 把String字串轉換成字元陣列格式
    //    client.publish(topic, msg);       // 發布MQTT主題與訊息
    //    msgStr = "";
  }
  //  delay(0.1);
}
