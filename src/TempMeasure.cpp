
#include "TempMeasure.hpp"

void PositiveTempMeasure::setup()
{
    if (!_ds.search(_addr))
    {
        Serial.println("No more addresses.");
        Serial.println();
        _ds.reset_search();
        delay(250);
        return;
    }

    Serial.print("ROM =");
    for (byte i = 0; i < 8; i++)
    {
        Serial.write(' ');
        Serial.print(_addr[i], HEX);
    }

    if (OneWire::crc8(_addr, 7) != _addr[7])
    {
        Serial.println("CRC is not valid!");
        return;
    }
    Serial.println();

    // the first ROM byte indicates which chip
    switch (_addr[0])
    {
    case 0x10:
        Serial.println("  Chip = DS18S20"); // or old DS1820
        _type_s = 1;
        break;
    case 0x28:
        Serial.println("  Chip = DS18B20");
        _type_s = 0;
        break;
    case 0x22:
        Serial.println("  Chip = DS1822");
        _type_s = 0;
        break;
    default:
        Serial.println("Device is not a DS18x20 family device.");
    }
}

void PositiveTempMeasure::loop()
{
    if (_type_s == 0xff)
        return; // not a valid device

    byte present = 0;    
    byte data[12];

    if (!_conversion_pending) {
        _ds.reset();
        _ds.select(_addr);
        _ds.write(0x44, 1); // start conversion, with parasite power on at the end
        _conversion_pending = true;
    }

    if (_ds.read_bit() == 0)    // conversion in progress
        return;
    _conversion_pending = false;

    present = _ds.reset();
    _ds.select(_addr);
    _ds.write(0xBE); // Read Scratchpad

    // Serial.print("  Data = ");
    // Serial.print(present, HEX);
    // Serial.print(" ");
    for (byte i = 0; i < 9; i++)
    { // we need 9 bytes
        data[i] = _ds.read();
        // Serial.print(data[i], HEX);
        // Serial.print(" ");
    }
    // Serial.print(" CRC=");
    // Serial.print(OneWire::crc8(data, 8), HEX);
    // Serial.println();

    // Convert the data to actual temperature
    // because the result is a 16 bit signed integer, it should
    // be stored to an "int16_t" type, which is always 16 bits
    // even when compiled on a 32 bit processor.
    int16_t value = (data[1] << 8) | data[0];
    if (_type_s)
    {
        value = value << 3; // 9 bit resolution default
        if (data[7] == 0x10)
        {
            // "count remain" gives full 12 bit resolution
            value = (value & 0xFFF0) + 12 - data[6];
        }
    }
    else
    {
        byte cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00)
            value = value & ~7; // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20)
            value = value & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40)
            value = value & ~1; // 11 bit res, 375 ms
                            //// default is 12 bit resolution, 750 ms conversion time
    }

    // Only measure positive temperatures to get rid of sign bit (i.e. 11 bits instead of 12)
    if (value > 0) {
        _value.add(value);
    }

    // Serial.print("  Temperature = ");
    // Serial.println(currentTemp());
    // Serial.print("  Raw temperature = ");
    // Serial.println(currentRawTemp(10), HEX);
    // Serial.print("  Temperature from raw = ");
    // Serial.println(convertRawTemp(currentRawTemp(10), 10));
}

uint16_t PositiveTempMeasure::currentRawTemp(byte precision_bits)
{
    uint16_t value = _value.avg();
    if (precision_bits >= 11)
        return value;
    return value >> (11 - precision_bits);
}

float PositiveTempMeasure::currentTemp()
{
    return (float)_value.avg() / 16.0; 
}

float PositiveTempMeasure::convertRawTemp(uint16_t raw, byte precision_bits)
{
    if (precision_bits < 11)
        raw = raw << (11 - precision_bits);

    return (float)raw / 16.0;
}
