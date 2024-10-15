# Tower Defense
Tower Defense is one of my first projects in C++ as well as using SDL2.
Tower Defense is 2D game using SDL2:
- **SDL2 version 2.30.8**
- **SDL2_image version 2.8.2**
- **SDL2_ttf version 2.22.0.**
  
At the moment I'm focused on releasing it only for Windows (as well as source code in Visual Studio) but it might change in future.

Adding more, about Visual Studio, the solution file as well as writing whole source code has been done in Visual Studio 2022 and it has been written in C++20.

## Assets
For making some assets I've mostly used **Pyxel Edit**, but some of them have been made by someone else:
- tileset from **oryxdesignlab.com**
- attackers and enemies from [**Puny Characters by Shade**](https://merchant-shade.itch.io/16x16-puny-characters)
- Towers from [**"Survival - Tower Defense"** *by FkgCluster*](https://fkgcluster.itch.io/survivaltowerdefense)
- Health bars from [Pixel Health Bar Asset Pack 2 by adwitr](https://adwitr.itch.io/pixel-health-bar-asset-pack-2)

## Idea of the game
Basically, while making the game I've got more than just one idea of designing it, but at the moment I ended up with specifying amount of levels in *app.cpp* to prepare all of them at the start of the game, but the main job is done in App::LoadLevel(), I tried to make all of it to be done automatically and I tried to avoid hard-coding anything to make it easier to be adjusted if somehow anybody tried to have some fun with the game and make some adjustments/changes/tests, whatever.

The map file can be done literally as you wish, but important thing is to make all 3 layers of tiles (counting from 0), because the idea of loading map was like that:
Every layer of tiles is a seperated text file with extension *.map* containing all tiles separated by a comma *(,)*, and every layer has its own job - it's all should be properly described in class Level.
Every level must have config file named like this: *.config* and it's important to keep correct values in correct lines, since the game takes it all in proper order. It should be properly described too in source code.

### Win32 and x64
Currently, I'm trying to keep it compatible with both (Win32 and x64), but I can't promise it will stay like this (although nothing says I will stop doing that)
