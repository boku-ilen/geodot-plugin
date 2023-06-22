# geodot-plugin

<img align="right" width="256" height="256" src="https://github.com/boku-ilen/geodot-plugin/blob/master/demo/icon.png">

A Godot plugin for loading geospatial data. Written as a GDExtension with C++ and GDAL.

Various types of Raster and Vector data can be loaded from georeferenced files and used in Godot. One might load a heightmap from a GeoTIFF, building footprints from a Shapefile, or an entire database of data from a GeoPackage. Thanks to efficient filtering and cropping, data can be loaded in real-time.

Automated builds of the addon with an included demo can be found [in the GitHub Actions tab](https://github.com/boku-ilen/geodot-plugin/actions). The `master` branch is a GDExtension plugin for Godot 4.x. For Godot 3.x with GDNative, use the `godot3` branch and its builds.

Documentation generated via Doxygen can be found here: https://boku-ilen.github.io/geodot-plugin/

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

__If you want to add the Geodot addon to your own game__, copy the `addons/geodot` directory to your project.

__You will need some sort of geodata to use this plugin.__ The bundled demo scenes include small snippets of example data such as a raster heightmap and a vector street dataset.

# Building

For building this project we need to install [Scons](https://scons.org) by following one of the [User Guide](https://scons.org/documentation.html) *Installing Scons* which is both used for Godot GDExtension and this project and we need the GDAL library which has different installation instruction listed below.

- [Linux](#building-on-linux)
- [Windows](#building-on-windows)
- [MacOS](#building-on-macos)

## Building on Linux

### Preparation

1. Install GDAL. (With apt: `sudo apt install libgdal-dev`)
2. Initialize all git submodules: `git submodule update --init --recursive`
3. Generate the GDExtension C++ bindings: `cd godot-cpp; scons platform=linux generate_bindings=yes`

### Compiling

Everything is built via SConstruct. Running `scons platform=linux` in the root directory will compile everything: First the processing libraries, then the finished GDExtension plugin.

If you're working on a processing library, you can also only compile this library with the same `scons platform=linux` command in that library's directory (e.g. in `src/raster-tile-extractor`).

### Packaging

For building a self-contained plugin for use in other projects, it is recommended to move `libgdal` into the same directory as `libgeodot` and the other libraries (`demo/addons/geodot/x11/`).


## Building on Windows

Because of very inconsistent results with native Visual Studio compilation, we use Docker to create Windows builds from a Linux environment using MinGW. This means that Windows builds can also be created on a Linux machine. We recommend the following process on Linux or on WSL:

### Preparation

1. Install Docker
2. Initialize all git submodules: `git submodule update --init --recursive`
3. Generate the GDExtension C++ bindings: `cd godot-cpp; scons platform=windows generate_bindings=yes`
4. Create the build container: `docker build -f DockerfileMinGW -t gdal-mingw .`


### Compiling

Build Geodot on the container: `docker run --name mingw-builder gdal-mingw:latest`

### Packaging

Copy the resulting Geodot library: `docker cp mingw-builder:/geodot/demo/addons/geodot/win64 ./demo/addons/geodot`

Copy the DLLs for GDAL and all dependencies: `docker cp mingw-builder:/usr/x86_64-w64-mingw32/sys-root/mingw/bin ./tmp` and `cp ./tmp/*.dll ./demo/addons/geodot/win64/`

## Building on MacOS

### Notes

- The current implementation only has support for x86 and **not for** the new M1 processor.
- VectorExtractor is causing the game to crash. See #51
- You have to move some generated files. See #52

### Preparation

1. Install using `brew install gdal` or manually https://github.com/OSGeo/gdal.
  1. Check installation with ie `gdalinfo --formats`
  1. Use `brew info gdal | grep '/usr/local' | cut -d ' ' -f 1` or `which gdalinfo` to find the GDAL install location.
  1. Make a note of this path like `/usr/local/Cellar/gdal/3.2.1` and use that below for `osgeo_path=...`
1. Initialize all git submodules: `git submodule update --init --recursive`
1. Generate the GDExtension C++ bindings:
```
cd godot-cpp
scons generate_bindings=yes target=release --jobs=$(sysctl -n hw.logicalcpu)
cd ..
```

### Compiling

1. Compile the project itself using the GDAL location like `scons target=release osgeo_path=/usr/local/Cellar/gdal/3.2.1`
  1. This generates the file `libgeodot.dylib` and 2 more located in `demo/addons/geodot/macos`
1. Move the files `libRasterTileExtractor.dylib` and `libVectorExtractor.dylib` from `demo/addons/geodot/macos` into `demo/build` directory. See #52
  1. `mkdir build ; mv demo/addons/geodot/macos/libRasterTileExtractor.dylib demo/addons/geodot/macos/libVectorExtractor.dylib demo/build`

# Developing

We recommend VSCodium for developing. For on-the-fly code completion, error reporting, etc., install the `clangd` extension in VSCodium and run `scons compiledb=yes` in order to generate the required compilation database.

# Project structure

## Demo

A minimal Godot project which the plugin is compiled into by default. Used to test its functionality.

## godot-cpp

Git submodule with everything that's required for building a GDExtension plugin with C++.

## Geodot

`geodot.h`, `geodata.h`, etc. - the GDExtension C++ code for the actual plugin. It mainly acts as an interface from Godot-compatible types to custom libraries and classes - it is kept as simple as possible so that adapting it to changes in Godot is easy.

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

The RasterDemo ships with a small sample of [Viennese test data](https://data.wien.gv.at/) (CC BY 4.0); the VectorDemo uses a sample of edges from the [GIP dataset](http://www.gip.gv.at/#ogd) (CC BY 4.0).
