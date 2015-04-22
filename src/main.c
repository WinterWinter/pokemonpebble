#include <pebble.h>

#define KEY_TEMPERATURE 1
#define KEY_SCALE 2
#define KEY_TEMP 3
#define KEY_POKEMON 4
#define KEY_TICKS 5
  
#define R_MIN 70
  
static Window *s_main_window;
static Layer *rootLayer, *layer;

time_t auto_hide;
  
static bool initiate_watchface = true;

int POKEMON_COUNT = 11; 

//Hand Layers
static GBitmap *minuteHandBitmap, *hourHandBitmap;
static RotBitmapLayer *minuteHandLayer, *hourHandLayer;

//Date Layer
TextLayer *text_date_layer;
static GFont *date_font;

//Weather Layer
static TextLayer *s_weather_layer;
static GFont s_weather_font;

//Battery
static uint8_t battery_level;
static Layer *battery_layer;

#define TOTAL_POKEMON 1
static BitmapLayer *pokemon_layers[TOTAL_POKEMON];
static GBitmap *pokemon_images[TOTAL_POKEMON];

#define TOTAL_POKEBALL 1
static GBitmap *pokeball_images[TOTAL_POKEBALL];
static BitmapLayer *pokeball_layers[TOTAL_POKEBALL];

#define TOTAL_BACKGROUND 1
static GBitmap *background_images[TOTAL_BACKGROUND];
static BitmapLayer *background_layers[TOTAL_BACKGROUND];


//Battery Container
const int POKEBALL_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_BATTERY_25,
  RESOURCE_ID_BATTERY_50,
  RESOURCE_ID_BATTERY_75,
  RESOURCE_ID_BATTERY_100
};


//Pokemon Container
const int POKEMON_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_BULBASAUR_WHITE,
  RESOURCE_ID_CHARMANDER_WHITE,
  RESOURCE_ID_EEVEE_WHITE,
  RESOURCE_ID_GYARDOS_WHITE,
  RESOURCE_ID_JIGGLYPUFF_WHITE,
  RESOURCE_ID_MEW_WHITE,
  RESOURCE_ID_PIKACHU_WHITE,
  RESOURCE_ID_PORYGON_WHITE,
  RESOURCE_ID_PSYDUCK_WHITE,
  RESOURCE_ID_SNORLAX_WHITE,
  RESOURCE_ID_SQUIRTLE_WHITE,
};


//Background Container
const int BACKGROUND_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_BACKGROUND,
  RESOURCE_ID_BACKGROUND_TICKS,
};


//Bitmap Container
static void set_container_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id, GPoint origin) {
GBitmap *old_image = *bmp_image;

 	*bmp_image = gbitmap_create_with_resource(resource_id);
 	GRect frame = (GRect) {
   	.origin = origin,
    .size = gbitmap_get_bounds(*bmp_image).size
      
};
 	bitmap_layer_set_bitmap(bmp_layer, *bmp_image);
 	layer_set_frame(bitmap_layer_get_layer(bmp_layer), frame);

 	if (old_image != NULL) {
 	gbitmap_destroy(old_image);
}
}

//Battery Layer
void battery_layer_update_callback(Layer *layer, GContext *ctx) {

  	if (battery_level > 75) {
    set_container_image(&pokeball_images[0], pokeball_layers[0], POKEBALL_IMAGE_RESOURCE_IDS[3], GPoint(30, 81));
      } 
      else if (battery_level > 50 && battery_level <= 75){
    set_container_image(&pokeball_images[0], pokeball_layers[0], POKEBALL_IMAGE_RESOURCE_IDS[2], GPoint(30, 81));
    }
      else if (battery_level > 25 && battery_level <= 50){
    set_container_image(&pokeball_images[0], pokeball_layers[0], POKEBALL_IMAGE_RESOURCE_IDS[1], GPoint(30, 81));
    }
      else if (battery_level <= 25){
    set_container_image(&pokeball_images[0], pokeball_layers[0], POKEBALL_IMAGE_RESOURCE_IDS[0], GPoint(30, 81));
    }
  	else {	
      
 	}
  
}


//Handle Battery
void battery_state_handler(BatteryChargeState charge) {
	battery_level = charge.charge_percent;
}


static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char date_text[] = "00.00";

  // Only update the date when it's changed.
  strftime(date_text, sizeof(date_text), "%m.%d", tick_time);
  text_layer_set_text(text_date_layer, date_text);
  
}


// Bluetooth Connectivity
static void handle_bluetooth(bool connected) {
if (connected) {
// Normal Mode 
if (initiate_watchface) {}
else {
vibes_double_pulse();
}}
else {
if (initiate_watchface) {}
else {
vibes_enqueue_custom_pattern( (VibePattern) {
   	.durations = (uint32_t []) {100, 100, 100, 100, 100},
   	.num_segments = 5
} );
}}}

static void Random(struct tm *current_time) {
  
   	if( ((current_time->tm_min == 0) && (current_time->tm_sec == 0)) || (initiate_watchface == true) ){ 
    int r = rand() % POKEMON_COUNT;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Random Pokemon generated [#%d].", r);
    set_container_image(&pokemon_images[0], pokemon_layers[0], POKEMON_IMAGE_RESOURCE_IDS[r], GPoint (21, 33));   
    } 
}


static void Who() {
  
    srand(time(NULL));
    int r = rand() % POKEMON_COUNT;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Random Pokemon generated [#%d].", r);
    set_container_image(&pokemon_images[0], pokemon_layers[0], POKEMON_IMAGE_RESOURCE_IDS[r], GPoint (21, 33));  
}


static void handleTick(struct tm *t, TimeUnits units_changed) {
    
  int check = persist_read_int(KEY_POKEMON);
    
    if (check == 12){
    Random(t);
  }
  
	GRect r;
	int32_t minuteAngle = t->tm_min * TRIG_MAX_ANGLE / 60;
	int32_t hourAngle = ((t->tm_hour%12)*60 + t->tm_min) * TRIG_MAX_ANGLE / 720;
	
	r = layer_get_frame((Layer *)hourHandLayer);
	r.origin.x = 72 - r.size.w/2 + 65 * cos_lookup((hourAngle + 3 * TRIG_MAX_ANGLE / 4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO;
	r.origin.y = 84 - r.size.h/2 + 65 * sin_lookup((hourAngle + 3 * TRIG_MAX_ANGLE / 4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO;
	layer_set_frame((Layer *)hourHandLayer, r);
  rot_bitmap_layer_set_angle(hourHandLayer, hourAngle);
  
  r = layer_get_frame((Layer *)minuteHandLayer);
	r.origin.x = 72 - r.size.w/2 + 65 * cos_lookup((minuteAngle + 3 * TRIG_MAX_ANGLE / 4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO;
	r.origin.y = 84 - r.size.h/2 + 65 * sin_lookup((minuteAngle + 3 * TRIG_MAX_ANGLE / 4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO;
	layer_set_frame((Layer *)minuteHandLayer, r);
	rot_bitmap_layer_set_angle(minuteHandLayer, minuteAngle); 
  
  if(time(NULL) > auto_hide){
    layer_set_hidden(text_layer_get_layer(s_weather_layer), true);
    layer_set_hidden(text_layer_get_layer(text_date_layer), true);
    layer_set_hidden(bitmap_layer_get_layer(pokemon_layers[0]), false); 

    for (int i = 0; i < TOTAL_POKEBALL; ++i) {
  layer_set_hidden(bitmap_layer_get_layer(pokeball_layers[i]), true);
    }
  }
  
  if(time(NULL) == auto_hide + 1 && check == 12){
    Who();  
  }  
}

static void handle_tap(AccelAxisType axis, int32_t direction)
{  
  auto_hide = time(NULL) + 10;
  layer_set_hidden(text_layer_get_layer(s_weather_layer), false);
  layer_set_hidden(text_layer_get_layer(text_date_layer), false);
  layer_set_hidden(bitmap_layer_get_layer(pokemon_layers[0]), true);
  
  for (int i = 0; i < TOTAL_POKEBALL; ++i) {
  layer_set_hidden(bitmap_layer_get_layer(pokeball_layers[i]), false);
      }  
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char weather_layer_buffer[32];
  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  int temperature;
  int Kelvin = persist_read_int(KEY_TEMPERATURE);
  int finalTemp = Kelvin;
  int temp = persist_read_int(KEY_TEMP);
  
  int r = rand() % POKEMON_COUNT;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Random Pokemon generated [#%d].", r);
    
  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
      
       case KEY_TICKS:
      
      if(strcmp(t->value->cstring, "off") == 0)
      {
        set_container_image(&background_images[0], background_layers[0], BACKGROUND_IMAGE_RESOURCE_IDS[0], GPoint(4, 17));
        persist_write_int(KEY_TICKS, 0);
      }
       else if(strcmp(t->value->cstring, "on") == 0)
      {
        set_container_image(&background_images[0], background_layers[0], BACKGROUND_IMAGE_RESOURCE_IDS[1], GPoint(4, 17)); 
        persist_write_int(KEY_TICKS, 1);
      }
      break;
      
      case KEY_POKEMON:
     
      if(strcmp(t->value->cstring, "Bulbasaur") == 0)
      {
        set_container_image(&pokemon_images[0], pokemon_layers[0], POKEMON_IMAGE_RESOURCE_IDS[0], GPoint(21,33));
        persist_write_int(KEY_POKEMON, 0);
      }
       else if(strcmp(t->value->cstring, "Charmander") == 0)
      {     
        set_container_image(&pokemon_images[0], pokemon_layers[0], POKEMON_IMAGE_RESOURCE_IDS[1], GPoint(21,33)); 
        persist_write_int(KEY_POKEMON, 1);
      }
       else if(strcmp(t->value->cstring, "Eevee") == 0)
      {     
        set_container_image(&pokemon_images[0], pokemon_layers[0], POKEMON_IMAGE_RESOURCE_IDS[2], GPoint(21,33)); 
        persist_write_int(KEY_POKEMON, 2);
      }
       else if(strcmp(t->value->cstring, "Gyardos") == 0)
      {
        set_container_image(&pokemon_images[0], pokemon_layers[0], POKEMON_IMAGE_RESOURCE_IDS[3], GPoint(21,33)); 
        persist_write_int(KEY_POKEMON, 3);
      }   
            
      else if(strcmp(t->value->cstring, "Jigglypuff") == 0)
      {     
        set_container_image(&pokemon_images[0], pokemon_layers[0], POKEMON_IMAGE_RESOURCE_IDS[4], GPoint(21,33)); 
        persist_write_int(KEY_POKEMON, 4);
      }
      
      else if(strcmp(t->value->cstring, "Mew") == 0)
      {     
        set_container_image(&pokemon_images[0], pokemon_layers[0], POKEMON_IMAGE_RESOURCE_IDS[5], GPoint(21,33)); 
        persist_write_int(KEY_POKEMON, 5);
      }
      
      else if(strcmp(t->value->cstring, "Pikachu") == 0)
      {     
        set_container_image(&pokemon_images[0], pokemon_layers[0], POKEMON_IMAGE_RESOURCE_IDS[6], GPoint(21,33)); 
        persist_write_int(KEY_POKEMON, 6);
      }
      
      else if(strcmp(t->value->cstring, "Porygon") == 0)
      {     
        set_container_image(&pokemon_images[0], pokemon_layers[0], POKEMON_IMAGE_RESOURCE_IDS[7], GPoint(21,33)); 
        persist_write_int(KEY_POKEMON, 7);
      }
      
      else if(strcmp(t->value->cstring, "Psyduck") == 0)
      {     
        set_container_image(&pokemon_images[0], pokemon_layers[0], POKEMON_IMAGE_RESOURCE_IDS[8], GPoint(21,33)); 
        persist_write_int(KEY_POKEMON, 8);
      }
      
      else if(strcmp(t->value->cstring, "Snorlax") == 0)
      {     
        set_container_image(&pokemon_images[0], pokemon_layers[0], POKEMON_IMAGE_RESOURCE_IDS[9], GPoint(21,33)); 
        persist_write_int(KEY_POKEMON, 9);
      }
      
      else if(strcmp(t->value->cstring, "Squirtle") == 0)
      {     
        set_container_image(&pokemon_images[0], pokemon_layers[0], POKEMON_IMAGE_RESOURCE_IDS[10], GPoint(21,33)); 
        persist_write_int(KEY_POKEMON, 10);
      }
      
        else if(strcmp(t->value->cstring, "Random") == 0)
      {
        set_container_image(&pokemon_images[0], pokemon_layers[0], POKEMON_IMAGE_RESOURCE_IDS[r], GPoint(21,33));
        persist_write_int(KEY_POKEMON, 12);
      }
      break;
      
    case KEY_SCALE:
      if(strcmp(t->value->cstring, "F") == 0){
        persist_write_int(KEY_TEMP, 0);
        DictionaryIterator *iter;
        app_message_outbox_begin(&iter);
        dict_write_uint8(iter, 0, 0);
        app_message_outbox_send();
      }
      else if(strcmp(t->value->cstring, "C") == 0){
        persist_write_int(KEY_TEMP, 1);
        DictionaryIterator *iter;
        app_message_outbox_begin(&iter);
        dict_write_uint8(iter, 0, 0);
        app_message_outbox_send();
      }
      break;
      
    case KEY_TEMPERATURE:
      if(temp == 0){
      temperature = (int)t->value->int32;
      persist_write_int(KEY_TEMPERATURE, temperature);
      Kelvin = persist_read_int(KEY_TEMPERATURE);
      finalTemp = (Kelvin - 273.15) * 1.8 + 32;
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%dF", finalTemp);
      }
      else if(temp == 1){
      temperature = (int)t->value->int32;
      persist_write_int(KEY_TEMPERATURE, temperature);
      Kelvin = persist_read_int(KEY_TEMPERATURE);
      finalTemp = Kelvin - 273.15;
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", finalTemp);
      }
      break;
    }
    // Look for next item
    t = dict_read_next(iterator);
  }
  

  // Assemble full string and display
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s", temperature_buffer);
  text_layer_set_text(s_weather_layer, weather_layer_buffer);
}


static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}


static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}


static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


static void init(void) {
  
  srand(time(NULL));
	time_t t = time(NULL);

	s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
	window_stack_push(s_main_window, true);
	rootLayer = window_get_root_layer(s_main_window);
  
   GRect dummy_frame = { {0, 0}, {0, 0} };

  
  //Create Background Layer
    for (int i = 0; i < TOTAL_BACKGROUND; ++i) {
   		background_layers[i] = bitmap_layer_create(dummy_frame);
   		layer_add_child(rootLayer, bitmap_layer_get_layer(background_layers[i]));
	}
  
  //Read persistent settings for Ticks
  int ticks = persist_read_int(KEY_TICKS);
  
  if (ticks == 0){
      set_container_image(&background_images[0], background_layers[0], BACKGROUND_IMAGE_RESOURCE_IDS[0], GPoint(4, 17)); 
  }
  else if (ticks == 1){
      set_container_image(&background_images[0], background_layers[0], BACKGROUND_IMAGE_RESOURCE_IDS[1], GPoint(4, 17)); 
  }
  
  //Create Date Layer
  date_font  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_Weather_20));
  text_date_layer = text_layer_create(GRect(0, 36, 144, 25));
  text_layer_set_text_color(text_date_layer, GColorBlack);
  text_layer_set_background_color(text_date_layer, GColorClear);
  text_layer_set_text_alignment(text_date_layer, GTextAlignmentCenter);
  text_layer_set_font(text_date_layer, date_font);
  layer_set_hidden(text_layer_get_layer(text_date_layer), true);
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(text_date_layer));
    
  //Create Battery Layer
 	BatteryChargeState initial = battery_state_service_peek();
 	battery_level = initial.charge_percent;
 	battery_layer = layer_create(GRect(0,90,144,25));
  layer_set_update_proc(battery_layer, &battery_layer_update_callback);
 	layer_add_child(window_get_root_layer(s_main_window), battery_layer);

  //Create Battery layer
 	for (int i = 0; i < TOTAL_POKEBALL; ++i) {
    pokeball_layers[i] = bitmap_layer_create(dummy_frame);
   	layer_add_child(rootLayer, bitmap_layer_get_layer(pokeball_layers[i]));
    layer_set_hidden(bitmap_layer_get_layer(pokeball_layers[i]), true);
}
  
   //Create Pokemon Layer
    for (int i = 0; i < TOTAL_POKEMON; ++i) {
   		pokemon_layers[i] = bitmap_layer_create(dummy_frame);
   		layer_add_child(rootLayer, bitmap_layer_get_layer(pokemon_layers[i]));
      bitmap_layer_set_compositing_mode(pokemon_layers[i], GCompOpAnd);
	}
  
  //Read persistent setting for Pokemon choice
  int pokemon = persist_read_int(KEY_POKEMON);
    
  
  //Set Pokemon image based off settings
  if (pokemon < POKEMON_COUNT){
      set_container_image(&pokemon_images[0], pokemon_layers[0], POKEMON_IMAGE_RESOURCE_IDS[pokemon], GPoint(21,33)); 
  }
  else if(pokemon == 12){
    uint8_t r = rand() % POKEMON_COUNT;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Random Pokemon generated [#%d].", r);
    set_container_image(&pokemon_images[0], pokemon_layers[0], POKEMON_IMAGE_RESOURCE_IDS[r], GPoint (21,33));
  }
  else {
      set_container_image(&pokemon_images[0], pokemon_layers[0], POKEMON_IMAGE_RESOURCE_IDS[0], GPoint(21,33)); 
  }
  
  layer = layer_create(GRect(0, 0, 144, 168));
	layer_add_child(rootLayer, layer);

  //Create Hour Layer
	hourHandBitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HAND_HOUR);
	hourHandLayer = rot_bitmap_layer_create(hourHandBitmap);
	rot_bitmap_set_compositing_mode(hourHandLayer, GCompOpAssign);
	layer_add_child(rootLayer, (Layer *)hourHandLayer);
  
  //Create Minute Layer
  minuteHandBitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HAND_MINUTE);
	minuteHandLayer = rot_bitmap_layer_create(minuteHandBitmap);
	rot_bitmap_set_compositing_mode(minuteHandLayer, GCompOpAssign);
	layer_add_child(rootLayer, (Layer *)minuteHandLayer);
  
  //Create temperature Layer
  s_weather_layer = text_layer_create(GRect(0, 108, 144, 25));
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorBlack);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "...");
  layer_set_hidden(text_layer_get_layer(s_weather_layer), true);

  //Create second custom font, apply it and add to Window
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_Weather_20));
  text_layer_set_font(s_weather_layer, s_weather_font);
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_weather_layer));
  
  //Subscribe to Acceleromter
  accel_tap_service_subscribe(&handle_tap);
  
  //Handle Bluetooth State
  handle_bluetooth(bluetooth_connection_service_peek());
  initiate_watchface = false;
  
  //Handle rotations
	handleTick(localtime(&t), SECOND_UNIT|MINUTE_UNIT|HOUR_UNIT);
	
	tick_timer_service_subscribe(SECOND_UNIT, handleTick);
  
  //Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  update_time();
}

static void deinit(void) {
  
  //Unsubscribe
	tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  
  //Minute Hand
	rot_bitmap_layer_destroy(minuteHandLayer);
	gbitmap_destroy(minuteHandBitmap);
  
  //Hour Hand
	rot_bitmap_layer_destroy(hourHandLayer);
	gbitmap_destroy(hourHandBitmap);
  
  //Date
  text_layer_destroy(text_date_layer);
  fonts_unload_custom_font(date_font);
  
  //Weather
  text_layer_destroy(s_weather_layer);
  fonts_unload_custom_font(s_weather_font);
  
  //Battery Layer
  layer_destroy(battery_layer);
	layer_destroy(layer);
  
  //Window
	window_destroy(s_main_window);
  
  //Battery
     	for (int i = 0; i < TOTAL_POKEBALL; i++) {
   	layer_remove_from_parent(bitmap_layer_get_layer(pokeball_layers[i]));
   	gbitmap_destroy(pokeball_images[i]);
   	bitmap_layer_destroy(pokeball_layers[i]);
}
   //Background
     	for (int i = 0; i < TOTAL_BACKGROUND; i++) {
   	layer_remove_from_parent(bitmap_layer_get_layer(background_layers[i]));
   	gbitmap_destroy(background_images[i]);
   	bitmap_layer_destroy(background_layers[i]);
}
    //Pokemon
     	for (int i = 0; i < TOTAL_POKEMON; i++) {
   	layer_remove_from_parent(bitmap_layer_get_layer(pokemon_layers[i]));
   	gbitmap_destroy(pokemon_images[i]);
   	bitmap_layer_destroy(pokemon_layers[i]);
} 
  
}


int main(void) {
  init();
  battery_state_service_subscribe (&battery_state_handler);
  bluetooth_connection_service_subscribe(&handle_bluetooth);
  app_event_loop();
  deinit();
}
