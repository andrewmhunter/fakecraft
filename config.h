#ifndef CONFIG_H
#define CONFIG_H

//#define FAST_LEAVES
//#define USE_IGNORED

#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 128
#define WORLD_MAX_CHUNK_WIDTH 16

#define SENSITIVITY 0.0025f

//#define DEFAULT_SET_SEED 1000

#define DIRT_LAYER 1
#define SURFACE_OFFSET 60
#define OCEAN_LEVEL 47

#define PLAYER_EYE 1.62f

//#define TIME_MESHER

//#define PROFILING_STARTUP

//#define SUPERFLAT

#ifdef PROFILING_STARTUP
#ifndef DEFAULT_SET_SEED
#define DEFAULT_SET_SEED 1000
#endif
#endif

#endif
