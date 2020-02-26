# geodot-plugin

A Godot plugin for loading geospatial data. Written in GDNative with C++.

__Work-in-progress - not functional yet__

## Building

0. Compile the RasterTileExtractor library to `/usr/lib`
1. Generate the bindings: Run `scons platform=<platform> generate_bindings=yes` in `godot-cpp`
2. Compile the plugin into the demo project: Run `scons platform=<platform>` in the root directory

`<platform>` must be replaced with your platform, e.g. `linux`.
