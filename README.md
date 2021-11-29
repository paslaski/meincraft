# MeinCraft
A Minecraft clone written in C++ and OpenGL.

## General Info
This is a personal project done purely for recreational and educational purposes. Many thanks to the authors of the libraries this project uses.

Current Features:
* infinite terrain generation from simplex noise
* different biomes
* batched rendering of chunks
* greedy meshing to dramatically reduce vertex data
* dynamic loading and unloading of chunks around the user
* sunlight with light checking across chunks (block light sources and light flow across chunks is in development)

Future Feature Wish List:
* improved terrain generation with unique structures (flora, buildings, etc.) as well as by simply increasing the uniqueness/cool-factor of generated terrain
* multithreaded chunk generation and meshing
* save/load worlds to disk
* block light sources and transparent blocks
* water with custom shaders
* collision + movement physics
* NPCs

## Technologies, External Libraries, and Dependencies
Project is created with:
* C++20
* [OpenGL](https://www.opengl.org/) 3.3
* [GLFW](https://www.glfw.org/) 3.3.5
* [glad](https://github.com/Dav1dde/glad) ([web generator](https://glad.dav1d.de/))
* [GLM](https://github.com/g-truc/glm)
* [EnTT](https://github.com/skypjack/entt)
* [FastNoiseLite](https://github.com/Auburn/FastNoiseLite)
* [stb](https://github.com/nothings/stb)
	
## Setup
In order to build this project, you will need cmake as well as GLFW version 3.3.4 or above. All other dependencies are included in the repository. You can install GLFW using the link above or your favorite package manager. My machine runs macOS, so my dynamic libraries files are dylib in the CMakeLists. You might be able to run this on Windows or Linux using the corresponding dll or so files for GLFW.

On macOS you can use [Homebrew](https://brew.sh/) to install GLFW:
```
$ brew install glfw
```

To run this project, install it locally using git and then build it with cmake.

```
$ git clone https://github.com/paslaski/meincraft.git
$ cd meincraft
$ cmake .
$ cmake --build .
```

Now run the project
```
$ ./meincraft
```

## Collaboration
Unfortunately, this project is not open for outside contribution. 
