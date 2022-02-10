@tool
extends EditorPlugin


func _enter_tree():
	add_autoload_singleton("Geodot", "res://addons/geodot/geodot.gd")


func _exit_tree():
	pass
