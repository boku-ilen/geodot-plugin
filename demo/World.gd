extends Spatial


# Declare member variables here. Examples:
# var a = 2
# var b = "text"


# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.


func _process(delta):
	print(Geodot.get_time_passed())
	
	Geodot.save_tile_from_heightmap("test", "test2", 0, 0, 1, 1)
