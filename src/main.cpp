#include <Arduino.h>

#include <SPI.h>
#include <SD.h>
#include <MFRC522.h>

void printDirectory(File dir, int numTabs);
void printFile(String path);
void useSDCard(bool shouldUse);
void useRFID(bool shouldUse);

const int RST_PIN = 22; // Reset pin
const int RFID_SELECT_PIN = 21; 
const int SD_SELECT_PIN = 5;

File sdCardRoot;
 
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
  if (SD.begin(5)) {
    Serial.println("initialization done.");
    sdCardRoot = SD.open("/");
    printDirectory(sdCardRoot, 0);
  } else {
    Serial.println("initialization failed!");
  }

  useRFID(true);

  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
  mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

void loop(void) {

  useRFID(true);
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  Serial.println("New card present");
  
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("UID tag:");
  String content= "";
  
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  Serial.println(content);

  if (content.substring(1) == "69 E9 FA 63") {
    Serial.println("Authorized access");
    useSDCard(true);
    printFile("/info.txt");
    delay(3000);
  } else {
    Serial.println("Access denied");
    delay(3000);
  }
  
  // Dump debug info about the card; PICC_HaltA() is automatically called
  // mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
}

// SD Card methods.

void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

void printFile(String path) {
  File file = SD.open(path);
  if (file) {
    Serial.println(path);
    
    // read from the file until there's nothing else in it:
    while (file.available()) {
    	Serial.write(file.read());
    }
    // close the file:
    file.close();
  } else {
  	// if the file didn't open, print an error:
    Serial.println("error opening ");
    Serial.print(path);
  }
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
