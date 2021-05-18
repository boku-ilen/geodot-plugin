#!python
import os
import subprocess
import sys
from shutil import copyfile

opts = Variables([], ARGUMENTS)

# Gets the standard flags CC, CCX, etc.
try:
    env = DefaultEnvironment(tools=['default', 'compilation_db'])
except:
    print("Compilation Database creation is not supported. Please consider upgrading to SCons 4.x.x!")
    env = DefaultEnvironment()

# Local dependency paths, adapt them to your setup
godot_headers_path = "godot-cpp/godot_headers/"
cpp_bindings_path = "godot-cpp/"
cpp_library = "libgodot-cpp"

rte_cpp_path = "src/raster-tile-extractor/"
rte_libpath = "src/raster-tile-extractor/build/"
rte_library = "libRasterTileExtractor"

vector_cpp_path = "src/vector-extractor/"
vector_libpath = "src/vector-extractor/build/"
vector_library = "libVectorExtractor"

demo_path = "demo/addons/geodot/"

# Try to detect the host platform automatically.
# This is used if no `platform` argument is passed
if sys.platform.startswith('linux'):
    host_platform = 'linux'
elif sys.platform == 'darwin':
    host_platform = 'osx'
elif sys.platform == 'win32' or sys.platform == 'msys':
    host_platform = 'windows'
else:
    host_platform = "Unknown platform: " + sys.platform

# Define our options
opts.Add(EnumVariable('target', "Compilation target", 'debug', ['d', 'debug', 'r', 'release']))
opts.Add(EnumVariable('platform', 'Compilation platform', host_platform,
                      allowed_values=('linux', 'osx', 'windows'), ignorecase=2))
opts.Add(BoolVariable('use_llvm', "Use the LLVM / Clang compiler", 'no'))
opts.Add(BoolVariable('compiledb', "Build a Compilation Database, e.g. for live error reporting in VSCodium", 'no'))
opts.Add(PathVariable('target_path', 'The path where the lib is installed.', demo_path))
opts.Add(PathVariable('target_name', 'The library name.', 'libgeodot', PathVariable.PathAccept))
opts.Add(PathVariable('osgeo_path', "(Windows only) path to OSGeo installation", "", PathVariable.PathAccept))

# only support 64 at this time..
bits = 64

# Updates the environment with the option variables.
opts.Update(env)

# Process some arguments
if env['use_llvm']:
    env['CC'] = 'clang'
    env['CXX'] = 'clang++'

if env['compiledb']:
    env.CompilationDatabase()

if env['platform'] == '':
    print("No valid target platform selected.")
    quit()


# Build the extractor libraries
subprocess.call(
    "cd " + rte_cpp_path + " && scons platform=" + env['platform'] + " osgeo_path=" + env['osgeo_path'],
    shell=True)
subprocess.call(
    "cd " + vector_cpp_path + " && scons platform=" + env['platform'] + " osgeo_path=" + env['osgeo_path'],
    shell=True)

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

env.Append(CXXFLAGS=['-std=c++17'])

# Check our platform specifics
if env['platform'] == "osx":
    env['target_path'] += 'osx/'
    cpp_library += '.osx'
    gdal_lib_name = 'gdal'

    env.Append(LINKFLAGS=['-arch', 'x86_64'])

    if env['target'] in ('debug', 'd'):
        env.Append(CCFLAGS=['-g', '-O2', '-arch', 'x86_64'])
    else:
        env.Append(CCFLAGS=['-g', '-O3', '-arch', 'x86_64'])

    env.Append(LIBS=['libgdal.dylib'])
    env.Append(LIBPATH=[os.path.join(env['osgeo_path'], "lib")])

elif env['platform'] in ('x11', 'linux'):
    env['target_path'] += 'x11/'
    cpp_library += '.linux'
    gdal_lib_name = 'gdal'

    env.Append(LINKFLAGS=[
        '-Wl,--disable-new-dtags,-rpath,\'$$ORIGIN\''
    ])

    if env['target'] in ('debug', 'd'):
        env.Append(CCFLAGS=['-fPIC', '-g3', '-Og'])
    else:
        env.Append(CCFLAGS=['-fPIC', '-g', '-O3'])
    
    # Arch needs different includes!
    import distro
    if distro.like() == "arch":
        env.Append(CPPDEFINES=["_ARCH"])

elif env['platform'] == "windows":
    env['target_path'] += 'win64/'
    env['target_name'] += ".dll"
    env.Append(LINKFLAGS=['-static-libgcc', '-static-libstdc++', '-static'])
    cpp_library += '.windows'
    gdal_lib_name = 'gdal.dll'

    # Set the compiler to MinGW (is this command valid on native Windows too?)
    env.Replace(CXX=['x86_64-w64-mingw32-g++'])

    gdal_include_path = os.path.join(env['osgeo_path'], "include")
    env.Append(CPPPATH=[gdal_include_path])


if env['target'] in ('debug', 'd'):
    cpp_library += '.debug'
else:
    cpp_library += '.release'

cpp_library += '.' + str(bits)

# make sure our binding library is properly includes
env.Append(CPPPATH=['.', godot_headers_path, cpp_bindings_path + 'include/',
                    cpp_bindings_path + 'include/core/', cpp_bindings_path + 'include/gen/', rte_cpp_path, vector_cpp_path])
env.Append(LIBPATH=[cpp_bindings_path + 'bin/', rte_libpath, vector_libpath, os.path.join(env['osgeo_path'], "lib")])
env.Append(LIBS=[cpp_library, rte_library, vector_library, gdal_lib_name])

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=['src/'])
env.Append(CPPPATH=['src/global/'])
sources = Glob('src/*.cpp')

library = env.SharedLibrary(target=env['target_path'] + env['target_name'], source=sources)

# Generates help for the -h scons option.
Help(opts.GenerateHelpText(env))
