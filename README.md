# geodot-plugin

<img align="right" width="256" height="256" src="https://github.com/boku-ilen/geodot-plugin/blob/master/demo/icon.png">

A Godot plugin for loading geospatial data. Written in GDNative with C++ and GDAL.

Various types of Raster and Vector data can be loaded from georeferenced files and used in Godot. One might load a heightmap from a GeoTIFF, building footprints from a Shapefile, or an entire database of data from a GeoPackage. Thanks to efficient filtering and cropping, data can be loaded in real-time.

Feel free to join our Discord for talking about geospatial Godot: https://discord.gg/MhB5sG7czF

## Usage

The included Godot project offers various demo scenes - we recommend looking through that code to get started. Just do give you an idea, this is how you would load a heightmap at a given position and size:

```gdscript
var heightmap_data = Geodot.get_raster_layer("/path/to/data.tif")

var img = heightmap_data.get_image(
	pos_x,
	pos_y,
	size_meters,
	resolution,
	1
)

var texture = img.get_image_texture()
```

An Array of Vector features near a given position can be loaded like this:

```gdscript
var dataset = Geodot.get_dataset("/path/to/data.gpkg")
var layer = dataset.get_feature_layer("layername")
	
var features = layer.get_features_near_position(pos_x, pos_y, radius, max_features)
```

__If you want to add the Geodot addon to your own game__, copy the `addons/geodot` directory and add `addons/geodot/geodot.gdns` as a Singleton.

[Until we have proper external documentation](https://github.com/boku-ilen/geodot-plugin/issues/9), check the examples and source code for additional functions and details. The exposed classes and functions are documented in `geodot.h`, `geodata.h`, `geofeatures.h` and `geoimage.h`.

__You will need some sort of geodata to use this plugin.__ A good starting point is a GeoTIFF with a heightmap. If you need such a file for testing, you can get a heightmap for Austria [at data.gv.at](https://www.data.gv.at/katalog/dataset/b5de6975-417b-4320-afdb-eb2a9e2a1dbf) (licensed under CC-BY-4.0).

# Building

## Building on Linux

### Preparation

1. Install GDAL. (With apt: `sudo apt install libgdal-dev`)
2. Initialize all git submodules: `git submodule update --init --recursive`
3. Generate the GDNative C++ bindings: `cd godot-cpp; scons platform=linux generate_bindings=yes`

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

# Developing

We recommend VSCodium for developing. For on-the-fly code completion, error reporting, etc., install the `clangd` extension in VSCodium and run `scons compiledb=yes` in order to generate the required compilation database.

# Project structure

## Demo

A minimal Godot project which the plugin is compiled into by default. Used to test its functionality.

## godot-cpp

Git submodule with everything that's required for building a GDNative plugin with C++.

## Geodot

`geodot.h`, `geodata.h`, etc. - the GDNative C++ code for the actual plugin. It mainly acts as an interface from Godot-compatible types to custom libraries and classes - it is kept as simple as possible so that adapting it to changes in Godot is easy.

## Processing Libraries

Processing Libraries handle calls to external libraries and convert the results to generic custom classes (independent and unaware of Godot). This way, Godot and external libraries are decoupled, with only one dependency per project.

The reason for this layer of abstraction is ease of compilation and extendibility: The libraries can remain the same, only the core Geodot must be kept consistent with Godot. Similarly, changes in external libraries only affect the corresponding processing library, not the Geodot layer.

### RasterTileExtractor

Processing library called by Geodot. Wraps GDAL's Raster functionality, primarily the extraction of Raster data into an array.

### VectorExtractor

Another processing library like the RasterTileExtractor. Also uses GDAL internally, but for vector data functionality, as well as general database and layer handling.

# Contributing
Help is greatly appreciated! You can test and report bugs, fix known [issues](https://github.com/boku-ilen/geodot-plugin/issues) or submit new functionality.

## Adding a new feature

- Actual processing code and external library calls should be put into a separate project (a processing library) which compiles to a library and is independent from Godot.
- Geodot should only translate Godot calls and types to calls to this custom library. Its only dependencies should be Godot and custom libraries.

Summed up, dependencies should only go in this direction:
Godot + Geodot -> Processing Library -> External Libraries

# Credits

The provided Linux build ships with libgdal.so, a build of the GDAL library. All credits for this library go to [OSGeo/gdal](https://github.com/OSGeo/gdal/) ([license](https://raw.githubusercontent.com/OSGeo/gdal/master/gdal/LICENSE.TXT)).

The RasterDemo ships with a small sample of [Viennese test data](https://data.wien.gv.at/) (CC BY 4.0).
