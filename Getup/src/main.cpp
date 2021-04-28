//------------------------------------------------------------------------------
// Getup! Firmware
// Nathaniel Hudson
// nhudson18@georgefox.edu
// 2021-04-17
//------------------------------------------------------------------------------
// TODO:
// - Display shows current time, updating each second:             x
// - Menu is navigable using buttons and has following structure:
//   - Menu displays current date:
//   - Menu displays current alarms and status:
//   - Menu allows alarms and date to be edited:
// - Alarm rings when time reaches alarm time:
// - Alarm disabled when unit is placed on Qi pad:
// - Alarm temporarily disabled on accelerometer shake:
// - System wakes and sleeps at proper time:
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
#include "Adafruit_SleepyDog.h"
#include "Adafruit_BluefruitLE_SPI.h"

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

// Pin defines
#define PIN_BTN_PLUS        (A1)
#define PIN_BTN_MINUS       (A2)
#define PIN_BTN_SEL         (A3)
#define PIN_BTN_SET         (A4)

#define PIN_QI_CHG          (A0)
#define PIN_BATT_LOW        (A5)

#define PIN_ACCEL_IRQ1      (10)
#define PIN_ACCEL_IRQ2      (9)

#define PIN_BT_CS           (8)
#define PIN_BT_IRQ          (7)

#define PIN_LED_WAIT        (6)
#define PIN_BUZZER          (3)

// I2C addresses
#define ACCEL_ADDR          (0x53)
#define RTC_ADDR            (0x68)
#define LCD_ADDR            (0x20)

// Device parameters

#define ACCEL_ID            (0x00000000)

#define LCD_WIDTH           (16)
#define LCD_HEIGHT          (2)

// Program values
#define BUTTON_UPDATE_TIME  (20)
#define RTC_UPDATE_TIME     (100)
#define ACCEL_UPDATE_TIME   (100)
#define QI_UPDATE_TIME      (100)
#define BATT_UPDATE_TIME    (100)
#define LCD_UPDATE_TIME     (250)
#define FSM_UPDATE_TIME     (20)

#define NUM_ALARMS          (5)
#define NUM_BUTTONS         (4)

#define BTN_PLUS            (0)
#define BTN_MINUS           (1)
#define BTN_SEL             (2)
#define BTN_SET             (3)

#define TIME_LCD_POS_Y      (0)
#define HR_LCD_POS_X        (4)
#define MIN_LCD_POS_X       (7)
#define SEC_LCD_POS_X       (10)

#define DATE_LCD_POS_Y      (1)
#define WORD_LCD_POS_X      (1)
#define YR_LCD_POS_X        (5)
#define MO_LCD_POS_X        (10)
#define DY_LCD_POS_X        (13)

#define HOUR                (3600)
#define MINUTE              (60)
#define SECOND              (1)
#define UPDATE_DELAY        (500)
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
  TIMER_QI,
  TIMER_BATT,
  TIMER_SPEAKER,
  TIMER_PIN_LED,
  TIMER_LCD,
  TIMER_FSM,
  NUM_TIMERS
} timers_t;

typedef enum {
  MENU_DATE,
  MENU_ALM,
  MENU_SET_DATE_HR,
  MENU_SET_DATE_MIN,
  MENU_SET_DATE_SEC,
  MENU_SET_DATE_YR,
  MENU_SET_DATE_MO,
  MENU_SET_DATE_DY,
  MENU_ALM_SET,
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
static volatile uint8_t change_sleep_mode = 0;
static RTC_DS3231 rtc_ext;
static RTCZero rtc_int;
static WatchdogSAMD wdt;
static Adafruit_LiquidCrystal lcd(LCD_ADDR);
static Adafruit_ADXL343 accel(ACCEL_ID);
static Adafruit_BluefruitLE_SPI ble(PIN_BT_CS, PIN_BT_IRQ);
static DateTime rtc_ext_time;

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------

void button_isr();
static char* to_weekday(uint8_t day_of_week);

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
  //wdt.enable(5000);

  //Pin configuration
  pinMode(PIN_BTN_PLUS, INPUT);
  pinMode(PIN_BTN_MINUS, INPUT);
  pinMode(PIN_BTN_SEL, INPUT);
  pinMode(PIN_BTN_SET, INPUT);

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
  rtc_ext_time = rtc_ext.now();
  rtc_int.setDate(rtc_ext_time.day(), rtc_ext_time.month(), rtc_ext_time.year());
  rtc_int.setTime(rtc_ext_time.hour(), rtc_ext_time.minute(), rtc_ext_time.second());
  rtc_int.setAlarmSeconds((rtc_int.getSeconds() + 1) % 60);
  rtc_int.enableAlarm(rtc_int.MATCH_SS);

}

//==============================================================================
// Infinite loop of science!
//==============================================================================
void loop() {
  // Local Variables.
  static uint8_t sleep_mode = 0;
  static uint8_t charging = 0;
  static uint8_t buttons[NUM_BUTTONS];
  static uint8_t buttons_d[NUM_BUTTONS];
  static uint8_t buttons_risen[NUM_BUTTONS];
  static uint32_t sys_time_tmp = 0;
  static uint32_t i;
  static uint64_t sys_time = 0;
  static uint64_t delta = 0;
  static uint64_t update_time = 0;
  static uint64_t timers[NUM_TIMERS];
  static char* lcd_line_0 = new char[16];
  static char* lcd_line_1 = new char[16];
  static fsm_t fsm_state = MENU_DATE;
  static DateTime time_tmp;
  static DateTime alarms[NUM_ALARMS];
  static TimeSpan offset;

  // Get current millis() value and update 64-bit counter.
  if(millis() != sys_time_tmp) {
    sys_time_tmp = millis();
    if(sys_time_tmp < (sys_time & 0x00000000FFFFFFFF)) {
      sys_time = ((sys_time & 0xFFFFFFFF00000000) | sys_time_tmp) +
        0x0000000100000000;
    }
    else {
      sys_time = (sys_time & 0xFFFFFFFF00000000) | sys_time_tmp;
    }
  }

  delta = sys_time - timers[TIMER_BUTTONS];
  if(delta >= BUTTON_UPDATE_TIME) {
    timers[TIMER_BUTTONS] = sys_time;

    for(i = 0; i < NUM_BUTTONS; i++) {
      buttons_d[i] = buttons[i];
    }

    buttons[BTN_PLUS] = digitalRead(PIN_BTN_PLUS);
    buttons[BTN_MINUS] = digitalRead(PIN_BTN_MINUS);
    buttons[BTN_SEL] = digitalRead(PIN_BTN_SEL);
    buttons[BTN_SET] = digitalRead(PIN_BTN_SET);

    for(i = 0; i < NUM_BUTTONS; i++) {
      buttons_risen[i] = buttons[i] && !buttons_d[i];
    }

  }

  delta = sys_time - timers[TIMER_RTC];
  if(delta >= RTC_UPDATE_TIME) {
    timers[TIMER_RTC] = sys_time;
    rtc_ext_time = rtc_ext.now();
  }
  
  delta = sys_time - timers[TIMER_QI];
  if(delta >= QI_UPDATE_TIME) {
    timers[TIMER_QI] = sys_time;
    charging = !digitalRead(PIN_QI_CHG);
  }
  delta = sys_time - timers[TIMER_BATT];
  if(delta >= BATT_UPDATE_TIME) {
    timers[TIMER_BATT] = sys_time;
  }

  delta = sys_time - timers[TIMER_LCD];
  if(delta >= LCD_UPDATE_TIME) {
    timers[TIMER_LCD] = sys_time;
    lcd.noCursor();
    lcd.setCursor(0, 0);
    lcd.print(lcd_line_0);
    lcd.setCursor(0, 1);
    lcd.print(lcd_line_1);
  }

  delta = sys_time - timers[TIMER_FSM];
  if(delta >= FSM_UPDATE_TIME) {
    timers[TIMER_FSM] = sys_time;
    switch(fsm_state) {
      case MENU_DATE:
        sprintf_P(lcd_line_0, "    %.2d:%.2d:%.2d    ",
          rtc_ext_time.hour(), rtc_ext_time.minute(), rtc_ext_time.second());
        sprintf_P(lcd_line_1, " %s %.4d-%.2d-%.2d ",
          to_weekday(rtc_ext_time.dayOfTheWeek()), rtc_ext_time.year(),
          rtc_ext_time.month(), rtc_ext_time.day());
        if(buttons_risen[BTN_SET]) {
          time_tmp = rtc_ext_time;
          fsm_state = MENU_SET_DATE_HR;
        }
        break;
      case MENU_SET_DATE_HR:
        sprintf_P(lcd_line_0, "    %.2d:%.2d:%.2d    ",
          time_tmp.hour(), time_tmp.minute(), time_tmp.second());
        sprintf_P(lcd_line_1, " %s %.4d-%.2d-%.2d ",
          to_weekday(time_tmp.dayOfTheWeek()), time_tmp.year(),
          time_tmp.month(), time_tmp.day());
        lcd.setCursor(HR_LCD_POS_X, TIME_LCD_POS_Y);
        lcd.cursor();

        if(buttons_risen[BTN_PLUS] ||
          (buttons[BTN_PLUS] && (sys_time >= update_time))) {
          offset = TimeSpan(HOUR);
          time_tmp = time_tmp + offset;
          update_time = sys_time + UPDATE_DELAY;
        }
        if(buttons_risen[BTN_MINUS] ||
          (buttons[BTN_MINUS] && (sys_time >= update_time))) {
          offset = TimeSpan(HOUR);
          time_tmp = time_tmp - offset;
          update_time = sys_time + UPDATE_DELAY;
        }
        else if(buttons_risen[BTN_SEL]) {
          fsm_state = MENU_SET_DATE_MIN;
        }
        else if(buttons_risen[BTN_SET]) {
          rtc_ext.adjust(time_tmp);
          lcd.noCursor();
          fsm_state = MENU_DATE;
        }
        break;
      case MENU_SET_DATE_MIN:
        sprintf_P(lcd_line_0, "    %.2d:%.2d:%.2d    ",
          time_tmp.hour(), time_tmp.minute(), time_tmp.second());
        sprintf_P(lcd_line_1, " %s %.4d-%.2d-%.2d ",
          to_weekday(time_tmp.dayOfTheWeek()), time_tmp.year(),
          time_tmp.month(), time_tmp.day());
        lcd.setCursor(MIN_LCD_POS_X, TIME_LCD_POS_Y);
        lcd.cursor();

        if(buttons_risen[BTN_PLUS] ||
          (buttons[BTN_PLUS] && (sys_time >= update_time))) {
          offset = TimeSpan(MINUTE);
          time_tmp = time_tmp + offset;
          update_time = sys_time + UPDATE_DELAY;
        }
        if(buttons_risen[BTN_MINUS] ||
          (buttons[BTN_MINUS] && (sys_time >= update_time))) {
          offset = TimeSpan(MINUTE);
          time_tmp = time_tmp - offset;
          update_time = sys_time + UPDATE_DELAY;
        }
        else if(buttons_risen[BTN_SEL]) {
          fsm_state = MENU_SET_DATE_SEC;
        }
        else if(buttons_risen[BTN_SET]) {
          rtc_ext.adjust(time_tmp);
          lcd.noCursor();
          fsm_state = MENU_DATE;
        }
        break;
      case MENU_SET_DATE_SEC:
        sprintf_P(lcd_line_0, "    %.2d:%.2d:%.2d    ",
          time_tmp.hour(), time_tmp.minute(), time_tmp.second());
        sprintf_P(lcd_line_1, " %s %.4d-%.2d-%.2d ",
          to_weekday(time_tmp.dayOfTheWeek()), time_tmp.year(),
          time_tmp.month(), time_tmp.day());
        lcd.setCursor(SEC_LCD_POS_X, TIME_LCD_POS_Y);
        lcd.cursor();

        if(buttons_risen[BTN_PLUS] ||
          (buttons[BTN_PLUS] && (sys_time >= update_time))) {
          offset = TimeSpan(SECOND);
          time_tmp = time_tmp + offset;
          update_time = sys_time + UPDATE_DELAY;
        }
        if(buttons_risen[BTN_MINUS] ||
          (buttons[BTN_MINUS] && (sys_time >= update_time))) {
          offset = TimeSpan(SECOND);
          time_tmp = time_tmp - offset;
          update_time = sys_time + UPDATE_DELAY;
        }
        else if(buttons_risen[BTN_SEL]) {
          fsm_state = MENU_SET_DATE_YR;
        }
        else if(buttons_risen[BTN_SET]) {
          rtc_ext.adjust(time_tmp);
          lcd.noCursor();
          fsm_state = MENU_DATE;
        }
        break;
      case MENU_SET_DATE_YR:
        sprintf_P(lcd_line_0, "    %.2d:%.2d:%.2d    ",
          time_tmp.hour(), time_tmp.minute(), time_tmp.second());
        sprintf_P(lcd_line_1, " %s %.4d-%.2d-%.2d ",
          to_weekday(time_tmp.dayOfTheWeek()), time_tmp.year(),
          time_tmp.month(), time_tmp.day());
        lcd.setCursor(YR_LCD_POS_X, DATE_LCD_POS_Y);
        lcd.cursor();

        if(buttons_risen[BTN_PLUS] ||
          (buttons[BTN_PLUS] && (sys_time >= update_time))) {
          if(time_tmp.year() < 2099) {
            time_tmp = DateTime(time_tmp.year() + 1, time_tmp.month(),
              time_tmp.day(), time_tmp.hour(),
              time_tmp.minute(), time_tmp.second());
          }
          else {
            time_tmp = DateTime(2000, time_tmp.month(),
              time_tmp.day(), time_tmp.hour(),
              time_tmp.minute(), time_tmp.second());
          }

          update_time = sys_time + UPDATE_DELAY;
        }
        if(buttons_risen[BTN_MINUS] ||
          (buttons[BTN_MINUS] && (sys_time >= update_time))) {
          if(time_tmp.year() > 2000) {
            time_tmp = DateTime(time_tmp.year() - 1, time_tmp.month(),
              time_tmp.day(), time_tmp.hour(),
              time_tmp.minute(), time_tmp.second());
          }
          else {
            time_tmp = DateTime(2099, time_tmp.month(),
              time_tmp.day(), time_tmp.hour(),
              time_tmp.minute(), time_tmp.second());
          }
          
          update_time = sys_time + UPDATE_DELAY;
        }
        else if(buttons_risen[BTN_SEL]) {
          fsm_state = MENU_SET_DATE_MO;
        }
        else if(buttons_risen[BTN_SET]) {
          rtc_ext.adjust(time_tmp);
          lcd.noCursor();
          fsm_state = MENU_DATE;
        }
        break;
      case MENU_SET_DATE_MO:
        sprintf_P(lcd_line_0, "    %.2d:%.2d:%.2d    ",
          time_tmp.hour(), time_tmp.minute(), time_tmp.second());
        sprintf_P(lcd_line_1, " %s %.4d-%.2d-%.2d ",
          to_weekday(time_tmp.dayOfTheWeek()), time_tmp.year(),
          time_tmp.month(), time_tmp.day());
        lcd.setCursor(MO_LCD_POS_X, DATE_LCD_POS_Y);
        lcd.cursor();

        if(buttons_risen[BTN_PLUS] ||
          (buttons[BTN_PLUS] && (sys_time >= update_time))) {
          if(time_tmp.month() < 12) {
            time_tmp = DateTime(time_tmp.year(), time_tmp.month() + 1,
              time_tmp.day(), time_tmp.hour(),
              time_tmp.minute(), time_tmp.second());
          }
          else {
            time_tmp = DateTime(time_tmp.year(), 0,
              time_tmp.day(), time_tmp.hour(),
              time_tmp.minute(), time_tmp.second());
          }

          update_time = sys_time + UPDATE_DELAY;
        }
        if(buttons_risen[BTN_MINUS] ||
          (buttons[BTN_MINUS] && (sys_time >= update_time))) {
          if(time_tmp.month() > 1) {
            time_tmp = DateTime(time_tmp.year(), time_tmp.month() - 1,
              time_tmp.day(), time_tmp.hour(),
              time_tmp.minute(), time_tmp.second());
          }
          else {
            time_tmp = DateTime(time_tmp.year(), 12,
              time_tmp.day(), time_tmp.hour(),
              time_tmp.minute(), time_tmp.second());
          }
          
          update_time = sys_time + UPDATE_DELAY;
        }
        else if(buttons_risen[BTN_SEL]) {
          fsm_state = MENU_SET_DATE_DY;
        }
        else if(buttons_risen[BTN_SET]) {
          rtc_ext.adjust(time_tmp);
          lcd.noCursor();
          fsm_state = MENU_DATE;
        }
        break;
      case MENU_SET_DATE_DY:
        sprintf_P(lcd_line_0, "    %.2d:%.2d:%.2d    ",
          time_tmp.hour(), time_tmp.minute(), time_tmp.second());
        sprintf_P(lcd_line_1, " %s %.4d-%.2d-%.2d ",
          to_weekday(time_tmp.dayOfTheWeek()), time_tmp.year(),
          time_tmp.month(), time_tmp.day());
        lcd.setCursor(DY_LCD_POS_X, DATE_LCD_POS_Y);
        lcd.cursor();

        if(buttons_risen[BTN_PLUS] ||
          (buttons[BTN_PLUS] && (sys_time >= update_time))) {
          if(time_tmp.month() < 31) {
            time_tmp = DateTime(time_tmp.year(), time_tmp.month(),
              time_tmp.day() + 1, time_tmp.hour(),
              time_tmp.minute(), time_tmp.second());
          }
          else {
            time_tmp = DateTime(time_tmp.year(), time_tmp.month(),
              1, time_tmp.hour(),
              time_tmp.minute(), time_tmp.second());
          }

          update_time = sys_time + UPDATE_DELAY;
        }
        if(buttons_risen[BTN_MINUS] ||
          (buttons[BTN_MINUS] && (sys_time >= update_time))) {
          if(time_tmp.day() > 1) {
            time_tmp = DateTime(time_tmp.year(), time_tmp.month(),
              time_tmp.day() - 1, time_tmp.hour(),
              time_tmp.minute(), time_tmp.second());
          }
          else {
            time_tmp = DateTime(time_tmp.year(), time_tmp.month(),
              31, time_tmp.hour(),
              time_tmp.minute(), time_tmp.second());
          }
          
          update_time = sys_time + UPDATE_DELAY;
        }
        else if(buttons_risen[BTN_SEL]) {
          fsm_state = MENU_SET_DATE_HR;
        }
        else if(buttons_risen[BTN_SET]) {
          rtc_ext.adjust(time_tmp);
          lcd.noCursor();
          fsm_state = MENU_DATE;
        }
        break;
    }
  }

  if(change_sleep_mode) {
    sleep_mode = !sleep_mode;
    change_sleep_mode = 0;
    if(sleep_mode) {
      attachInterrupt(digitalPinToInterrupt(PIN_BTN_PLUS), button_isr, RISING);
      attachInterrupt(digitalPinToInterrupt(PIN_BTN_MINUS), button_isr, RISING);
      attachInterrupt(digitalPinToInterrupt(PIN_BTN_SEL), button_isr, RISING);
      attachInterrupt(digitalPinToInterrupt(PIN_BTN_SET), button_isr, RISING);
      rtc_int.setAlarmSeconds((rtc_int.getSeconds()) % 60);
      rtc_int.standbyMode();
    }
    else {
      detachInterrupt(digitalPinToInterrupt(PIN_BTN_PLUS));
      detachInterrupt(digitalPinToInterrupt(PIN_BTN_MINUS));
      detachInterrupt(digitalPinToInterrupt(PIN_BTN_SEL));
      detachInterrupt(digitalPinToInterrupt(PIN_BTN_SET));
      rtc_int.disableAlarm();
    }
  }
}


//------------------------------------------------------------------------------
//      __   __              ___  ___
//     |__) |__) | \  /  /\   |  |__
//     |    |  \ |  \/  /~~\  |  |___
//
//------------------------------------------------------------------------------

//==============================================================================
static char* to_weekday(uint8_t day_of_week) {
  static char* weekday = new char[3];
  switch(day_of_week) {
    case 0:
      sprintf_P(weekday, "Sun", 0);
      break;
    case 1:
      sprintf_P(weekday, "Mon", 0);
      break;
    case 2:
      sprintf_P(weekday, "Tue", 0);
      break;
    case 3:
      sprintf_P(weekday, "Wed", 0);
      break;
    case 4:
      sprintf_P(weekday, "Thu", 0);
      break;
    case 5:
      sprintf_P(weekday, "Fri", 0);
      break;
    case 6:
      sprintf_P(weekday, "Sat", 0);
      break;
  }
  return weekday;
}
//------------------------------------------------------------------------------
//        __   __   __
//     | /__` |__) /__`
//     | .__/ |  \ .__/
//
//------------------------------------------------------------------------------

//==============================================================================
void button_isr() {
  change_sleep_mode = 1;
}