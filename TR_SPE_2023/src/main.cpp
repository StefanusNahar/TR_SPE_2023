#include <device.h>

void WifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.print("System connected with IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("RSSI: %d\n", WiFi.RSSI());
}

void whiteMode() {
  Serial.println("Entering white mode");
  // Set all LEDs to white
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.show();
}

void blueMode() { //aquamarine_mode
  Serial.println("Entering Blue mode");
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();
}

void redMode() { //crimson_mode
  Serial.println("Entering Red mode");
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
}

void purpleMode() { //amethyst_mode
  Serial.println("Entering Purple mode");
  fill_solid(leds, NUM_LEDS, CRGB::DarkViolet);
  FastLED.show();
}

void mintMode() { //green
Serial.println("Entering Mint mode");
fill_solid(leds, NUM_LEDS, CRGB::Green);
FastLED.show();
}

void sunflowerMode() {
  Serial.println("Entering Sunflower mode");
  fill_solid(leds, NUM_LEDS, CRGB::Yellow);
  FastLED.show();
}

void warmMode() {
  Serial.println("Entering warm mode");
  // Set all LEDs to a warm color (e.g., soft orange)
  fill_solid(leds, NUM_LEDS, CRGB(255, 255, 128));  // Adjust the RGB values for the desired warm color
  FastLED.show();
}

void rgbCycleMode() {
  Serial.println("Entering RGB cycle mode");
  // Set the RGB cycle mode flag to true
  rgbCycleFlag = true;
  do {
    for (int i = 0; i < 256; ++i) {
      for (int j = 0; j < NUM_LEDS; ++j) {
        leds[j] = CHSV((i + j * 2) % 256, 255, 255);
      }
      FastLED.show();
      delay(10);  // Adjust the delay to control the speed of color cycling
    }
  } while (rgbCycleFlag);
  // Clear the RGB cycle mode flag when the loop exits
  rgbCycleFlag = false;
}

void showTime() {
  time_t currentTime = time(nullptr);
  struct tm *localTime = localtime(&currentTime);

  char formattedDay[20];
  strftime(formattedDay, sizeof(formattedDay), "%A", localTime);

  char formattedDate[20];
  strftime(formattedDate, sizeof(formattedDate), "%d %b %Y", localTime); // Format for day, day-month-year

  char formattedTime[20];
  strftime(formattedTime, sizeof(formattedTime), "%H:%M", localTime);

  // Serial.println(formattedDate);
  // Serial.println(formattedTime);

  // Determine whether it's morning or night
  bool isMorning = localTime->tm_hour >= 5 && localTime->tm_hour < 18;
  display.clearDisplay();

  if (isMorning) {
    display.drawBitmap(96, 1, morning_icon, 32, 32, 1);
  } else {
    display.drawBitmap(96, 1, night_icon, 32, 32, 1);
  }
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);  // Text size for day
  int dayX = 5;  
  int dayY = 1;  

  display.setCursor(dayX, dayY);
  display.println(formattedDay);

  display.setTextSize(1);  // Text size for date
  int dateX = 5;  
  int dateY = 10;  

  display.setCursor(dateX, dateY);
  display.println(formattedDate);

  display.setTextSize(3);  // Text size for time
  int timeX = 5;
  int timeY = 32;

  // Print the formatted time on the OLED display
  display.setCursor(timeX, timeY);
  display.println(formattedTime);

  display.display();
}

void readAHTSensor() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  // display temperature
  display.drawBitmap(30, 5, temperature_icon, 16, 16 ,1);
  display.setTextSize(3);
  display.setCursor(50,5);
  display.print(temperature.temperature, 0);
  display.print("");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");

  // display humidity
  display.drawBitmap(30, 40, humidity_icon, 16, 16 ,1);
  display.setTextSize(3);
  display.setCursor(50, 40);
  display.print(humidity.relative_humidity, 0);
  display.print("%"); 

  display.display();
}

void setup() {
  Serial.begin(115200);
  WifiConnect(); 
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  Wire.begin();
  configTime(7 * 3600, 0, "pool.ntp.org");  // Set the UTC offset for Jakarta time and NTP server
  pinMode(OLED_SW, INPUT_PULLUP);
  // Initialize with the I2C oled
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0X3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  // Initialize AHT10 sensor
  if (!aht.begin()) {
    Serial.println(F("Couldn't find AHT10 sensor!"));
    for (;;) {}
  }
  display.clearDisplay();
  Serial.println(" & Connecting to Firebase...");
  Firebase_Init("Lamp/cmd");
  Serial.println("System ready.");
}
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastAHT10data >= 1000) {
    lastAHT10data = currentMillis;
    // Read sensor data
    aht.getEvent(&humidity, &temperature);
    // Set the temperature and humidity values in Firebase
    Firebase.RTDB.setFloat(&fbdo, "Lamp/sensors/temperature", temperature.temperature);
    Firebase.RTDB.setFloat(&fbdo, "Lamp/sensors/humidity", humidity.relative_humidity);
  }
  buttonState = digitalRead(OLED_SW);
  if (buttonState != lastButtonState) {     // Button state has changed
    if (buttonState == LOW) {               // Button is pressed
      OLEDshow = !OLEDshow;
      delay(50);                           // Debounce delay
    }
    lastButtonState = buttonState;
  }

  if (OLEDshow) {
    readAHTSensor();
  } else {
    showTime();
  }
}