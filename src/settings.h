#include <pebble.h>

#define DEBUG 0
#define COLOR_THEME 1 //0 debug mode text layer visible, 1 black with white text, 2 white with black text, 3 backgroung image
#define TIME_SIZE 3 //1 tiny, 2 small, 3 medium, 4 large, 5 huge
#define DATE_SIZE 2 //0 no date, 1 small, 2 medium, 3 large
#define BLUETOOTH_ALARM 2 //0 off, 1 icon, 2 icon+alarm
#define BATTERY_ICON 1 //0 off, 1 10%, 2 20% & 10%
#define DATE_FORMAT 1 //1 - 27 Apr, Wed, 2 - Apr 27, Wed, 3 - Wed, 27 Apr, 4 - Wed, Apr 27
#define CROATIAN_DATE 0 //0 off, 1 on
#define COLOR_TIME_BACKGROUND 1
#define COLOR_TIME_TEXT 16777215
#define COLOR_DATE_BACKGROUND 1
#define COLOR_DATE_TEXT 16777215

//settings match
typedef enum {
  AppKeyTimeSize = 0,
  AppKeyDateSize = 1,
  AppKeyBluetoothAlarm = 2,
  AppKeyBatteryIcon = 3,
  AppKeyDateFormat = 4,
  AppKeyCroatianDate = 5,
  AppKeyColorTimeBackground = 6,
  AppKeyColorTimeText = 7,
  AppKeyColorDateBackground = 8,
  AppKeyColorDateText = 9  
} AppKey;

static int settings[10]={TIME_SIZE, DATE_SIZE, BLUETOOTH_ALARM, BATTERY_ICON, DATE_FORMAT, CROATIAN_DATE, COLOR_TIME_BACKGROUND, COLOR_TIME_TEXT, COLOR_DATE_BACKGROUND, COLOR_DATE_TEXT};

static void settings_read () {
  if (persist_exists(AppKeyTimeSize)) {
    settings[AppKeyTimeSize] = persist_read_int(AppKeyTimeSize);
  }
  if (persist_exists(AppKeyDateSize)) {
    settings[AppKeyDateSize] = persist_read_int(AppKeyDateSize);
  }
  if (persist_exists(AppKeyBluetoothAlarm)) {
    settings[AppKeyBluetoothAlarm] = persist_read_int(AppKeyBluetoothAlarm);
  }
  if (persist_exists(AppKeyBatteryIcon)) {
    settings[AppKeyBatteryIcon] = persist_read_int(AppKeyBatteryIcon);
  }
  if (persist_exists(AppKeyDateFormat)) {
    settings[AppKeyDateFormat] = persist_read_int(AppKeyDateFormat);
  }
  if (persist_exists(AppKeyCroatianDate)) {
    settings[AppKeyCroatianDate] = persist_read_int(AppKeyCroatianDate);
  }
  if (persist_exists(AppKeyColorTimeBackground)) {
    settings[AppKeyColorTimeBackground] = persist_read_int(AppKeyColorTimeBackground);
  }
  if (persist_exists(AppKeyColorTimeText)) {
    settings[AppKeyColorTimeText] = persist_read_int(AppKeyColorTimeText);
  }
  if (persist_exists(AppKeyColorDateBackground)) {
    settings[AppKeyColorDateBackground] = persist_read_int(AppKeyColorDateBackground);
  }
  if (persist_exists(AppKeyColorDateText)) {
    settings[AppKeyColorDateText] = persist_read_int(AppKeyColorDateText);
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "settings: ");
  APP_LOG(APP_LOG_LEVEL_INFO, "TimeSize:%d ", settings[AppKeyTimeSize]);
  APP_LOG(APP_LOG_LEVEL_INFO, "DateSize:%d ", settings[AppKeyDateSize]);
  APP_LOG(APP_LOG_LEVEL_INFO, "BluetoothAlarm:%d ", settings[AppKeyBluetoothAlarm]);
  APP_LOG(APP_LOG_LEVEL_INFO, "BatteryIcon:%d ", settings[AppKeyBatteryIcon]);
  APP_LOG(APP_LOG_LEVEL_INFO, "DateFormat:%d ", settings[AppKeyDateFormat]);
  APP_LOG(APP_LOG_LEVEL_INFO, "CroatianDate:%d ", settings[AppKeyCroatianDate]);
  APP_LOG(APP_LOG_LEVEL_INFO, "ColorTimeBackground:%d ", settings[AppKeyColorTimeBackground]);
  APP_LOG(APP_LOG_LEVEL_INFO, "ColorTimeText:%d ", settings[AppKeyColorTimeText]);
  APP_LOG(APP_LOG_LEVEL_INFO, "ColorDateBackground:%d ", settings[AppKeyColorDateBackground]);
  APP_LOG(APP_LOG_LEVEL_INFO, "ColorDateText:%d ", settings[AppKeyColorDateText]);
}