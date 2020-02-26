extends Spatial


# Declare member variables here. Examples:
# var a = 2
# var b = "text"


# Called when the node enters the scene tree for the first time.
func _ready():
	Geodot.save_tile_from_heightmap(
		"/home/karl/Data/BOKU/geodot-plugin/src/raster-tile-extractor/data/webm.tif",
		"/home/karl/Data/BOKU/geodot-plugin/src/raster-tile-extractor/data/tile.tif",
		1470287.0,
		6013574.0,
		50000.0,
		256
	)


func _process(delta):
	print(Geodot.get_time_passed())
