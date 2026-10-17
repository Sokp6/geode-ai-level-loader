#include "LevelConverter.hpp"
#include <sstream>
#include <algorithm>
#include <iomanip>

int LevelConverter::getEasingId(const std::string& name) {
    static const std::unordered_map<std::string, int> easings = {
        {"none", 0},
        {"ease_in_out", 1},
        {"ease_in", 2},
        {"ease_out", 3},
        {"elastic_in_out", 4},
        {"elastic_in", 5},
        {"elastic_out", 6},
        {"bounce_in_out", 7},
        {"bounce_in", 8},
        {"bounce_out", 9},
        {"exponential_in_out", 10},
        {"exponential_in", 11},
        {"exponential_out", 12},
        {"sine_in_out", 13},
        {"sine_in", 14},
        {"sine_out", 15},
        {"back_in_out", 16},
        {"back_in", 17},
        {"back_out", 18}
    };
    auto it = easings.find(name);
    if (it != easings.end()) return it->second;
    return 0;
}

int LevelConverter::getColorChannelId(const std::string& name) {
    static const std::unordered_map<std::string, int> channels = {
        {"bg", 1000},
        {"background", 1000},
        {"g1", 1001},
        {"ground1", 1001},
        {"line", 1002},
        {"3dl", 1003},
        {"obj", 1004},
        {"object", 1004},
        {"g2", 1009},
        {"ground2", 1009}
    };
    auto it = channels.find(name);
    if (it != channels.end()) return it->second;
    try {
        return std::stoi(name);
    } catch (...) {
        return 1000;
    }
}

int LevelConverter::getObjectIdByType(const std::string& type) {
    static const std::unordered_map<std::string, int> objectTypes = {
        {"block", 1},
        {"block_outline", 6},
        {"slab", 7},
        {"half_block", 7},
        {"slab_outline", 8},
        {"spike", 2},
        {"spike_short", 3},
        {"spike_hanging", 4},
        {"spike_upsidedown", 5},
        {"orb_yellow", 35},
        {"orb_pink", 84},
        {"orb_red", 140},
        {"orb_blue", 1022},
        {"orb_green", 1005},
        {"orb_black", 1704},
        {"orb_dash_green", 1332},
        {"orb_dash_pink", 1333},
        {"pad_yellow", 36},
        {"pad_pink", 141},
        {"pad_red", 1331},
        {"pad_blue", 67},
        {"coin", 1329},
        {"portal_cube", 10},
        {"portal_ship", 11},
        {"portal_ball", 12},
        {"portal_ufo", 13},
        {"portal_wave", 47},
        {"portal_robot", 1330},
        {"portal_spider", 1331},
        {"portal_swing", 2013},
        {"portal_speed_slow", 13},
        {"portal_speed_normal", 12},
        {"portal_speed_fast", 47},
        {"portal_speed_faster", 200},
        {"portal_speed_fastest", 1334},
        {"portal_gravity_down", 10},
        {"portal_gravity_up", 11},
        {"portal_size_normal", 14},
        {"portal_size_mini", 101},
        {"trigger_color", 899},
        {"trigger_move", 901},
        {"trigger_pulse", 1006},
        {"trigger_alpha", 1007},
        {"trigger_toggle", 1049},
        {"trigger_spawn", 1268},
        {"trigger_rotate", 1815},
        {"trigger_shake", 1520},
        {"trigger_stop", 1611}
    };
    auto it = objectTypes.find(type);
    if (it != objectTypes.end()) return it->second;
    return 1;
}

bool LevelConverter::isTrigger(int objectId) {
    return objectId == 899 || objectId == 901 || objectId == 1006 ||
           objectId == 1007 || objectId == 1049 || objectId == 1268 ||
           objectId == 1815 || objectId == 1520 || objectId == 1611;
}

std::string LevelConverter::getDefaultHeader(const std::string& speed, int bgR, int bgG, int bgB, int gR, int gG, int gB) {
    int speedVal = 0;
    if (speed == "0.5x") speedVal = 1;
    else if (speed == "2x") speedVal = 2;
    else if (speed == "3x") speedVal = 3;
    else if (speed == "4x") speedVal = 4;

    std::ostringstream ss;
    ss << "kS38,1_" << bgR << "_2_" << bgG << "_3_" << bgB << "_11_255_12_255_13_255_4_-1_6_1000_7_1_15_1_18_0_8_1|";
    ss << "1_" << gR << "_2_" << gG << "_3_" << gB << "_11_255_12_255_13_255_4_-1_6_1001_7_1_15_1_18_0_8_1,";
    ss << "kA13,0,kA15,0,kA16,0,kA14,0,kA6,0,kA7,0,kA17,0,kA18,0,kS39,0,kA2," << speedVal << ",kA3,0,kA8,0,kA4,0,kA9,0,kA10,0,kA11,0;";
    return ss.str();
}

ConversionResult LevelConverter::convertJson(const std::string& jsonContent) {
    ConversionResult result;
    
    auto parseRes = matjson::parse(jsonContent);
    if (!parseRes) {
        result.errorMessage = "JSON parsing error: " + parseRes.unwrapErr();
        return result;
    }

    auto root = parseRes.unwrap();
    if (!root.isObject()) {
        result.errorMessage = "Root JSON must be an object with an 'objects' array.";
        return result;
    }

    int bgR = 40, bgG = 125, bgB = 255;
    if (root.contains("bg_color") && root["bg_color"].isArray() && root["bg_color"].asArray().unwrap().size() >= 3) {
        auto arr = root["bg_color"].asArray().unwrap();
        bgR = arr[0].asInt().unwrapOr(40);
        bgG = arr[1].asInt().unwrapOr(125);
        bgB = arr[2].asInt().unwrapOr(255);
    }

    int gR = 0, gG = 102, gB = 255;
    if (root.contains("ground_color") && root["ground_color"].isArray() && root["ground_color"].asArray().unwrap().size() >= 3) {
        auto arr = root["ground_color"].asArray().unwrap();
        gR = arr[0].asInt().unwrapOr(0);
        gG = arr[1].asInt().unwrapOr(102);
        gB = arr[2].asInt().unwrapOr(255);
    }

    std::string speed = "1x";
    if (root.contains("speed") && root["speed"].isString()) {
        speed = root["speed"].asString().unwrap();
    }

    if (!root.contains("objects") || !root["objects"].isArray()) {
        result.errorMessage = "Missing or invalid 'objects' array in JSON.";
        return result;
    }

    auto objects = root["objects"].asArray().unwrap();
    std::ostringstream objStream;

    for (const auto& item : objects) {
        if (!item.isObject()) continue;

        int objId = 1;
        if (item.contains("id") && item["id"].isNumber()) {
            objId = item["id"].asInt().unwrap();
        } else if (item.contains("type") && item["type"].isString()) {
            objId = getObjectIdByType(item["type"].asString().unwrap());
        }

        // Calculate positions
        float posX = 15.0f;
        float posY = 15.0f;

        if (item.contains("raw_x") && item["raw_x"].isNumber()) {
            posX = static_cast<float>(item["raw_x"].asDouble().unwrap());
        } else if (item.contains("x") && item["x"].isNumber()) {
            posX = static_cast<float>(item["x"].asDouble().unwrap()) * 30.0f + 15.0f;
        }

        if (item.contains("raw_y") && item["raw_y"].isNumber()) {
            posY = static_cast<float>(item["raw_y"].asDouble().unwrap());
        } else if (item.contains("y") && item["y"].isNumber()) {
            posY = static_cast<float>(item["y"].asDouble().unwrap()) * 30.0f + 15.0f;
        }

        objStream << "1," << objId << ",2," << posX << ",3," << posY;

        // Common properties
        if (item.contains("rotation") && item["rotation"].isNumber()) {
            objStream << ",6," << item["rotation"].asDouble().unwrap();
        }
        if (item.contains("flip_x") && item["flip_x"].isBool() && item["flip_x"].asBool().unwrap()) {
            objStream << ",4,1";
        }
        if (item.contains("flip_y") && item["flip_y"].isBool() && item["flip_y"].asBool().unwrap()) {
            objStream << ",5,1";
        }

        // Groups
        if (item.contains("groups") && item["groups"].isArray()) {
            auto grps = item["groups"].asArray().unwrap();
            if (!grps.empty()) {
                objStream << ",57,";
                for (size_t g = 0; g < grps.size(); ++g) {
                    if (g > 0) objStream << ".";
                    objStream << grps[g].asInt().unwrapOr(0);
                }
            }
        }

        // Specific trigger handling
        if (objId == 901) { // Move Trigger
            int targetGroup = item.contains("target_group") ? item["target_group"].asInt().unwrapOr(0) : 0;
            float duration = item.contains("duration") ? static_cast<float>(item["duration"].asDouble().unwrapOr(0.5)) : 0.5f;
            
            float moveX = 0.0f;
            if (item.contains("move_x_blocks") && item["move_x_blocks"].isNumber()) {
                moveX = static_cast<float>(item["move_x_blocks"].asDouble().unwrap()) * 10.0f;
            } else if (item.contains("move_x") && item["move_x"].isNumber()) {
                moveX = static_cast<float>(item["move_x"].asDouble().unwrap());
            }

            float moveY = 0.0f;
            if (item.contains("move_y_blocks") && item["move_y_blocks"].isNumber()) {
                moveY = static_cast<float>(item["move_y_blocks"].asDouble().unwrap()) * 10.0f;
            } else if (item.contains("move_y") && item["move_y"].isNumber()) {
                moveY = static_cast<float>(item["move_y"].asDouble().unwrap());
            }

            int easingId = 0;
            if (item.contains("easing")) {
                if (item["easing"].isString()) {
                    easingId = getEasingId(item["easing"].asString().unwrap());
                } else if (item["easing"].isNumber()) {
                    easingId = item["easing"].asInt().unwrap();
                }
            }
            float easingRate = item.contains("easing_rate") ? static_cast<float>(item["easing_rate"].asDouble().unwrapOr(2.0)) : 2.0f;

            objStream << ",28," << targetGroup << ",30," << moveX << ",31," << moveY
                      << ",10," << duration << ",35," << easingId << ",36," << easingRate;
            result.triggerCount++;
        }
        else if (objId == 899) { // Color Trigger
            int channelId = 1000;
            if (item.contains("channel")) {
                if (item["channel"].isString()) {
                    channelId = getColorChannelId(item["channel"].asString().unwrap());
                } else if (item["channel"].isNumber()) {
                    channelId = item["channel"].asInt().unwrap();
                }
            }

            int r = item.contains("r") ? item["r"].asInt().unwrapOr(255) : 255;
            int g = item.contains("g") ? item["g"].asInt().unwrapOr(255) : 255;
            int b = item.contains("b") ? item["b"].asInt().unwrapOr(255) : 255;
            float duration = item.contains("duration") ? static_cast<float>(item["duration"].asDouble().unwrapOr(0.0)) : 0.0f;
            bool blending = item.contains("blending") ? item["blending"].asBool().unwrapOr(false) : false;

            objStream << ",23," << channelId << ",7," << r << ",8," << g << ",9," << b
                      << ",10," << duration << ",17," << (blending ? 1 : 0);
            result.triggerCount++;
        }
        else if (objId == 1006) { // Pulse Trigger
            int targetType = 0; // 0 = Channel, 1 = Group
            int targetId = 1000;
            if (item.contains("target_group")) {
                targetType = 1;
                targetId = item["target_group"].asInt().unwrap();
            } else if (item.contains("target_type") && item["target_type"].asString().unwrapOr("") == "group") {
                targetType = 1;
                targetId = item.contains("target_id") ? item["target_id"].asInt().unwrapOr(1) : 1;
            } else if (item.contains("channel")) {
                targetType = 0;
                targetId = getColorChannelId(item["channel"].asString().unwrapOr("bg"));
            }

            int r = item.contains("r") ? item["r"].asInt().unwrapOr(255) : 255;
            int g = item.contains("g") ? item["g"].asInt().unwrapOr(255) : 255;
            int b = item.contains("b") ? item["b"].asInt().unwrapOr(255) : 255;
            float fadeIn = item.contains("fade_in") ? static_cast<float>(item["fade_in"].asDouble().unwrapOr(0.0)) : 0.0f;
            float hold = item.contains("hold") ? static_cast<float>(item["hold"].asDouble().unwrapOr(0.0)) : 0.0f;
            float fadeOut = item.contains("fade_out") ? static_cast<float>(item["fade_out"].asDouble().unwrapOr(0.0)) : 0.0f;

            objStream << ",51," << targetType << ",52," << targetId << ",7," << r << ",8," << g << ",9," << b
                      << ",45," << fadeIn << ",46," << hold << ",47," << fadeOut << ",48,0";
            result.triggerCount++;
        }
        else if (objId == 1007) { // Alpha Trigger
            int targetGroup = item.contains("target_group") ? item["target_group"].asInt().unwrapOr(0) : 0;
            float opacity = item.contains("opacity") ? static_cast<float>(item["opacity"].asDouble().unwrapOr(1.0)) : 1.0f;
            float duration = item.contains("duration") ? static_cast<float>(item["duration"].asDouble().unwrapOr(0.0)) : 0.0f;

            objStream << ",28," << targetGroup << ",85," << opacity << ",10," << duration;
            result.triggerCount++;
        }
        else if (objId == 1815) { // Rotate Trigger
            int targetGroup = item.contains("target_group") ? item["target_group"].asInt().unwrapOr(0) : 0;
            int centerGroup = item.contains("center_group") ? item["center_group"].asInt().unwrapOr(0) : 0;
            float degrees = item.contains("degrees") ? static_cast<float>(item["degrees"].asDouble().unwrapOr(0.0)) : 0.0f;
            int times360 = item.contains("times_360") ? item["times_360"].asInt().unwrapOr(0) : 0;
            bool lockRot = item.contains("lock_rotation") ? item["lock_rotation"].asBool().unwrapOr(false) : false;
            float duration = item.contains("duration") ? static_cast<float>(item["duration"].asDouble().unwrapOr(1.0)) : 1.0f;

            objStream << ",28," << targetGroup << ",29," << centerGroup << ",68," << degrees
                      << ",69," << times360 << ",70," << (lockRot ? 1 : 0) << ",10," << duration;
            result.triggerCount++;
        }
        else if (objId == 1520) { // Shake Trigger
            float strength = item.contains("strength") ? static_cast<float>(item["strength"].asDouble().unwrapOr(2.0)) : 2.0f;
            float interval = item.contains("interval") ? static_cast<float>(item["interval"].asDouble().unwrapOr(0.05)) : 0.05f;
            float duration = item.contains("duration") ? static_cast<float>(item["duration"].asDouble().unwrapOr(0.5)) : 0.5f;

            objStream << ",75," << strength << ",76," << interval << ",10," << duration;
            result.triggerCount++;
        }
        else if (objId == 1049) { // Toggle Trigger
            int targetGroup = item.contains("target_group") ? item["target_group"].asInt().unwrapOr(0) : 0;
            bool activate = item.contains("activate") ? item["activate"].asBool().unwrapOr(false) : false;

            objStream << ",28," << targetGroup << ",56," << (activate ? 1 : 0);
            result.triggerCount++;
        }
        else if (objId == 1268) { // Spawn Trigger
            int targetGroup = item.contains("target_group") ? item["target_group"].asInt().unwrapOr(0) : 0;
            float delay = item.contains("delay") ? static_cast<float>(item["delay"].asDouble().unwrapOr(0.0)) : 0.0f;

            objStream << ",28," << targetGroup << ",63," << delay;
            result.triggerCount++;
        }
        else if (objId == 1611) { // Stop Trigger
            int targetGroup = item.contains("target_group") ? item["target_group"].asInt().unwrapOr(0) : 0;
            objStream << ",28," << targetGroup;
            result.triggerCount++;
        } else if (isTrigger(objId)) {
            result.triggerCount++;
        }

        objStream << ";";
        result.objectCount++;
    }

    result.objectsString = objStream.str();
    result.fullLevelString = getDefaultHeader(speed, bgR, bgG, bgB, gR, gG, gB) + result.objectsString;
    result.success = true;
    return result;
}
