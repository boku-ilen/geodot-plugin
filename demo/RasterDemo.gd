extends Spatial

export(float) var start_position_webmercator_x = 1815513.0
export(float) var start_position_webmercator_y = 6137496.0
export(float) var tile_size_meters = 400.0
export(int) var tile_size_pixels = 256

export(String) var heightmap_data_path
export(String) var heightmap_data_ending

export(String) var ortho_data_path
export(String) var ortho_data_ending

var instanced_image_count = 0
var time_passed = 0


func _process(delta):
	time_passed += delta
	
	var heightmap_data = Geodot.get_dataset(heightmap_data_path).get_raster_layer("")
	var ortho_data = Geodot.get_raster_layer_for_pyramid(ortho_data_path, ortho_data_ending)
	
	var img = heightmap_data.get_image(
		start_position_webmercator_x + time_passed * 10.0,
		start_position_webmercator_y,
		tile_size_meters,
		tile_size_pixels,
		1
	)
	var ortho = ortho_data.get_image(
		start_position_webmercator_x + time_passed * 10.0,
		start_position_webmercator_y,
		tile_size_meters,
		tile_size_pixels,
		1
	)
	
	instanced_image_count += 1
	
	get_node("MeshInstance").mesh.surface_get_material(0).set_shader_param("heightmap", img.get_image_texture())
	get_node("MeshInstance").mesh.surface_get_material(0).set_shader_param("ortho", ortho.get_image_texture())
	
	print(instanced_image_count / time_passed)
