//#define DEBUG_SERIAL    1
#define DEBUG_MQTT      1 
#define MODBUS_RATE     9600      // Modbus speed of Growatt, do not change
#define SERIAL_RATE     115200    // Serial speed for status info
#define MAX485_DE       15        // D8, DE pin on the TTL to RS485 converter
#define MAX485_RE_NEG   4         // D2, RE pin on the TTL to RS485 converter
#define MAX485_RX       14        // D5, RO pin on the TTL to RS485 converter
#define MAX485_TX       12        // D6, DI pin on the TTL to RS485 converter
#define RELAY           5         // D1, switch relay on and off
#define SLAVE_ID        1         // Default slave ID of Growatt
#define STATUS_LED      2         // Status LED on the Wemos D1 mini (D4)
#define UPDATE_MODBUS   60        // 1: modbus device is read every second
#define UPDATE_STATUS   95        // 10: status mqtt message is sent every 10 seconds
#define RGBSTATUSDELAY  500       // delay for turning off the status led
#define WIFICHECK       500       // how often check lost wifi connection

// Update the below parameters for your project
// Also check NTP.h for some parameters as well
const char* ssid = "<ssid>";           // Wifi SSID
const char* password = "<password>";    // Wifi password
const char* mqtt_server = "<mqttip>";     // MQTT server
const char* mqtt_user = "";             // MQTT userid
const char* mqtt_password = "";         // MQTT password
const char* clientID = "easun";                // MQTT client ID
const char* topicRoot = "easun";             // MQTT root topic for the device, keep / at the end


// Comment the entire second below for dynamic IP (including the define)
// #define FIXEDIP   1
IPAddress local_IP(192, 168, 0, 91);         // Set your Static IP address
IPAddress gateway(192, 168, 0, 1);          // Set your Gateway IP address
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 0, 1);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional
