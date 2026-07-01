#include "config.hpp"
#include "ini_parser.hpp"

Config::Config(std::filesystem::path filePath) {
    IniFile ini{filePath};

    player.name = ini.getString("player", "name", "steve");

    graphics.fastLeaves = ini.getBool("graphics", "fast_leaves", false);
    graphics.renderDistance = ini.getInt("graphics", "render_distance", 7);
    graphics.fov = ini.getFloat("graphics", "fov", 90.f);
    graphics.vsync = ini.getBool("graphics", "vsync", false);

    game.saveChunks = ini.getBool("game", "save_chunks", true);
    game.loadChunks = ini.getBool("game", "load_chunks", true);

    world.superflat = ini.getBool("world", "superflat", false);
    world.generateCaves = ini.getBool("world", "generate_caves", false);
    world.generateFeatures = ini.getBool("world", "generate_features", true);
    if (ini.getBool("world", "use_set_seed", false)) {
        world.setSeed = ini.getInt("world", "set_seed");
    }

    controls.sensitivity = ini.getFloat("controls", "sensitivity", 0.0075);

    gui.scale = ini.getInt("gui", "scale", 2);
}

std::optional<Config> Config::settings{std::nullopt};
