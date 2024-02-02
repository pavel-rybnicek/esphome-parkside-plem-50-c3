#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include <Arduino.h>

namespace esphome {
namespace parkside_plem_50_c3 {

class ParksidePlem50C3Component :  public PollingComponent {
  public:
    void setup() override;
    void update() override;
    void dump_config() override;


    void set_distance_sensor(sensor::Sensor *s) { distance_sensor_ = s; }
    void set_error_sensor(text_sensor::TextSensor *s) { error_sensor_ = s; }
    void set_attempt_count(uint32_t s) { attempt_count_ = s; }

  protected:


    sensor::Sensor *distance_sensor_;

    text_sensor::TextSensor *error_sensor_;

    uint32_t attempt_count_ = 1;

    void wait_for_packet(byte packet[]);
    void log_data_packet(const byte packet[], int len);
    void decode_unit(char result[], const char unit_code);
    void decode_last_line(char result[], const byte * line);
    void decode_digit_last_line(char result[], const byte digit1, const byte digit2);
    void decode_line(char result[], const byte * line);
    void decode_digit(char result[], const byte digit1, const byte digit2);
    void process_error (const char * buffer, const char * errorText);
    void process_measurement (const char * line3, const char * line4);

};

} // parkside_plem_50_c3
} // esphome
