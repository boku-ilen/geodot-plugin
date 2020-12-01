extends Node


export(String) var path
export(String) var layer_name
export(String) var attribute_name


# Called when the node enters the scene tree for the first time.
func _ready():
	var dataset = Geodot.get_dataset(path)
	var layer = dataset.get_feature_layer(layer_name)
	
	var features = layer.get_all_features()
	
	for feature in features:
		print(feature.get_attribute(attribute_name))
