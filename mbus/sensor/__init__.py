import esphome.codegen as cg
from esphome.components import mbus, sensor
import esphome.config_validation as cv

from .. import mbus_ns

DEPENDENCIES = ["mbus"]

CONF_MBUS_ID = "mbus_id"
CONF_MBUS_STORAGE = "mbus_storage"
CONF_MBUS_FUNCTION = "mbus_function"
CONF_MBUS_TARIFF = "mbus_tariff"
CONF_MBUS_SUBUNIT = "mbus_subunit"
CONF_MBUS_VIFE = "mbus_vife"

MbusSensor = mbus_ns.class_(
    "MbusSensor", sensor.Sensor, cg.Component
)

MbusDIFFunction = mbus_ns.enum("MbusDIFFunction")
MBUS_DIF_FUNCTION = {
    "INSTANT": MbusDIFFunction.MBUS_INSTANT_VALUE,
    "MAXIMUM": MbusDIFFunction.MBUS_MAXIMUM_VALUE,
    "MINIMUM": MbusDIFFunction.MBUS_MINIMUM_VALUE,
    "ERROR": MbusDIFFunction.MBUS_ERROR_VALUE,
}

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        MbusSensor,
        accuracy_decimals=1,
    )
    .extend(
        {
            cv.GenerateID(CONF_MBUS_ID): cv.use_id(mbus.Mbus),
            cv.Optional(CONF_MBUS_STORAGE, default=0): cv.int_range(0, 0x1ffffffffff),
            cv.Optional(CONF_MBUS_FUNCTION, default="INSTANT"): cv.enum(
                MBUS_DIF_FUNCTION, upper=True
            ),
            cv.Optional(CONF_MBUS_TARIFF, default=0): cv.int_range(0, 0xfffff),
            cv.Optional(CONF_MBUS_SUBUNIT, default=0): cv.int_range(0, 0x3ff),
            cv.Required(CONF_MBUS_VIFE): cv.int_range(0x0000000000000000, 0xffffffffffffffff),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)

    parent = await cg.get_variable(config[CONF_MBUS_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_mbus_storage(config[CONF_MBUS_STORAGE]))
    cg.add(var.set_mbus_function(config[CONF_MBUS_FUNCTION]))
    cg.add(var.set_mbus_tariff(config[CONF_MBUS_TARIFF]))
    cg.add(var.set_mbus_subunit(config[CONF_MBUS_SUBUNIT]))
    cg.add(var.set_mbus_vife(config[CONF_MBUS_VIFE]))
