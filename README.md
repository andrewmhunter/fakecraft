# Fakecraft
A voxel sandbox game based on Minecraft

## Dependencies

Fakecraft depends on
1. [raylib](https://www.raylib.com/) for graphics and window management. You'll have to install raylib to build fakecraft.
   I have plans to eventually remove the dependancy on raylib and switch to using straight OpenGL
3. [stb_perlin](https://github.com/nothings/stb/blob/master/stb_perlin.h) for perlin noise. Used in terrain generation. This is already included
   in the project as a single header file in `external/`

## Building
The git repository does not contain any copyrighted material. To make the game playable you'll first need to get some textures.
1. Create a directory `resources/` in the repositories root directory
2. Add a spritefont as `resources/defaultSpritefont.png`. You can find some online, the size might be a bit off
3. Add a file called `resources/alphaTerrain.png` this should be in the format of Minecrafts `terrain.png`.
   Any version of `terrain.png` should work but I suggest using the file from version Alpha v1.0.14. You can extract it from
   a legitimate copy of the game or download it [here](https://minecraft.wiki/w/File:201007301722_terrain.png)
4. Run `make`

## Controls

Movement: WASD

Sprint: Left Shift

Break Block: Left Click

Place Block: Right Click

Pick Block: Middle Click

Cycle Through Blocks: Scroll Wheel

Fly: F

NoClip: N

Spawn Entity: H

Decrease Render Distance: F8

Increase Render Distance: F9

Close Game: ESC

## Images

![house](/images/house.png)
![world](/images/world.png)
![cave](/images/cave.png)
![features](/images/features.png)

