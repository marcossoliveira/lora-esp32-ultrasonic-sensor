#include "EspMQTTClient.h"
#include "heltec.h"

#define TOMATE 1000 //(ms)
#define TOPIC_NAME "rotativo"
#define BAND    433E6  //you can set band here directly,e.g. 868E6,915E6

// Device type (0: Central | 1: Station)
byte deviceType = 0;
// Device ID
byte deviceId = 2;

byte globalIncomingDeviceType = 0;
byte globalIncomingDeviceId = 0;
byte globalIncomingDeviceStatus = 0;

EspMQTTClient client(
  "AUIM",
  "!!**Auim**!!2019",
  "18.231.61.129",   // MQTT Broker server ip
  "auim",             // Can be omitted if not needed
  "auimDev2019",      // Can be omitted if not needed
  "Rotativo",         // Client name that uniquely identify your device
  1883                // The MQTT port, default to 1883. this line can be omitted
);

String outgoing;              // outgoing message
byte msgCount = 0;            // count of outgoing messages
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends

void setup() {
  //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);

  Serial.begin(115200);


  // Optionnal functionnalities of EspMQTTClient :

  // Enable debugging messages sent to serial output
  client.enableDebuggingMessages();

  // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword.
  client.enableHTTPWebUpdater();

  //These can be overrited with enableHTTPWebUpdater("user", "password").
  // You can activate the retain flag by setting the third parameter to true
  client.enableLastWillMessage("TestClient/lastwill", "I am going offline");


  LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.println("Heltec.LoRa init succeeded.");

}

void onConnectionEstablished() {
  client.executeDelayed(5 * 1000, []() {
    String toServer = generateSerialJson(globalIncomingDeviceId, globalIncomingDeviceStatus);
    client.publish(TOPIC_NAME, toServer);
  });
}

void loop() {
  client.loop();


  if (millis() - lastSendTime > interval) {
    String message = "Central";   // send a message
    sendMessage(message);
    // println("Sending " + message);
    lastSendTime = millis();                // timestamp the message
    //interval = random(2000) + 1000;     // 2-3 seconds
    interval = 10000;                    // 10 seconds to advice everyone
    LoRa.receive();                     // go back into receive mode
  }
}

void sendMessage(String outgoing) {
  Serial.println("");
  Serial.println("[ " + String(msgCount) +  + " >> (Central #" + String(deviceId) + ") ] is now sending active signal.");
  Serial.println("");
  LoRa.beginPacket();                   // start packet
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(deviceType);               // add device type
  LoRa.write(deviceId);                 // add device ID
  // LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  byte incomingMsgId = LoRa.read();             // incoming msg ID
  byte incomingDeviceType = LoRa.read();       // incoming device type
  byte incomingDeviceId =  LoRa.read();       // incoming device ID
  byte incomingDeviceStatus = LoRa.read();   // incoming device status

  // setando essas variaveis acima de maneira GLOBAL

  globalIncomingDeviceType = incomingDeviceType;
  globalIncomingDeviceId = incomingDeviceId;
  globalIncomingDeviceStatus = incomingDeviceStatus;


  Serial.println();
  Serial.println("[ === Message ID: " + String(incomingMsgId) + " === ]");
  Serial.println("Device Type: " + String(incomingDeviceType));
  Serial.println("Device ID: " + String(incomingDeviceId));
  Serial.println("Device Status: " + String(incomingDeviceStatus));
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println("[ =================== ]");
  Serial.println();
}

String generateSerialJson(byte incDevId, boolean incDevSts) {
  String data = "{";
  data += "\"centralId\": ";
  data += String(deviceId);
  data += ", ";
  data += "\"stationId\": ";
  data += String(incDevId);
  data += ", ";
  data += "\"status\": ";
  if (incDevSts) {
    data += "true";
  }
  else {
    data += "false";
  }
  data += "}";
  return data;
}
