# Atlas

## Overview

Atlas is a minimalist engine designed with a focus on programming enjoyment and rapid iteration. It provides a foundational structure with a few essential features.

## Naming Scheme

The naming scheme for Atlas is inspired by Greek mythology, organized in a hierarchical structure. While you are free to name your game projects as you wish, I choose to give them codenames as working titles from demi-gods. For example, the Daedalus project is a simple Space Invaders game used to test the engine and drive its development.

### Example Naming View:

- **Engine**
  - Atlas: Core engine
- **Modules (Gods)**
  - Hermes: Networking layer
  - Hephaestus: Resource management
  - [Other modules as needed]
- **Game Projects (Demi-Gods)**
  - Daedalus: Space Invaders trial run project
  - [Future game projects]

## Features

Atlas includes:
- **Entity Component System (ECS)**: Using `entt` for efficient entity management.
- **Rendering and Window Management**: Powered by `raylib`.
- **Build System**: Utilizes `CMake` for easy configuration and build management.
- **Package Management**: Managed with `Conan` to handle dependencies.
- **Static Analysis**: Employs `clang static analyzer` (`scan-build`) for code quality.
- **Unit Testing**: Integrated with `Google Tests` for robust testing.

## Usage

To use Atlas for a project:

1. **Create a new repository** for the project.
2. **Add Atlas as a submodule**.
3. **Configure your project** and choose to include any optional Atlas modules via CMake. Unused modules will not be compiled or included in the game project.
