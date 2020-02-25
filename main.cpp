#include <iostream>
#include <gdal/gdal_priv.h>
#include <gdal/gdalwarper.h>

int main() {
    GDALDriverH hDriver;
    GDALDataType eDT;
    GDALDatasetH hDstDS;
    GDALDatasetH hSrcDS;

    GDALAllRegister();

    // Open the source file.
    hSrcDS = GDALOpen("data/25m_EU_clip.tif", GA_ReadOnly);
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
    double transform[6];
    GDALGetGeoTransform(hSrcDS, transform);

    double new_top_left_x = 4563262.0;
    double new_top_left_y = 2707690.0;
    double new_size = 1000.0;

    double x_diff = new_top_left_x - transform[0];
    double y_diff = new_top_left_y - transform[3];

    adfDstGeoTransform[0] += x_diff;
    adfDstGeoTransform[3] += y_diff;

    std::cout << transform[0] << std::endl;
    std::cout << transform[3] << std::endl;

    //adjust top left coordinates
    transform[0] = new_top_left_x;
    transform[3] = new_top_left_y;

    //determine dimensions of the new (cropped) raster in cells
    int xSize = round(new_size/transform[1]);
    int ySize = round(new_size/transform[1]);

    std::cout << nPixels << std::endl;
    std::cout << nLines << std::endl;

    // Create the output file.
    hDstDS = GDALCreate(hDriver, "data/out2.tif", nPixels / 4.0, nLines / 4.0,
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

    return 0;
}

int oldmain() {
    std::string filename = "data/25m_EU_clip.tif";
    float new_top_left_x = 4563262.0;
    float new_top_left_y = 2707690.0;
    float new_size = 1000.0;

    GDALDataset *poDataset;
    GDALAllRegister();
    poDataset = (GDALDataset *) GDALOpen(filename.c_str(), GA_ReadOnly);

    if (poDataset == nullptr) {
        return 1;
    }

    double adfGeoTransform[6];
    printf("Driver: %s/%s\n",
           poDataset->GetDriver()->GetDescription(),
           poDataset->GetDriver()->GetMetadataItem(GDAL_DMD_LONGNAME));
    printf("Size is %dx%dx%d\n",
           poDataset->GetRasterXSize(), poDataset->GetRasterYSize(),
           poDataset->GetRasterCount());
    if (poDataset->GetProjectionRef() != NULL)
        printf("Projection is `%s'\n", poDataset->GetProjectionRef());
    if (poDataset->GetGeoTransform(adfGeoTransform) == CE_None) {
        printf("Origin = (%.6f,%.6f)\n",
               adfGeoTransform[0], adfGeoTransform[3]);
        printf("Pixel Size = (%.6f,%.6f)\n",
               adfGeoTransform[1], adfGeoTransform[5]);
    }

    GDALRasterBand *poBand;
    int nBlockXSize, nBlockYSize;
    int bGotMin, bGotMax;
    double adfMinMax[2];
    poBand = poDataset->GetRasterBand(1);
    poBand->GetBlockSize(&nBlockXSize, &nBlockYSize);
    printf("Block=%dx%d Type=%s, ColorInterp=%s\n",
           nBlockXSize, nBlockYSize,
           GDALGetDataTypeName(poBand->GetRasterDataType()),
           GDALGetColorInterpretationName(
                   poBand->GetColorInterpretation()));
    adfMinMax[0] = poBand->GetMinimum(&bGotMin);
    adfMinMax[1] = poBand->GetMaximum(&bGotMax);
    if (!(bGotMin && bGotMax))
        GDALComputeRasterMinMax((GDALRasterBandH) poBand, TRUE, adfMinMax);
    printf("Min=%.3fd, Max=%.3f\n", adfMinMax[0], adfMinMax[1]);
    if (poBand->GetOverviewCount() > 0)
        printf("Band has %d overviews.\n", poBand->GetOverviewCount());
    if (poBand->GetColorTable() != NULL)
        printf("Band has a color table with %d entries.\n",
               poBand->GetColorTable()->GetColorEntryCount());

    float *pafScanline;
    int nXSize = poBand->GetXSize();
    pafScanline = (float *) CPLMalloc(sizeof(float) * nXSize);
    poBand->RasterIO(GF_Read, 0, 0, nXSize, 1,
                     pafScanline, nXSize, 1, GDT_Float32,
                     0, 0);

    return 0;
}