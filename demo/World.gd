extends Spatial


# Declare member variables here. Examples:
# var a = 2
# var b = "text"


# Called when the node enters the scene tree for the first time.
func _ready():
	var img = Geodot.save_tile_from_heightmap(
		"/home/retour/LandscapeLab/testdata/DGM_K_5m.tif",
		"/home/retour/LandscapeLab/testdata/tile.tif",
		1546670.0,
		5918250.0,
		50.0,
		256
	)
	
	var tex = ImageTexture.new()
	tex.create_from_image(img)
	
	get_node("MeshInstance").mesh.surface_get_material(0).albedo_texture = tex


func _process(delta):
	var img = Geodot.save_tile_from_heightmap(
		"/home/retour/LandscapeLab/testdata/DGM_K_5m.tif",
		"/home/retour/LandscapeLab/testdata/tile.tif",
		1546670.0,
		5918250.0,
		50.0,
		256
	)
	
	print(Geodot.get_time_passed())
