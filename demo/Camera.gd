extends Camera

# Basic mouse and keyboard movement

var rot_x = 0
var rot_y = 0

const MOVE_SPEED = 2
const LOOKAROUND_SPEED = 0.005

var is_right_mouse_button_down = false


func _process(delta):
	if is_right_mouse_button_down == true:
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
		if is_right_mouse_button_down == true:
			rot_x -= event.relative.x * LOOKAROUND_SPEED
			rot_y -= event.relative.y * LOOKAROUND_SPEED
			
			transform.basis = Basis()
			
			rotate_object_local(Vector3(0, 1, 0), rot_x)
			rotate_object_local(Vector3(1, 0, 0), rot_y)
	
	elif event is InputEventMouseButton:
		if event.button_index == BUTTON_RIGHT:
			is_right_mouse_button_down = event.pressed
			
			if is_right_mouse_button_down:
				Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)
			else:
				Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE)
	
	elif event is InputEventKey:
		if event.is_action_released("ui_cancel"):
			get_tree().quit()
