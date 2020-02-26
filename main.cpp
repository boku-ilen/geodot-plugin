#include <iostream>
#include <gdal/gdal_priv.h>
#include <gdal/gdalwarper.h>

// TODO: As a proof-of-concept, this is currently a standalone executable. It should be turned into a library and called
//  from Godot.

/// Reproject the raster file at infile to Webmercator and save the result to outfile.
/// Adapted from https://gdal.org/tutorials/warp_tut.html
void reproject_to_webmercator(const std::string &infile, const std::string &outfile) {
    GDALDriverH hDriver;
    GDALDataType eDT;
    GDALDatasetH hDstDS;
    GDALDatasetH hSrcDS;

    // Open the source file.
    hSrcDS = GDALOpen(infile.c_str(), GA_ReadOnly);
    CPLAssert(hSrcDS != NULL)

    // Create output with same datatype as first input band.
    eDT = GDALGetRasterDataType(GDALGetRasterBand(hSrcDS, 1));

    // Get output driver (GeoTIFF format)
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
    hDstDS = GDALCreate(hDriver, outfile.c_str(), nPixels, nLines,
                        GDALGetRasterCount(hSrcDS), eDT, nullptr);
    CPLAssert(hDstDS != NULL)

    // Write out the projection definition.
    GDALSetProjection(hDstDS, pszDstWKT);
    GDALSetGeoTransform(hDstDS, adfDstGeoTransform);

    // Copy the color table, if required.
    GDALColorTableH hCT;
    hCT = GDALGetRasterColorTable(GDALGetRasterBand(hSrcDS, 1));
    if (hCT != nullptr)
        GDALSetRasterColorTable(GDALGetRasterBand(hDstDS, 1), hCT);

    // Setup warp options.
    GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();
    psWarpOptions->hSrcDS = hSrcDS;
    psWarpOptions->hDstDS = hDstDS;
    psWarpOptions->nBandCount = 1;
    psWarpOptions->panSrcBands =
            (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount);
    psWarpOptions->panSrcBands[0] = 1;
    psWarpOptions->panDstBands =
            (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount);
    psWarpOptions->panDstBands[0] = 1;
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

/// Clip the infile to a 256x256 image starting at top_left_x, top_left_y with a given size (in meters).
void clip(const std::string& infile, const std::string& outfile, double top_left_x, double top_left_y, double size) {
    GDALDatasetH source, dest;
    GDALDriverH pDriver;
    pDriver = GDALGetDriverByName("GTiff");

    source = GDALOpen(infile.c_str(), GA_ReadOnly);

    // Get the current Transform of the source image
    double transform[6];
    GDALGetGeoTransform(source, transform);

    // Adjust the top left coordinates according to the input variables
    transform[0] = top_left_x;
    transform[3] = top_left_y;

    int img_size = 256;

    // We want to fit an image of the given size (in meters) into our img_size (in pixels)
    double new_pixel_size = size / img_size;

    // Adjust the pixel size
    transform[1] = new_pixel_size;
    transform[5] = -new_pixel_size;

    // Create a new geoimage at the given path with our img_size
    dest = GDALCreate(pDriver, outfile.c_str(), img_size, img_size,
                        GDALGetRasterCount(source), GDT_Float32, nullptr);

    // Get Source coordinate system.
    const char *pszDstWKT = nullptr;

    // Get Webmercator coordinate system
    OGRSpatialReference oSRS;
    oSRS.importFromEPSG(3857);
    oSRS.exportToWkt(const_cast<char **>(&pszDstWKT));

    // Apply Webmercator and our previously built Transform to the destination file
    GDALSetProjection(dest, pszDstWKT);
    GDALSetGeoTransform(dest, transform);

    // Copy the color table, if required.
    GDALColorTableH hCT;
    hCT = GDALGetRasterColorTable(GDALGetRasterBand(source, 1));
    if (hCT != nullptr)
        GDALSetRasterColorTable(GDALGetRasterBand(dest, 1), hCT);

    // Warp the data from the input file into our new destination with the new Transform and size
    // Setup warp options
    GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();
    psWarpOptions->hSrcDS = source;
    psWarpOptions->hDstDS = dest;
    psWarpOptions->nBandCount = 1;
    psWarpOptions->panSrcBands =
            (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount);
    psWarpOptions->panSrcBands[0] = 1;
    psWarpOptions->panDstBands =
            (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount);
    psWarpOptions->panDstBands[0] = 1;
    psWarpOptions->pfnProgress = GDALTermProgress;

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

    // Cleanup
    GDALClose(dest);
    GDALClose(source);
}

int main() {
    GDALAllRegister();

    // TODO: Only call this if the input file is not Webmercator already or if we haven't reprojected
    reproject_to_webmercator("data/25m_EU_clip.tif", "data/25m_EU_clip_webm.tif");

    float new_top_left_x = 1470287.0;
    float new_top_left_y = 6013574.0;
    float new_size = 500000.0;

    clip("data/25m_EU_clip_webm.tif", "data/25m_EU_clip_webm_tile.tif", new_top_left_x, new_top_left_y, new_size);

    return 0;
}