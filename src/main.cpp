#include <Arduino.h>

#include <SPI.h>
#include <SD.h>
#include <MFRC522.h>
#include <Audio.h>

#include <ConfigServer.h>
#include <SDCardHelper.h>

void playFile(String path);
void stopAudio();
void checkRFIDInterval();
void checkRFIDCard();
void useSDCard(bool shouldUse);
void useRFID(bool shouldUse);

const int RST_PIN = 22; // Reset pin
const int RFID_SELECT_PIN = 21; 
const int SD_SELECT_PIN = 5;

const int I2S_DOUT = 25;
const int I2S_BCLK = 27;
const int I2S_LRC = 26;

const int STOP_BTN_PIN = T0;

Audio audio;
File sdCardRoot;
bool isPlayingAudio = false;
time_t lastSwitchTime = 0;
ConfigServer configServer;
SDCardHelper sdCard;
 
MFRC522 mfrc522(RFID_SELECT_PIN, RST_PIN); // Create MFRC522 instance 

void setup()
{
  pinMode(RFID_SELECT_PIN, OUTPUT);
  pinMode(SD_SELECT_PIN, OUTPUT);

  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  useSDCard(true);

  Serial.print("Initializing SD card...");
  if (SD.begin(SD_SELECT_PIN)) {
    Serial.println("initialization done.");
    sdCardRoot = SD.open("/");
    sdCard.printDirectory(sdCardRoot, 0, false);
  } else {
    Serial.println("initialization failed!");
  }

  useRFID(true);

  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
  mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
  
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(2);

  configServer.setup();
}

void loop(void) {

  time_t currTime = millis();

  if (isPlayingAudio) {
    audio.loop();
  }
  
  configServer.loop();
  
  if (currTime - lastSwitchTime > 300 && digitalRead(STOP_BTN_PIN) == LOW) {
    stopAudio();
    lastSwitchTime = currTime;
  }

  checkRFIDInterval();
}

// Audio related.

void playFile(String path) {
  stopAudio();
  useSDCard(true);
  Serial.print("Playing file ");
  Serial.println(path);
  audio.connecttoSD(path);
  isPlayingAudio = true;
}

void stopAudio() {
  if (isPlayingAudio) {
    audio.stopSong();
    isPlayingAudio = false;
  }
}

// RFID related.

void checkRFIDInterval() {
	static const unsigned long REFRESH_INTERVAL = 1000; // ms
	static unsigned long lastRefreshTime = 0;
	
	if(millis() - lastRefreshTime >= REFRESH_INTERVAL) {
		lastRefreshTime += REFRESH_INTERVAL;
    checkRFIDCard();
	}
}

void checkRFIDCard() {

  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  stopAudio();
  //Serial.println("New card present");
  
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  //Serial.print("UID tag:");
  String uid = "";
  
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     uid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     uid.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  uid.trim();
  uid.toLowerCase();
  uid.replace(" ", "_");

  Serial.println(uid);
  configServer.RFIDCardUid = uid;

  if (uid == "69_e9_fa_63") {
    playFile("/320k_test.mp3");
  } else {
    // playFile("/KOGNITIF_Soul_Food_03.mp3");
    playFile("/Da_Josen_One_Seite_B.mp3 ");
  }
  
  // Dump debug info about the card; PICC_HaltA() is automatically called
  // mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
}

// SPI related.

void useSDCard(bool shouldUse) {
  digitalWrite(RFID_SELECT_PIN, HIGH);
  digitalWrite(SD_SELECT_PIN, (shouldUse ? LOW : HIGH));
}

void useRFID(bool shouldUse) {
  digitalWrite(SD_SELECT_PIN, HIGH);
  digitalWrite(RFID_SELECT_PIN, (shouldUse ? LOW : HIGH));
}
