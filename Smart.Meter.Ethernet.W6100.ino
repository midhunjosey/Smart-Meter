#include <SPI.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

#include <Ethernet.h>
#include <EthernetClient.h>
#include <Dns.h>
#include <Dhcp.h>



/************************* Ethernet Client Setup *****************************/
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "...your AIO username (see https://accounts.adafruit.com)..."
#define AIO_KEY         "...your AIO key..."


/************ Global State (you don't need to change this!) ******************/

//Set up the ethernet client
EthernetClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

#define halt(s) { Serial.println(F( s )); while(1);  }

/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish power = Adafruit_MQTT_Publish(&mqtt,  AIO_USERNAME "/feeds/Power");
Adafruit_MQTT_Publish bill = Adafruit_MQTT_Publish(&mqtt,  AIO_USERNAME "/feeds/bill");


//Setting up the Current Sensor
#include "ACS712.h"
ACS712 sensor(ACS712_30A, A0);
unsigned long last_time =0;
unsigned long current_time =0;
float Wh =0 ; 
float bill_amount = 0;   
unsigned int energyTariff = 8.0; 



/*************************** Sketch Code ************************************/

void setup() {
  Serial.begin(115200);

  Serial.println(F("Smart Energy Meter"));

  // Initialise the Client
  Serial.print(F("\nConfiguring Ethernet using DHCP..."));
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
  } else {
    Serial.print("  DHCP assigned IP Address: ");
    Serial.println(Ethernet.localIP());
  }

  delay(1000); //give the ethernet a second to initialize
  

}


uint32_t x=0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.

 MQTT_connect();

 //Calculating the Current
  float V = 230;
  float I = sensor.getCurrentAC();
  Serial.print("Current Consumed     : ");
  Serial.println(I);
  float P = V * I;
  last_time = current_time;
  current_time = millis();    
  Wh = Wh+  P *(( current_time -last_time) /3600000.0) ; 
  Serial.println();
  Serial.print("Total Power Consumed  : ");
  Serial.println(Wh);
  Serial.println();
  //Calculating the Bill
  bill_amount = Wh * energyTariff;      // 1unit = 1kwH

     
   Serial.print(F("\nSending Power value "));
  Serial.print(Wh);
  Serial.print("...");
  Serial.println();

  if (! power.publish(Wh)) {
    Serial.println(F("Sending to Cloud Failed..."));
  } else {
    Serial.println(F("Sent to Cloud!..."));
    Serial.println();
  }
  Serial.print(F("\nBill Amount is : "));
     Serial.print(bill_amount);
      Serial.print("...");
  Serial.println();
   Serial.println(F("\nSending Bill Amount "));
   if (! bill.publish(bill_amount)) {
    Serial.println(F("Sending to Cloud Failed..."));
  } else {
    Serial.println(F("Sent to Cloud!..."));
    Serial.println();
  }

delay(5000);
  
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret; 

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!....");
}
