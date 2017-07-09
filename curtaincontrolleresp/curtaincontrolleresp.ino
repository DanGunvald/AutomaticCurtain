#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

const char* ssid = "<ssid>";
const char* password = "<password>";
char chipid[12];
#define HOST "CURTAIN-%s"
#define TOPIC "curtain/%s/action"
#define TOPICSTATUS "curtain/%s/status/"
//#define DEBUG_TELNET
const char* mqttserver="<mqttserver>";
char mqtttopic[48] = "";
char mqtttopicstatus[48] = "";
static char serbuffer[64];
static int serbufferpos = 0;

WiFiServer server(80);
WiFiClient client;
PubSubClient mqttclient(client);

#ifdef DEBUG_TELNET
  WiFiServer  telnetServer(23);
  WiFiClient  telnetClient;
#endif

// Macros for debugging
#ifdef DEBUG_TELNET
  #define     DEBUG_PRINT(x)    telnetClient.print(x)
  #define     DEBUG_PRINTLN(x)  telnetClient.println(x)
  #define     DEBUG_PRINTF(x,y)   telnetClient.printf(x,y)
#else
  #define     DEBUG_PRINT(x)    Serial.print(x)
  #define     DEBUG_PRINTLN(x)  Serial.println(x)
  #define     DEBUG_PRINTF(x,y)   Serial.printf(x,y)
#endif


#ifdef DEBUG_TELNET
void handleTelnet(void) {
  if (telnetServer.hasClient()) {
    if (!telnetClient || !telnetClient.connected()) {
      if (telnetClient) {
        telnetClient.stop();
      }
      telnetClient = telnetServer.available();
    } else {
      telnetServer.available().stop();
    }
  }
}
#endif


void setup() {
  char host[64];
  Serial.begin(115200);
#ifdef DEBUG_TELNET
  // start the Telnet server
  telnetServer.begin();
  telnetServer.setNoDelay(true);
#endif
  pinMode(2, OUTPUT);
  
  sprintf(chipid, "%08X", ESP.getChipId());
  sprintf(host, HOST, chipid);
  sprintf(mqtttopic, TOPIC, chipid);
  sprintf(mqtttopicstatus, TOPICSTATUS, chipid);
  WiFi.hostname(host);
  ArduinoOTA.setHostname(host);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  for (int i = 0; i < 8;i++) {
      digitalWrite(2, HIGH);
      delay(250);
      digitalWrite(2, LOW);
      delay(250);
    }
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    for (int i = 0; i < 8;i++) {
      digitalWrite(2, HIGH);
      delay(100);
      digitalWrite(2, LOW);
      delay(100);
    }
    delay(1000);
    ESP.restart();
  }
  digitalWrite(2, LOW);

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\n", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  mqttclient.setServer(mqttserver, 1883);
  mqttclient.setCallback(mqttcallback);
  digitalWrite(2, HIGH);
}


void mqttcallback(char* topic, byte* payload, unsigned int length) {
  digitalWrite(2, LOW);
  if (strcmp(topic, mqtttopic) == 0) {
      char *s = (char *)malloc(length + 1);
      strncpy(s, (char *)payload, length);
      Serial.println(s);
  }
  digitalWrite(2, HIGH);
}

void mqttreconnect() {
  int numretries = 0;
  while (!mqttclient.connected() && numretries < 3) {
    if (mqttclient.connect(chipid)) {
      mqttclient.subscribe(mqtttopic);
    } else {
      delay(1000);
    }
    numretries ++;
  }
}


char * getSerialData() {
char * result = NULL;
while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 10 || c== 13) {
      serbuffer[serbufferpos] = 0;
      result = serbuffer;
      serbufferpos = 0;
    } else {
      serbuffer[serbufferpos++] = c;
    }
    if (serbufferpos > 63) {
      serbufferpos = 0;
    }
  }
  return result;
}

long t = 0;
void loop() {
  #ifdef DEBUG_TELNET
  // handle Telnet connection for debugging
  handleTelnet();
#endif
 

  ArduinoOTA.handle();
  char *ser = getSerialData();
  if (ser != NULL) {
    char *s = strstr(ser,":");
    if (s) {
      *s = 0;
      s++;
      String mytopic=mqtttopicstatus;
      mytopic += ser;
      mqttclient.publish(mytopic.c_str(),s);
      
    }
    
    
  }
  if (!mqttclient.connected()) {
    mqttreconnect();
  }

  if (mqttclient.connected()) {
    mqttclient.loop();
  }
  
 
}
