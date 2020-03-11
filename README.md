# geodot-plugin

A Godot plugin for loading geospatial data. Written in GDNative with C++.

## Functional features

Images and ImageTextures can be created from geodata anywhere in the file system. Georeferenced images (such as GeoTIFFs) and pre-tiled pyramids (according to the OSM standard) are supported. The desired area is extracted using geocoordinates and a size.

## Building

### Preparation

1. Install GDAL. (With apt: `sudo apt install libgdal-dev`)
2. Initialize all git submodules: `git submodule update --init --recursive`
3. Generate the GDNative C++ bindings: `scons platform=<platform> generate_bindings=yes` (in `godot-cpp`)


### RasterTileExtractor

The RasterTileExtractor is a separate CMake project. CLion is recommended for development; for building, CMake is enough.

### Geodot-Plugin

Run `scons platform=<platform>` in the root directory. The compiled plugin and the RasterTileExtractor library dependency will be placed in the demo project. (`<platform>` must be replaced with your platform, e.g. `linux`.)

### Packaging

For building a self-contained plugin for use in other projects, it is recommended to move `libgdal` into the same directory as `libgeodot` and `libRasterTileExtractor` (`demo/addons/geodot/<platform>/`)

Note: This is currently only possible on Linux because rpath is used. [Help is needed for building and packaging on Windows!](https://github.com/boku-ilen/geodot-plugin/issues/1)

## Project structure

### Demo

A minimal Godot project which the plugin is compiled into by default. Used to test its functionality.

### godot-cpp

Git submodule with everything that's required for building a GDNative plugin with C++.

### Geodot

`geodot.h` and `geodot.cpp` - the GDNative C++ code for the actual plugin. It mainly acts as an interface from Godot-compatible types to custom libraries - it is kept as simple as possible so that adapting it to changes in Godot is easy.

### RasterTileExtractor

Processing library called by Geodot. Mainly wraps GDAL functionality so that Geodot does not need to depend on GDAL. Responsible for the actual image extraction, warping, etc.

The reason why Geodot doesn't directly call GDAL functions without this additional layer of abstraction is ease of compilation and extendibility: The libraries can remain the same, only the core Geodot must be kept consistent with Godot.

## Contributing

Help is greatly appreciated! You can check out known [issues](https://github.com/boku-ilen/geodot-plugin/issues) or submit new functionality.

### Adding a new feature

- Actual processing code and external library calls should be put into a separate project (a processing library) which compiles to a library and is independent from Godot.
- Geodot should only translate Godot calls and types to calls to this custom library. Its only dependencies should be Godot and custom libraries.

Summed up, dependencies should only go in this direction:
Godot + Geodot -> Processing Library -> External Libraries
