extends Node


export(String) var path
export(String) var layer_name


# Called when the node enters the scene tree for the first time.
func _ready():
	var features = Geodot.get_all_features(path, layer_name)
	
	for feature in features:
		print(feature.get_attribute("NAME"))
