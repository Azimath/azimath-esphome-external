import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_OCCUPANCY,
)
from . import CONF_C1001_ID, C1001Component

DEPENDENCIES = ["C1001"]

CONF_BED_OCCUPIED = "bed_occupied"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_C1001_ID): cv.use_id(C1001Component),
    cv.Optional(CONF_BED_OCCUPIED): binary_sensor.binary_sensor_schema(
        device_class=DEVICE_CLASS_OCCUPANCY, icon="mdi:motion-sensor"
    ),
}

async def to_code(config):
    C1001_component = await cg.get_variable(config[CONF_C1001_ID])

    if bed_occupied_config := config.get(CONF_BED_OCCUPIED):
        sens = await binary_sensor.new_binary_sensor(bed_occupied_config)
        cg.add(C1001_component.set_bed_occupied_binary_sensor(sens))