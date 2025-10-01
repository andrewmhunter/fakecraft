# Fakcraft
A voxel game based on Minecraft

## Dependencies

Fakecraft depends on
1. [raylib](https://www.raylib.com/) for graphics and window management. You'll have to install Raylib to build fakecraft.
   I have plans to eventually remove the dependancy on raylib and switch to using straight OpenGL
3. [stb_perlin](https://github.com/nothings/stb/blob/master/stb_perlin.h) for terrain generation. This is already included
   in the project as a single header file in external/

## Building
The git repository does not contain any copyrighted material. To make the game playable you'll first need to get some textures.
1. Create a directory resources/ in the repositories root directory
2. Add a spritefont to resources/ under the name defaultSpritefont.png. You can find some online, the size might be a bit off
3. Add a file called alphaTerrain.png to resources/ this should be in the format of Minecrafts terrain.png
   Any version of terrain.png should work but I suggest using the file from version Alpha v1.0.14. You can extract it from
   a legitimate copy of the game or download it [here](https://minecraft.wiki/w/File:201007301722_terrain.png)
4. Run `make`

