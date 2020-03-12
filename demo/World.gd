extends Spatial


var instanced_image_count = 0
var time_passed = 0


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
