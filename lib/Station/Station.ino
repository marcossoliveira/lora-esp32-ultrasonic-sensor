#include "heltec.h"
#include "Ultrasonic.h"

#define BAND    433E6  //you can set band here directly,e.g. 868E6,915E6

// Device type (0: Central | 1: Station)
byte deviceType = 1;
// Device ID
byte deviceId = 2;
// Device Status
boolean deviceStatus = 0;

String outgoing;              // outgoing message
byte msgCount = 0;            // count of outgoing messages
byte distance = 0;
long lastSendTime = 0;        // last send time
int interval = 500;          // interval between sends

//variável responsável por armazenar a distância lida pelo sensor ultrassônico
unsigned int distancia = 0;

//conexão dos pinos para o sensor ultrasonico
#define PIN_TRIGGER   16
#define PIN_ECHO      17

//Inicializa o sensor nos pinos definidos acima
Ultrasonic ultrasonic(PIN_TRIGGER, PIN_ECHO);

void setup() {
  //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);

  LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.println("Heltec.LoRa init succeeded.");
}

void loop() {
  if (millis() - lastSendTime > interval) {
    String message = "Distancia: ";   // send a message
    message += String(getDistance());

    distance = getDistance();
    if (distance != 0) {
      boolean prevDeviceStatus = deviceStatus;
      deviceStatus = getStatus(distance);

      if (prevDeviceStatus != deviceStatus) {
        sendMessage(message, deviceStatus);
      }
    }

    Serial.println("Distancia atual: " + String(distance));
    lastSendTime = millis();            // timestamp the message
    //interval = random(2000) + 1000;   // 2-3 seconds
    interval = 1000;
    LoRa.receive();                     // gos back into receive mode
  }
}

void sendMessage(String outgoing, boolean sts) {

  Serial.println("");

  String msg = "[ " + String(msgCount) +  + " >> (Station #" + String(deviceId) + ") ] is now sending signal: ";

  if (sts == true) {
    msg += " TRUE";
  } else {
    msg += " FALSE";
  }
  Serial.println(msg);

  LoRa.beginPacket();                   // start packet
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(deviceType);               // add device type
  LoRa.write(deviceId);                 // add device ID
  LoRa.write(sts);                      // add device ID
  // LoRa.print(outgoing);              // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingDeviceType = LoRa.read();       // incoming device type
  byte incomingDeviceId =  LoRa.read();       // incoming device ID

  String incoming = "";                 // payload of packet

  /* while (LoRa.available()) {            // can't use readString() in callback
    incoming += (char)LoRa.read();      // add bytes one by one
    }

    if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
    } */


  // if message is for this device, or broadcast, print details:
  if (incomingDeviceType == 0) {
    Serial.println();
    Serial.println("[ << " + String(incomingMsgId) + " ] - I'm #" + String(incomingDeviceId) + " device and I'm active.");
    Serial.println();
  }

  // Serial.println("RSSI: " + String(LoRa.packetRssi()));
  // Serial.println("Snr: " + String(LoRa.packetSnr()));

}


int getDistance() {
  //faz a leitura das informacoes do sensor (em cm)
  int distanciaCM;
  long microsec = ultrasonic.timing();
  // pode ser um float ex: 20,42 cm se declarar a var float
  distanciaCM = ultrasonic.convert(microsec, Ultrasonic::CM);

  return distanciaCM;
}

boolean getStatus(byte dist) {
  if (dist > 60) {
    return false;
  };
  return true;
}
