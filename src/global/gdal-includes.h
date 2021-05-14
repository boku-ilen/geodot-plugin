// Simple header for including various GDAL includes without the need for platform-checks.

#ifdef _ARCH
#include <cpl_error.h>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#elif _WIN32
#include <cpl_error.h>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#elif __APPLE__
#include <cpl_error.h>
#include <cpl_port.h>
#include <gdal.h>
#include <gdal_priv.h>
#include <gdalwarper.h>
#include <ogr_feature.h>
#include <ogrsf_frmts.h>
#elif __unix__
#include <gdal/cpl_error.h>
#include <gdal/gdal_priv.h>
#include <gdal/ogrsf_frmts.h>
#endif