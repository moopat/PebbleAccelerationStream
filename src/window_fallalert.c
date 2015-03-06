#include <pebble.h>
#include <main.h>

static Window *s_fallalert_window;
static BitmapLayer *s_fallalert_icon_layer;
static GBitmap *s_fallalert_icon;
static GBitmap *s_icon_cancel;
static TextLayer *s_fallalert_text_layer;
static ActionBarLayer *s_fallalert_actionbar;

static void deinit_fallalert_window() {
  // Destroy main Window
  window_destroy(s_fallalert_window);
}

void middle_button_fallalert(ClickRecognizerRef recognizer, Window *window) {
	cancel_alert();
	window_stack_pop(true);
}

void config_provider_fallalert(void* context) {
	window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) middle_button_fallalert);
}

/*
 * Fall Icon: http://www.freepik.com, CC BY 3.0
 * Cancel Icon: http://www.finest.graphics, CC BY 3.0
 */
static void fallalert_window_load(Window *window) {
  // Create GBitmap, then set to created BitmapLayer
	s_fallalert_icon = gbitmap_create_with_resource(RESOURCE_ID_ICON_FALL);
	s_fallalert_icon_layer = bitmap_layer_create(GRect(37, 15, 70, 70));
	bitmap_layer_set_bitmap(s_fallalert_icon_layer, s_fallalert_icon);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_fallalert_icon_layer));
	
	s_fallalert_text_layer = text_layer_create(GRect(5, 90, 134, 80));
  text_layer_set_background_color(s_fallalert_text_layer, GColorClear);
  text_layer_set_text_color(s_fallalert_text_layer, GColorBlack);
  text_layer_set_text(s_fallalert_text_layer, "Hilfe wird geholt!\nAbbrechen?");

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_fallalert_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_fallalert_text_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_fallalert_text_layer));
	
	// Set up ActionBar
	s_icon_cancel = gbitmap_create_with_resource(RESOURCE_ID_MENU_CANCEL);
	s_fallalert_actionbar = action_bar_layer_create();
	action_bar_layer_set_click_config_provider(s_fallalert_actionbar, (ClickConfigProvider) config_provider_fallalert);
	action_bar_layer_set_icon(s_fallalert_actionbar, BUTTON_ID_SELECT, s_icon_cancel);
	action_bar_layer_add_to_window(s_fallalert_actionbar, s_fallalert_window);
	
	// Vibrate!
	static const uint32_t const segments[] = { 200, 50, 200, 50, 200, 50, 200, 50, 200, 50, 200 };
	VibePattern pat = {
		.durations = segments,
		.num_segments = ARRAY_LENGTH(segments),
	};
	vibes_enqueue_custom_pattern(pat);
	light_enable_interaction();

}

static void fallalert_window_unload(Window *window) {
	gbitmap_destroy(s_fallalert_icon);
	bitmap_layer_destroy(s_fallalert_icon_layer);
	action_bar_layer_destroy(s_fallalert_actionbar);
	gbitmap_destroy(s_icon_cancel);
	
	deinit_fallalert_window();
}

void init_fallalert_window() {
  // Create Fall Alert Window
  s_fallalert_window = window_create();
  window_set_window_handlers(s_fallalert_window, (WindowHandlers) {
    .load = fallalert_window_load,
    .unload = fallalert_window_unload,
  });
  window_stack_push(s_fallalert_window, true);
}