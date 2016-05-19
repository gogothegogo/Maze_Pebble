#include <pebble.h>

#define DEBUG 0
#define COLOR_THEME 1 //0 debug mode text layer visible, 1 black with white text, 2 white with black text, 3 backgroung image
#define BACKGROUND_TYPE 3 //1 tiny, 2 small, 3 medium, 4 large, 5 huge
#define SHAKE_WINDOW 2 //0 no window, 1 date, 2 seconds and date
#define BLUETOOTH_ALARM 2 //0 off, 1 icon, 2 icon+alarm
#define BATTERY_ICON 1 //0 off, 1 10%, 2 20% & 10%
#define DATE_FORMAT 1 //1 - 27 Apr, 2 - Apr 27
#define CROATIAN_DATE 0 //0 off, 1 on
#define COLOR_TIME_BACKGROUND 1
#define COLOR_TIME_TEXT 1
#define COLOR_NOTIF_TEXT 16777215
#define SHAKE_TIMEOUT 3

//settings match
typedef enum {
  AppKeyBackgroundType = 0,
  AppKeyShakeWindow = 1,
  AppKeyBluetoothAlarm = 2,
  AppKeyBatteryIcon = 3,
  AppKeyDateFormat = 4,
  AppKeyCroatianDate = 5,
  AppKeyColorTimeBackground = 6,
  AppKeyColorTimeText = 7,
  AppKeyColorNotificationText = 8,
  AppKeyShakeTimeout = 9
} AppKey;

static int settings[10]={BACKGROUND_TYPE, SHAKE_WINDOW, BLUETOOTH_ALARM, BATTERY_ICON, DATE_FORMAT, CROATIAN_DATE, COLOR_TIME_BACKGROUND, COLOR_TIME_TEXT, COLOR_NOTIF_TEXT, SHAKE_TIMEOUT};

static void settings_read () {
  if (persist_exists(AppKeyBackgroundType)) {
    settings[AppKeyBackgroundType] = persist_read_int(AppKeyBackgroundType);
  }
  if (persist_exists(AppKeyShakeWindow)) {
    settings[AppKeyShakeWindow] = persist_read_int(AppKeyShakeWindow);
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
  if (persist_exists(AppKeyColorNotificationText)) {
    settings[AppKeyColorNotificationText] = persist_read_int(AppKeyColorNotificationText);
  }
  if (persist_exists(AppKeyShakeTimeout)) {
    settings[AppKeyShakeTimeout] = persist_read_int(AppKeyShakeTimeout);
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "settings: ");
  APP_LOG(APP_LOG_LEVEL_INFO, "BackgroundType:%d ", settings[AppKeyBackgroundType]);
  APP_LOG(APP_LOG_LEVEL_INFO, "ShakeWindow:%d ", settings[AppKeyShakeWindow]);
  APP_LOG(APP_LOG_LEVEL_INFO, "BluetoothAlarm:%d ", settings[AppKeyBluetoothAlarm]);
  APP_LOG(APP_LOG_LEVEL_INFO, "BatteryIcon:%d ", settings[AppKeyBatteryIcon]);
  APP_LOG(APP_LOG_LEVEL_INFO, "DateFormat:%d ", settings[AppKeyDateFormat]);
  APP_LOG(APP_LOG_LEVEL_INFO, "CroatianDate:%d ", settings[AppKeyCroatianDate]);
  APP_LOG(APP_LOG_LEVEL_INFO, "ColorTimeBackground:%d ", settings[AppKeyColorTimeBackground]);
  APP_LOG(APP_LOG_LEVEL_INFO, "ColorTimeText:%d ", settings[AppKeyColorTimeText]);
  APP_LOG(APP_LOG_LEVEL_INFO, "ColorNotificationText:%d ", settings[AppKeyColorNotificationText]);
  APP_LOG(APP_LOG_LEVEL_INFO, "ShakeTimeout:%d ", settings[AppKeyShakeTimeout]);
}