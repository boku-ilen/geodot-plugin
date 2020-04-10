# geodot-plugin

<img align="right" width="256" height="256" src="https://github.com/boku-ilen/geodot-plugin/blob/master/demo/icon.png">

A Godot plugin for loading geospatial data. Written in GDNative with C++.

## Functional features

Images and ImageTextures can be created from geodata anywhere in the file system. Georeferenced images (such as GeoTIFFs) and pre-tiled pyramids (according to the OSM standard) are supported. The desired area is extracted using geocoordinates and a size.

Vector data with LineString features such as OSM street data is supported. Line geometry near a given position, within a given radius, can be extracted from the dataset as a Curve3D. Attributes of the feature can also be retrieved.

## Usage

The demo Godot project offers a RasterDemo and a VectorDemo. I recommend looking through that code to get started.

If you want to add the geodot addon to your game, copy the `addons/geodot` directory and add `addons/geodot/geodot.gdns` as a Singleton. Then, you can get GeoImages like this:

```gdscript
var geoimg = Geodot.get_image(
	path_to_file,
	file_ending,
	position_m_x,
	position_m_y,
	size_meters,
	size_pixels,
	interpolation
)
```

An Array of LineFeatures can be retreived like this:

```gdscript
var lines = Geodot.get_lines(
	path_to_file,
	position_m_x,
	position_m_y,
	radius_meters,
	max_lines
)
```

[Until we have a proper documentation](https://github.com/boku-ilen/geodot-plugin/issues/9), check the source (`geodot.h`) for additional functions and details.

__You will need some sort of geodata to use this plugin.__ A good starting point is a GeoTIFF with a heightmap. If you need such a file for testing, you can get a heightmap for Austria [at data.gv.at](https://www.data.gv.at/katalog/dataset/b5de6975-417b-4320-afdb-eb2a9e2a1dbf) (licensed under CC-BY-4.0).

# Building

## Building on Linux

### Preparation

1. Install GDAL. (With apt: `sudo apt install libgdal-dev`)
2. Initialize all git submodules: `git submodule update --init --recursive`
3. Generate the GDNative C++ bindings: `scons platform=linux generate_bindings=yes` (in `godot-cpp`)

### Compiling

Everything is built via SConstruct. Running `scons platform=linux` in the root directory will compile everything: First the processing libraries, then the finished GDNative plugin.

If you're working on a processing library, you can also only compile this library with the same `scons platform=linux` command in that library's directory (e.g. in `src/raster-tile-extractor`).

### Packaging

For building a self-contained plugin for use in other projects, it is recommended to move `libgdal` into the same directory as `libgeodot` and the other libraries (`demo/addons/geodot/x11/`).


## Building on Windows

### Preparation

1. Install the newest Visual Studio
2. Install [Scons](https://scons.org/pages/download.html) for building
3. Install [OSGeo](http://download.osgeo.org/osgeo4w/osgeo4w-setup-x86_64.exe) for the GDAL library
4. In the Visual Studio Installer, tab "Individual Components", make sure the following are installed:
    - SQL Server ODBC Driver
    - C++ core features
    - MSVC v142 - VS 2019 C++ x64/x86 build tools
    - MSVC v142 - VS 2019 C++ x64/x86 Spectre-mitigated libs
    - MSBuild
    - C++/CLI support for v142 build tools
5. In `geodot-plugin`, initialize all git submodules: `git submodule update --init --recursive`
6. In `geodot-plugin/godot-cpp`, generate the GDNative C++ bindings: `scons platform=windows generate_bindings=yes bits=64 target=release`

### Compiling

We got the best results with the "x64 Native Tools Command Prompt for VS 2019" command line.

Everything is built via SConstruct. Running `scons platform=windows osgeo_path=C:/path/to/osgeo target=release` in the root directory will compile everything: First the processing libraries, then the finished GDNative plugin. (Building for debug caused crashes for us, which is why `target=release` is recommended on Windows.)

If you're working on a processing library, you can also only compile this library with the same `scons platform=windows osgeo_path=C:/path/to/osgeo` command in that library's directory (e.g. in `src/raster-tile-extractor`).

### Packaging

When using GDAL on Windows, problems with DLLs not being found are pretty frequent. We got the best results by simply copying all DLLs from the OSGeo `bin` directory to `demo/addons/geodot/win64`. Depending on your luck, you may or may not have to do this.

If you still get `Error 126: The specified module could not be found.` when starting the demo project, we recommend checking the Geodot DLL and the GDAL DLL with [Dependencies](https://github.com/lucasg/Dependencies).

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

### VectorExtractor

Another processing library like the RasterTileExtractor. Also uses GDAL internally, but for vector data functionality.

## Contributing
Help is greatly appreciated! You can test and report bugs, fix known [issues](https://github.com/boku-ilen/geodot-plugin/issues) or submit new functionality.

### Adding a new feature

- Actual processing code and external library calls should be put into a separate project (a processing library) which compiles to a library and is independent from Godot.
- Geodot should only translate Godot calls and types to calls to this custom library. Its only dependencies should be Godot and custom libraries.

Summed up, dependencies should only go in this direction:
Godot + Geodot -> Processing Library -> External Libraries
