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
#include "Adafruit_BluefruitLE_SPI.h"

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

// Pin defines
#define BTN_PLUS     (A1)
#define BTN_MINUS    (A2)
#define BTN_SEL      (A3)
#define BTN_SET      (A4)

#define QI_CHG       (A0)
#define BATT_LOW     (A5)

#define ACCEL_IRQ1   (10)
#define ACCEL_IRQ2   (9)

#define BT_CS        (8)
#define BT_IRQ       (7)

#define LED_WAIT     (6)
#define BUZZER       (3)

// I2C addresses
#define ACCEL_ADDR   (0x3A >> 1)
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
  TIMER_LED,
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
static uint32_t sys_time_temp = 0;
static uint64_t delta = 0;
static uint64_t timers[NUM_TIMERS];
static volatile uint64_t sys_time = 0;
static RTC_DS3231 rtc;
static Adafruit_LiquidCrystal lcd(LCD_ADDR);
static Adafruit_ADXL343 accel(ACCEL_ID);
static Adafruit_BluefruitLE_SPI ble(BT_CS, BT_IRQ);
static DateTime rtc_time;
static char* lcd_time = new char[12];

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
  // Local variables




  // System and driver initialization
  rtc.begin();
  lcd.begin(LCD_WIDTH, LCD_HEIGHT);
  accel.begin(ACCEL_ADDR);

  //Pin configuration
  pinMode(BTN_PLUS, INPUT);
  pinMode(BTN_MINUS, INPUT);
  pinMode(BTN_SEL, INPUT);

  pinMode(QI_CHG, INPUT);
  pinMode(BATT_LOW, INPUT);

  pinMode(ACCEL_IRQ1, INPUT);
  pinMode(ACCEL_IRQ2, INPUT);

  pinMode(LED_WAIT, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  


  digitalWrite(LED_WAIT, LOW);
  // tone(BUZZER, 440);
  lcd.setBacklight(HIGH);

  sys_time = millis();
  delta = sys_time;
  // Infinite loop of science!
}
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
    rtc_time = rtc.now();
    lcd.setCursor(0, 0);
    sprintf_P(lcd_time, "%.2d:%.2d:%.2d", rtc_time.hour(), rtc_time.minute(), rtc_time.second());
    lcd.print(lcd_time);
    lcd.setCursor(0, 1);
    if(!digitalRead(QI_CHG)) {
      lcd.print("    Charging    ");
    }
    else {
      lcd.print("  Not Charging  ");
    }
  }
}
// int main() {
//   // Local variables
// 
//   uint64_t delta = sys_time;
//   uint64_t timers[NUM_TIMERS];
// 
//   RTC_DS3231 rtc;
//   Adafruit_LiquidCrystal lcd(0);
//   Adafruit_ADXL343 accel(ACCEL_ID);
//   Adafruit_BluefruitLE_SPI ble(BT_CS, BT_IRQ);
// 
// 
//   // System and driver initialization
//   SystemInit();
//   rtc.begin();
//   lcd.begin(LCD_WIDTH, LCD_HEIGHT);
//   accel.begin(ACCEL_ADDR);
// 
//   //Pin configuration
//   pinMode(BTN_PLUS, INPUT);
//   pinMode(BTN_MINUS, INPUT);
//   pinMode(BTN_SEL, INPUT);
// 
//   pinMode(QI_CHG, INPUT);
//   pinMode(BATT_LOW, INPUT);
// 
//   pinMode(ACCEL_IRQ1, INPUT);
//   pinMode(ACCEL_IRQ2, INPUT);
// 
//   pinMode(LED_WAIT, OUTPUT);
//   pinMode(BUZZER, OUTPUT);
//   
// 
// 
//   digitalWrite(LED_WAIT, LOW);
//   lcd.setBacklight(LOW);
//   lcd.print("Hello, World!");
// 
//   sys_time = millis();
//   delta = sys_time;
//   // Infinite loop of science!
//   while(1) {
//   }
// }

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