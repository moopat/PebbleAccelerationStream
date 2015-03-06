#include <pebble.h>

static Window *s_medication_window;
static BitmapLayer *s_icon_layer;
static GBitmap *s_medication_bitmap;
static TextLayer *s_text_layer;

// Pill Icon Source: http://www.iconarchive.com/show/windows-8-icons-by-icons8/Healthcare-Pill-icon.html
static void medication_window_load(Window *window) {
  // Create GBitmap, then set to created BitmapLayer
	s_medication_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_PILL);
	s_icon_layer = bitmap_layer_create(GRect(37, 15, 70, 70));
	bitmap_layer_set_bitmap(s_icon_layer, s_medication_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_icon_layer));
	
	s_text_layer = text_layer_create(GRect(5, 90, 134, 80));
  text_layer_set_background_color(s_text_layer, GColorClear);
  text_layer_set_text_color(s_text_layer, GColorBlack);
  text_layer_set_text(s_text_layer, "Bitte nehmen Sie Ihre Tabletten ein!");

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_text_layer));
	
	// Vibrate!
	static const uint32_t const segments[] = { 200, 50, 200, 50, 200 };
	VibePattern pat = {
		.durations = segments,
		.num_segments = ARRAY_LENGTH(segments),
	};
	vibes_enqueue_custom_pattern(pat);
	light_enable_interaction();

}

static void deinit_medication_window() {
  // Destroy main Window
  window_destroy(s_medication_window);
}

static void medication_window_unload(Window *window) {
	gbitmap_destroy(s_medication_bitmap);
	bitmap_layer_destroy(s_icon_layer);
	
	deinit_medication_window();
}

void init_medication_window() {
  // Create medication Window
  s_medication_window = window_create();
  window_set_window_handlers(s_medication_window, (WindowHandlers) {
    .load = medication_window_load,
    .unload = medication_window_unload,
  });
  window_stack_push(s_medication_window, true);
}