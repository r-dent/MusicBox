#include <SDCardHelper.h>


void SDCardHelper::printDirectory(File dir, int numTabs, bool recursive) {
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
      if (recursive) {
        printDirectory(entry, numTabs + 1, true);
      }
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

void SDCardHelper::printFile(String path) {
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

void SDCardHelper::handleFiles(String directory, TFilePathHandler handler) {

    File dir = SD.open(directory);
    while (true) {

        File entry =  dir.openNextFile();
        if (!entry) {
            // no more files
            break;
        }
        if (!entry.isDirectory()) {
            handler(entry);
            entry.close();
        }
  }
}
