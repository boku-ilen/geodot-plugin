extends Node


func get_dataset(path: String) -> GeoDataset:
	var dataset := GeoDataset.new()
	dataset.load_from_file(path)
	
	return dataset


func get_raster_layer(path: String) -> GeoRasterLayer:
	var layer := GeoRasterLayer.new()
	layer.load_from_file(path)
	
	return layer
