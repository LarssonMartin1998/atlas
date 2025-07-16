# Atlas

## Overview

**Atlas** is a modular, compile-time-configurable C++23 game engine designed for performance, clarity, and developer ergonomics. It emphasizes clean architecture, predictable behavior, and a Unix-inspired workflow.

Atlas is built for flexibility: everything is opt-in. Systems like ECS, rendering, physics, and networking are exposed as modular components that can be toggled per game project. This allows developers to tailor the engine to fit the exact needs of their game without including unnecessary bloat.

## Naming Scheme

Atlas follows a naming convention inspired by Greek mythology:

* **Atlas** — The core of the engine.
* **Modules** — Named after gods (e.g. `Hephaestus` for ECS, `Iris` for the renderer).
* **Games** — Named after demi-gods (e.g. [`Daedalus`](https://www.github.com/LarssonMartin1998/daedalus.git) is a reference project built on Atlas).

## Features

* **Modular Architecture** — Compile-time selection of modules for clean builds.
* **Hephaestus ECS Module** — High-performance, cache-friendly SoA ECS with multithreaded system execution.
* **Zero Dependency Core** — Modules depend on external libraries, but the core engine stays minimal.
* **Cross-Platform Support** — Linux-first, but designed with Mac & Windows as first class citizens.
* **Modern C++23** — Full usage of modern language features.
* **[Nix](https://nixos.org/download)** — Full Nix Support, providing a reproducible and portable environment.

## Getting Started

Atlas is intended to be included as a **Git submodule** in your own game repository.

```bash
# If using https
git submodule add https://github.com/LarssonMartin1998/atlas.git atlas
# If using ssh
git submodule add git@github.com:LarssonMartin1998/atlas.git atlas

git submodule update --init --recursive --force
```

> Note: You don't need a game repo to develop Atlas, the build instructions also apply to just the engine, but use `make test` instead of `make run`.

---

## Build Instructions

### Nix Users (Recommended)

- **Build your game** (You'll find the output binary in `your_game_repo/result/bin/your_game_name`):

```bash
nix build .
```

#### Nix workflow for development

- **Enter development shell**:

```bash
nix develop
```

This gives you a fully configured environment with the correct compiler and tools. For a full list of dependencies, see [`flake.nix`](./flake.nix). When inside the nix shell, refer to the Non-Nix Users section for working with the game/engine.

**To cross-compile for Windows from Linux using Nix**:
> This uses an example from the Daedalus project. You can refer to the `flake.nix` in that repository as a working example. Change the name from Daedalus to whatever you choose to name your game project.

```bash
nix build .#daedalus-windows
```

This produces a Windows-compatible executable in `./result/bin/daedalus.exe`.

Make sure your flake defines a `packages.daedalus-windows` output using `pkgs.pkgsCross.mingwW64` and a separate `xstdenv.mkDerivation`. 

---

### Non-Nix Users

**⚠️ Important:** Atlas is designed with Nix as the first-class citizen build environment. While we provide alternative build instructions, they may need additional configuration.

For detailed non-Nix build instructions, see **[BUILD.md](./BUILD.md)**.

> 💡 **Recommendation:** Consider using the Nix environment even on non-NixOS systems for the most reliable development experience.
---

## Notes

Atlas is in active development and not production-ready yet. Contributions, feedback, and curiosity are welcome!

Check out [Daedalus](https://github.com/LarssonMartin1998/daedalus.git) to see Atlas in action.

---

## License

Atlas is licensed under the MIT License. See `LICENSE` for details.
