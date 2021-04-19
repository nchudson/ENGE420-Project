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
#define BUZZER       (4)

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

static volatile uint64_t sysTime = 0;

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

  uint64_t delta = sysTime;
  uint64_t timers[NUM_TIMERS];

  RTC_DS3231 rtc;
  Adafruit_LiquidCrystal lcd(0);
  Adafruit_ADXL343 accel(ACCEL_ID);
  Adafruit_BluefruitLE_SPI ble(BT_CS, BT_IRQ);


  // System and driver initialization
  SystemInit();
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
  lcd.setBacklight(LOW);
  lcd.print("Hello, World!");

  sysTime = millis();
  delta = sysTime;
  // Infinite loop of science!
}
void loop() {

}
// int main() {
//   // Local variables
// 
//   uint64_t delta = sysTime;
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
//   sysTime = millis();
//   delta = sysTime;
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