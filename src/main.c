#include "settings.h"

static Window *s_main_window;
static TextLayer *s_battery_icon_layer, *s_bluetooth_icon_layer, *s_shaketime_layer, *s_shakedate_layer;
static GFont s_icon_font;
static GColor color_background, color_time_text, color_time_textlayer, color_notification_text;
static BitmapLayer *s_background_layer;
static BitmapLayer *s_canvas_layer;
static GBitmap *s_background_bitmap;
static uint32_t s_numbers_bitmap[10];
static GSize sprite_size_number;
static GPoint number_point_UL, number_point_UR, number_point_DL, number_point_DR;
static GAlign background_bitmap_alignment;
static char *croMonths[12];
static char *croDays[7];
AppTimer *timer;
static int show_shake_delay = 3000;
static bool show_shake = false;

static void main_window_unload(Window *);
static void init();
//static void deinit();

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *timeSize = dict_find(iter, AppKeyBackgroundType);
  Tuple *shakeWindow = dict_find(iter, AppKeyShakeWindow);
  Tuple *bluetoothAlarm = dict_find(iter, AppKeyBluetoothAlarm);
  Tuple *batteryIcon = dict_find(iter, AppKeyBatteryIcon);
  Tuple *dateFormat = dict_find(iter, AppKeyDateFormat);
  Tuple *croatianDate = dict_find(iter, AppKeyCroatianDate);
  Tuple *colorTimeBackground = dict_find(iter, AppKeyColorTimeBackground);
  Tuple *colorTimeText = dict_find(iter, AppKeyColorTimeText);
  Tuple *colorNotificationText = dict_find(iter, AppKeyColorNotificationText);
  Tuple *shakeTimeout = dict_find(iter, AppKeyShakeTimeout);

  if (timeSize) {
    persist_write_int(AppKeyBackgroundType, timeSize->value->int32);
  }
  if (shakeWindow) {
    persist_write_int(AppKeyShakeWindow, shakeWindow->value->int32);
  }
  if (bluetoothAlarm) {
    persist_write_int(AppKeyBluetoothAlarm, bluetoothAlarm->value->int32);
  }
  if (batteryIcon) {
    persist_write_int(AppKeyBatteryIcon, batteryIcon->value->int32);
  }
  if (dateFormat) {
    persist_write_int(AppKeyDateFormat, dateFormat->value->int32);
  }
  if (croatianDate) {
    persist_write_int(AppKeyCroatianDate, croatianDate->value->int32);
  }
  if (colorTimeBackground) {
    persist_write_int(AppKeyColorTimeBackground, colorTimeBackground->value->int32);
  }
  if (colorTimeText) {
    persist_write_int(AppKeyColorTimeText, colorTimeText->value->int32);
  }
  if (colorNotificationText) {
    persist_write_int(AppKeyColorNotificationText, colorNotificationText->value->int32);
  }
  if (shakeTimeout) {
    persist_write_int(AppKeyShakeTimeout, shakeTimeout->value->int32);
  }
  main_window_unload(s_main_window);
  //deinit();
  init();
}


static void draw_number(GContext *ctx, GPoint origin, int number) {
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
	// Create temporary GBitmap of digit we want to display
	struct GBitmap *temp_bitmap = gbitmap_create_with_resource(s_numbers_bitmap[number]);
#if defined(PBL_COLOR)
  graphics_context_set_fill_color(ctx, color_time_text);
  graphics_fill_rect(ctx, (GRect) {.origin = origin, .size = sprite_size_number}, 0, GCornerNone);
#endif
	// Draw digit GBitmap to canvas
	graphics_draw_bitmap_in_rect(ctx, temp_bitmap, (GRect) {.origin = origin, .size = sprite_size_number});

	// Destroy temporary GBitmap
	gbitmap_destroy(temp_bitmap);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  int hour = (DEBUG==0) ? tick_time->tm_hour : 12;
	int min = (DEBUG==0) ? tick_time->tm_min : 48;
  // Convert hours to 12 hours if the user prefers
	if (clock_is_24h_style() == false && hour > 12) {
		hour -= 12;
	}
  //APP_LOG(APP_LOG_LEVEL_INFO, "canvas update proc, time is: %d : %d", hour, min);
  draw_number(ctx, number_point_UL, (int) (hour / 10));
  draw_number(ctx, number_point_UR, (int) (hour % 10));
  draw_number(ctx, number_point_DL, (int) (min / 10));
  draw_number(ctx, number_point_DR, (int) (min % 10));
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(s_main_window));
}

//  if (settings[AppKeyBluetoothAlarm]>0) {
  static void bluetooth_callback(bool connected) {
    // Show icon if disconnected
    layer_set_hidden(text_layer_get_layer(s_bluetooth_icon_layer), connected);
    if (settings[AppKeyBluetoothAlarm]==2) {
      if(connected) {
        // Issue a vibrating alert
        vibes_double_pulse();
      } else {
        // Issue a vibrating alert
        vibes_double_pulse();
      }
    }
  }
//}

static void battery_callback(BatteryChargeState state) {
  if (state.is_charging) {
    text_layer_set_text(s_battery_icon_layer, "D");
    APP_LOG(APP_LOG_LEVEL_INFO, "Battery charging");
    }
  else if (state.charge_percent <= 10 && settings[AppKeyBatteryIcon] > 0) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Battery 10");  
    text_layer_set_text(s_battery_icon_layer, "B");
    }
  else if (state.charge_percent <= 20 && settings[AppKeyBatteryIcon] == 2) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Battery 20");
    text_layer_set_text(s_battery_icon_layer, "C");
    }
  else {
    layer_set_hidden(text_layer_get_layer(s_battery_icon_layer), true);
    return;
  }
  layer_set_hidden(text_layer_get_layer(s_battery_icon_layer), false);
}


void timer_callback(void *data) {
  //text_layer_destroy(s_shake_layer);
  layer_set_hidden(text_layer_get_layer(s_shaketime_layer), true);
  layer_set_hidden(text_layer_get_layer(s_shakedate_layer), true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  show_shake=false;
}

static void update_time_shake () {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  
  // Write the current hours and minutes into a buffer
  static char s_buffer_date[30];
  static char s_buffer_time[16];
  strftime(s_buffer_time, sizeof(s_buffer_time), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  
  if (settings[AppKeyCroatianDate] == 1) { //croatian
    if (settings[AppKeyShakeWindow] > 1) { //seconds
      if (settings[AppKeyDateFormat] == 1) {
        snprintf(s_buffer_date, sizeof(s_buffer_date), "\n%d\n\n%d %s\n%s", tick_time->tm_sec, tick_time->tm_mday, croMonths[tick_time->tm_mon], croDays[tick_time->tm_wday]);
      } else {
        snprintf(s_buffer_date, sizeof(s_buffer_date), "\n%d\n\n%s %d\n%s", tick_time->tm_sec, croMonths[tick_time->tm_mon], tick_time->tm_mday, croDays[tick_time->tm_wday]);
      } 
    } else { //no seconds
      if (settings[AppKeyDateFormat] == 1) {
        snprintf(s_buffer_date, sizeof(s_buffer_date), "\n\n\n%d %s\n%s", tick_time->tm_mday, croMonths[tick_time->tm_mon], croDays[tick_time->tm_wday]);
      } else {
        snprintf(s_buffer_date, sizeof(s_buffer_date), "\n\n\n%s %d\n%s", croMonths[tick_time->tm_mon], tick_time->tm_mday, croDays[tick_time->tm_wday]);
      }
    }
  } else { //not croatian
    if (settings[AppKeyShakeWindow] > 1) { //seconds
      if (settings[AppKeyDateFormat] == 1) {
        strftime(s_buffer_date, sizeof(s_buffer_date), clock_is_24h_style() ? "\n%S\n\n%d %B\n%A" : "\n%S\n\n%d %B\n%A", tick_time);
      } else {
        strftime(s_buffer_date, sizeof(s_buffer_date), clock_is_24h_style() ? "\n%S\n\n%B %d\n%A" : "\n%S\n\n%B %d\n%A", tick_time);
      }
    } else { //no seconds
      if (settings[AppKeyDateFormat] == 1) {
        strftime(s_buffer_date, sizeof(s_buffer_date), clock_is_24h_style() ? "\n\n\n%d %B\n%A" : "\n\n\n%d %B\n%A", tick_time);
      } else {
        strftime(s_buffer_date, sizeof(s_buffer_date), clock_is_24h_style() ? "\n\n\n%B %d\n%A" : "\n\n\n%B %d\n%A", tick_time);
      }
    }
  }
  // Display this time on the TextLayer
  text_layer_set_text(s_shaketime_layer, s_buffer_time+(('0' == s_buffer_time[0])?1:0));
  text_layer_set_text(s_shakedate_layer, s_buffer_date);
}

static void tick_handler_shake(struct tm *tick_time, TimeUnits units_changed) {
  update_time_shake();
}

static void init_shake_window () {
  s_shaketime_layer = text_layer_create(GRect(13, 13, 118, 142));
  s_shakedate_layer = text_layer_create(GRect(13, 18, 118, 142));
  layer_set_hidden(text_layer_get_layer(s_shaketime_layer), true);
  layer_set_hidden(text_layer_get_layer(s_shakedate_layer), true);
  text_layer_set_background_color(s_shaketime_layer, GColorWhite);
  text_layer_set_background_color(s_shakedate_layer, GColorClear);
  text_layer_set_text_color(s_shaketime_layer, GColorBlack);
  text_layer_set_text_color(s_shakedate_layer, GColorBlack);
  //text_layer_set_text(s_shake_layer, "\n00:00");
  text_layer_set_text_alignment(s_shaketime_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_shakedate_layer, GTextAlignmentCenter);
  text_layer_set_font(s_shaketime_layer, fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS));
  text_layer_set_font(s_shakedate_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_shaketime_layer));
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_shakedate_layer));
}

static void draw_shake_window (AccelAxisType axis, int32_t direction) {
  if (show_shake==false) {
    show_shake=true;
    layer_set_hidden(text_layer_get_layer(s_shaketime_layer), false);
    layer_set_hidden(text_layer_get_layer(s_shakedate_layer), false);
    update_time_shake();
    if (settings[AppKeyShakeWindow] > 1) {
      tick_timer_service_subscribe(SECOND_UNIT, tick_handler_shake);
    }
    APP_LOG(APP_LOG_LEVEL_INFO, "Shake Timeout calculated:%d ", settings[AppKeyShakeTimeout]*1000);
    timer = app_timer_register(settings[AppKeyShakeTimeout]*1000, (AppTimerCallback)timer_callback, NULL);
  }
}


static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  if (settings[AppKeyBluetoothAlarm]>0 || settings[AppKeyBatteryIcon] > 0) {
    s_icon_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ICONS_16));
  }

// background image
    // Create GBitmap
  if (settings[AppKeyBackgroundType]>0) {
    if (settings[AppKeyBackgroundType]==2) {
      s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_0);
    } else if (settings[AppKeyBackgroundType]==3) {
      s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_3);
    } else if (settings[AppKeyBackgroundType]==4) {
      s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_4);
    } else if (settings[AppKeyBackgroundType]==1) {
      s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_1);
    }
    //create bitmap layer
    s_background_layer = bitmap_layer_create(bounds);
    // Set the bitmap onto the layer and add to the window
    bitmap_layer_set_alignment(s_background_layer, background_bitmap_alignment);
    bitmap_layer_set_compositing_mode(s_background_layer, GCompOpSet);
    bitmap_layer_set_background_color(s_background_layer, color_time_textlayer);
    bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
  }
  
  // Create canvas layer and add to window 
	s_canvas_layer = bitmap_layer_create(bounds);
  //bitmap_layer_set_background_color(s_canvas_layer, color_time_text);
  //bitmap_layer_set_compositing_mode(s_canvas_layer, GCompOpSet);
	layer_set_update_proc(bitmap_layer_get_layer(s_canvas_layer), canvas_update_proc);
	layer_add_child(window_layer, bitmap_layer_get_layer(s_canvas_layer));
  
  if (settings[AppKeyBluetoothAlarm]>0) {
    s_bluetooth_icon_layer = text_layer_create(GRect(3, 168-22, 16, 16+1));
    text_layer_set_font(s_bluetooth_icon_layer, s_icon_font);
    text_layer_set_text_color(s_bluetooth_icon_layer, color_notification_text);
    text_layer_set_background_color(s_bluetooth_icon_layer, GColorClear);
    text_layer_set_text_alignment(s_bluetooth_icon_layer, GTextAlignmentCenter);
    text_layer_set_text(s_bluetooth_icon_layer, "A");
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_bluetooth_icon_layer));
    layer_set_hidden(text_layer_get_layer(s_bluetooth_icon_layer), connection_service_peek_pebble_app_connection() ? true : false);
    
  }
  
  if (settings[AppKeyBatteryIcon] > 0) {
    //s_battery_icon_layer = text_layer_create(GRect(144-16-3, 0, 16, 16));
    s_battery_icon_layer = text_layer_create(GRect(144-16-3, 168-22, 16, 16+1));
    text_layer_set_font(s_battery_icon_layer, s_icon_font);
    text_layer_set_text_color(s_battery_icon_layer, color_notification_text);
    text_layer_set_background_color(s_battery_icon_layer, GColorClear);
    text_layer_set_text_alignment(s_battery_icon_layer, GTextAlignmentCenter);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_icon_layer));
    layer_set_hidden(text_layer_get_layer(s_battery_icon_layer), true);
    battery_callback(battery_state_service_peek());
  }
  
  if (settings[AppKeyCroatianDate] == 1) {
    //static char *croMonths[12];
    croMonths[0] = "Siječanj"; croMonths[1] = "Veljača"; croMonths[2] = "Ožujak"; croMonths[3] = "Travanj"; croMonths[4] = "Svibanj"; croMonths[5] = "Lipanj";
    croMonths[6] = "Srpanj"; croMonths[7] = "Kolovoz"; croMonths[8] = "Rujan"; croMonths[9] = "Listopad"; croMonths[10] = "Studeni"; croMonths[11] = "Prosinac";
    //static char *croDays[7];
    croDays[1] = "Ponedjeljak"; croDays[2] = "Utorak"; croDays[3] = "Srijeda"; croDays[4] = "Četvrtak"; croDays[5] = "Petak"; croDays[6] = "Subota"; croDays[0] = "Nedjelja";
  
  }
  
  init_shake_window();
   
}

static void main_window_unload(Window *window) {
  APP_LOG(APP_LOG_LEVEL_INFO, "entered main_window_unload");

  text_layer_destroy(s_shaketime_layer);
  text_layer_destroy(s_shakedate_layer);
  bitmap_layer_destroy(s_canvas_layer);
  
// destroy background image
    // Destroy GBitmap
  if (settings[AppKeyBackgroundType]>0) {
    gbitmap_destroy(s_background_bitmap);
  }
    //Destroy BitmapLayer
  if (settings[AppKeyBackgroundType]>0) {
    bitmap_layer_destroy(s_background_layer);
  }

  if (settings[AppKeyBluetoothAlarm]>0 || settings[AppKeyBatteryIcon] > 0) {
    fonts_unload_custom_font(s_icon_font);
  }
  
  if (settings[AppKeyBluetoothAlarm]>0) {
    text_layer_destroy(s_bluetooth_icon_layer);
  }
  
  if (settings[AppKeyBatteryIcon] > 0) {
    text_layer_destroy(s_battery_icon_layer);
  }

}

//---------------INIT-----------------------------
static void init() {
  
  APP_LOG(APP_LOG_LEVEL_INFO, "entered init");
  settings_read();
  sprite_size_number=GSize(42, 54);
  number_point_UL=GPoint(26, 26);
  number_point_UR=GPoint(76, 26);
  number_point_DL=GPoint(26, 88);
  number_point_DR=GPoint(76, 88);
  
  s_numbers_bitmap[0] = RESOURCE_ID_IMAGE_NUMBER_0;
  s_numbers_bitmap[1] = RESOURCE_ID_IMAGE_NUMBER_1;
  s_numbers_bitmap[2] = RESOURCE_ID_IMAGE_NUMBER_2;
  s_numbers_bitmap[3] = RESOURCE_ID_IMAGE_NUMBER_3;
  s_numbers_bitmap[4] = RESOURCE_ID_IMAGE_NUMBER_4;
  s_numbers_bitmap[5] = RESOURCE_ID_IMAGE_NUMBER_5;
  s_numbers_bitmap[6] = RESOURCE_ID_IMAGE_NUMBER_6;
  s_numbers_bitmap[7] = RESOURCE_ID_IMAGE_NUMBER_7;
  s_numbers_bitmap[8] = RESOURCE_ID_IMAGE_NUMBER_8;
  s_numbers_bitmap[9] = RESOURCE_ID_IMAGE_NUMBER_9;
  
  color_background=GColorFromHEX(settings[AppKeyColorTimeBackground]);
  color_time_text=GColorFromHEX(settings[AppKeyColorTimeText]);
  color_time_textlayer=GColorClear;
  color_notification_text=GColorFromHEX(settings[AppKeyColorNotificationText]);
  
    // Create main Window element and assign to pointer
  s_main_window = window_create();
  // Set window background color
  window_set_background_color(s_main_window, color_background);


  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

#if DEBUG == 0
  // Make sure the time is displayed from the start
  //update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
#endif
  
  if (settings[AppKeyBluetoothAlarm]>0) {
    // Register for Bluetooth connection updates
    bluetooth_connection_service_subscribe(bluetooth_callback);
    //connection_service_subscribe( (ConnectionHandlers) {.pebble_app_connection_handler = bluetooth_callback} );
  }
  
  if (settings[AppKeyBatteryIcon] > 0) {
    // Register for battery level updates
    battery_state_service_subscribe(battery_callback);
  }
  
  app_message_register_inbox_received(inbox_received_handler);
  //app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  app_message_open(256, 256);
  
  if (settings[AppKeyShakeWindow] > 0) {
    accel_tap_service_subscribe(draw_shake_window);
  }
}

static void deinit() {
  APP_LOG(APP_LOG_LEVEL_INFO, "entered deinit");
  app_timer_cancel(timer);
  
  if (settings[AppKeyShakeWindow] > 0) {
	  accel_tap_service_unsubscribe();
  }
	tick_timer_service_unsubscribe();
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}