#include <pebble.h>
#include <inttypes.h>

#define KEY_COMMAND 0
#define COMMAND_DATA 1

#define CFG_RATE 10 // 10 samples/second
#define CFG_RANGE 3 // samples are recorded for 10 secs
#define CFG_BATCH 10 // samples are reported every 10 samples, so every second
#define CFG_PARAM_COUNT 3 // number of parameters for every sample

static Window *s_main_window;

static void data_handler(AccelData *data, uint32_t num_samples) {	
	DictionaryIterator *iterator;
	app_message_outbox_begin(&iterator);

	dict_write_int8(iterator, KEY_COMMAND, COMMAND_DATA);
	
	for(uint sample = 0; sample < num_samples; sample++){
		
		/*
		X = 0
		Y = 1
		Z = 2
		+1 is added to ensure that 0 is always the command.
		*/

		// Add acceleration data
		Tuplet t3 = TupletInteger(CFG_PARAM_COUNT * sample + 0 + 1, data[sample].x);
		dict_write_tuplet(iterator, &t3);
		
		Tuplet t4 = TupletInteger(CFG_PARAM_COUNT * sample + 1 + 1, data[sample].y);
		dict_write_tuplet(iterator, &t4);
		
		Tuplet t5 = TupletInteger(CFG_PARAM_COUNT * sample + 2 + 1, data[sample].z);
		dict_write_tuplet(iterator, &t5);
	}
	
	app_message_outbox_send();
}

static void main_window_load(Window *window) {
  // Not used
}

static void main_window_unload(Window *window) {
  // Not used
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
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
  app_event_loop();
  deinit();
}