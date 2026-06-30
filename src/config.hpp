#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <filesystem>

//#define USE_IGNORED

#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 256

#define DIRT_LAYER 2
#define SURFACE_OFFSET 73
#define OCEAN_LEVEL 60

#define PLAYER_EYE 1.62f

//#define TIME_MESHER

class Config {
public:
    struct {
        std::string name;
    } player;

    struct {
        bool fastLeaves;
        int renderDistance;
    } graphics;

    struct {
        bool saveChunks;
        bool loadChunks;
    } game;

    struct {
        bool superflat;
        bool generateCaves;
        bool generateFeatures;
        std::optional<int> setSeed;
    } worldgen;

    struct {
        float sensitivity;
    } controls;

    struct {
        int scale;
    } gui;

    Config(std::filesystem::path filePath);

    static std::optional<Config> settings;
};

#endif
