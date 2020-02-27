# geodot-plugin

A Godot plugin for loading geospatial data. Written in GDNative with C++.

__Work-in-progress - not functional yet__

## Building

### Dependencies

0. Install GDAL. (With apt: `sudo apt install libgdal-dev`)
1. Compile the RasterTileExtractor library.
2. Generate the bindings: Run `scons platform=<platform> generate_bindings=yes` in `godot-cpp`

### Plugin
To compile the plugin into the demo project: Run `scons platform=<platform>` in the root directory. (`<platform>` must be replaced with your platform, e.g. `linux`.)

The demo project will now use the new plugin when running it.
