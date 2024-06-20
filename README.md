# Orion Engine {#mainpage}

## Overview

Welcome to the documentation for the Orion Engine project! This project is a solo game engine written in C, designed to support multiple graphics APIs, starting with Vulkan. The goal of this engine is to provide primarily a hands on learning experience for myself, and serve as a 'proving ground' for new rendering techniques and continually futhering my knowledge.

## Current Features

The game engine aims to support a wide range of features, for now the following are available:

- **Graphics APIs**: Initial support for Vulkan, with structure in place to enable multiple graphics APIs without changing game API
- **Shader Mangement**: Support for GLSL shader compilation and linkage, with plans to support HLSL as well
- **Input Handling**: Cross platform input handling, with plans and structure in place to support gamepads and custom controllers
- **Event System**: Configurable system with event registration for a variety of events both user level and system level
- **Memory Allocator**: Cross platform memory allocator with standard API exposed to both game and engine
- **Math Library**: Engine provided math library featuring 2D and 3D matrices, Quaternion structures, and trig functions

## Planned Features

In addition to existing features, there a number of modules that are planned for development:

- **Material Management**: Expose a Material api, including textures, shader maps (normal, displacement, etc) and PBR support
- **Scene Graph**: A hierarchical structure to manage and render game objects.
- **Physics Engine**: Custom physics engine to support various simulations
- **Audio Engine**: Support for 3D sound and sound effects.
- **Scripting**: Support for scripting languages to allow for flexible game logic.

## Current Development Status

At the moment, the primary focus is a solid foundation of engine structure and familiarity with Vulkan. The latest version supports the 'Current Features' as well as the following in-development features:

- **Texture Loading**: Implementing functionality to load textures from various formats.
- **Texture Mapping**: Developing methods to map textures onto 3D models.
- **Shader Integration**: Linking texture samplers into shaders for object rendering and render-to-texture support
- **Unit Tests**: Proper support for unit testing of various features, such as memory allocator, resource loading, and others

## Getting Started

To get started with the game engine, follow these steps:

0. **Install pre-requisites**:

#### [CMAKE](https://cmake.org/)
#### [clang](https://clang.llvm.org/)
#### [Vulkan](https://www.vulkan.org/)

1. **Clone the Repository**:
    ```bash
    git clone https://github.com/mmorgan00/Orion.git
    cd engine
    ```

2. **Build the Project**:
    ```bash
    ./build-all.sh #Linux
    ```
    or
    ```bash
    .\build-all.bat #Windows
    ```

3. **Run the Engine**:
Currently the project has a very basic testbed set up to launch the engine and render a single primitive with a basic shrinking animation.
As more features are implemented, this testbed will expand in functionality
    ```bash
    cd bin
    ./testbed
    ```

## Documentation

For detailed documentation on the engine's architecture and internal API, see the [documentation](/docs/index.html)

## Contact

If you have any questions or need further assistance, feel free to reach out:

- **Email**: maxwellmorgan23@gmail.com

---

*This documentation is generated using [Doxygen](https://www.doxygen.nl/).*
