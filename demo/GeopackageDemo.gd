extends Node


export(String) var path
export(String) var layer_name


# Called when the node enters the scene tree for the first time.
func _ready():
	var dataset = Geodot.get_dataset(path)
	var layer = dataset.get_feature_layer(layer_name)
	
	# Get all features
	var features = layer.get_all_features()
	
	print("Features and attributes:")
	for feature in features:
		print(feature.get_attributes())
	
	# Create a new feature
	var new_feature = layer.create_feature()
	new_feature.set_attribute("Test", "Another Success?")
	
	print("New feature:")
	print(new_feature.get_attributes())
