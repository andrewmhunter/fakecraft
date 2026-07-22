# Fakecraft
A voxel sandbox game based on Minecraft, written in C++ using OpenGL. Previously it was written in C99 using
Raylib and it is currently in the process of being ported to C++ so the code structure is still more similar
to C than C++ in some places.

## Dependencies

1. [GLFW](https://github.com/glfw/glfw) for the game window, input and OpenGL context (must be installed)
2. [glm](https://github.com/g-truc/glm) for vector and matrix math (must be installed)
3. [glad](https://github.com/Dav1dde/glad) loads the OpenGL function pointers (already included in `external/`)
4. [stb libraries](https://github.com/nothings/stb):
   [stb_perlin](https://github.com/nothings/stb/blob/master/stb_perlin.h),
   [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) and
   [stb_image_write](https://github.com/nothings/stb/blob/master/stb_image_write.h)
   for perlin noise (used in terrain generation), loading images and writing images
   (already included in `external/`)

## Building
The git repository does not contain any copyrighted material. To make the game playable you'll first need to get some textures.
1. Extract the resources from Minecraft version Beta 1.7.3 to
   `assets/resources/`. Alternatively you can use extract a texture pack
   to this location
2. Copy `assets/resources/terrain.png` to
   `assets/resources/alphaTerrain.png`. Since the game doesn't have
   grass or foliage tinting yet grass and trees will look grey. To fix
   this you can instead use an older version of `terrain.png` such as
   [this one](https://minecraft.wiki/w/File:201007301722_terrain.png)
3. Run `cmake -B build`
4. Run `cmake --build build`
5. The final executable will be at `build/fakecraft`

## Controls

Movement: WASD

Sprint: Left Shift

Break Block: Left Click

Place Block: Right Click

Pick Block: Middle Click

Cycle Through Blocks: Scroll Wheel

Toggle Fly: F

Toggle NoClip: N

Spawn Entity: H

Decrease Render Distance: F8

Increase Render Distance: F9

Toggle Fullscreen: F11

Close Game: ESC

## Images

![house](images/house.png)
![world](images/world.png)
![cave](images/cave.png)
![features](images/features.png)

