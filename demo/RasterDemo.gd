extends Spatial


var instanced_image_count = 0
var time_passed = 0


func _process(delta):
	time_passed += delta
	
	var img = Geodot.get_image(
		"/media/karl/loda/geodata/wien-dhm-1m",
		"tif",
		1815513 + time_passed * 10.0,
		6137496,
		400.0,
		256,
		1
	)
	var ortho = Geodot.get_image(
		"/media/karl/loda/geodata/wien-orthos-15cm",
		"tif",
		1815513 + time_passed * 10.0,
		6137496,
		400.0,
		256,
		1
	)
	
	instanced_image_count += 1
	
	get_node("MeshInstance").mesh.surface_get_material(0).set_shader_param("heightmap", img.get_image_texture())
	get_node("MeshInstance").mesh.surface_get_material(0).set_shader_param("ortho", ortho.get_image_texture())
	
	print(instanced_image_count / time_passed)
