Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function() {
  var url;
  if (Pebble.getActiveWatchInfo().platform == "aplite") {
    url = 'http://gogothegogo.github.io/SimpleSquare_Pebble/index_a.html';
  } else {
    url = 'http://gogothegogo.github.io/SimpleSquare_Pebble/index_b.html';
  }

  console.log('Showing configuration page: ' + url);

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));

  console.log('Configuration page returned: ' + JSON.stringify(configData));

  if (configData.timeSize) {
    Pebble.sendAppMessage({
      timeSize: parseInt(configData.timeSize),
      dateSize: parseInt(configData.dateSize),
      bluetoothAlarm: parseInt(configData.bluetoothAlarm),
      batteryIcon: parseInt(configData.batteryIcon),
      dateFormat: parseInt(configData.dateFormat),
      croatianDate: parseInt(configData.croatianDate),
      colorTimeBackground: parseInt(configData.colorTimeBackground, 16),
      colorTimeText: parseInt(configData.colorTimeText, 16),
      colorDateBackground: parseInt(configData.colorDateBackground, 16),
      colorDateText: parseInt(configData.colorDateText, 16)
    }, function() {
      console.log('Send successful!');
    }, function() {
      console.log('Send failed!');
    });
  }
});