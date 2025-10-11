# M-bus ESPHome component

This repository contains a custom ESPHome component for reading various metering devices trough
M-bus (a.k.a. EN-13757).

## Description

The `mbus` component implements an M-bus master on an ESPHome-compatible node. It uses a UART for
communication to the bus, in conjunction with a suitable M-bus master transciever (e.g. [this one](https://www.az-delivery.de/en/products/mbus-shield-fur-arduino-mkr)).

There may be one or more UARTs (and thus buses) on a single node, with one or more meters
attached to each bus and one or more entities (sensors) attached to each meter.

### What this component is for:

- Reading M-bus meters that the user owns via an ESPHome-based M-bus master that the user
  owns and forwarding measurement data to e.g. HomeAssistant

### What this component is not for:

- Sniffing on M-bus traffic between a provider's meter and the provider's M-bus gateway

- Capturing data from smart electricity meters that transmit DLMS (a.k.a. COSEM or OBIS)
  data over M-bus wiring

## Installation

The `mbus` directory from this repository is to be copied into the `components` directory
(location varies depending on the installation environment, consult with relevant
[ESPHome Docs](https://esphome.io/components/external_components/) for details).

Add the following to the YAML configuration:
```yaml
external_components:
  - source: components
    components: [mbus]
```

## Configuration

Usage of the `mbus` component requires at least one `mbus` instance in YAML configuration.
Each `mbus` instance corresponds to a single meter.

It is important that the `mbus` component uses the UART prvided to it as-is: it is the user's
responsiblity to declare the `uart` and configure it correctly (proper baud rate, `EVEN` parity).

Simple example configuration with one UART, one meter and one sensor (using Siemens WFM502
 heat meter as example):
```yaml
external_components:
  - source: components
    components: [mbus]

uart:
  - tx_pin: TX
    rx_pin: RX
    baud_rate: 2400
    parity: EVEN

mbus:

sensor:
  - platform: mbus
    name: "Heat consumed"
    mbus_vife: 0x05
    filters:
     - multiply: 0.1
    unit_of_measurement: kWh
```

More complex configuration (multiple meters with multiple sensors):
```yaml
external_components:
  - source: components
    components: [mbus]

uart:
    #This (hardware) UART will be used for something else (e.g. logging)
  - tx_pin: TX
    rx_pin: RX
    baud_rate: 115200
    parity: NONE
    id: main_uart

    #This (soft-)UART will be used for M-bus
  - tx_pin: GPIO5
    rx_pin: GPIO2
    baud_rate: 2400
    parity: EVEN
    id: mbus_uart

mbus:
  - uart_id: mbus_uart
    secondary_address: 0x6814323065322904 #Fully specified address
    id: house

  - uart_id: mbus_uart
    secondary_address: 0x67512895FFFFFFFF #Wildcard address, see below
    id: workshop

sensor:
  - platform: mbus
    name: "Workshop heat consumption"
    mbus_id: workshop
    mbus_vife: 0x05
    filters:
     - multiply: 0.1
    unit_of_measurement: kWh
    
  - platform: mbus
    name: "Home heat consumption"
    mbus_id: house
    mbus_vife: 0x05
    filters:
     - multiply: 0.1
    unit_of_measurement: kWh
    
  - platform: mbus
    name: "Home heating water volume"
    mbus_id: house
    mbus_vife: 0x13
    filters:
     - multiply: 0.001
    unit_of_measurement: m3
```

### Configuration values for mbus instance

- **id** (*Optional*, string): Manually specify the ID for code generation.
  When there are multiple `mbus` instances, it is required.
- **uart_id** (*Optional*, [ID](#config-id)): Manually specify the ID of the UART hub.
- **update_interval** (*Optional*, [Time](#config-time)): The interval
  to query the meter and update attached sensors. Defaults to `60s`.
- **secondary_address** (*Optional*, integer): Secondary M-bus address of the meter. See below.
  When there are multiple meters on the same bus, required. Defaults to `0xFFFFFFFFFFFFFFFF` .

### Configuration values for mbus Sensor

- **mbus_id** (*Optional*, [ID](#config-id)): Manually specify the ID of the `mbus` instance.
  When there are multiple `mbus` instances, it is required.
- **mbus_vife** (*Required*, integer): The Value Information Field of the register the value
  of which is to be returned. See below.
- **mbus_function** (*Optional*): The Function of the register the value of which is to be returned.
  Options: `INSTANT`, `MAXIMUM`, `MINIMUM`, `ERROR`. Defaults to `INSTANT`.
- **mbus_storage** (*Optional*, integer): The Storage number of the register the value of which is
  to be returned. Defaults to `0`.
- **mbus_tariff** (*Optional*, integer): The Tariff number of the register the value of which is
  to be returned. Defaults to `0`.
- **mbus_subunit** (*Optional*, integer): The Subunit the register the value of which is
  to be returned belongs to. Defaults to `0`.
- All other options from [Sensor](#config-sensor).

### M-bus secondary address

The Secondary Address is a 16-digit decimal number uniquely identifying the metering device. It is
permanently assigned during manufacturing. The first 8 digits are a unique serial number, which is
usually printed on the device itself. The last 8 digits encode the meter's vendor, model and the medium
it measures.

Within the M-bus protocol, the Secondary address is represented in BCD (binary coded decimal): each
decimal digit corresponds to a hexadecimal digit: the SA of a Siemens WFM502 meter (`653229`) that
measures heat (`04`) and has a serial number of `68 143 230` becomes `0x6814323065322904`.

An important feature is the so-called "wildcard addressing": the hexdigit `F` matches all decimal
digits. Therefore, if only the serial number is known, the SA `0x68143230FFFFFFFF` can also be
used. Consequently, if there is only a single meter on the bus, the all-wildcard `0xFFFFFFFFFFFFFFFF`
can be used to address it without collision.

### Value Information Field

In an M-bus metering device, measurement and other data is represented as a (potentially large) number
of registers. Each of these is identified by a quintet of a tariff number (for e.g. time-of-use tariffs),
storage number (0 is usually the current value, others are values at given key dates, etc.), function
(like instantaneous value, maximum, minimum), subunit (for meters that measure multiple media or
customers), and the Value Information Field (VIF) and its extension (VIFE).

The VIF (and VIFE, if present) is a variable-length binary descriptor of the quantity or information
the register holds, its unit, scale factor, etc. Its meaning is defined in chapter 6.4 of EN-13757-3.
There are more than 300 possible values for VIF alone, plus extensions and vendor-defined forms.

Due to the complexity involved with decoding and representing these, the approach taken when designing
the `mbus` component is to omit decoding completely and handle the VIF/VIFE as an opaque value in a
raw binary form.

What registers a meter sends back on readout and what value they hold is mostly manufacturer and device
specific. The most straigthforward way of figuring out what quantities are available and the
corresponding VIF values is to configure a dummy sensor (e.g. with `mbus_vife: 0x00`), set the ESPHome
node's loglevel to `DEBUG` (see [ESPHome Docs on logging](https://esphome.io/components/logger/)) and
observe logs as the `mbus` component is interacting with the meter. It will dump all registers
sent back by the meter. Also, correlating this log output with output produced by `mbus-serial-request-data`
from the [libmbus](https://manpages.opensuse.org/Tumbleweed/libmbus/libmbus.1.en.html) package is a
good way to start.

