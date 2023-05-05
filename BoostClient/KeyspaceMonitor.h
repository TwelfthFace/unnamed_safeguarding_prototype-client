#pragma once

#include <string>
#include <vector>
#include <atomic>
#include <string>
#include "ScreenLocker.h"

#define log_length 1024

class KeyspaceMonitor {
public:
    KeyspaceMonitor(ScreenLocker* screenLocker);
    ~KeyspaceMonitor();
    void startMonitoring();
    void stopMonitoring();
    void getLog(std::vector<u_char>& outLog);

private:
    void monitorKeyPresses();
    void appendToLog(std::string word, bool blacklisted);
    std::vector<std::string> blacklist;
    ScreenLocker* screenLocker;
    std::atomic<bool> running;
    std::vector<u_char>* typeLog;
};