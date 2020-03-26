#include <iostream>
#include "RasterTileExtractor.h"
#include <gdal/gdal_priv.h>
#include <gdal/gdalwarper.h>
#include <filesystem>

void RasterTileExtractor::initialize() {
    // Register all drivers - without this, GDALGetDriverByName doesn't work
    GDALAllRegister();
}

void RasterTileExtractor::reproject_to_webmercator(const char *base_path, const char *outfile) {
    GDALDriverH hDriver;
    GDALDataType eDT;
    GDALDatasetH hDstDS;
    GDALDatasetH hSrcDS;

    // Open the source file.
    hSrcDS = GDALOpen(base_path, GA_ReadOnly);
    CPLAssert(hSrcDS != NULL)

    // Create output with same datatype as first input band.
    eDT = GDALGetRasterDataType(GDALGetRasterBand(hSrcDS, 1));


    hDriver = GDALGetDriverByName("GTiff");
    CPLAssert(hDriver != NULL)

    // Get Source coordinate system.
    const char *pszSrcWKT, *pszDstWKT = nullptr;
    pszSrcWKT = GDALGetProjectionRef(hSrcDS);
    CPLAssert(pszSrcWKT != NULL && strlen(pszSrcWKT) > 0)

    // Get Webmercator coordinate system
    OGRSpatialReference oSRS;
    oSRS.importFromEPSG(3857);
    oSRS.exportToWkt(const_cast<char **>(&pszDstWKT));

    // Create a transformer that maps from source pixel/line coordinates
    // to destination georeferenced coordinates (not destination
    // pixel line).  We do that by omitting the destination dataset
    // handle (setting it to NULL).
    void *hTransformArg;
    hTransformArg =
            GDALCreateGenImgProjTransformer(hSrcDS, pszSrcWKT, nullptr, pszDstWKT,
                                            FALSE, 0, 1);
    CPLAssert(hTransformArg != NULL)

    // Get approximate output georeferenced bounds and resolution for file.
    double adfDstGeoTransform[6];
    int nPixels = 0, nLines = 0;
    CPLErr eErr;
    eErr = GDALSuggestedWarpOutput(hSrcDS,
                                   GDALGenImgProjTransform, hTransformArg,
                                   adfDstGeoTransform, &nPixels, &nLines);
    CPLAssert(eErr == CE_None)

    GDALDestroyGenImgProjTransformer(hTransformArg);

    // Create the output file.
    hDstDS = GDALCreate(hDriver, outfile, nPixels, nLines,
                        GDALGetRasterCount(hSrcDS), eDT, nullptr);
    CPLAssert(hDstDS != NULL)

    // Write out the projection definition.
    GDALSetProjection(hDstDS, pszDstWKT);
    GDALSetGeoTransform(hDstDS, adfDstGeoTransform);

    // Copy the color table, if required.
    // TODO: Only supports one raster band - are more required?
    GDALColorTableH hCT;
    hCT = GDALGetRasterColorTable(GDALGetRasterBand(hSrcDS, 1));
    if (hCT != nullptr)
        GDALSetRasterColorTable(GDALGetRasterBand(hDstDS, 1), hCT);

    int band_count = GDALGetRasterCount(hSrcDS);

    // Setup warp options.
    GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();
    psWarpOptions->hSrcDS = hSrcDS;
    psWarpOptions->hDstDS = hDstDS;
    psWarpOptions->nBandCount = band_count;
    psWarpOptions->panSrcBands =
            (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount);
    psWarpOptions->panSrcBands[0] = band_count;
    psWarpOptions->panDstBands =
            (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount);
    psWarpOptions->panDstBands[0] = band_count;
    psWarpOptions->pfnProgress = GDALTermProgress;

    // Establish reprojection transformer.
    psWarpOptions->pTransformerArg =
            GDALCreateGenImgProjTransformer(hSrcDS,
                                            GDALGetProjectionRef(hSrcDS),
                                            hDstDS,
                                            GDALGetProjectionRef(hDstDS),
                                            FALSE, 0.0, 1);
    psWarpOptions->pfnTransformer = GDALGenImgProjTransform;

    // Initialize and execute the warp operation.
    GDALWarpOperation oOperation;
    oOperation.Initialize(psWarpOptions);
    oOperation.ChunkAndWarpImage(0, 0,
                                 GDALGetRasterXSize(hDstDS),
                                 GDALGetRasterYSize(hDstDS));
    GDALDestroyGenImgProjTransformer(psWarpOptions->pTransformerArg);
    GDALDestroyWarpOptions(psWarpOptions);

    GDALClose(hDstDS);
    GDALClose(hSrcDS);
}

GDALDataset
*RasterTileExtractor::clip(const char *base_path, double top_left_x, double top_left_y, double size_meters, int img_size,
                          int interpolation_type) {
    GDALDataset *source, *dest;
    GDALDriverH pDriver;

    // TODO: Add the option of saving to a GTiff for caching
    if (false) {
        pDriver = GDALGetDriverByName("GTiff");
    } else {
        pDriver = GDALGetDriverByName("MEM");
    }

    source = (GDALDataset *) GDALOpen(base_path, GA_ReadOnly);
    GDALDataType datatype = source->GetRasterBand(1)->GetRasterDataType();

    int band_count = source->GetRasterCount();

    // Get the current Transform of the source image
    double transform[6];
    source->GetGeoTransform(transform);

    // Adjust the top left coordinates according to the input variables
    transform[0] = top_left_x;
    transform[3] = top_left_y;

    // We want to fit an image of the given size (in meters) into our img_size (in pixels)
    double new_pixel_size = size_meters / img_size;
    double previous_pixel_size = transform[1];

    // Adjust the pixel size
    transform[1] = new_pixel_size;
    transform[5] = -new_pixel_size;

    // Create a new geoimage at the given path with our img_size
    // The outfile path is empty since it's only in RAM
    dest = (GDALDataset *) GDALCreate(pDriver, "", img_size, img_size, band_count, datatype, nullptr);

    // Get Source coordinate system.
    const char *pszDstWKT = nullptr;

    // Get Webmercator coordinate system
    OGRSpatialReference oSRS;
    oSRS.importFromEPSG(3857);
    oSRS.exportToWkt(const_cast<char **>(&pszDstWKT));

    // Apply Webmercator and our previously built Transform to the destination file
    dest->SetProjection(pszDstWKT);
    dest->SetGeoTransform(transform);

    // Copy the color table, if required.
    // TODO: Only supports one raster band - are more required?
    GDALColorTable *hCT = source->GetRasterBand(1)->GetColorTable();
    if (hCT != nullptr) {
        dest->GetRasterBand(1)->SetColorTable(hCT);
    }

    // Get the band names to pass to the warp options
    // We need identical arrays for the source and destination band names because GDAL does a double free otherwise
    int *src_bands = new int[band_count];
    int *dst_bands = new int[band_count];
    int i = 0;
    for (auto *band : source->GetBands()) {
        src_bands[i] = band->GetBand();
        dst_bands[i] = band->GetBand();
        i++;
    }

    // Warp the data from the input file into our new destination with the new Transform and size
    // Setup warp options
    GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();
    psWarpOptions->hSrcDS = source;
    psWarpOptions->hDstDS = dest;
    psWarpOptions->nBandCount = band_count;
    psWarpOptions->panSrcBands = src_bands;
    psWarpOptions->panDstBands = dst_bands;
    psWarpOptions->pfnProgress = GDALTermProgress;

    // If we are going beyond the available resolution, use bilinear scaling
    if (new_pixel_size < previous_pixel_size) {
        psWarpOptions->eResampleAlg = static_cast<GDALResampleAlg>(interpolation_type);
    } else {
        psWarpOptions->eResampleAlg = GRA_NearestNeighbour;
    }

    // Establish reprojection transformer.
    psWarpOptions->pTransformerArg =
            GDALCreateGenImgProjTransformer(source,
                                            GDALGetProjectionRef(source),
                                            dest,
                                            GDALGetProjectionRef(dest),
                                            FALSE, 0.0, 1);
    psWarpOptions->pfnTransformer = GDALGenImgProjTransform;

    // Initialize and execute the warp operation.
    GDALWarpOperation oOperation;
    oOperation.Initialize(psWarpOptions);
    oOperation.ChunkAndWarpImage(0, 0, img_size, img_size);
    GDALDestroyGenImgProjTransformer(psWarpOptions->pTransformerArg);

    GDALDestroyWarpOptions(psWarpOptions);

    GDALClose(source);

    return dest;
}

#define PYRAMID_DIRECTORY_ENDING "pyramid"

GeoRaster *RasterTileExtractor::get_raster_at_position(const char *base_path, const char *file_ending, double top_left_x, double top_left_y,
                                                      double size_meters, int img_size, int interpolation_type) {
    // First, check if we have a pre-tiled pyramid of this data
    std::string pyramid_name_string = std::string(base_path) + "." + std::string(PYRAMID_DIRECTORY_ENDING);

    if (std::filesystem::exists(pyramid_name_string)) {
        // We have a pre-tiled pyramid
        GDALDataset *dataset_from_pyramid = get_from_pyramid(pyramid_name_string.c_str(), file_ending, top_left_x, top_left_y, size_meters, img_size, interpolation_type);

        if (dataset_from_pyramid != nullptr) {
            return new GeoRaster(dataset_from_pyramid);
        }
    } else {
        // Check if there is a single image with the given path
        std::string raster_path_string = std::string(base_path) + "." + std::string(file_ending);

        if (std::filesystem::exists(raster_path_string)) {
            return new GeoRaster(clip(raster_path_string.c_str(), top_left_x, top_left_y, size_meters, img_size, interpolation_type));
        }
    }

    // If there was neither a single file nor a pyramid, return null
    return nullptr;
}

#define WEBMERCATOR_MAX 20037508.0
#define PI 3.14159265358979323846
#define CIRCUMEFERENCE 40075016.686

class path;

GDALDataset *
RasterTileExtractor::get_from_pyramid(const char *base_path, const char *file_ending, double top_left_x, double top_left_y, double size_meters,
                                      int img_size, int interpolation_type) {
    // Norm webmercator position (in meters) to value between -1 and 1
    double norm_x = 0.5 + ((top_left_x + size_meters / 2.0) / WEBMERCATOR_MAX) * 0.5;

    double norm_y = 1.0 - (0.5 + ((top_left_y - size_meters / 2.0) / WEBMERCATOR_MAX) * 0.5);

    // Get latitude and use it to calculate the zoom level here
    double latitude = 0.81777; // TODO: Finding the actual latitude requires a more complex calculation due to projection
    // Original formula: size = C * cos(latitude) / pow(2, zoom_level) (from https://wiki.openstreetmap.org/wiki/Zoom_levels)
    int zoom_level = (int)round(log2(CIRCUMEFERENCE * cos(latitude) / size_meters)) + 1;

    // Number of tiles at this zoom level
    int num_tiles = pow(2.0, zoom_level);

    // Tile coordinates in OSM pyramid
    int tile_x = (int)floor(norm_x * num_tiles);
    int tile_y = (int)floor(norm_y * num_tiles);

    // Build the complete path
    std::filesystem::path path = std::filesystem::path(base_path);
    path /= std::filesystem::path(std::to_string(zoom_level));
    path /= std::filesystem::path(std::to_string(tile_x));
    path /= std::filesystem::path(std::to_string(tile_y));
    path += ".";
    path += file_ending;

    // Load the result as a GDALDataset and return it
    return (GDALDataset *) GDALOpen(path.c_str(), GA_ReadOnly);
}
