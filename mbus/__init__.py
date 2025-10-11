import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv

from esphome.const import CONF_ID

DEPENDENCIES = ["uart"]

mbus_ns = cg.esphome_ns.namespace("mbus")
Mbus = mbus_ns.class_("Mbus", cg.PollingComponent, uart.UARTDevice)
MULTI_CONF = True

CONF_SECONDARY_ADDRESS = "secondary_address"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Mbus),
            cv.Optional(CONF_SECONDARY_ADDRESS, default=0xffffffffffffffff): cv.int_range(0x0000000000000000, 0xffffffffffffffff),
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    await uart.register_uart_device(var, config)

    cg.add(var.set_secondary_address(config[CONF_SECONDARY_ADDRESS]))
    
