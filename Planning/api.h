//==============================================================================
// Checks list of alarms to see if one should go off.
//
// param cur_time  System time in ms.
// param *alarms   Reference to list of alarms.
// return  True if alarm should go off, false otherwise.
//==============================================================================
bool check_alarm(uint64_t cur_time, alarms_t *alarms);

//==============================================================================
// Start alarm sound.
//
// param cur_sound  Alarm sound selected by user.
//==============================================================================
void start_alarm(sound_t cur_sound);

//==============================================================================
// Stops alarm sound.
//==============================================================================
void stop_alarm();

//==============================================================================
// Synchronize system time with RTC time.
//
// return  Current time from RTC.
//==============================================================================
void sync_RTC(uint64_t cur_time);

//==============================================================================
// Checks to see if ringing alarm is being shaken.
//
// return  True if alam is being shaken, false otherwise.
//==============================================================================
bool is_shaken();

//==============================================================================
// Checks to see if shake countdown has elapsed.
//
// return  True if shake countown has elapsed, false otherwise.
//==============================================================================
bool check_shake_countdown(uint64_t cur_time, uint16_t shake_dealy)

//==============================================================================
// Updates display with current time and other statistics.
//
// param cur_time  System time in ms.
// param *alarms   Reference to list of alarms.
//==============================================================================
void update_display(uint64_t cur_time, alarms_t *alarms, uint8_t bat_percent, disp_state_t state);

//==============================================================================
// Determines whether clock is charging.
//
// return  True if battery is charging, false otherwise.
//==============================================================================
bool is_charging();

//==============================================================================
// Checks to see which buttons are currently pressed.
//
// return  Coded 8-bit value showing which buttons are pressed and held.
//==============================================================================
buttons_t check_buttons();

//==============================================================================
// Returns current battery level.
//
// return  current battery level.
//==============================================================================
uint8_t read_bat();

//==============================================================================
// Adds new alarm to list of alarms.
//
// param alarms     List of alarms.
// param new_alarm  Alarm to add to list.
//==============================================================================
void add_alarm(alarms_t *alarms, uint64_t new_alarm);

//==============================================================================
// Removes alarm to list of alarms.
//
// param alarms     List of alarms.
// param to_remove  Index of alarm to remove.
//==============================================================================
void remove_alarm(alarms_t *alarms, uint8_t to_remove);
