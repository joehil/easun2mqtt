// modbusrs485 Solar Inverter to MQTT
// Repo: https://github.com/nygma2004/modbusrs4852mqtt
// author: Csongor Varga, csongor.varga@gmail.com
// 1 Phase, 2 string inverter version such as MIN 3000 TL-XE, MIC 1500 TL-X

// Libraries:
// - FastLED by Daniel Garcia
// - ModbusMaster by Doc Walker
// - ArduinoOTA
// - SoftwareSerial
// Hardware:
// - Wemos D1 mini
// - RS485 to TTL converter: https://www.aliexpress.com/item/1005001621798947.html
// - To power from mains: Hi-Link 5V power supply (https://www.aliexpress.com/item/1005001484531375.html), fuseholder and 1A fuse, and varistor


#include <SoftwareSerial.h>       // Leave the main serial line (USB) for debugging and flashing
#include <ModbusMaster.h>         // Modbus master library for ESP8266
#include <ESP8266WiFi.h>          // Wifi connection
#include <ESP8266WebServer.h>     // Web server for general HTTP response
#include <PubSubClient.h>         // MQTT support
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include "globals.h"
#include "settings.h"



os_timer_t myTimer;
ESP8266WebServer server(80);
WiFiClient espClient;
PubSubClient mqtt(mqtt_server, 1883, 0, espClient);
// SoftwareSerial modbus(MAX485_RX, MAX485_TX, false, 256); //RX, TX
SoftwareSerial modbus(MAX485_RX, MAX485_TX, false); //RX, TX
ModbusMaster modbusrs485;
CRGB leds[NUM_LEDS];

void callback(char* topic, byte* payload, unsigned int length);

void preTransmission() {
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission() {
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

void sendModbusError(uint8_t result) {
  String message = "";
  if (result==modbusrs485.ku8MBIllegalFunction) {
    message = "Illegal function";
  }
  if (result==modbusrs485.ku8MBIllegalDataAddress) {
    message = "Illegal data address";
  }
  if (result==modbusrs485.ku8MBIllegalDataValue) {
    message = "Illegal data value";
  }
  if (result==modbusrs485.ku8MBSlaveDeviceFailure) {
    message = "Slave device failure";
  }
  if (result==modbusrs485.ku8MBInvalidSlaveID) {
    message = "Invalid slave ID";
  }
  if (result==modbusrs485.ku8MBInvalidFunction) {
    message = "Invalid function";
  }
  if (result==modbusrs485.ku8MBResponseTimedOut) {
    message = "Response timed out";
  }
  if (result==modbusrs485.ku8MBInvalidCRC) {
    message = "Invalid CRC";
  }
  if (message=="") {
    message = result;
  }
  Serial.println(message);    
  char topic[80];
  char value[30];
  sprintf(topic,"%s/error",topicRoot);
  mqtt.publish(topic, message.c_str());
  delay(5);
}

void ReadInputRegisters() {
  char json[1024];
  char topic[80];
  char value[10]; 

  leds[0] = CRGB::Yellow;
  FastLED.show();
  uint8_t result;

  digitalWrite(STATUS_LED, 0);

  ESP.wdtDisable();
  result = modbusrs485.readHoldingRegisters(15,64);
  ESP.wdtEnable(1);
  if (result == modbusrs485.ku8MBSuccess)   {

    leds[0] = CRGB::Green;
    FastLED.show();
    lastRGB = millis();
    ledoff = true;
    
    sprintf(json,"{");
    for(int i=0;i<64;i++) {
      #ifdef DEBUG_MQTT
        sprintf(json,"%s \"r%02d\":%d,",json,i,modbusrs485.getResponseBuffer(i));
      #endif
    }
    sprintf(json,"%s \"current\":%.2f,",json,(double)((int16_t)modbusrs485.getResponseBuffer(1)*0.01));
    sprintf(json,"%s \"SOC\":%.2f,",json,(double)(modbusrs485.getResponseBuffer(11)*0.01));
    sprintf(json,"%s \"voltage\":%.2f,",json,(double)(modbusrs485.getResponseBuffer(7)*0.01));
    sprintf(json,"%s \"end\":1 }",json);

    #ifdef DEBUG_MQTT
      sprintf(topic,"%s/raw",topicRoot);
      mqtt.publish(topic, json);
    #endif

    if (setcounter==0) {
      /*
      // Status and PV data
      modbusdata.status = modbusrs485.getResponseBuffer(0);
      modbusdata.solarpower = ((modbusrs485.getResponseBuffer(1) << 16) | modbusrs485.getResponseBuffer(2))*0.1;
      
      modbusdata.pv1voltage = modbusrs485.getResponseBuffer(3)*0.1;
      modbusdata.pv1current = modbusrs485.getResponseBuffer(4)*0.1;
      modbusdata.pv1power = ((modbusrs485.getResponseBuffer(5) << 16) | modbusrs485.getResponseBuffer(6))*0.1;
  
      modbusdata.pv2voltage = modbusrs485.getResponseBuffer(7)*0.1;
      modbusdata.pv2current = modbusrs485.getResponseBuffer(8)*0.1;
      modbusdata.pv2power = ((modbusrs485.getResponseBuffer(9) << 16) | modbusrs485.getResponseBuffer(10))*0.1;
  
      // Output
      modbusdata.outputpower = ((modbusrs485.getResponseBuffer(35) << 16) | modbusrs485.getResponseBuffer(36))*0.1;
      modbusdata.gridfrequency = modbusrs485.getResponseBuffer(37)*0.01;
      modbusdata.gridvoltage = modbusrs485.getResponseBuffer(38)*0.1;
  
      // Energy
      modbusdata.energytoday = ((modbusrs485.getResponseBuffer(53) << 16) | modbusrs485.getResponseBuffer(54))*0.1;
      modbusdata.energytotal = ((modbusrs485.getResponseBuffer(55) << 16) | modbusrs485.getResponseBuffer(56))*0.1;
      modbusdata.totalworktime = ((modbusrs485.getResponseBuffer(57) << 16) | modbusrs485.getResponseBuffer(58))*0.5;
  
      modbusdata.pv1energytoday = ((modbusrs485.getResponseBuffer(59) << 16) | modbusrs485.getResponseBuffer(60))*0.1;
      modbusdata.pv1energytotal = ((modbusrs485.getResponseBuffer(61) << 16) | modbusrs485.getResponseBuffer(62))*0.1;
      overflow = modbusrs485.getResponseBuffer(63); */
    }
    if (setcounter==1) {
      /*
      modbusdata.pv2energytoday = ((overflow << 16) | modbusrs485.getResponseBuffer(64-64))*0.1;
      modbusdata.pv2energytotal = ((modbusrs485.getResponseBuffer(65-64) << 16) | modbusrs485.getResponseBuffer(66-64))*0.1;
  
      // Temperatures
      modbusdata.tempinverter = modbusrs485.getResponseBuffer(93-64)*0.1;
      modbusdata.tempipm = modbusrs485.getResponseBuffer(94-64)*0.1;
      modbusdata.tempboost = modbusrs485.getResponseBuffer(95-64)*0.1;
  
      // Diag data
      modbusdata.ipf = modbusrs485.getResponseBuffer(100-64);
      modbusdata.realoppercent = modbusrs485.getResponseBuffer(101-64);
      modbusdata.opfullpower = ((modbusrs485.getResponseBuffer(102-64) << 16) | modbusrs485.getResponseBuffer(103-64))*0.1;
      modbusdata.deratingmode = modbusrs485.getResponseBuffer(104-64); */
      }

      setcounter++;
      if (setcounter==2) {
        setcounter = 0;      
  /* 
        // Generate the modbus MQTT message
        sprintf(json,"{",json);
        sprintf(json,"%s \"status\":%d,",json,modbusdata.status);
        sprintf(json,"%s \"solarpower\":%.1f,",json,modbusdata.solarpower);
        sprintf(json,"%s \"pv1voltage\":%.1f,",json,modbusdata.pv1voltage);
        sprintf(json,"%s \"pv1current\":%.1f,",json,modbusdata.pv1current);
        sprintf(json,"%s \"pv1power\":%.1f,",json,modbusdata.pv1power);
        sprintf(json,"%s \"pv2voltage\":%.1f,",json,modbusdata.pv2voltage);
        sprintf(json,"%s \"pv2current\":%.1f,",json,modbusdata.pv2current);
        sprintf(json,"%s \"pv2power\":%.1f,",json,modbusdata.pv2power);
        
        sprintf(json,"%s \"outputpower\":%.1f,",json,modbusdata.outputpower);
        sprintf(json,"%s \"gridfrequency\":%.2f,",json,modbusdata.gridfrequency);
        sprintf(json,"%s \"gridvoltage\":%.1f,",json,modbusdata.gridvoltage);
    
        sprintf(json,"%s \"energytoday\":%.1f,",json,modbusdata.energytoday);
        sprintf(json,"%s \"energytotal\":%.1f,",json,modbusdata.energytotal);
        sprintf(json,"%s \"totalworktime\":%.1f,",json,modbusdata.totalworktime);
        sprintf(json,"%s \"pv1energytoday\":%.1f,",json,modbusdata.pv1energytoday);
        sprintf(json,"%s \"pv1energytotal\":%.1f,",json,modbusdata.pv1energytotal);
        sprintf(json,"%s \"pv2energytoday\":%.1f,",json,modbusdata.pv2energytoday);
        sprintf(json,"%s \"pv2energytotal\":%.1f,",json,modbusdata.pv2energytotal);
        sprintf(json,"%s \"opfullpower\":%.1f,",json,modbusdata.opfullpower);
    
        sprintf(json,"%s \"tempinverter\":%.1f,",json,modbusdata.tempinverter);
        sprintf(json,"%s \"tempipm\":%.1f,",json,modbusdata.tempipm);
        sprintf(json,"%s \"tempboost\":%.1f,",json,modbusdata.tempboost);
    
        sprintf(json,"%s \"ipf\":%d,",json,modbusdata.ipf);
        sprintf(json,"%s \"realoppercent\":%d,",json,modbusdata.realoppercent);
        sprintf(json,"%s \"deratingmode\":%d,",json,modbusdata.deratingmode);
        sprintf(json,"%s \"faultcode\":%d,",json,modbusdata.faultcode);
        sprintf(json,"%s \"faultbitcode\":%d,",json,modbusdata.faultbitcode);
        sprintf(json,"%s \"warningbitcode\":%d }",json,modbusdata.warningbitcode); */
  
        #ifdef DEBUG_SERIAL
        Serial.println(json);
        #endif
        sprintf(topic,"%s/data",topicRoot);
        mqtt.publish(topic,json);      
        Serial.println("Data MQTT sent");
    }
    //sprintf(topic,"%s/error",topicRoot);
    //mqtt.publish(topic,"OK");

  } else {
    leds[0] = CRGB::Red;
    FastLED.show();
    lastRGB = millis();
    ledoff = true;

    Serial.print(F("Error: "));
    sendModbusError(result);
  }
  digitalWrite(STATUS_LED, 1);
}



// This is the 1 second timer callback function
void timerCallback(void *pArg) {

  
  seconds++;

  // Query the modbus device 
  if (seconds % UPDATE_MODBUS==0) {
    ReadInputRegisters();
  }

  // Send RSSI and uptime status
  if (seconds % UPDATE_STATUS==0) {
    // Send MQTT update
    if (mqtt_server!="") {
      char topic[80];
      char value[300];
      sprintf(value,"{\"rssi\": %d, \"uptime\": %d, \"ssid\": \"%s\", \"ip\": \"%d.%d.%d.%d\", \"clientid\":\"%s\", \"version\":\"%s\"}",WiFi.RSSI(),uptime,WiFi.SSID().c_str(),WiFi.localIP()[0],WiFi.localIP()[1],WiFi.localIP()[2],WiFi.localIP()[3],newclientid,buildversion);
      sprintf(topic,"%s/%s",topicRoot,"status");
      mqtt.publish(topic, value);
      Serial.println(F("MQTT status sent"));
    }
  }


}

// MQTT reconnect logic
void reconnect() {
  //String mytopic;
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    byte mac[6];                     // the MAC address of your Wifi shield
    WiFi.macAddress(mac);
    sprintf(newclientid,"%s-%02x%02x%02x",clientID,mac[2],mac[1],mac[0]);
    Serial.print(F("Client ID: "));
    Serial.println(newclientid);
    // Attempt to connect
    if (mqtt.connect(newclientid, mqtt_user, mqtt_password)) {
      Serial.println(F("connected"));
      // ... and resubscribe
      char topic[80];
      sprintf(topic,"%swrite/#",topicRoot);
      mqtt.subscribe(topic);
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(mqtt.state());
      Serial.println(F(" try again in 5 seconds"));
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void setup() {

  FastLED.addLeds<LED_TYPE, RGBLED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalSMD5050 );
  FastLED.setBrightness( BRIGHTNESS );
  leds[0] = CRGB::Pink;
  FastLED.show();

  Serial.begin(SERIAL_RATE);
  Serial.println(F("\nmodbusrs485 Solar Inverter to MQTT Gateway"));
  // Init outputs, RS485 in receive mode
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

  // Initialize some variables
  uptime = 0;
  seconds = 0;
  leds[0] = CRGB::Pink;
  FastLED.show();

  // Connect to Wifi
  Serial.print(F("Connecting to Wifi"));
  WiFi.mode(WIFI_STA);

  #ifdef FIXEDIP
  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  #endif
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
    seconds++;
    if (seconds>180) {
      // reboot the ESP if cannot connect to wifi
      ESP.restart();
    }
  }
  seconds = 0;
  Serial.println("");
  Serial.println(F("Connected to wifi network"));
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());
  Serial.print(F("Signal [RSSI]: "));
  Serial.println(WiFi.RSSI());

  // Set up the Modbus line
  modbusrs485.begin(SLAVE_ID , modbus);
  // Callbacks allow us to configure the RS485 transceiver correctly
  modbusrs485.preTransmission(preTransmission);
  modbusrs485.postTransmission(postTransmission);
  Serial.println("Modbus connection is set up");

  // Create the 1 second timer interrupt
  os_timer_setfn(&myTimer, timerCallback, NULL);
  os_timer_arm(&myTimer, 1000, true);

  server.on("/", [](){                        // Dummy page
    server.send(200, "text/plain", "modbusrs485 Solar Inverter to MQTT Gateway");
  });
  server.begin();
  Serial.println(F("HTTP server started"));

  // Set up the MQTT server connection
  if (mqtt_server!="") {
    mqtt.setServer(mqtt_server, 1883);
    mqtt.setBufferSize(1024);
    mqtt.setCallback(callback);
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  byte mac[6];                     // the MAC address of your Wifi shield
  WiFi.macAddress(mac);
  char value[80];
  sprintf(value,"%s-%02x%02x%02x",clientID,mac[2],mac[1],mac[0]);
  ArduinoOTA.setHostname(value);

  // No authentication by default
  ArduinoOTA.setPassword((const char *)"esp6161");

  ArduinoOTA.onStart([]() {
    os_timer_disarm(&myTimer);
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
    os_timer_arm(&myTimer, 1000, true);
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
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

  modbus.begin(MODBUS_RATE);
  
  leds[0] = CRGB::Black;
  FastLED.show();
  
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Convert the incoming byte array to a string
  String mytopic = (char*)topic;
  payload[length] = '\0'; // Null terminator used to terminate the char array
  String message = (char*)payload;

  Serial.print(F("Message arrived on topic: ["));
  Serial.print(topic);
  Serial.print(F("], "));
  Serial.println(message);


}

void loop() {
  // Handle HTTP server requests
  server.handleClient();
  ArduinoOTA.handle();

  // Handle MQTT connection/reconnection
  if (mqtt_server!="") {
    if (!mqtt.connected()) {
      reconnect();
    }
    mqtt.loop();
  }

  // Uptime calculation
  if (millis() - lastTick >= 60000) {            
    lastTick = millis();            
    uptime++;            
  }    

  if (millis() - lastWifiCheck >= WIFICHECK) {
    // reconnect to the wifi network if connection is lost
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnecting to wifi...");
      WiFi.reconnect();
    }
    lastWifiCheck = millis();
  }

  if (ledoff && (millis() - lastRGB >= RGBSTATUSDELAY)) {
    ledoff = false;
    leds[0] = CRGB::Black;
    FastLED.show();
  }


}
