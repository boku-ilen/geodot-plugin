extends Spatial


var instanced_image_count = 0


# Called when the node enters the scene tree for the first time.
func _ready():
	#Geodot.reproject_to_webmercator("/home/retour/LandscapeLab/testdata/DGM_K_5m.tif",
	#	"/home/retour/LandscapeLab/testdata/webm.tif")
	
	var img = Geodot.save_tile_from_heightmap(
		"/home/retour/LandscapeLab/testdata/webm.tif",
		1546670.0,
		5918250.0,
		500.0,
		256,
		1
	)
	
	get_node("MeshInstance").mesh.surface_get_material(0).set_shader_param("heightmap", img)


func _process(delta):
	var ti = Geodot.get_image()
	
	ti.test_print()
	
	var img = Geodot.save_tile_from_heightmap(
		"/home/retour/LandscapeLab/testdata/webm.tif",
		1546670.0,
		5918250.0,
		500.0,
		256,
		1
	)
	
	instanced_image_count += 1
	
	get_node("MeshInstance").mesh.surface_get_material(0).set_shader_param("heightmap", img)
	
	print(instanced_image_count / Geodot.get_time_passed())
