extends Spatial


# Declare member variables here. Examples:
# var a = 2
# var b = "text"


# Called when the node enters the scene tree for the first time.
func _ready():
	#Geodot.reproject_to_webmercator("/home/retour/LandscapeLab/testdata/DGM_K_5m.tif",
	#	"/home/retour/LandscapeLab/testdata/webm.tif")
	
	var img = Geodot.save_tile_from_heightmap(
		"/home/retour/LandscapeLab/testdata/webm.tif",
		"/home/retour/LandscapeLab/testdata/tile.tif",
		1546670.0,
		5918250.0,
		50000.0,
		256
	)
	
	get_node("MeshInstance").mesh.surface_get_material(0).set_shader_param("heightmap", img)


func _process(delta):
	var img = Geodot.save_tile_from_heightmap(
		"/home/retour/LandscapeLab/testdata/webm.tif",
		"/home/retour/LandscapeLab/testdata/tile.tif",
		1546670.0,
		5918250.0,
		50000.0,
		256
	)
	
	get_node("MeshInstance").mesh.surface_get_material(0).set_shader_param("heightmap", img)
	
	print(Geodot.get_time_passed())
