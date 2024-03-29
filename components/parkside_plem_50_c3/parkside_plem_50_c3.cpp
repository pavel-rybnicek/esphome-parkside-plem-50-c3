#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/log.h"
#include "parkside_plem_50_c3.h"
#include "Wire.h"
#include <cstring>
#include <soc/rtc_wdt.h>

namespace esphome {
namespace parkside_plem_50_c3 {

static const char *TAG = "parkside_plem_50_c3";

static uint8_t I2C_DEV_ADDR = 0x3F;

static const int PACKET_LEN = 99;

static const int BUFSIZE = 200; 

static byte packet_incoming[BUFSIZE]; // buffer for currently coming buffer
static byte packet_last[BUFSIZE];     // last completely received packet

int i = 0;
int messages_count = 0;

void onReceive(int len){
  //ESP_LOGE(TAG, "onReceive[%d]: ", len);
  while(Wire1.available()){
    packet_incoming[i++] = Wire1.read();
  }
  if (!(0x80 == packet_incoming[0] && 0xB0 == packet_incoming[1]))
  {
    i = 0;
    return;
  }
  if (i >= 198)
  {
    memcpy (packet_last, packet_incoming, 198);
    i = 0;
    messages_count++;
  }
}

void ParksidePlem50C3Component::setup() {
  // init the arrays
  memset (packet_incoming, 0, BUFSIZE);
  memset (packet_last, 0, BUFSIZE);
  
  pinMode(4, OUTPUT);
  pinMode(this->pin_laser_power_, OUTPUT);
  pinMode(this->pin_power_button_, OUTPUT);
  pinMode(this->pin_keyboard_, OUTPUT);
  digitalWrite(this->pin_laser_power_, 1); // vypnuti laseru
  Wire1.onReceive(onReceive);
  Wire1.begin(I2C_DEV_ADDR, this->pin_sda_, this->pin_scl_, 400000);
}

void ParksidePlem50C3Component::decode_digit_last_line(char result[], const byte digit1, const byte digit2)
{
  const char * digit_string;
  switch (digit1 << 8 | digit2)
  {
    case 0x0000: digit_string =  ""; break;
    case 0x0400: digit_string =  "-"; break;
    case 0x121E: digit_string =  "0"; break;
    case 0x1A1E: digit_string =  ".0"; break;
    case 0x0006: digit_string =  "1"; break;
    case 0x0806: digit_string =  ".1"; break;
    case 0x061C: digit_string =  "2"; break;
    case 0x0E1C: digit_string =  ".2"; break;
    case 0x041E: digit_string =  "3"; break;
    case 0x0C1E: digit_string =  ".3"; break;
    case 0x1406: digit_string =  "4"; break;
    case 0x1C06: digit_string =  ".4"; break;
    case 0x141A: digit_string =  "5"; break;
    case 0x1C1A: digit_string =  ".5"; break;
    case 0x161A: digit_string =  "6"; break;
    case 0x1E1A: digit_string =  ".6"; break;
    case 0x0016: digit_string =  "7"; break;
    case 0x0816: digit_string =  ".7"; break;
    case 0x161E: digit_string =  "8"; break;
    case 0x1E1E: digit_string =  ".8"; break;
    case 0x141E: digit_string =  "9"; break;
    case 0x1C1E: digit_string =  ".9"; break;
    default: digit_string =  "?";
  }
  strcat (result, digit_string);
}

void ParksidePlem50C3Component::decode_last_line(char result[], const byte * line)
{
  // this->log_data_packet(line, 18);
  
  this->decode_digit_last_line(result, line[1], line[3]);
  this->decode_digit_last_line(result, line[5], line[7]);
  this->decode_digit_last_line(result, line[9], line[11]);
  this->decode_digit_last_line(result, line[13], line[15]);
  this->decode_digit_last_line(result, line[17], line[19]);
  this->decode_digit_last_line(result, line[21], line[23]);
  // position 25 contains unit - we have a separate method to parse it
  // positions 27-36 containt ft/in fractions - we don't need to parse these
}

void ParksidePlem50C3Component::decode_digit(char result[], const byte digit1, const byte digit2)
{
  const char * digit_string = "";
  switch (digit1 << 8 | digit2)
  {
    case 0x0000: digit_string = ""; break;
    case 0x0002: digit_string = "-"; break;
    case 0x1E0C: digit_string = "0"; break;
    case 0x1E1C: digit_string = ".0"; break;
    case 0x0600: digit_string = "1"; break;
    case 0x0610: digit_string = ".1"; break;
    case 0x1A06: digit_string = "2"; break;
    case 0x1A16: digit_string = ".2"; break;
    case 0x1E02: digit_string = "3"; break;
    case 0x1E12: digit_string = ".3"; break;
    case 0x060A: digit_string = "4"; break;
    case 0x061A: digit_string = ".4"; break;
    case 0x1C0A: digit_string = "5"; break;
    case 0x1C1A: digit_string = ".5"; break;
    case 0x1C0E: digit_string = "6"; break;
    case 0x1C1E: digit_string = ".6"; break;
    case 0x0E00: digit_string = "7"; break;
    case 0x0E10: digit_string = ".7"; break;
    case 0x1E0E: digit_string = "8"; break;
    case 0x1E1E: digit_string = ".8"; break;
    case 0x1E0A: digit_string = "9"; break;
    case 0x1E1A: digit_string = ".9"; break;
    case 0x180E: digit_string = "E"; break;
    case 0x1406: digit_string = "o"; break;
    case 0x0006: digit_string = "r"; break;
    default: digit_string =  "?";
  }
  strcat (result, digit_string);
}

void ParksidePlem50C3Component::decode_line(char result[], const byte * line)
{
  // this->log_data_packet(line, 17);

  this->decode_digit(result, line[31], line[33]);
  this->decode_digit(result, line[27], line[29]);
  this->decode_digit(result, line[23], line[25]);
  this->decode_digit(result, line[19], line[21]);
  this->decode_digit(result, line[15], line[17]);
  // position 1 contains unit
  // positions 3-13 containt fractions of ft/in
  // we don't need to parse these
}

void ParksidePlem50C3Component::decode_unit(char result[], byte unit_code)
{
  // XXX would be nicer to avoid multiple strcat
  // or use constants for the result
  switch (unit_code)
  {
    case 0x00: strcat (result, ""); break;
    case 0x0A: strcat (result, "m"); break;
    case 0x10: strcat (result, "ft"); break;
    case 0x06: strcat (result, "in"); break;
    default: strcat (result, "?"); break;
  }
}

void ParksidePlem50C3Component::log_data_packet(const byte packet[], int len_to_log) {
  char formatted_out[BUFSIZE] = "";
  for (int j = 0; j < len_to_log; j++)
  {
    sprintf(formatted_out + j*2, "%02X", packet[j*2+1]); 
  }
  ESP_LOGD(TAG, formatted_out);
}

void ParksidePlem50C3Component::wait_for_packet (byte packet[])
{
  int messages_count_current = messages_count;

  while (messages_count_current == messages_count)
  {
    delay (1); // if no message arrives, the watchdog will reset the device
  }
  ESP_LOGD (TAG, "%d messages arrived", messages_count);
  cli(); // disable interrupts
  memcpy (packet, packet_last, BUFSIZE);
  memset (packet_last, 0, BUFSIZE);
  messages_count = 0;
  sei(); // allow interrupts
}

void ParksidePlem50C3Component::update() {

  // we need to start powered off
  if (digitalRead(this->pin_laser_power_) == LOW) {
    ESP_LOGD (TAG, "Powered at the start, switch off");
    digitalWrite(this->pin_laser_power_, 1); 
    delay (200);
  }

  // switch on - hold button
  digitalWrite(this->pin_laser_power_, 0);
  delay (10); // this delay is important
  digitalWrite(this->pin_power_button_, 0);
  digitalWrite(this->pin_keyboard_, 1);
  delay (300); // probably the safe minimum delay, 200 does not work
  // here we expect 2 first messages - first is FFs, second is zeroes
  ESP_LOGD (TAG, "%d messages, power on", messages_count);

  // switch on - release button
  digitalWrite(this->pin_power_button_, 1);
  byte packet_to_process2[BUFSIZE];
  // wait for one message - dashes on the main line, laser is off
  this->wait_for_packet(packet_to_process2);
  // wait for one message - dashes on the main line, laser is on
  this->wait_for_packet(packet_to_process2);

  // ready to take a measure
  // measurement - press button
  digitalWrite(this->pin_keyboard_, 0);
  delay (100);
  // measurement - release button
  digitalWrite(this->pin_keyboard_, 1);

  byte packet_to_process[BUFSIZE];
  this->wait_for_packet(packet_to_process);

  // this->log_data_packet(packet_to_process, PACKET_LEN);

  // we don't need to read first two lines
  //char line1[10] = "";
  //this->decode_line (line1, packet_to_process + 10*2);
  //ESP_LOGD(TAG, line1);
  //char line2[10] = "";
  //this->decode_line (line2, packet_to_process + 82*2);
  //ESP_LOGD(TAG, line2);

  // third line can contain information about error
  char line3[10] = "";
  this->decode_line (line3, packet_to_process + 65*2);
  ESP_LOGD(TAG, line3);

  // last line contains value, error number or nothing
  char line4[10] = "";
  this->decode_last_line (line4, packet_to_process + 94);
  this->decode_unit(line4, packet_to_process[119]);
  ESP_LOGD(TAG, line4);
  
  this->process_measurement (line3, line4);
  digitalWrite(this->pin_laser_power_, 1);
}

void ParksidePlem50C3Component::process_error (const char * line4)
{
  int error_code = std::stol(line4);
  const char * err_msg;

  switch (error_code)
  {
    case 204: err_msg = "204 Calculation error, should never happen"; break;
    case 220: err_msg = "220 Low power"; break;
    case 255: err_msg = "255 Received signal too weak or measurement time too long"; break;
    case 256: err_msg = "256 Received signal too strong"; break;
    case 261: err_msg = "261 Out of measuring range"; break;
    case 500: err_msg = "500 Hardware error"; break;
    default: err_msg = "Unknown error";
  }
  ESP_LOGE(TAG, err_msg);
  this->error_sensor_->publish_state(err_msg);
  this->distance_sensor_->publish_state(NAN);
}

void ParksidePlem50C3Component::process_measurement (const char * line3, const char * line4)
{
  // check for an error
  if (!strcmp("Error", line3))
  {
    this->process_error (line4);
    return;
  }

  // check we got value in meters
  if ('m' != line4[strlen(line4) - 1])
  {
    this->distance_sensor_->publish_state(NAN);
    this->error_sensor_->publish_state("Result is not in meters. Switch the range finder to metric.");
    return;
  }

  // parse value
  float measured_value = std::stof(line4);

  // publish
  this->distance_sensor_->publish_state(measured_value);
  this->error_sensor_->publish_state("");
}

void ParksidePlem50C3Component::dump_config() {
  ESP_LOGCONFIG(TAG, "ParksidePlem50C3 Component:");
  // not implemented ESP_LOGCONFIG(TAG, "  attempt_count: %d", this->attempt_count_);
  ESP_LOGCONFIG(TAG, "  pin_sda         : %d", this->pin_sda_         );
  ESP_LOGCONFIG(TAG, "  pin_scl         : %d", this->pin_scl_         );
  ESP_LOGCONFIG(TAG, "  pin_power_button: %d", this->pin_power_button_);
  ESP_LOGCONFIG(TAG, "  pin_keyboard    : %d", this->pin_keyboard_    );
  ESP_LOGCONFIG(TAG, "  pin_laser_power : %d", this->pin_laser_power_ );
}

} // parkside_plem_50_c3
} // esphome
