#include <iostream>
#include "RasterTileExtractor.h"

RasterTileExtractor::RasterTileExtractor() {
    // Register all drivers - without this, GDALGetDriverByName doesn't work
    GDALAllRegister();
}

void RasterTileExtractor::reproject_to_webmercator(const char *infile, const char *outfile) {
    bool save_to_disk = false;

    GDALDriverH hDriver;
    GDALDataType eDT;
    GDALDatasetH hDstDS;
    GDALDatasetH hSrcDS;

    // Open the source file.
    hSrcDS = GDALOpen(infile, GA_ReadOnly);
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

void
RasterTileExtractor::clip(const char *infile, const char *outfile, double top_left_x, double top_left_y,
                          double size_meters, int img_size, float *result_target) {
    GDALDatasetH source, dest;
    GDALDriverH pDriver;

    if (false) {
        pDriver = GDALGetDriverByName("GTiff");
    } else {
        pDriver = GDALGetDriverByName("MEM");
    }

    source = GDALOpen(infile, GA_ReadOnly);

    // Get the current Transform of the source image
    double transform[6];
    GDALGetGeoTransform(source, transform);

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
    dest = GDALCreate(pDriver, outfile, img_size, img_size,
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

    // If we are going beyond the available resolution, use bilinear scaling
    if (new_pixel_size < previous_pixel_size) {
        psWarpOptions->eResampleAlg = GRA_Bilinear;
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

    GDALRasterBandH hBand = GDALGetRasterBand(dest, 1);

    float *pafScanline;
    int   nXSize = GDALGetRasterBandXSize( hBand );
    size_t img_mem_size = sizeof(float) * nXSize * nXSize;

    pafScanline = (float *) CPLMalloc(img_mem_size);

    GDALRasterIO( hBand, GF_Read, 0, 0, nXSize, nXSize,
                  pafScanline, nXSize, nXSize, GDT_Float32,
                  0, 0 );

    // Freeing 'result_target' is up to the caller.
    memcpy(result_target, pafScanline, img_mem_size);

    // Since we copied pafScanline into the caller's result_target. pafScanline can be freed.
    CPLFree(pafScanline);

    // Cleanup
    GDALClose(dest);
    GDALClose(source);
}
