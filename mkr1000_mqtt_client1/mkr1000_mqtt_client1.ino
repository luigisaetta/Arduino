
/*
  MQTT client for Arduino MKR1000
  based on WiFi101 and PubSubClient library

  Reads Temperature and Humidity from DHT22
  Read from a Light sensor (PhotoTransistor)
  and send the values as a MQTT message, in JSON format

  Author: L. Saetta, 2016
  Version: 3.0
*/

#include <WiFi101.h>
#include <PubSubClient.h>
#include <DHT.h>

// enable (uncomment) or disable debug print to Serial
//#define FLAG_DEBUG

// specify that the sensor is DHT22 (and not DHT11)
#define DHTTYPE DHT22

// The input pin used for DHT22 signal on MKR1000
#define DHTPIN 6
#define LIGHTPIN A2

// Definition for the WIFI network
char ssid[] = "FASTWEB-1-2690D3";      //  your network SSID (name)
char pass[] = "B318AF7971";   // your network password
// end WIFI credentials

// user and pwd for MQTT +
// clientId and Topic where to publish
char user[10] = "sensori";
char pwd[10] = "welcome3";

char clientId[] = "sn1";
char roomId[] = "soggiorno";
char outTopic[100];  // "soggiorno/deviceId/msg";

// to identify the msg
long progr = 1;

int status = WL_IDLE_STATUS;

// Initialize the Wifi client Library
WiFiClient client;

// Initialize MQTT Client Library
// still troubles char* mqttBroker = "iotgateway3.local";
PubSubClient mqClient(client);

// your MQTT broker address:
IPAddress server(192, 168, 1, 77);

// initialize DHT sensor -temp, hum-
DHT dht(DHTPIN, DHTTYPE);

/*
 * Helper function
 * Build the JSON msg that is sent to  MQTT broker
 * 
 * example: {"Source":"sn1","Progr":53,"Tstamp":287804,"Data":{"Temp":"21.30","Hum":"53.20","Light":"0.0"}}
 */
char theMsg[120];

char * buildJSONMsg(char * resultMsg, const char * sensorId, long lTstamp,
    float vTemp, float vHum, float vLight)
{
  // formats the JSON msg
  sprintf(resultMsg,
      "{\"Source\":\"%s\",\"Progr\":%ld,\"Tstamp\":%ld,\"Data\":{\"Temp\":%.2f,\"Hum\":%.2f,\"Light\":%.1f}}",
      sensorId, progr, lTstamp, vTemp, vHum, vLight);

  progr++;

  return resultMsg;
}

/*
 * Define the Topic for Output Msgs 
 * 
 */
void defineOutTopic()
{
  strcpy(outTopic, roomId);
  strcat(outTopic, "/");
  strcat(outTopic, clientId);
  strcat(outTopic, "/msg");

  return;
}

/*
 * Connect to WIFI 
 */
void connectWIFI()
{
   // attempt to connect to Wifi network:
   while (status != WL_CONNECTED)
   {
     #ifdef FLAG_DEBUG
     Serial.print("Attempting to connect to SSID: ");
     Serial.println(ssid);
     #endif
    
     // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
     status = WiFi.begin(ssid, pass);

     // wait 1 second before retry connection:
     delay(1000);
   }
}

/**
 * Setup
 */
void setup() 
{
  //Initialize serial
  #ifdef FLAG_DEBUG
  Serial.begin(9600);
  #endif
  
  // attempt to connect to Wifi network:
  connectWIFI();
  
  // you're connected now, so print out the status:
  #ifdef FLAG_DEBUG
  printWifiStatus();
  #endif
  
  // define MQTT broker
  mqClient.setServer(server, 1883);

  // define the name for the outTopic
  defineOutTopic();
  
  //initialize DHT22 sensor
  dht.begin();
}

// With two delays in order to avoid connection timeout
long DELAY_LONG = 30000;
long DELAY_SHORT = 5000;

long lastTimeSent = millis();

/**
 * Loop 
 */
void loop()
{
  // if not connected reconnect !
  if (!mqClient.connected()) 
  {
    // test if connected to WIFI
    status = WiFi.status();

    if (status != WL_CONNECTED)
      connectWIFI();
       
    reconnect();
  }

  // Publish a message every DELAY_LONG msec
  if (mqClient.connected() && ((millis() - lastTimeSent) > DELAY_LONG))
  {
    // read values from DHT22
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    float l = analogRead(LIGHTPIN);
  
    // build JSON msg
    char* msgToSend = buildJSONMsg(theMsg, clientId, millis(), t, h, l);

    #ifdef FLAG_DEBUG
    Serial.println(msgToSend);
    #endif
    
    // last parms is true, this way the message is retained !
    mqClient.publish(outTopic, msgToSend, true);
    
    lastTimeSent = millis();
  }
  
  mqClient.loop();

  delay(DELAY_SHORT);
}

void printWifiStatus() 
{
  // print the SSID of the network you're attached to:

  #ifdef FLAG_DEBUG
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  #endif
}

#define DELAY_RECONN 5000
 
/*
 * Handles reconnection to MQTT broker
 */
void reconnect()
{
  // Loop until we're reconnected
  while (!mqClient.connected())
  {
    #ifdef FLAG_DEBUG
    Serial.print("Attempting MQTT connection...");
    #endif
    // Attempt to connect

    if (mqClient.connect(clientId, user, pwd)) 
    {
      #ifdef FLAG_DEBUG
      Serial.println("OK.");
      Serial.println("");
      #endif
    } else
    {
      #ifdef FLAG_DEBUG
      Serial.print("failed, Return Code = ");
      Serial.println(mqClient.state());
      
      Serial.println("Retry in 5 seconds");
      #endif
      
      // Wait 5 seconds before retrying
      delay(DELAY_RECONN);
    }
  }
}

