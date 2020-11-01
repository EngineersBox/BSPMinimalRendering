#pragma once

#include <string>

#include "../io/INIReader.hpp"
#include "../exceptions/config/INIReadError.hpp"
#include "mappers/MinimapCfg.hpp"
#include "mappers/LoggingCfg.hpp"
#include "mappers/PlayerCfg.hpp"
#include "mappers/RenderCfg.hpp"

using namespace std;

#define RES_DIR string("resources/")
#define CFG_DIR RES_DIR + "configs/"
#define DEFAULT_CONFIG CFG_DIR + "config.ini"

#define PLAYER_SECTION "player"
#define MINIMAP_SECTION "minimap"
#define LOGGING_SECTION "logging"
#define RENDER_SECTION "rendering"

class ConfigInit {
    private:
        INIReader reader;
        string config_file;

    public:
        ConfigInit(const string& cfg_file = DEFAULT_CONFIG);
        ~ConfigInit();

        void processCfg();

        PlayerCfg initPlayerConfig();
        MinimapCfg initMinimapConfig();
        LoggingCfg initLoggingConfig();
        RenderCfg initRenderConfig();
        void initAll(PlayerCfg& p_cfg, MinimapCfg& m_cfg, LoggingCfg& l_cfg, RenderCfg& r_cfg);
};

ConfigInit::ConfigInit(const string& cfg_file) {
    this->config_file = cfg_file;
    processCfg();
};

ConfigInit::~ConfigInit() {};

void ConfigInit::processCfg() {
    this->reader = INIReader(config_file);
    if (this->reader.ParseError() < 0) {
        throw INIReadError("[" + config_file + "] Can't load INI file");
    }
}

PlayerCfg ConfigInit::initPlayerConfig() {
    return PlayerCfg(
        reader.GetFloat(PLAYER_SECTION, "fov", 70.0f),
        reader.GetFloat(PLAYER_SECTION, "dof", 8.0f),
        reader.GetFloat(PLAYER_SECTION, "move_speed", 3.0f),
        reader.GetFloat(PLAYER_SECTION, "rotation_speed", 2.0f)
    );
};

MinimapCfg ConfigInit::initMinimapConfig() {
    return MinimapCfg(
        reader.GetBoolean(MINIMAP_SECTION, "enable", false),
        reader.GetBoolean(MINIMAP_SECTION, "render_rays", false),
        parseMinimapPos(reader.Get(MINIMAP_SECTION, "pos", "TOP_RIGHT")),
        parseMinimapSize(reader.Get(MINIMAP_SECTION, "size", "MEDIUM")));
};

LoggingCfg ConfigInit::initLoggingConfig() {
    return LoggingCfg(
        reader.GetBoolean(LOGGING_SECTION, "gl_debug", false),
        reader.GetBoolean(LOGGING_SECTION, "player_pos", false),
        reader.GetBoolean(LOGGING_SECTION, "tex_skip_invalid", false),
        reader.GetBoolean(LOGGING_SECTION, "map_skip_invalid", false),
        reader.GetBoolean(LOGGING_SECTION, "hide_warnings", false),
        reader.GetBoolean(LOGGING_SECTION, "hide_infos", false),
        reader.GetBoolean(LOGGING_SECTION, "log_verbose", false)
    );
};

RenderCfg ConfigInit::initRenderConfig() {
    return RenderCfg(
        reader.GetBoolean(RENDER_SECTION, "headless_mode", false),
        reader.GetBoolean(RENDER_SECTION, "double_buffer", false),
        reader.GetBoolean(RENDER_SECTION, "render_walls", true),
        reader.GetBoolean(RENDER_SECTION, "render_floor_ceiling", true),
        reader.GetBoolean(RENDER_SECTION, "render_sprites", true),
        reader.GetInteger(RENDER_SECTION, "refresh_rate", 60),
        reader.GetInteger(RENDER_SECTION, "ray_count", 80));
}

void ConfigInit::initAll(PlayerCfg& p_cfg, MinimapCfg& m_cfg, LoggingCfg& l_cfg, RenderCfg& r_cfg) {
    p_cfg = initPlayerConfig();
    m_cfg = initMinimapConfig();
    l_cfg = initLoggingConfig();
    r_cfg = initRenderConfig();
}