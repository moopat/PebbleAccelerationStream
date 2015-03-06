#include <pebble.h>
#include <inttypes.h>
#include <math.h>
#include <window_medication.h>
#include <window_fallalert.h>
	
#define WINDOW_MEDICATION 1
#define WINDOW_FALLALERT 2

#define KEY_COMMAND 0

#define COMMAND_MEDICATION 1
#define COMMAND_HELP 2
#define COMMAND_DATA 3
#define COMMAND_FALLDETECTED 4
#define COMMAND_CANCELALERT 5

#define CFG_RATE 10 // 10 samples/second
#define CFG_RANGE 3 // samples are recorded for 10 secs
#define CFG_BATCH 10 // samples are reported every 10 samples, so every second
#define CFG_PARAM_COUNT 4 // number of parameters for every sample
#define CFG_TIME_SIZE 5 // number of digits being kept when minifying timestamp

static Window *s_main_window;
static TextLayer *s_output_layer;
static ActionBarLayer *s_actionbar;
static uint counter = 0;
static GBitmap *s_icon_help;

// http://forums.getpebble.com/discussion/5792/sqrt-function
float my_sqrt(const float num) {
  const uint MAX_STEPS = 40;
  const float MAX_ERROR = 0.001;
  
  float answer = num;
  float ans_sqr = answer * answer;
  uint step = 0;
  while((ans_sqr - num > MAX_ERROR) && (step++ < MAX_STEPS)) {
    answer = (answer + (num / answer)) / 2;
    ans_sqr = answer * answer;
  }
  return answer;
}

static void show_window(int id){
	switch(id){
		case WINDOW_MEDICATION:
			init_medication_window();
			break;
		case WINDOW_FALLALERT:
			init_fallalert_window();
			break;
	}
}

unsigned int calculate_norm(AccelData data){
	return my_sqrt(pow(data.x, 2) + pow(data.y, 2) + pow(data.z, 2));
}

static void data_handler(AccelData *data, uint32_t num_samples) {
  // Long lived buffer
  static char s_buffer[128];
	
	DictionaryIterator *iterator;
	app_message_outbox_begin(&iterator);
	
	//Tuplet command = TupletInteger(KEY_COMMAND, COMMAND_DATA);
	//dict_write_tuplet(iterator, &command);
	dict_write_int8(iterator, KEY_COMMAND, COMMAND_DATA);
	
	for(uint sample = 0; sample < num_samples; sample++){
		counter++;
		
		unsigned long int timestamp = data[sample].timestamp % (int) pow(10, CFG_TIME_SIZE);
		
		/*
		TIME = 0
		X = 1
		Y = 2
		Z = 3
		+1 is added to ensure that 0 is always the command.
		*/
		
		// Add time information
		Tuplet t = TupletInteger(CFG_PARAM_COUNT * sample + 0 + 1, timestamp);
		dict_write_tuplet(iterator, &t);
		
		// Add acceleration data
		Tuplet t3 = TupletInteger(CFG_PARAM_COUNT * sample + 1 + 1, data[sample].x);
		dict_write_tuplet(iterator, &t3);
		
		Tuplet t4 = TupletInteger(CFG_PARAM_COUNT * sample + 2 + 1, data[sample].y);
		dict_write_tuplet(iterator, &t4);
		
		Tuplet t5 = TupletInteger(CFG_PARAM_COUNT * sample + 3 + 1, data[sample].z);
		dict_write_tuplet(iterator, &t5);
	}
	
	app_message_outbox_send();
	
	snprintf(s_buffer, sizeof(s_buffer), "%d measurements", counter);
  text_layer_set_text(s_output_layer, s_buffer);
}

void sound_alarm(){
	DictionaryIterator *iterator;
	app_message_outbox_begin(&iterator);
	dict_write_int8(iterator, KEY_COMMAND, COMMAND_HELP);
	app_message_outbox_send();
}

void cancel_alert(){
	DictionaryIterator *iterator;
	app_message_outbox_begin(&iterator);
	dict_write_int8(iterator, KEY_COMMAND, COMMAND_CANCELALERT);
	app_message_outbox_send();
}

void top_button_main(ClickRecognizerRef recognizer, Window *window)
{
	sound_alarm();
}

void bottom_button_main(ClickRecognizerRef recognizer, Window *window)
{
	sound_alarm();
}

void config_provider_main(void* context) {
	window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) top_button_main);
	window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) bottom_button_main);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create output TextLayer
  s_output_layer = text_layer_create(GRect(5, 0, window_bounds.size.w - 10, window_bounds.size.h));
  text_layer_set_font(s_output_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(s_output_layer, "No data yet.");
  text_layer_set_overflow_mode(s_output_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));
	
	// Set up ActionBar
	s_icon_help = gbitmap_create_with_resource(RESOURCE_ID_MENU_EXCLAMATION);
	s_actionbar = action_bar_layer_create();
	action_bar_layer_set_click_config_provider(s_actionbar, (ClickConfigProvider) config_provider_main);
	action_bar_layer_set_icon(s_actionbar, BUTTON_ID_DOWN, s_icon_help);
	action_bar_layer_set_icon(s_actionbar, BUTTON_ID_UP, s_icon_help);
	action_bar_layer_add_to_window(s_actionbar, s_main_window);
}

static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_output_layer);
	action_bar_layer_destroy(s_actionbar);
	gbitmap_destroy(s_icon_help);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
	Tuple *t = dict_read_first(iterator);

  // Process all pairs present
  while(t != NULL) {
    // Process this pair's key
    switch (t->key) {
      case KEY_COMMAND:
			  APP_LOG(APP_LOG_LEVEL_INFO, "A new command was received: %d", (int8_t)t->value->int8);
				switch((int8_t)t->value->int8){
					case COMMAND_MEDICATION:
						show_window(WINDOW_MEDICATION);
						break;
					case COMMAND_FALLDETECTED:
						show_window(WINDOW_FALLALERT);
						break;
				}
				
        break;
    }

    // Get next pair, if any
    t = dict_read_next(iterator);
  }
}

static void resend_failed_message(DictionaryIterator *iterator){
	app_message_outbox_begin(&iterator);
	//dict_write_int8(iterator, KEY_COMMAND, COMMAND_HELP);
	app_message_outbox_send();
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
	//resend_failed_message(iterator);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  // Create main Window
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);

  // Subscribe to the accelerometer data service
	accel_data_service_subscribe(CFG_BATCH, data_handler);

	// Choose update rate
	accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
	
	// Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy main Window
  window_destroy(s_main_window);
	accel_data_service_unsubscribe();
}

int main(void) {
  init();
	//init_medication_window();
  app_event_loop();
	//deinit_medication_window();
  deinit();
}