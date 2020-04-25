#ifndef SD_CARD_HELPER_H_
#define SD_CARD_HELPER_H_

#include <Arduino.h>
#include <FS.h>
#include <SD.h>

class SDCardHelper {
public:
    typedef std::function<void(File)> TFilePathHandler;

    void printDirectory(File dir, int numTabs, bool recursive);
    void printFile(String path);
    void handleFiles(String directory, TFilePathHandler handler);
};

#endif