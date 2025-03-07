import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_DURATION,
    ICON_HEART_PULSE,
    ICON_PULSE,
    ICON_SIGNAL,
    ICON_COUNTER,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_BEATS_PER_MINUTE,
    UNIT_MINUTE
)

from . import CONF_C1001_ID, C1001Component

DEPENDENCIES = ["C1001"]

CONF_SLEEP_STATE = "sleep_state"
CONF_WAKE_DURATION = "wake_duration"
CONF_LIGHT_SLEEP_DURATION = "light_sleep_duration"
CONF_DEEP_SLEEP_DURATION = "deep_sleep_duration"
CONF_SLEEP_SCORE = "sleep_score"
CONF_BREATH_RATE = "breath_rate"
CONF_HEART_RATE = "heart_rate"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_C1001_ID): cv.use_id(C1001Component),
        cv.Optional(CONF_SLEEP_STATE): sensor.sensor_schema(
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
            icon=ICON_PULSE,
        ),
        cv.Optional(CONF_WAKE_DURATION): sensor.sensor_schema(
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
            device_class=DEVICE_CLASS_DURATION,
            unit_of_measurement=UNIT_MINUTE,
            icon=ICON_PULSE,
        ),
        cv.Optional(CONF_LIGHT_SLEEP_DURATION): sensor.sensor_schema(
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
            device_class=DEVICE_CLASS_DURATION,
            unit_of_measurement=UNIT_MINUTE,
            icon=ICON_PULSE,
        ),
        cv.Optional(CONF_DEEP_SLEEP_DURATION): sensor.sensor_schema(
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
            device_class=DEVICE_CLASS_DURATION,
            unit_of_measurement=UNIT_MINUTE,
            icon=ICON_PULSE,
        ),
        cv.Optional(CONF_SLEEP_SCORE): sensor.sensor_schema(
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
            icon=ICON_PULSE,
        ),
        cv.Optional(CONF_BREATH_RATE): sensor.sensor_schema(
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
            icon=ICON_PULSE,
        ),
        cv.Optional(CONF_HEART_RATE): sensor.sensor_schema(
            accuracy_decimals=0,
            icon=ICON_HEART_PULSE,
            state_class=STATE_CLASS_MEASUREMENT,
            unit_of_measurement=UNIT_BEATS_PER_MINUTE,
        ),
    }
)

async def to_code(config):
    C1001_component = await cg.get_variable(config[CONF_C1001_ID])
    if sleep_state_config := config.get(CONF_SLEEP_STATE):
        sens = await sensor.new_sensor(sleep_state_config)
        cg.add(C1001_component.set_sleep_state_sensor(sens))
    if wake_duration_config := config.get(CONF_WAKE_DURATION):
        sens = await sensor.new_sensor(wake_duration_config)
        cg.add(C1001_component.set_wake_duration_sensor(sens))
    if light_sleep_duration_config := config.get(CONF_LIGHT_SLEEP_DURATION):
        sens = await sensor.new_sensor(light_sleep_duration_config)
        cg.add(C1001_component.set_light_sleep_duration_sensor(sens))
    if deep_sleep_duration_config := config.get(CONF_DEEP_SLEEP_DURATION):
        sens = await sensor.new_sensor(deep_sleep_duration_config)
        cg.add(C1001_component.set_deep_sleep_duration_sensor(sens))
    if sleep_score_config := config.get(CONF_SLEEP_SCORE):
        sens = await sensor.new_sensor(sleep_score_config)
        cg.add(C1001_component.set_sleep_score_sensor(sens))
    if breath_rate_config := config.get(CONF_BREATH_RATE):
        sens = await sensor.new_sensor(breath_rate_config)
        cg.add(C1001_component.set_breath_rate_sensor(sens))
    if heart_rate_config := config.get(CONF_HEART_RATE):
        sens = await sensor.new_sensor(heart_rate_config)
        cg.add(C1001_component.set_heart_rate_sensor(sens))