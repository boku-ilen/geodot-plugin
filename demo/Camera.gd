extends Camera

# Basic mouse and keyboard movement

var rot_x:float = 0
var rot_y:float = 0

const MOVE_SPEED:float = 2.0
const LOOKAROUND_SPEED:float = 0.005

func _ready():
	Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)

func _process(delta):
	# Move the camera based on the key pressed
	if Input.is_action_pressed("ui_left"):
		translate(Vector3.LEFT * delta * MOVE_SPEED)
	if Input.is_action_pressed("ui_right"):
		translate(Vector3.RIGHT * delta * MOVE_SPEED)
	if Input.is_action_pressed("ui_up"):
		translate(Vector3.FORWARD * delta * MOVE_SPEED)
	if Input.is_action_pressed("ui_down"):
		translate(-Vector3.FORWARD * delta * MOVE_SPEED)


func _input(event):
	if event is InputEventMouseMotion:
		rot_x -= event.relative.x * LOOKAROUND_SPEED
		rot_y -= event.relative.y * LOOKAROUND_SPEED

		transform.basis = Basis()

		rotate_object_local(Vector3(0, 1, 0), rot_x)
		rotate_object_local(Vector3(1, 0, 0), rot_y)

	elif event is InputEventKey:
		if event.is_action_released("ui_cancel"):
			Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE)
			get_tree().quit()
