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
#define LED_PIN 2
// #define DEBUG_MODE 1 // shows all packets

ESPiLight rf(CC1101_GDO0);

enum CC1101Mode { RX_MODE,
                  TX_MODE };
CC1101Mode ccMode = RX_MODE; // current mode

enum CC1101Band {
  BAND_433,
  BAND_868
};
CC1101Band ccBand = BAND_868; // current band

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

  #ifdef DEBUG_MODE
    Serial.print("RX: [");
    Serial.print(protocol);
    Serial.print("] ");
    Serial.print(message);
    Serial.println();
  #else // show VALID or FIRST message only by default to prevent spam
  // status:
  // FIRST   - first message of this protocoll within the last 0.5 s
  // INVALID - message repeat is not equal to the previous message
  // VALID   - message is equal to the previous message
  // KNOWN   - repeat of a already valid message
    if (status == VALID || status == FIRST) {
      Serial.print("RX: [");
      Serial.print(protocol);
      Serial.print("] ");
      Serial.print(message);
      Serial.println();
    }
  #endif
}

// this will reflect rf activity on led, so even if protocol is not understood the led will light up during RX
void showActivity(void)
{
    #ifdef LED_PIN
      if (digitalRead(CC1101_GDO0) == HIGH) {
          digitalWrite(LED_PIN, LOW);
      } 
      else {
          digitalWrite(LED_PIN, HIGH);
      }
    #endif
}

void receive(void) {
  ccMode = RX_MODE;

  pinMode(CC1101_GDO0, INPUT);  // GDO0 drives RX interrupt
  delay(10);
  ELECHOUSE_cc1101.Init();

  // Setup CC1101 and init RX
    switch (ccBand) {
    case BAND_433:
      Serial.println("Init RX for 433 Mhz band..");

      
      ELECHOUSE_cc1101.setMHZ(433.92);    // Frequency
      ELECHOUSE_cc1101.setDRate(4.8);     // Typical bitrate
      ELECHOUSE_cc1101.setModulation(2);  // OOK mode
      ELECHOUSE_cc1101.setPA(10);         // set power amplifier to 10dBm max
      ELECHOUSE_cc1101.setRxBW(203);      // Narrow RX bandwidth
      break;

    case BAND_868:
      Serial.println("Init RX for 868 Mhz band..");

      ELECHOUSE_cc1101.setMHZ(868.3);     // Frequency
      ELECHOUSE_cc1101.setDRate(17.24);   // Typical bitrate
      ELECHOUSE_cc1101.setModulation(0);  // 2-FSK mode
      ELECHOUSE_cc1101.setDeviation(40);  // 40kHz deviation for 2-FSK mode (test if better without, as used initially)
      ELECHOUSE_cc1101.setPA(10);         // set power amplifier to 10dBm max
      ELECHOUSE_cc1101.setRxBW(270);      // RX bandwidth
      break;

    default:
      Serial.println("ERROR: Unknown band. Restarting..");
      ESP.restart();
      break;
  }

  ELECHOUSE_cc1101.SetRx();  // put radio in receive mode
  delay(10);

  rf.setCallback(rfCallback);
  rf.initReceiver(CC1101_GDO0);
}

void transmit(void) {
  ccMode = TX_MODE;
  rf.disableReceiver();
  delay(10);

  #ifdef LED_PIN
    digitalWrite(LED_PIN, LOW);
  #endif

  detachInterrupt(digitalPinToInterrupt(CC1101_GDO0));  // detach interrupt
  pinMode(CC1101_GDO0, OUTPUT);                         // drive TX pulses manually
  digitalWrite(CC1101_GDO0, LOW);                       // put low
  delay(10);

  ELECHOUSE_cc1101.SetTx();  // put radio in transmit mode
  delay(10);
}

void setup() {
  #ifdef LED_PIN
    pinMode(LED_PIN, OUTPUT);                        
    digitalWrite(LED_PIN, LOW);     
  #endif

  Serial.begin(115200);
  delay(500);

  #ifdef LED_PIN
    digitalWrite(LED_PIN, HIGH);     
  #endif

  // ELECHOUSE_cc1101.SetTx();  // cc1101 set Transmit on
  if (ELECHOUSE_cc1101.getCC1101()) {  // Check the CC1101 Spi connection.
    Serial.println("CC1101 detected OK");
  } else {
    Serial.println("CC1101 not detected");
  }

  Serial.println("Configuring CC1101...");

  receive();

  Serial.println("CC1101 configured correctly!");

  Serial.println("Received codes will show up here");

  Serial.println("To send any code you can enter valid ESPiLight command ie: [elro_800_switch] {\"systemcode\":1,\"unitcode\":2,\"off\":1}");
}

void sendBurst(void) {
  String protocol, message;

  if (!parseCommand(lastCmd, protocol, message)) {
    Serial.println("Invalid command format!");
    return;
  }

  unsigned long start = millis();

  Serial.print("TX: [");
  Serial.print(protocol);
  Serial.print("] ");
  Serial.println(message);

  transmit();
  delay(20);
  // keep sending code for 3 seconds
  while (millis() - start < 3000) {
    rf.send(protocol.c_str(), message.c_str());
    delay(5); // small gaps like real remote
  }

  receive();
  delay(20);
  #ifdef DEBUG_MODE
    Serial.println("TRANSMITTING DONE. RX MODE ON.");
  #endif
}

void loop() {
  // process input queue and may fire calllback
  if (ccMode == RX_MODE) {
    rf.loop();
    delay(2);
  }

  // show any RX activity even if not understood - carrier detection
  showActivity();

  // read serial input if available
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (lastCmd.length() > 0) {
        sendBurst();
        lastCmd = "";  // clear buffer after sending
      }
    } else {
      lastCmd += c;
    }
  }
  delay(5);
}
