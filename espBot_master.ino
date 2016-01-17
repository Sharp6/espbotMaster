#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// I2C --------------------------------------------
#include <Wire.h>

/*
class MicroService
{
  public:
  char* name;
  char* topic;
  
  MicroService(char* newName, char* newTopic)
  {
    name = newName;
    topic = newTopic;
  }
};
*/


// Update these with values suitable for your network.
const char* ssid = "vitalnet";
const char* password = "vitalvital";
const char* mqtt_server = "192.168.1.124";

//MicroService runmonitor = MicroService("Run Monitor", "runmonitor");
//MicroService gethomedry = MicroService("Get Home Dry", "gethomedry");

// dirty manual hardcoded setting of array length. 
// Lijkt niet mogelijk om de lengte manueel op te vragen.
//int numberOfServices = 2;
//MicroService services[] = { runmonitor, gethomedry };

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// =====================================================================================
// =====================================================================================
void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // I2C --------------------------------------
  Wire.begin();


  // BUTTONS ----------------------------------
  pinMode(12, INPUT_PULLUP);
  //digitalWrite(12, HIGH);

  pinMode(13, INPUT_PULLUP);
  //digitalWrite(13, HIGH);
}

// =====================================================================================
// =====================================================================================
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  if(digitalRead(12) == HIGH || digitalRead(13) == HIGH) {
    String message = "CMDNEO|REFRESH";
    char buf[50];
    message.toCharArray(buf,50);
  
    Wire.beginTransmission(8);
    Wire.write(buf);
    Wire.endTransmission();
  }
  
  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
    // Kick ass!
    client.publish("req", "all");
    Serial.println("Requested all services");
  }
}

// =====================================================================================
// =====================================================================================
 
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
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String strPayload = String((char*)payload);
  String stripped = strPayload.substring(1,strPayload.length()-1);
  String message = String(topic)+'|'+stripped;
  char buf[50];
  message.toCharArray(buf,50);
  
  Serial.print("Sending message:");
  Serial.println(buf);

  Wire.beginTransmission(8);
  Wire.write(buf);
  Wire.endTransmission();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      subscribeToServices();
      client.subscribe("inTopic");
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void subscribeToServices() {
  /*int i;
  for(i = 0; i < numberOfServices; i++) {
    client.subscribe(services[i].topic);
  }
  */
  client.subscribe("runmonitor");
  client.subscribe("gethomedry");
  client.subscribe("CMDSCR");
  client.subscribe("CMDNEO");
}

void sendI2Cmessage(String message) {
  Wire.beginTransmission(8);
  char buf[50];
  message.toCharArray(buf,50);
  Wire.write(buf);
  Wire.endTransmission();
}

