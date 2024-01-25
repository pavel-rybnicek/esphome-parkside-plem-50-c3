#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/log.h"
#include "parkside_plem_50_c3.h"
#include "Wire.h"
#include <cstring>

namespace esphome {
namespace parkside_plem_50_c3 {

static const char *TAG = "parkside_plem_50_c3";

static uint8_t I2C_DEV_ADDR = 0x3F;

static const int PIN_VYPINAC = 12;
static const int PIN_KLAVESNICE = 13;
static const int PIN_LASER_PWR = 3;

static const int PACKET_LEN = 99;

static const int BUFSIZE = 200; // TODO udělat jako dvojnásobek packet_len

static byte packet_incoming[BUFSIZE]; // buffer for currently coming buffer
static byte packet_last[BUFSIZE];     // last completely received packet

int i = 0;

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
  if (i >= 198) // XXX prověřit možnost přetečení
  {
    memcpy (packet_last, packet_incoming, 198);
    i = 0;
  }
}

void ParksidePlem50C3Component::setup() {
  // init the arrays
  memset (packet_incoming, 0, BUFSIZE);
  memset (packet_last, 0, BUFSIZE);
  
  pinMode(4, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(4, 0); // XXX zhasnuti diody
  digitalWrite(PIN_VYPINAC, 1);
  digitalWrite(PIN_KLAVESNICE, 1);
  Wire1.onReceive(onReceive);
  Wire1.begin(I2C_DEV_ADDR, 14, 15, 400000); // 14 je fialovy
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

void ParksidePlem50C3Component::decode_unit(char result[], const char unit_code)
{
  switch (unit_code) // XXX mozna by melo byt spis byte - obecne
  {
    case 0x00: strcat (result, ""); break; // XXX do promenne a delat jeden strcat
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

void ParksidePlem50C3Component::update() {

  digitalWrite(PIN_LASER_PWR, 1);
  digitalWrite(PIN_VYPINAC, 0);
  //digitalWrite(PIN_KLAVESNICE, 0);
  //digitalWrite(13, 1);
  delay (1000);
  digitalWrite(PIN_VYPINAC, 1);
  delay (100);
  digitalWrite(PIN_KLAVESNICE, 0);
  delay (100);
  digitalWrite(PIN_KLAVESNICE, 1);

  delay (1000);
  //digitalWrite(PIN_KLAVESNICE, 1);
  //delay (3000);
  //digitalWrite(PIN_VYPINAC, 1);*/
  //digitalWrite(PIN_KLAVESNICE, 0);
  // ESP_LOGD(TAG, "update()");
  // vypnuti
  //  digitalWrite(16, 0);
  //  delay(5000);
  //  digitalWrite(16, 1);
  //  ESP_LOGD(TAG, "switched on");
  //cli();
  this->log_data_packet(packet_last, PACKET_LEN);

  // we don't need to read first two lines
  //char line1[10] = "";
  //this->decode_line (line1, packet_last + 10*2);
  //ESP_LOGD(TAG, line1);
  //char line2[10] = "";
  //this->decode_line (line2, packet_last + 82*2);
  //ESP_LOGD(TAG, line2);

  // third line can contain information about error
  char line3[10] = "";
  this->decode_line (line3, packet_last + 65*2);
  ESP_LOGD(TAG, line3);

  // last line contains value, error number or nothing
  char line4[10] = "";
  this->decode_last_line (line4, packet_last + 94);
  this->decode_unit(line4, (char)packet_last[119]); // FIXME tohle je fuj
  ESP_LOGD(TAG, line4);

  digitalWrite(PIN_LASER_PWR, 0);
  //sei();
}

void ParksidePlem50C3Component::process_error (const char * buffer, const char * errorText)
{
  char err_msg[100] = "";
  sprintf(err_msg, "%s, buffer: '%s'", errorText, buffer);
  ESP_LOGE(TAG, "%s", err_msg);
  this->error_sensor_->publish_state(err_msg);
}

int ParksidePlem50C3Component::process_measurement (const char * measurement)
{
  return 0;
}

void ParksidePlem50C3Component::dump_config() {
  ESP_LOGCONFIG(TAG, "ParksidePlem50C3 Component:");
  ESP_LOGCONFIG(TAG, "  attempt_count: %d", this->attempt_count_);
  // TODO
}

} // parkside_plem_50_c3
} // esphome
