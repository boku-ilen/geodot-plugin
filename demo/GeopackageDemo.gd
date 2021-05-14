extends Node


export(String) var path
export(String) var layer_name


# Called when the node enters the scene tree for the first time.
func _ready():
	print("GeoDot info")
	print(' path              : ', path)
	print(' layer_name        : ', layer_name)
	print(' attribute_name    : ', attribute_name)

	print(' fetching dataset  : ', path)
	var dataset = Geodot.get_dataset(path)

	print(' fetching layer    : ', layer_name)
	var layer = dataset.get_feature_layer(layer_name)

	# Create a new feature
	var new_feature = layer.create_feature()
	new_feature.set_attribute("Test", "Another Success?")
	
	# Get all features
	var features = layer.get_all_features()
	
	print("Features and attributes:")
	for feature in features:
		print(feature.get_attributes())
