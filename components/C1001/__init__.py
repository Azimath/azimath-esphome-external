import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@Azimath"]
DEPENDENCIES = ["uart"]
MULTI_CONF = True

C1001_ns = cg.esphome_ns.namespace("C1001")

C1001Component = C1001_ns.class_(
    "C1001Component", cg.PollingComponent
)

CONF_C1001_ID = "C1001_id"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(C1001Component),
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.polling_component_schema("60s"))
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "C1001",
    require_tx=True,
    require_rx=True,
    baud_rate=115200,
    parity="NONE",
    stop_bits=1,
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)