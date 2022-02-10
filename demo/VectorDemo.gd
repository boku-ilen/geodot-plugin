extends Node3D

var center_x := 1700339.7
var center_y := 6151034.9
var radius :=  1000.0
var max_streets = 50

var streets_shapefile_path = "geodata/streets_webm.shp"

var street_scene = preload("res://Street.tscn")


func _ready() -> void:
	var dataset = Geodot.get_dataset(streets_shapefile_path)
	var layer = dataset.get_feature_layer("streets_webm")
	
	#var lines = layer.get_all_features()
	var lines = layer.get_features_near_position(center_x, center_y, radius, max_streets)
	print(lines.size())

	for line in lines:
		var street = street_scene.instantiate()
		var curve = line.get_offset_curve3d(int(-center_x), 0, int(-center_y))
		
		street.curve = curve
#
#		var width = 5.0
#
#		street.get_node("PathFollow/CSGPolygon").polygon[0] = Vector2(-width, 0)
#		street.get_node("PathFollow/CSGPolygon").polygon[1] = Vector2(-width, 3)
#		street.get_node("PathFollow/CSGPolygon").polygon[2] = Vector2(0, 3)
#		street.get_node("PathFollow/CSGPolygon").polygon[3] = Vector2(width, 3)
#		street.get_node("PathFollow/CSGPolygon").polygon[4] = Vector2(width, 0)

		add_child(street)
