#include <Geode/Geode.hpp>
#include <Geode/modify/EditLevelLayer.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include "LevelConverter.hpp"
#include <fstream>
#include <filesystem>

using namespace geode::prelude;

static const std::string DEMO_JSON = R"({
  "name": "AI Showcase Level",
  "author": "Antigravity AI",
  "description": "Demo level generated with AI triggers",
  "speed": "1x",
  "bg_color": [15, 20, 50],
  "ground_color": [25, 35, 75],
  "objects": [
    { "type": "portal_speed_normal", "x": 0, "y": 1 },
    { "type": "trigger_color", "x": 2, "y": 1, "channel": "bg", "r": 80, "g": 20, "b": 120, "duration": 1.5 },
    { "type": "block", "x": 5, "y": 1, "groups": [1] },
    { "type": "block", "x": 6, "y": 1, "groups": [1] },
    { "type": "block", "x": 7, "y": 1, "groups": [1] },
    { "type": "spike", "x": 8, "y": 1 },
    { "type": "orb_yellow", "x": 8, "y": 3 },
    { "type": "block", "x": 9, "y": 1 },
    { "type": "trigger_move", "x": 3, "y": 2, "target_group": 1, "move_x_blocks": 0, "move_y_blocks": 2, "duration": 0.5, "easing": "ease_in_out" },
    { "type": "trigger_pulse", "x": 6, "y": 3, "target_group": 1, "r": 255, "g": 255, "b": 100, "fade_in": 0.05, "hold": 0.2, "fade_out": 0.3 },
    { "type": "trigger_alpha", "x": 4, "y": 2, "target_group": 1, "opacity": 0.6, "duration": 0.3 },
    { "type": "pad_yellow", "x": 10, "y": 1 },
    { "type": "block", "x": 12, "y": 4 },
    { "type": "block", "x": 13, "y": 4 },
    { "type": "trigger_shake", "x": 10, "y": 2, "strength": 2.0, "interval": 0.04, "duration": 0.4 },
    { "type": "portal_wave", "x": 15, "y": 3 }
  ]
})";

static std::string getLevelFilePath() {
    auto pathStr = Mod::get()->getSettingValue<std::string>("json-path");
    if (pathStr.empty()) {
        pathStr = "/sdcard/Download/ai_level.json";
    }
    return pathStr;
}

static bool ensureFileExists(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        try {
            auto parent = std::filesystem::path(path).parent_path();
            if (!parent.empty() && !std::filesystem::exists(parent)) {
                std::filesystem::create_directories(parent);
            }
            std::ofstream out(path);
            if (out.is_open()) {
                out << DEMO_JSON;
                out.close();
                return true;
            }
        } catch (...) {
            return false;
        }
        return false;
    }
    return true;
}

class $modify(AILoaderEditLevelLayer, EditLevelLayer) {
    bool init(GJGameLevel* level) {
        if (!EditLevelLayer::init(level)) return false;

        CCMenu* menu = nullptr;
        if (auto actionsMenu = this->getChildByID("level-actions-menu")) {
            menu = typeinfo_cast<CCMenu*>(actionsMenu);
        }
        if (!menu) {
            if (auto infoMenu = this->getChildByID("info-menu")) {
                menu = typeinfo_cast<CCMenu*>(infoMenu);
            }
        }
        if (!menu) {
            menu = CCMenu::create();
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            menu->setPosition({ winSize.width / 2, 45.0f });
            this->addChild(menu);
        }

        auto spr = ButtonSprite::create("AI JSON", "goldFont.fnt", "GJ_button_01.png", .8f);
        auto btn = CCMenuItemSpriteExtra::create(
            spr, this, menu_selector(AILoaderEditLevelLayer::onImportAILevel)
        );
        btn->setID("import-ai-level-btn"_spr);

        menu->addChild(btn);
        menu->updateLayout();

        return true;
    }

    void onImportAILevel(CCObject* sender) {
        auto filePath = getLevelFilePath();
        bool createdSample = false;

        if (!std::filesystem::exists(filePath)) {
            if (ensureFileExists(filePath)) {
                createdSample = true;
            } else {
                FLAlertLayer::create(
                    "AI Level Loader",
                    fmt::format("Could not find or create file at:\n<cy>{}</c>", filePath),
                    "OK"
                )->show();
                return;
            }
        }

        std::ifstream file(filePath);
        if (!file.is_open()) {
            FLAlertLayer::create(
                "AI Level Loader",
                fmt::format("Failed to open file at:\n<cy>{}</c>", filePath),
                "OK"
            )->show();
            return;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        auto result = LevelConverter::convertJson(buffer.str());
        if (!result.success) {
            FLAlertLayer::create(
                "JSON Error",
                fmt::format("<cr>{}</c>", result.errorMessage),
                "OK"
            )->show();
            return;
        }

        // Apply level string to the current level
        m_level->m_levelString = result.fullLevelString;

        // Optionally copy objects to clipboard
        if (Mod::get()->getSettingValue<bool>("paste-to-clipboard")) {
            clipboard::write(result.objectsString);
        }

        std::string msg;
        if (createdSample) {
            msg = fmt::format(
                "Created demo JSON at:\n<cy>{}</c>\n\n"
                "Imported <cg>{}</c> objects (<cy>{}</c> triggers).\n\n"
                "Press <cg>Edit</c> to see your level in action!",
                filePath, result.objectCount, result.triggerCount
            );
        } else {
            msg = fmt::format(
                "Successfully loaded from:\n<cy>{}</c>\n\n"
                "Imported <cg>{}</c> objects (<cy>{}</c> triggers)!\n\n"
                "Press <cg>Edit</c> to view and playtest.",
                filePath, result.objectCount, result.triggerCount
            );
        }

        FLAlertLayer::create("AI Level Loaded", msg, "Awesome!")->show();
    }
};

class $modify(AILoaderEditorPauseLayer, EditorPauseLayer) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorPauseLayer::init(editorLayer)) return false;

        CCMenu* menu = nullptr;
        if (auto actionsMenu = this->getChildByID("actions-menu")) {
            menu = typeinfo_cast<CCMenu*>(actionsMenu);
        }
        if (!menu) {
            if (auto bottomMenu = this->getChildByID("bottom-menu")) {
                menu = typeinfo_cast<CCMenu*>(bottomMenu);
            }
        }
        if (!menu) {
            menu = CCMenu::create();
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            menu->setPosition({ winSize.width / 2, 40.0f });
            this->addChild(menu);
        }

        auto spr = ButtonSprite::create("AI JSON", "goldFont.fnt", "GJ_button_04.png", .7f);
        auto btn = CCMenuItemSpriteExtra::create(
            spr, this, menu_selector(AILoaderEditorPauseLayer::onCopyAIObjects)
        );
        btn->setID("copy-ai-objects-btn"_spr);

        menu->addChild(btn);
        menu->updateLayout();

        return true;
    }

    void onCopyAIObjects(CCObject* sender) {
        auto filePath = getLevelFilePath();
        ensureFileExists(filePath);

        std::ifstream file(filePath);
        if (!file.is_open()) {
            FLAlertLayer::create("Error", fmt::format("Cannot open {}", filePath), "OK")->show();
            return;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        auto result = LevelConverter::convertJson(buffer.str());
        if (!result.success) {
            FLAlertLayer::create("JSON Error", result.errorMessage, "OK")->show();
            return;
        }

        clipboard::write(result.objectsString);

        FLAlertLayer::create(
            "Copied to Clipboard",
            fmt::format(
                "<cg>{}</c> objects (<cy>{}</c> triggers) copied to clipboard!\n\n"
                "Resume the editor and press <cy>Paste</c> to place them anywhere.",
                result.objectCount, result.triggerCount
            ),
            "Got it!"
        )->show();
    }
};
