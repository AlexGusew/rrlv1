# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Build the project
mkdir -p build && cd build
cmake ..
make

# Run the executable
./asteroids
```

## Project Architecture

This is a C++ game project using raylib and raygui, implementing an asteroids-style game with a unique node-based power system.

### Core Systems

**Game State Management**: Three states managed in `main.cpp`:
- MAIN_MENU: Initial screen
- GAMEPLAY: Active game with player, enemies, bullets
- GAME_OVER: Death screen

**Node System**: The game's central mechanic is a programmable node network where players connect different types of nodes:
- CPU_CORE: Powers the entire system, starts action sequences
- STAT nodes: Modify player attributes (health, speed, damage, fire rate)
- ACTION nodes: Executable abilities (fire blast, energy shield, phase shift)
- POWER nodes: Modify connected nodes (duration reduction, value addition)

**Connection Rules**: Nodes have specific connection patterns:
- CPU connects to any node type to power them
- STAT nodes can connect to ACTION nodes to provide buffs
- POWER nodes can modify connected STAT/ACTION nodes
- ACTION nodes can chain together in sequences

**Game Loop**: Standard game loop with:
- Player movement (WASD)
- Manual shooting (mouse left-click)
- Automated action system based on node connections
- Enemy spawning and AI
- Collision detection

### Key Files

- `sources/main.cpp`: Single-file implementation containing all game logic
- `CMakeLists.txt`: Build configuration fetching raylib and raygui from GitHub
- `assets/`: Contains game assets referenced via ASSETS_PATH macro

### Development Notes

The project uses C++17 and fetches dependencies (raylib/raygui) automatically via CMake FetchContent. The ASSETS_PATH macro is defined at compile time to point to the assets directory.

The node system state is managed through the Player class which contains both inventory nodes and placed nodes, with complex connection and activation logic handled in UpdateNodeActivation() and related functions.