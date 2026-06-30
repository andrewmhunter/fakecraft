#include "config.hpp"
#include "ini_parser.hpp"

Config::Config(std::filesystem::path filePath) {
    IniFile ini{filePath};

    player.name = ini.getString("player", "name", "steve");

    graphics.fastLeaves = ini.getBool("graphics", "fast_leaves", false);
    graphics.renderDistance = ini.getInt("graphics", "render_distance", 7);

    game.saveChunks = ini.getBool("game", "save_chunks", true);
    game.loadChunks = ini.getBool("game", "load_chunks", true);

    worldgen.superflat = ini.getBool("worldgen", "superflat", false);
    worldgen.generateCaves = ini.getBool("worldgen", "generate_caves", false);
    worldgen.generateFeatures = ini.getBool("worldgen", "generate_features", true);
    worldgen.setSeed = ini.getInt("worldgen", "set_seed");

    controls.sensitivity = ini.getFloat("controls", "sensitivity", 0.0075);

    gui.scale = ini.getInt("gui", "scale", 2);
}

std::optional<Config> Config::settings{std::nullopt};
