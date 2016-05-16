#include <pebble.h>
#include "settings.h"

static Window *s_main_window;
static TextLayer *s_date_layer, *s_battery_icon_layer, *s_bluetooth_icon_layer, *s_shake_layer;
static GFont s_icon_font;
static GColor color_background, color_time_text, color_time_textlayer, color_date_text, color_date_textlayer, color_date_background;
static BitmapLayer *date_background_layer, *s_background_layer;
static BitmapLayer *s_canvas_layer;
static GBitmap *s_background_bitmap;
static uint32_t s_numbers_bitmap[10];
static GSize sprite_size_number;
static GPoint number_point_UL, number_point_UR, number_point_DL, number_point_DR;
static GAlign background_bitmap_alignment;
static char *croMonths[12];
static char *croDays[7];
AppTimer *timer;
static int show_shake_delay = 6000;

static void init();
//static void deinit();

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *timeSize = dict_find(iter, AppKeyTimeSize);
  Tuple *dateSize = dict_find(iter, AppKeyDateSize);
  Tuple *bluetoothAlarm = dict_find(iter, AppKeyBluetoothAlarm);
  Tuple *batteryIcon = dict_find(iter, AppKeyBatteryIcon);
  Tuple *dateFormat = dict_find(iter, AppKeyDateFormat);
  Tuple *croatianDate = dict_find(iter, AppKeyCroatianDate);
  Tuple *colorTimeBackground = dict_find(iter, AppKeyColorTimeBackground);
  Tuple *colorTimeText = dict_find(iter, AppKeyColorTimeText);
  Tuple *colorDateBackground = dict_find(iter, AppKeyColorDateBackground);
  Tuple *colorDateText = dict_find(iter, AppKeyColorDateText);

  if (timeSize) {
    persist_write_int(AppKeyTimeSize, timeSize->value->int32);
  }
  if (dateSize) {
    persist_write_int(AppKeyDateSize, dateSize->value->int32);
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
  if (colorDateBackground) {
    persist_write_int(AppKeyColorDateBackground, colorDateBackground->value->int32);
  }
  if (colorDateText) {
    persist_write_int(AppKeyColorDateText, colorDateText->value->int32);
  }
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
  int hour = tick_time->tm_hour;
	int min = tick_time->tm_min;
  // Convert hours to 12 hours if the user prefers
	if (clock_is_24h_style() == false && hour > 12) {
		hour -= 12;
	}
  //APP_LOG(APP_LOG_LEVEL_INFO, "canvas update proc, time is: %d : %d", hour, min);
  draw_number(ctx, number_point_UL, (int) (hour / 10));
  draw_number(ctx, number_point_UR, (int) (hour % 10));
  draw_number(ctx, number_point_DL, (int) (min / 10));
  draw_number(ctx, number_point_DR, (int) (min % 10));
  
 if (settings[AppKeyDateSize]>0) {
  // Copy date into buffer from tm structure
  static char date_buffer[16];
  if (settings[AppKeyCroatianDate] == 0) {
    switch (settings[AppKeyDateFormat]) { //1 - 27 Apr, Wed, 2 - Apr 27, Wed, 3 - Wed, 27 Apr, 4 - Wed, Apr 27
      case 2 : strftime(date_buffer, sizeof(date_buffer), "%b %d, %a", tick_time);break;
      case 3 : strftime(date_buffer, sizeof(date_buffer), "%a, %d %b", tick_time);break;
      case 4 : strftime(date_buffer, sizeof(date_buffer), "%a, %b %d", tick_time);break;
      default : strftime(date_buffer, sizeof(date_buffer), "%d %b, %a", tick_time);
    }
  } else {
    
    switch (settings[AppKeyDateFormat]) { //1 - 27 Apr, Wed, 2 - Apr 27, Wed, 3 - Wed, 27 Apr, 4 - Wed, Apr 27
      case 2 : snprintf(date_buffer, sizeof(date_buffer), "%s %d, %s", croMonths[tick_time->tm_mon], tick_time->tm_mday, croDays[tick_time->tm_wday]);break;
      case 3 : snprintf(date_buffer, sizeof(date_buffer), "%s, %d %s", croDays[tick_time->tm_wday], tick_time->tm_mday, croMonths[tick_time->tm_mon]);break;
      case 4 : snprintf(date_buffer, sizeof(date_buffer), "%s, %s %d", croDays[tick_time->tm_wday], croMonths[tick_time->tm_mon], tick_time->tm_mday);break;
      default : snprintf(date_buffer, sizeof(date_buffer), "%d %s, %s", tick_time->tm_mday, croMonths[tick_time->tm_mon], croDays[tick_time->tm_wday]);
    }
  }
  // Show the date
  text_layer_set_text(s_date_layer, date_buffer);
 }
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
  text_layer_destroy(s_shake_layer);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void update_time_shake () {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  // Write the current hours and minutes into a buffer
  static char s_buffer[26];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M:%S\n\n%d %b\n%A" : "%I:%M:%S\n\n%d %b\n%A", tick_time);
  // Display this time on the TextLayer
  text_layer_set_text(s_shake_layer, s_buffer+(('0' == s_buffer[0])?1:0));
}

static void tick_handler_shake(struct tm *tick_time, TimeUnits units_changed) {
  update_time_shake();
}

static void draw_shake_window (AccelAxisType axis, int32_t direction) {
  s_shake_layer = text_layer_create(GRect(24, 24, 96, 120));
  text_layer_set_background_color(s_shake_layer, GColorWhite);
  text_layer_set_text_color(s_shake_layer, GColorBlack);
  text_layer_set_text(s_shake_layer, "\n00:00");
  text_layer_set_text_alignment(s_shake_layer, GTextAlignmentCenter);
  text_layer_set_font(s_shake_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_shake_layer));
  update_time_shake();
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler_shake);
  timer = app_timer_register(show_shake_delay, (AppTimerCallback)timer_callback, NULL);
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
  if (settings[AppKeyTimeSize]>0) {
    if (settings[AppKeyTimeSize]==2) {
      s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_0);
    } else if (settings[AppKeyTimeSize]==3) {
      s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_3);
    } else if (settings[AppKeyTimeSize]==4) {
      s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_4);
    } else if (settings[AppKeyTimeSize]==1) {
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

  
  if (settings[AppKeyDateSize]>0) {
    date_background_layer = bitmap_layer_create(GRect(0, 144, 144, 24));
    bitmap_layer_set_background_color(date_background_layer, color_date_background);
    layer_add_child(window_layer, bitmap_layer_get_layer(date_background_layer));
  // Create date TextLayer
    if (settings[AppKeyDateSize]==1) {
      s_date_layer = text_layer_create(GRect(0, bounds.size.h-24+2, 144, 16));
      text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    } else if (settings[AppKeyDateSize]==2) {
      s_date_layer = text_layer_create(GRect(0, bounds.size.h-24, 144, 20));
      text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    } else { //datesize==3
      s_date_layer = text_layer_create(GRect(0, bounds.size.h-24-6, 144, 26));
      text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    }
      text_layer_set_text_color(s_date_layer, color_date_text);
      text_layer_set_background_color(s_date_layer, color_date_textlayer);
      text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
      text_layer_set_text(s_date_layer, "1 May, Sun");
      layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  }
  
  if (settings[AppKeyBluetoothAlarm]>0) {
    s_bluetooth_icon_layer = text_layer_create(GRect(3, 168-22, 16, 16+1));
    text_layer_set_font(s_bluetooth_icon_layer, s_icon_font);
    text_layer_set_text_color(s_bluetooth_icon_layer, color_date_text);
    text_layer_set_background_color(s_bluetooth_icon_layer, color_date_textlayer);
    text_layer_set_text_alignment(s_bluetooth_icon_layer, GTextAlignmentCenter);
    text_layer_set_text(s_bluetooth_icon_layer, "A");
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_bluetooth_icon_layer));
    layer_set_hidden(text_layer_get_layer(s_bluetooth_icon_layer), connection_service_peek_pebble_app_connection() ? true : false);
    
  }
  
  if (settings[AppKeyBatteryIcon] > 0) {
    //s_battery_icon_layer = text_layer_create(GRect(144-16-3, 0, 16, 16));
    s_battery_icon_layer = text_layer_create(GRect(144-16-3, 168-22, 16, 16+1));
    text_layer_set_font(s_battery_icon_layer, s_icon_font);
    text_layer_set_text_color(s_battery_icon_layer, color_date_text);
    text_layer_set_background_color(s_battery_icon_layer, color_date_textlayer);
    text_layer_set_text_alignment(s_battery_icon_layer, GTextAlignmentCenter);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_icon_layer));
    layer_set_hidden(text_layer_get_layer(s_battery_icon_layer), true);
    battery_callback(battery_state_service_peek());
  }
  
  if (settings[AppKeyCroatianDate] == 1) {
    //static char *croMonths[12];
    croMonths[0] = "Sij"; croMonths[1] = "Vlj"; croMonths[2] = "Ožu"; croMonths[3] = "Tra"; croMonths[4] = "Svi"; croMonths[5] = "Lip";
    croMonths[6] = "Srp"; croMonths[7] = "Kol"; croMonths[8] = "Ruj"; croMonths[9] = "Lis"; croMonths[10] = "Stu"; croMonths[11] = "Pro";
    //static char *croDays[7];
    croDays[1] = "Pon"; croDays[2] = "Uto"; croDays[3] = "Sri"; croDays[4] = "Čet"; croDays[5] = "Pet"; croDays[6] = "Sub"; croDays[0] = "Ned";
  
  }
   
}

static void main_window_unload(Window *window) {
  APP_LOG(APP_LOG_LEVEL_INFO, "entered main_window_unload");
  // Destroy TextLayers
/*  text_layer_destroy(s_time_layer2);
  text_layer_destroy(s_time_layer); */
  text_layer_destroy(s_shake_layer);
  bitmap_layer_destroy(s_canvas_layer);
  
  if (settings[AppKeyDateSize]>0) {
    text_layer_destroy(s_date_layer);
  }
  
// destroy background image
    // Destroy GBitmap
    gbitmap_destroy(s_background_bitmap);
    //Destroy BitmapLayer
    bitmap_layer_destroy(s_background_layer);

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
  color_date_text=GColorFromHEX(settings[AppKeyColorDateText]);
  color_date_textlayer=GColorClear;
  color_date_background=GColorFromHEX(settings[AppKeyColorDateBackground]);
  
  if (settings[AppKeyDateSize]>0) {
    //time_position_offset_withdate=9;
    //background_bitmap_alignment=GAlignTop;
  } else {
    //time_position_offset_withdate=0;
    //background_bitmap_alignment=GAlignCenter;
  }
  
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
  
  accel_tap_service_subscribe(draw_shake_window);
}

static void deinit() {
  APP_LOG(APP_LOG_LEVEL_INFO, "entered deinit");
  app_timer_cancel(timer);
	accel_tap_service_unsubscribe();
	tick_timer_service_unsubscribe();
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}