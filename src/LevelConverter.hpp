#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <matjson.hpp>

struct ConversionResult {
    bool success = false;
    std::string objectsString;
    std::string fullLevelString;
    std::string errorMessage;
    int objectCount = 0;
    int triggerCount = 0;
};

class LevelConverter {
public:
    static ConversionResult convertJson(const std::string& jsonContent);
    static std::string getDefaultHeader(const std::string& speed, int bgR, int bgG, int bgB, int gR, int gG, int gB);
    static int getEasingId(const std::string& name);
    static int getColorChannelId(const std::string& name);
    static int getObjectIdByType(const std::string& type);
    static bool isTrigger(int objectId);
};
