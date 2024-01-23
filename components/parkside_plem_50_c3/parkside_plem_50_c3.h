#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
//#include "esphome/components/uart/uart.h" // TODO vyhazet zavislost

namespace esphome {
namespace parkside_plem_50_c3 {

class ParksidePlem50C3Component :  public PollingComponent {
  public:
    void setup() override;
    void update() override;
    void dump_config() override;


    void read_message(char buffer[]);
    void write_message(const char * message);
    void set_distance_sensor(sensor::Sensor *s) { distance_sensor_ = s; }
    void set_error_sensor(text_sensor::TextSensor *s) { error_sensor_ = s; }
    void set_attempt_count(uint32_t s) { attempt_count_ = s; }

  protected:


    sensor::Sensor *distance_sensor_;

    text_sensor::TextSensor *error_sensor_;

    uint32_t attempt_count_ = 1;

    void decodeLastLine(char * result, const char * line);
    char * decodeDigitLastLine(const char digit1, const char digit2);
    void process_error (const char * buffer, const char * errorText);
    int process_measurement (const char * measurement);

};

} // parkside_plem_50_c3
} // esphome
