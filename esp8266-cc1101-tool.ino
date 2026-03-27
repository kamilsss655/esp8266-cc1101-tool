#include "src/ESPiLight/src/ESPiLight.h"
#include "src/SmartRC-CC1101-Driver-Lib/ELECHOUSE_CC1101_SRC_DRV.h"

/* Copyright 2026 kamilsss655
 * https://github.com/kamilsss655
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

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
// #define DEBUG_MODE 1  // shows all packets

ESPiLight rf(CC1101_GDO0);

enum CC1101Mode { RX_MODE,
                  TX_MODE };
CC1101Mode ccMode = RX_MODE;  // current mode

enum CC1101Band {
  BAND_433,
  BAND_868
};
CC1101Band ccBand = BAND_868;  // current band

const char *bandToString[] = {
  "433",
  "868"
};

// Buffer to hold last received command
String lastCmd = "";

// RSSI monitoring variables
int lastRssiMean = 0;

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

// show available commands
void showCommands(void) {
  Serial.println("\nTo send any code you can enter valid ESPiLight command ie:");
  Serial.println("[elro_800_switch] {\"systemcode\":1,\"unitcode\":2,\"off\":1}");

  Serial.println("Available extra commands:");
  Serial.println("/band 433 - switch to 433 band");
  Serial.println("/band 868 - switch to 868 band\n");
}

// handle extra commands starting with /
// /band 433 - switch to 433 band
// /band 868 - switch to 868 band
bool handleExtraCommand(const String &cmd) {
  if (!cmd.startsWith("/")) return false;  // only handle extra commands starting with /

  int spaceIdx = cmd.indexOf(' ');
  String command = (spaceIdx == -1) ? cmd : cmd.substring(0, spaceIdx);
  String arg = (spaceIdx == -1) ? "" : cmd.substring(spaceIdx + 1);

  command.toLowerCase();
  arg.trim();

  if (command == "/band") {
    if (arg == "433") {
      ccBand = BAND_433;
      Serial.println("OK. Switched to 433 MHz.");
      reset();
    } else if (arg == "868") {
      ccBand = BAND_868;
      Serial.println("OK. Switched to 868 MHz.");
      reset();
    } else {
      Serial.println("Wrong arg. Usage: /band 433 | 868");
    }
    return true;
  }

  showCommands();  // show commands if cmd not found
  return true;
}

// callback function. It is called on successfully received and parsed rc signal
// status:
// FIRST   - first message of this protocoll within the last 0.5 s
// INVALID - message repeat is not equal to the previous message
// VALID   - message is equal to the previous message
// KNOWN   - repeat of a already valid message
void rfCallback(const String &protocol, const String &message, int status,
                size_t repeats, const String &deviceID) {

#ifdef DEBUG_MODE
  if (true) {  // show all in debug
#else
  // show VALID or FIRST message only by default to prevent spam
  // for 433 band show only VALID codes to deduplicate
  // for 868 show all since there are less duplicates
  if ((status == VALID && ccBand == BAND_433) || ccBand == BAND_868) {
#endif
    Serial.print("RX(BAND: ");
    Serial.print(bandToString[ccBand]);
    Serial.print(" | RSSI: ");
    Serial.print(lastRssiMean);
    Serial.print("): [");

    Serial.print(protocol);
    Serial.print("] ");
    Serial.print(message);
    Serial.println();
  }
}

// monitor rssi
void monitorRssi(void) {
  // if ((ELECHOUSE_cc1101.getMode() != 2) || (ccMode != RX_MODE)) return;  // exit if modem is not in RX mode

  bool signal = digitalRead(CC1101_GDO0);
  int rssi = ELECHOUSE_cc1101.getRssi();

  if (signal) {
    lastRssiMean = (lastRssiMean + rssi)/2; // single pole filter averaging
  }
}

// this will reflect rf activity on led, so even if protocol is not understood the led will light up during RX
void showActivity(void) {
#ifdef LED_PIN
  if (digitalRead(CC1101_GDO0) == HIGH) {
    digitalWrite(LED_PIN, LOW);
  } else {
    digitalWrite(LED_PIN, HIGH);
  }
#endif
}

// configure radio
void config(void) {
  ELECHOUSE_cc1101.SpiStrobe(CC1101_SIDLE);  // disable RX/TX and idle
  delay(10);
  ELECHOUSE_cc1101.Reset();              // reset chip
  ELECHOUSE_cc1101.RegConfigSettings();  // apply default config
  delay(10);

  switch (ccBand) {
    case BAND_433:
      // Serial.println("Config 433 Mhz band");

      ELECHOUSE_cc1101.setMHZ(433.92);    // Frequency
      ELECHOUSE_cc1101.setDRate(4.8);     // Typical bitrate
      ELECHOUSE_cc1101.setModulation(2);  // OOK mode
      ELECHOUSE_cc1101.setRxBW(203);      // Narrow RX bandwidth
      ELECHOUSE_cc1101.setPA(10);         // set power amplifier to 10dBm max
      break;

    case BAND_868:
      // Serial.println("Config 868 Mhz band");

      ELECHOUSE_cc1101.setMHZ(868.3);     // Frequency
      ELECHOUSE_cc1101.setDRate(17.24);   // Typical bitrate
      ELECHOUSE_cc1101.setModulation(0);  // 2-FSK mode
      ELECHOUSE_cc1101.setDeviation(40);  // 40kHz deviation for 2-FSK mode (test if better without, as used initially)
      ELECHOUSE_cc1101.setRxBW(270);      // RX bandwidth
      ELECHOUSE_cc1101.setPA(10);         // set power amplifier to 10dBm max
      break;

    default:
      Serial.println("ERROR: Unknown band. Restarting..");
      ESP.restart();
      break;
  }
  delay(10);
}

// put radio in RX mode
void receive(void) {
  ccMode = RX_MODE;

  pinMode(CC1101_GDO0, INPUT);  // GDO0 drives RX interrupt

#ifdef LED_PIN
  digitalWrite(LED_PIN, HIGH);
#endif

  config();
  delay(5);
  ELECHOUSE_cc1101.SetRx();  // put radio in receive mode
  delay(50);

  rf.setCallback(rfCallback);
  rf.initReceiver(CC1101_GDO0);
}

// put radio in TX mode
void transmit(void) {
  ccMode = TX_MODE;

  detachInterrupt(digitalPinToInterrupt(CC1101_GDO0));  // detach interrupt
  pinMode(CC1101_GDO0, OUTPUT);                         // drive TX pulses manually
  digitalWrite(CC1101_GDO0, LOW);                       // put low

  rf.disableReceiver();  // disable processing of incoming codes

#ifdef LED_PIN
  digitalWrite(LED_PIN, LOW);
#endif

  config();  // config radio
  delay(5);

  ELECHOUSE_cc1101.SetTx();  // put radio in transmit mode
  delay(50);
}

// reset radio into default RX state (detaches interrupts etc)
void reset(void) {
  if (ccMode == RX_MODE) {
    transmit();
    receive();
  } else if (ccMode == TX_MODE) {
    receive();
  } else {
    Serial.println("RESET ERROR: unknown ccMode.");
    ESP.restart();
  }
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

  ELECHOUSE_cc1101.Init();  // init SPI etc (make sure to invoke only once!)

  receive();  // start RX

  showCommands();

  Serial.println("CC1101 configured correctly!");

  Serial.println("Received codes will show up here: \n");
}

void sendBurst(void) {
  String protocol, message;

  if (!parseCommand(lastCmd, protocol, message)) {
    Serial.println("ERROR: Invalid command format!");
    showCommands();
    return;
  }

  unsigned long start = millis();

  Serial.print("TX: [");
  Serial.print(protocol);
  Serial.print("] ");
  Serial.println(message);

  transmit();
  // keep sending code for 3 seconds
  while (millis() - start < 3000) {
    rf.send(protocol.c_str(), message.c_str());
    delay(5);  // small gaps like real remote
  }

  receive();
#ifdef DEBUG_MODE
  Serial.println("TRANSMITTING DONE. RX MODE ON.");
#endif
}

void loop() {
  // process input queue and may fire calllback
  if (ccMode == RX_MODE) {
    rf.loop();       // process incoming codes
    
    showActivity();  // show any RX activity even if not understood - carrier detection
    
    monitorRssi();   // get rssi params from the radio environment

    // read serial input if available
    while (Serial.available() > 0) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        if (lastCmd.length() > 0) {
          if (!handleExtraCommand(lastCmd)) {  // handle extra command
            sendBurst();                       // fallback if no extra command detected it must be TX command
          }
          lastCmd = "";
        }
      } else {
        lastCmd += c;
      }
    }

    delay(1);
  }
}