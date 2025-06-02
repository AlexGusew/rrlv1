# Robo Roguelike: Power Nodes v2 (RRL v1)

A unique asteroids-style roguelike game with a programmable node-based power system. Players build and connect computational nodes to create custom ability sequences, stat modifications, and power enhancements.

## 🎮 Game Overview

**Robo Roguelike** combines classic asteroids gameplay with an innovative node programming mechanic. Players survive waves of enemies while constructing and managing a node network that determines their abilities, stats, and action sequences.

### Key Features

- **Node-Based Programming System**: Connect different node types to create custom ability chains
- **Dynamic Stat System**: STAT nodes modify player attributes (health, speed, damage, fire rate)
- **Action Sequences**: Chain ACTION nodes to create complex ability rotations
- **Power Modifiers**: POWER nodes enhance connected nodes with value/duration changes
- **Real-Time Combat**: Classic asteroids movement with modern node-powered abilities
- **Progressive Difficulty**: Enemies drop new nodes, expanding your system capabilities

## 🏗️ Project Architecture

This project has been **fully refactored** from a monolithic structure to clean **Object-Oriented Programming (OOP)** principles while maintaining 100% feature parity.

### Core Classes

#### **Node System**
- **`BaseNode`** - Abstract base class with virtual `getType()` method
- **Individual Node Classes** - Each node type implements specific behavior:
  - `CPUCoreNode` - Central processing unit that powers other nodes
  - `HealthStatNode`, `SpeedStatNode`, `DamageStatNode`, `FireRateStatNode` - Stat modifiers
  - `FireActionNode`, `ShieldActionNode`, `ShiftActionNode` - Executable abilities
  - `DurationReducePowerNode`, `ValueAddPowerNode` - Enhancement modifiers

#### **Game Systems**
- **`Player`** - Encapsulates player state, node inventory, and stat calculations
- **`NodesController`** - Manages node system logic, activation, and action sequences
- **`ControlPanel`** - Handles node management UI (drag/drop, connections, visualization)
- **`HUD`** - Manages all game UI display (stats, menus, game states)

### Node Connection Rules

The node system follows specific connection patterns:

- **CPU Core → Any Node**: Powers STAT/POWER nodes, starts ACTION sequences
- **STAT → ACTION**: Provides buffs to connected actions
- **ACTION → ACTION**: Creates sequential execution chains
- **POWER → STAT/ACTION**: Modifies connected node values or durations

## 🛠️ Building and Running

### Prerequisites
- CMake 3.16+
- C++17 compatible compiler
- Git (for fetching dependencies)

### Build Instructions

```bash
# Create build directory
mkdir -p build && cd build

# Configure and build
cmake ..
make

# Run the game
./asteroids
```

### Dependencies
- **raylib** - Automatically fetched and built via CMake FetchContent
- **raygui** - UI components, also auto-fetched

## 🎯 Gameplay Instructions

### Basic Controls
- **WASD** - Move player
- **Left Click** - Manual shooting (outside panel area)
- **TAB** - Toggle control panel
- **ENTER** - Start game / restart after game over

### Node Management (TAB Panel)
- **Left Click + Drag** - Move nodes between inventory and grid
- **Right Click + Drag** - Connect nodes (shows preview line)
- **Hover** - View node tooltips with detailed information
- **Mouse Wheel** - Scroll inventory / zoom grid view

### Node Types

#### **Core Nodes**
- **🟡 CPU Core** - Required for all operations, cannot be removed
- Powers connected nodes and initiates action sequences

#### **STAT Nodes** (Passive Modifiers)
- **🟢 Health Chip** - Increases max health, buffs shield duration
- **🔵 Speed Chip** - Increases movement speed, buffs phase shift
- **🔴 Damage Chip** - Increases bullet damage, buffs fire abilities
- **🟦 FireRate Chip** - Decreases fire cooldown

#### **ACTION Nodes** (Active Abilities)
- **🟠 Fire Blast** - Launches enhanced projectiles
- **🟣 Energy Shield** - Temporary damage immunity
- **🩷 Phase Shift** - Massive speed boost

#### **POWER Nodes** (Modifiers)
- **🟢 Duration Mod** - Modifies connected action durations
- **🟦 Value Mod** - Enhances connected node values

## 🎨 Visual System

### Connection Visualization
- **🟡 Yellow Lines** - CPU to STAT/POWER connections
- **🔵 Sky Blue Lines** - CPU to ACTION connections  
- **🟦 Cyan Lines** - ACTION sequence chains
- **🟣 Purple Lines** - STAT to ACTION buffs
- **🟢 Green Lines** - POWER modifications

### Node States
- **Yellow Border** - Active nodes (powered by CPU)
- **White Border** - Currently executing action
- **Pulsing Effect** - Active action nodes
- **Semi-transparent** - Nodes being dragged

## 📁 Project Structure

```
rrlv1/
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
├── CLAUDE.md              # Development guidance
├── assets/                # Game assets
│   └── test.png
├── build/                 # Build output directory
└── sources/               # Source code
    ├── main.cpp           # Main game loop and integration
    ├── BaseNode.h/.cpp    # Abstract node base class
    ├── NodeTypes.h/.cpp   # Concrete node implementations
    ├── Player.h/.cpp      # Player state and logic
    ├── NodesController.h/.cpp  # Node system management
    ├── ControlPanel.h/.cpp     # Node UI management
    └── HUD.h/.cpp         # Game UI display
```

## 🔄 Development History

This project evolved from a monolithic `main.cpp` implementation to a fully object-oriented architecture through comprehensive refactoring:

### Refactoring Achievements
✅ **Extracted Node System** - From structs to polymorphic classes  
✅ **Separated UI Logic** - ControlPanel and HUD classes  
✅ **Isolated Game Logic** - NodesController for system management  
✅ **Encapsulated Player State** - Clean Player class interface  
✅ **Maintained Feature Parity** - 100% original functionality preserved  
✅ **Implemented All Features** - Removed all stub implementations  

### Technical Improvements
- **Polymorphism** - Virtual methods for node type handling
- **Encapsulation** - Proper data hiding and interface design
- **Separation of Concerns** - Each class has single responsibility
- **Memory Management** - Smart pointers for safe node handling
- **Type Safety** - Strong typing for node connections

## 🚀 Current Status

**COMPLETE**: The project is fully functional with all features implemented:
- ✅ Node creation and inventory management
- ✅ Drag and drop interface
- ✅ Node connection system with visual feedback
- ✅ Action sequence execution
- ✅ Stat modification system
- ✅ Enemy AI and combat
- ✅ Progressive difficulty
- ✅ Complete UI system with tooltips
- ✅ Clean OOP architecture

The game maintains the original's unique blend of action gameplay and programming concepts while providing a solid foundation for future enhancements.

## 🎮 Version Information

- **Current Version**: v0.0.3
- **Engine**: raylib 5.6-dev
- **Language**: C++17
- **Architecture**: Object-Oriented Programming
- **Platform**: Cross-platform (tested on macOS)