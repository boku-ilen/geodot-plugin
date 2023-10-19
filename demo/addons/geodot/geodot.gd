@tool
extends Node


static func get_dataset(path: String, write_access=false) -> GeoDataset:
	var dataset := GeoDataset.new()
	dataset.load_from_file(path, write_access)
	
	return dataset


static func get_raster_layer(path: String, write_access=false) -> GeoRasterLayer:
	var layer := GeoRasterLayer.new()
	layer.load_from_file(path, write_access)
	
	return layer
