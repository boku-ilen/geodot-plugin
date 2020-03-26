extends Spatial


var street_scene = preload("res://Street.tscn")


func _ready() -> void:
	var lines = Geodot.get_lines_near_position("/home/karl/BOKU/geodata/streets/2_linknetz_ogd_WM.shp", 1747139.5, 6154178.7, 1000.0, 300)
	print(lines.size())

	for line in lines:
		var street = street_scene.instance()
		street.curve = line.get_as_curve3d_offset(- 1747139, 0, -6154178)

		var width = float(line.get_attribute("WIDTH"))
		width = max(width, 2) # It's sometimes -1 in the data

		street.get_node("PathFollow/CSGPolygon").polygon[0] = Vector2(-width, 0)
		street.get_node("PathFollow/CSGPolygon").polygon[1] = Vector2(-width, 3)
		street.get_node("PathFollow/CSGPolygon").polygon[2] = Vector2(0, 3)
		street.get_node("PathFollow/CSGPolygon").polygon[3] = Vector2(width, 3)
		street.get_node("PathFollow/CSGPolygon").polygon[4] = Vector2(width, 0)

		print(street.curve.get_baked_points()[0])
		add_child(street)
