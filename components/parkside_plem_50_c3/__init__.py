import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, text_sensor
from esphome.const import *
parkside_plem_50_c3_ns = cg.esphome_ns.namespace('parkside_plem_50_c3')
ParksidePlem50C3Component = parkside_plem_50_c3_ns.class_('ParksidePlem50C3Component', cg.PollingComponent)

DEPENDENCIES = []
AUTO_LOAD = ['sensor', 'text_sensor'] 

CONF_DISTANCE = "distance"
CONF_ERROR = "error"
CONF_ATTEMPT_COUNT = "attempt_count"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(ParksidePlem50C3Component),

    cv.Optional(CONF_DISTANCE):
      sensor.sensor_schema(device_class=DEVICE_CLASS_DISTANCE,accuracy_decimals=0,state_class=STATE_CLASS_MEASUREMENT).extend(),

    cv.Optional(CONF_ERROR):
      text_sensor.text_sensor_schema().extend(),

    cv.Optional(CONF_ATTEMPT_COUNT): cv.positive_int,

}).extend(cv.polling_component_schema('10s'))

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)

    if CONF_ATTEMPT_COUNT in config:
      conf = config[CONF_ATTEMPT_COUNT]
      cg.add(var.set_attempt_count(conf))
 
    if CONF_DISTANCE in config:
      conf = config[CONF_DISTANCE]
      sens = yield sensor.new_sensor(conf)
      cg.add(var.set_distance_sensor(sens))
    if CONF_ERROR in config:
      conf = config[CONF_ERROR]
      sens = yield text_sensor.new_text_sensor(conf)
      cg.add(var.set_error_sensor(sens))
