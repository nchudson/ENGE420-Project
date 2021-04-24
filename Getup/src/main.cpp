//------------------------------------------------------------------------------
// Getup! Firmware
// Nathaniel Hudson
// nhudson18@georgefox.edu
// 2021-04-17
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include <Arduino.h>
#include "Adafruit_ADXL343.h"
#include "Adafruit_LiquidCrystal.h"
#include "RTClib.h"
#include "RTCZero.h"
#include "Adafruit_BluefruitLE_SPI.h"

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

// Pin defines
#define PIN_PIN_BTN_PLUS     (A1)
#define PIN_PIN_BTN_MINUS    (A2)
#define PIN_PIN_BTN_SEL      (A3)
#define PIN_PIN_BTN_SET      (A4)

#define PIN_QI_CHG       (A0)
#define PIN_BATT_LOW     (A5)

#define PIN_ACCEL_IRQ1   (10)
#define PIN_ACCEL_IRQ2   (9)

#define PIN_BT_CS        (8)
#define PIN_BT_IRQ       (7)

#define PIN_LED_WAIT     (6)
#define PIN_BUZZER       (3)

// I2C addresses
#define ACCEL_ADDR   (0x53)
#define RTC_ADDR     (0x68)
#define LCD_ADDR     (0x20)

// Device parameters

#define ACCEL_ID     (0x00000000)

#define LCD_WIDTH    (16)
#define LCD_HEIGHT   (2)

// Program values
#define BUTTON_UPDATE_TIME  (20)
#define RTC_UPDATE_TIME     (86400000)
#define ACCEL_UPDATE_TIME   (100)
#define LCD_UPDATE_TIME     (100)
#define FSM_UPDATE_TIME     (100)

//------------------------------------------------------------------------------
//     ___      __   ___  __   ___  ___  __
//      |  \ / |__) |__  |  \ |__  |__  /__`
//      |   |  |    |___ |__/ |___ |    .__/
//
//------------------------------------------------------------------------------

typedef enum {
  TIMER_BUTTONS,
  TIMER_RTC,
  TIMER_ACCEL,
  TIMER_BATT,
  TIMER_SPEAKER,
  TIMER_PIN_LED,
  TIMER_LCD,
  TIMER_FSM,
  NUM_TIMERS
} timers_t;

typedef enum {
  SETUP,
  NUM_STATES,
} fsm_t;

typedef enum {
  SET_ALARM,
  NUM_ITEMS
} menu_items_t;



//------------------------------------------------------------------------------
//                __          __        ___  __
//     \  /  /\  |__) |  /\  |__) |    |__  /__`
//      \/  /~~\ |  \ | /~~\ |__) |___ |___ .__/
//
//------------------------------------------------------------------------------
static uint8_t accel_conf = 0;
static uint8_t buttons[]
static uint32_t sys_time_temp = 0;
static uint64_t delta = 0;
static uint64_t timers[NUM_TIMERS];
static volatile uint64_t sys_time = 0;
static RTC_DS3231 rtc_ext;
static RTC rtc_int;
static Adafruit_LiquidCrystal lcd(LCD_ADDR);
static Adafruit_ADXL343 accel(ACCEL_ID);
static Adafruit_BluefruitLE_SPI ble(BT_CS, BT_IRQ);
static DateTime rtc_ext_time;
static char* lcd_line_0 = new char[16];
static char* lcd_line_1 = new char[16];

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

//==============================================================================
void setup() {
  // System and driver initialization
  rtc_ext.begin();
  rtc_int.begin();
  lcd.begin(LCD_WIDTH, LCD_HEIGHT);
  accel_conf = accel.begin(ACCEL_ADDR);

  //Pin configuration
  pinMode(PIN_PIN_BTN_PLUS, INPUT);
  pinMode(PIN_PIN_BTN_MINUS, INPUT);
  pinMode(PIN_PIN_BTN_SEL, INPUT);

  pinMode(PIN_QI_CHG, INPUT);
  pinMode(PIN_BATT_LOW, INPUT);

  pinMode(PIN_ACCEL_IRQ1, INPUT);
  pinMode(PIN_ACCEL_IRQ2, INPUT);

  pinMode(PIN_LED_WAIT, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  
  // Output configuration
  digitalWrite(PIN_LED_WAIT, LOW);
  lcd.setBacklight(HIGH);

  // RTC configuration
  rtc_int.setDate(rtc_ext.day, rtc_ext.month, rtc_ext.year);
  rtc_int.setTime(rtc_ext.hour, rtc_ext.minute, rtc_ext.second);

  sys_time = millis();
  delta = sys_time;
}

//==============================================================================
// Infinite loop of science!
//==============================================================================
void loop() {
  // Get current millis() value and update 64-bit counter.
  if(millis() != sys_time_temp) {
    if(sys_time_temp < (sys_time & 0x00000000FFFFFFFF)) {
      sys_time = ((sys_time & 0xFFFFFFFF00000000) | sys_time_temp) +
        0x0000000100000000;
    }
    else {
      sys_time = (sys_time & 0xFFFFFFFF00000000) | sys_time_temp;
    }
  }

  delta = sys_time - timers[TIMER_LCD];
  if(delta >= LCD_UPDATE_TIME) {
    rtc_ext_time = rtc_ext.now();
    rtc_int_time = rtc_int.time
    sprintf_P(lcd_line_0, "    %.2d:%.2d:%.2d    ",
      rtc_ext_time.hour(), rtc_ext_time.minute(), rtc_ext_time.second());
    if(!digitalRead(PIN_QI_CHG)) {
      lcd_line_1 = "    Charging    ";
    }
    else {
      sprintf_P(lcd_line_1, "x:%.2dy:%.2dz:%.2d",
        accel.getX(), accel.getY(), accel.getZ());
    }
    lcd.setCursor(0, 0);
    lcd.print(lcd_line_0);
    lcd.setCursor(0, 1);
    lcd.print(lcd_line_1);
  }

  if(delta >= FSM_)
}

//------------------------------------------------------------------------------
//      __   __              ___  ___
//     |__) |__) | \  /  /\   |  |__
//     |    |  \ |  \/  /~~\  |  |___
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//        __   __   __
//     | /__` |__) /__`
//     | .__/ |  \ .__/
//
//------------------------------------------------------------------------------

//==============================================================================
// Tick tock tick tock...
//==============================================================================