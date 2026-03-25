// #include <ELECHOUSE_CC1101_SRC_DRV.h>
// #include <ESPiLight.h>


#include "src/ESPiLight/src/ESPiLight.h"
#include "src/SmartRC-CC1101-Driver-Lib/ELECHOUSE_CC1101_SRC_DRV.h"

// 2026 by kamilsss655
// ESP8266 + CC1101 433Mhz Remote Debug Tool

// HARDWARE CONNECTIONS:
// esp8266 - cc1101
// 4   - GDO0
// 16  - GDO2
// 14  - SCK
// 3V3 - VCC
// 13  - MOSI
// 12  - MISO
// 15  - CSN
// GND - GND

#define CC1101_GDO0 4

ESPiLight rf(CC1101_GDO0);

enum CC1101Mode { RX_MODE,
                  TX_MODE };
CC1101Mode ccMode = RX_MODE;

// Buffer to hold last received command
String lastCmd = "";

bool parseCommand(const String &input, String &protocol, String &message) {
  int start = input.indexOf('[');
  int end = input.indexOf(']');

  if (start == -1 || end == -1 || end <= start) {
    return false;
  }

  protocol = input.substring(start + 1, end);
  message = input.substring(end + 1);
  message.trim();  // remove spaces before JSON

  return true;
}

// callback function. It is called on successfully received and parsed rc signal
void rfCallback(const String &protocol, const String &message, int status,
                size_t repeats, const String &deviceID) {
  // Serial.print("RF signal arrived [");
  // Serial.print(protocol);  // protocoll used to parse
  // Serial.print("][");
  // Serial.print(deviceID);  // value of id key in json message
  // Serial.print("] (");
  // Serial.print(status);  // status of message, depending on repeat, either:
  //                        // FIRST   - first message of this protocoll within the
  //                        //           last 0.5 s
  //                        // INVALID - message repeat is not equal to the
  //                        //           previous message
  //                        // VALID   - message is equal to the previous message
  //                        // KNOWN   - repeat of a already valid message
  // Serial.print(") ");
  // Serial.print(message);  // message in json format
  // Serial.println();

  // check if message is valid and process it
  if (status == VALID) {
    Serial.print("Valid message: [");
    Serial.print(protocol);
    Serial.print("] ");
    Serial.print(message);
    Serial.println();
  }
}

void receive(void) {
  ccMode = RX_MODE;

  pinMode(CC1101_GDO0, INPUT);  // GDO0 drives RX interrupt
  delay(50);
  rf.setCallback(rfCallback);
  delay(50);
  rf.initReceiver(CC1101_GDO0);
  delay(50);

  delay(50);

  ELECHOUSE_cc1101.SetRx();  // put radio in receive mode
  delay(5);
}

void transmit(void) {
  ccMode = TX_MODE;
  rf.disableReceiver();
  delay(50);

  detachInterrupt(digitalPinToInterrupt(CC1101_GDO0));  // detach interrupt
  pinMode(CC1101_GDO0, OUTPUT);                         // drive TX pulses manually
  digitalWrite(CC1101_GDO0, LOW);                       // put low

  delay(50);
  ELECHOUSE_cc1101.SetTx();  // put radio in transmit mode
  delay(50);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  // ELECHOUSE_cc1101.SetTx();  // cc1101 set Transmit on
  if (ELECHOUSE_cc1101.getCC1101()) {  // Check the CC1101 Spi connection.
    Serial.println("CC1101 detected OK");
  } else {
    Serial.println("CC1101 not detected");
  }

  Serial.println("Configuring CC1101...");

  ELECHOUSE_cc1101.Init();
  ELECHOUSE_cc1101.setMHZ(433.92);    // Frequency
  ELECHOUSE_cc1101.setDRate(4.8);     // ELRO 800 bitrate
  ELECHOUSE_cc1101.setModulation(2);  // OOK mode
  ELECHOUSE_cc1101.setRxBW(203);      // Narrow RX bandwidth
  ELECHOUSE_cc1101.SetRx();           // Start RX
  Serial.println("CC1101 configured correctly!");

  Serial.println("Received codes will show up here");

  Serial.println("To send any code you can enter valid ESPiLight command ie: [elro_800_switch] {\"systemcode\":1,\"unitcode\":2,\"off\":1}");

  receive();
}

void sendBurst(void) {
  String protocol, message;

  if (!parseCommand(lastCmd, protocol, message)) {
    Serial.println("Invalid command format!");
    return;
  }

  unsigned long start = millis();

  Serial.print("TRANSMITTING [");
  Serial.print(protocol);
  Serial.print("] ");
  Serial.println(message);
  delay(50);

  transmit();
  delay(500);
  while (millis() - start < 3000) {
    rf.send(protocol.c_str(), message.c_str());
    delay(5);
  }

  receive();
  delay(100);

  Serial.println("TRANSMITTING DONE. RX MODE ON.");
}

void loop() {
  // process input queue and may fire calllback
  if (ccMode == RX_MODE) {
    rf.loop();
    delay(2);
  }


  // read serial input if available
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (lastCmd.length() > 0) {
        Serial.print("TXing last command: ");
        Serial.println(lastCmd);
        sendBurst();
        lastCmd = "";  // clear buffer after sending
      }
    } else {
      lastCmd += c;
    }
  }
  delay(5);
}
