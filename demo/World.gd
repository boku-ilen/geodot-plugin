extends Spatial


var instanced_image_count = 0
var time_passed = 0

var street_scene = preload("res://Street.tscn")


func _ready():
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


func _process(delta):
	time_passed += delta
	
	var img = Geodot.get_image(
		"/home/karl/Documents/dhm/5m_K_clip",
		"tif",
		1492193 + time_passed * 2000.0,
		5919331,
		10000.0,
		256,
		1
	)
	
	instanced_image_count += 1
	
	get_node("MeshInstance").mesh.surface_get_material(0).set_shader_param("heightmap", img.get_image_texture())
	get_node("MeshInstance").mesh.surface_get_material(0).set_shader_param("ortho", img.get_image_texture())
	
	print(instanced_image_count / time_passed)
