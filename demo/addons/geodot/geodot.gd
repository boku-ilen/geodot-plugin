@tool
extends Object
class_name Geodot


static func get_dataset(path: String) -> GeoDataset:
	if not FileAccess.file_exists(path):
		push_error("Trying to load non existent file at '%s'" % [path])
	
	return load(path)


static func get_raster_layer(path: String) -> GeoRasterLayer:
	if not FileAccess.file_exists(path):
		push_error("Trying to load non existent file at '%s'" % [path])
	
	return load(path)
