#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
//#include "esphome/components/uart/uart.h"
#include "esphome/core/log.h"
#include "parkside_plem_50_c3.h"
#include "Wire.h"
#include <Arduino.h>
#include <cstring>

namespace esphome {
namespace parkside_plem_50_c3 {

static const char *TAG = "parkside_plem_50_c3";

static const int BUFSIZE = 200;

static uint8_t I2C_DEV_ADDR = 0x3F;

static byte data[BUFSIZE];
static byte dataToPrint[BUFSIZE] = " N o t h i n g   y e t";

int i = 0;

void onRequest(){
  ESP_LOGE(TAG, "onRequest");
}

void onReceive(int len){
  //ESP_LOGE(TAG, "onReceive[%d]: ", len);
  while(Wire1.available()){
    data[i++] = Wire1.read();
  }
  if (!(0x80 == data[0] && 0xB0 == data[1]))
  {
    i = 0;
    return;
  }
  if (i >= 198)
  {
    memcpy (dataToPrint, data, 198);
    i = 0;
  }
}

void ParksidePlem50C3Component::setup() {
  // nothing to do here
  //    delay(10000);
  pinMode(4, OUTPUT);
  digitalWrite(4, 0); // XXX zhasnuti diody
  Wire1.onReceive(onReceive);
  Wire1.onRequest(onRequest);
  //Wire1.setClock(400000);
  Wire1.begin(I2C_DEV_ADDR, 14, 15, 400000); // 14 je fialovy
}

void ParksidePlem50C3Component::read_message(char buffer[])
{/*
  uint8_t buffer_index = 0;

  memset (buffer, 0, BUFSIZE);

  for (int i = 0; buffer_index < BUFSIZE; i++) {
    if (i > 10000)
    {
      ESP_LOGE(TAG, "Timeout, message not complete. Received: %s", (char *) buffer);
      break;
    }
    if (!available())
    {
      delay(1);
      continue;
    }
    uint8_t data;
    read_byte(&data);
    buffer[buffer_index++] = (char) data;
    if ('>' == data)
    {
      break;
    }
  }
  // terminate the string
  buffer[buffer_index] = 0;
  ESP_LOGD(TAG, "Received: %s", (char *) buffer);*/
}

void ParksidePlem50C3Component::write_message(const char * message)
{/*
  ESP_LOGD(TAG, "Sending: %s", message);
  write_str(message);*/
}

const char * ParksidePlem50C3Component::decodeDigitLastLine(const char digit1, const char digit2)
{
char formattedOut[10] = "";
    sprintf(formattedOut, "%04X", digit1 << 8 | digit2); 
ESP_LOGD(TAG, "%02X %02X", digit1, digit2);
ESP_LOGD(TAG, formattedOut);
  switch (digit1 << 8 | digit2)
  {
    case 0x0000: return "";
    case 0x0400: return "-";
    case 0x121E: return "0";
    case 0x1A1E: return ".0";
    case 0x0006: return "1";
    case 0x0806: return ".1";
    case 0x061C: return "2";
    case 0x0E1C: return ".2";
    case 0x041E: return "3";
    case 0x0C1E: return ".3";
    case 0x1406: return "4";
    case 0x1C06: return ".4";
    case 0x141A: return "5";
    case 0x1C1A: return ".5";
    case 0x161A: return "6";
    case 0x1E1A: return ".6";
    case 0x0016: return "7";
    case 0x0816: return ".7";
    case 0x161E: return "8";
    case 0x1E1E: return ".8";
    case 0x141E: return "9";
    case 0x1C1E: return ".9";
    default: return "?";
  }
}

void ParksidePlem50C3Component::decodeLastLine(char * result, const char * line)
{
}

void ParksidePlem50C3Component::update() {
  char formattedOut[BUFSIZE] = "";

  ESP_LOGD(TAG, "update()");
// vypnuti
//  digitalWrite(16, 0);
//  delay(5000);
//  digitalWrite(16, 1);
//  ESP_LOGD(TAG, "switched on");
  //cli();
  for (int j = 0; j < 99; j++)
  {
    sprintf(formattedOut + j*2, "%02X", dataToPrint[j*2+1]); 
  }
  ESP_LOGD(TAG, formattedOut);

  char line[10] = "";
  strcat (line, this->decodeDigitLastLine(dataToPrint[47], dataToPrint[49]));
  strcat (line, this->decodeDigitLastLine(dataToPrint[51], dataToPrint[53]));
  strcat (line, this->decodeDigitLastLine(dataToPrint[55], dataToPrint[57]));
  strcat (line, this->decodeDigitLastLine(dataToPrint[59], dataToPrint[61]));
  strcat (line, this->decodeDigitLastLine(dataToPrint[63], dataToPrint[65]));
  strcat (line, this->decodeDigitLastLine(dataToPrint[67], dataToPrint[69]));
  ESP_LOGD(TAG, line);


  //sei();
  delay(1000);
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
  // measurement looks like <m30m356>
  // where 3rd char is length of the value. Value itself
  // starts at 6th character
  if (!strncmp ("<0E", measurement, 3))
  {
    this->process_error (measurement, "Error received");
    return -1;
  }
  if (strncmp ("<m", measurement, 2))
  {
    this->process_error (measurement, "Response should start with '<m'");
    return -2;
  }

  // get value len
  int valueLen = measurement[2] - '0';
  if ( valueLen < 0 || valueLen > '9')
  {
    this->process_error (measurement, "3rd character must be digit - len of value");
    return -3;
  }

  char value[10] = "";
  strncpy (value, measurement + 5, valueLen);
  if ( valueLen != strlen (value))
  {
    this->process_error (measurement, "Error parsing value - invalid length?");
    return -4;
  }

  // previous checks aren't really necessary...
  int val = atoi(measurement + 5);
  if (val <= 0)
  {
    this->process_error (measurement, "Cannot get meaningfull value");
    return -5;
  }
  ESP_LOGD(TAG, "Buffer: '%s', value is %d", measurement, val);
  this->distance_sensor_->publish_state(val);
  this->error_sensor_->publish_state("");

  return 0;
}

void ParksidePlem50C3Component::dump_config() {
  ESP_LOGCONFIG(TAG, "ParksidePlem50C3 Component:");
  ESP_LOGCONFIG(TAG, "  attempt_count: %d", this->attempt_count_);
}

} // parkside_plem_50_c3
} // esphome
