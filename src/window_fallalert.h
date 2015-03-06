#ifndef W_FALLALERT_H_
#define W_FALLALERT_H_

void deinit_fallalert_window();
void middle_button_fallalert(ClickRecognizerRef recognizer, Window *window);
void config_provider_fallalert(void* context);
void fallalert_window_load(Window *window);
void fallalert_window_unload(Window *window);
void init_fallalert_window();
	
#endif