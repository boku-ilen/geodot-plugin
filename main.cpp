#include <iostream>
#include <gdal/gdal_priv.h>
#include <gdal/gdalwarper.h>

void reproject_to_webmercator(const std::string &infile, const std::string &outfile) {
    GDALDriverH hDriver;
    GDALDataType eDT;
    GDALDatasetH hDstDS;
    GDALDatasetH hSrcDS;

    // Open the source file.
    hSrcDS = GDALOpen(infile.c_str(), GA_ReadOnly);
    CPLAssert(hSrcDS != NULL);

    // Create output with same datatype as first input band.
    eDT = GDALGetRasterDataType(GDALGetRasterBand(hSrcDS, 1));

    // Get output driver (GeoTIFF format)
    hDriver = GDALGetDriverByName("GTiff");
    CPLAssert(hDriver != NULL);

    // Get Source coordinate system.
    const char *pszSrcWKT, *pszDstWKT = NULL;
    pszSrcWKT = GDALGetProjectionRef(hSrcDS);
    CPLAssert(pszSrcWKT != NULL && strlen(pszSrcWKT) > 0);

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
            GDALCreateGenImgProjTransformer(hSrcDS, pszSrcWKT, NULL, pszDstWKT,
                                            FALSE, 0, 1);
    CPLAssert(hTransformArg != NULL);

    // Get approximate output georeferenced bounds and resolution for file.
    double adfDstGeoTransform[6];
    int nPixels = 0, nLines = 0;
    CPLErr eErr;
    eErr = GDALSuggestedWarpOutput(hSrcDS,
                                   GDALGenImgProjTransform, hTransformArg,
                                   adfDstGeoTransform, &nPixels, &nLines);
    CPLAssert(eErr == CE_None);

    GDALDestroyGenImgProjTransformer(hTransformArg);

    //the affine transformation information, you will need to adjust this to properly
    //display the clipped raster

    // Create the output file.
    hDstDS = GDALCreate(hDriver, outfile.c_str(), nPixels, nLines,
                        GDALGetRasterCount(hSrcDS), eDT, NULL);
    CPLAssert(hDstDS != NULL);

    // Write out the projection definition.
    GDALSetProjection(hDstDS, pszDstWKT);
    GDALSetGeoTransform(hDstDS, adfDstGeoTransform);

    // Copy the color table, if required.
    GDALColorTableH hCT;
    hCT = GDALGetRasterColorTable(GDALGetRasterBand(hSrcDS, 1));
    if (hCT != NULL)
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

void clip(std::string infile, std::string outfile, double top_left_x, double top_left_y, double size) {
    GDALDatasetH source, dest;
    GDALDriverH pDriver;
    pDriver = GDALGetDriverByName("GTiff");

    source = GDALOpen(infile.c_str(), GA_ReadOnly);

    //the affine transformation information, you will need to adjust this to properly
    //display the clipped raster
    double transform[6];
    GDALGetGeoTransform(source, transform);

    std::cout << transform[1] << std::endl;
    std::cout << transform[5] << std::endl;

    //adjust top left coordinates
    transform[0] = top_left_x;
    transform[3] = top_left_y;

    //determine dimensions of the new (cropped) raster in cells
    int img_size = 256;

    double previous_pixel_size = transform[1];

    // TODO: Varying the pixel size changes the resolution!
    //  But increasing the new_size by a factor of 10 also increases the new_pixel_size by a factor of 10.
    double new_pixel_size = size / img_size;

    transform[1] = new_pixel_size;
    transform[5] = -new_pixel_size;

    int map_size = round(size / transform[1]);

    std::cout << map_size << std::endl;

    //create the new (cropped) dataset
    dest = GDALCreate(pDriver, outfile.c_str(), map_size, map_size,
                        GDALGetRasterCount(source), GDT_Float32, NULL);

    // Get Source coordinate system.
    const char *pszDstWKT = NULL;

    // Get Webmercator coordinate system
    OGRSpatialReference oSRS;
    oSRS.importFromEPSG(3857);
    oSRS.exportToWkt(const_cast<char **>(&pszDstWKT));

    GDALSetProjection(dest, pszDstWKT);
    GDALSetGeoTransform(dest, transform);

    // Copy the color table, if required.
    GDALColorTableH hCT;
    hCT = GDALGetRasterColorTable(GDALGetRasterBand(source, 1));
    if (hCT != NULL)
        GDALSetRasterColorTable(GDALGetRasterBand(dest, 1), hCT);

    // FIXME: We reproject to get the data into the image. This is unnecessarily complex though, there must be a simpler way
    // Setup warp options.
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
    oOperation.ChunkAndWarpImage(0, 0,
                                 map_size,
                                 map_size);
    GDALDestroyGenImgProjTransformer(psWarpOptions->pTransformerArg);
    GDALDestroyWarpOptions(psWarpOptions);

    GDALClose(dest);
    GDALClose(source);
}

int main() {
    GDALAllRegister();

    // reproject_to_webmercator("data/25m_EU_clip.tif", "data/25m_EU_clip_webm.tif");

    float new_top_left_x = 1470287.0;
    float new_top_left_y = 6013574.0;
    float new_size = 500000.0;

    clip("data/25m_EU_clip_webm.tif", "data/25m_EU_clip_webm_tile.tif", new_top_left_x, new_top_left_y, new_size);

    return 0;
}