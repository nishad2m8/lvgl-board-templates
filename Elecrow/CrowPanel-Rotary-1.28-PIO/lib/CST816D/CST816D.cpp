#include "CST816D.h"

CST816D::CST816D(int8_t sda_pin, int8_t scl_pin, int8_t rst_pin, int8_t int_pin)
{
    _sda = sda_pin;
    _scl = scl_pin;
    _rst = rst_pin;
    _int = int_pin;
}

void CST816D::begin(void)
{
    // Initialize I2C on Wire1
    if (_sda != -1 && _scl != -1)
    {
        Wire1.begin(_sda, _scl);
    }
    else
    {
        Wire1.begin();
    }

    // Int Pin Configuration
    if (_int != -1)
    {
        pinMode(_int, OUTPUT);
        digitalWrite(_int, HIGH);
        delay(1);
        digitalWrite(_int, LOW);
        delay(1);
    }

    // Reset Pin Configuration
    if (_rst != -1)
    {
        pinMode(_rst, OUTPUT);
        digitalWrite(_rst, LOW);
        delay(10);
        digitalWrite(_rst, HIGH);
        delay(300);
    }

    // Initialize Touch
    i2c_write(0xFE, 0XFF);
}

bool CST816D::getTouch(uint16_t *x, uint16_t *y, uint8_t *gesture)
{
    bool FingerIndex = false;
    FingerIndex = (bool)i2c_read(0x02);

    *gesture = i2c_read(0x01);
    if (!(*gesture == SlideUp || *gesture == SlideDown))
    {
        *gesture = None;
    }

    uint8_t data[4];
    i2c_read_continuous(0x03, data, 4);
    *x = ((data[0] & 0x0f) << 8) | data[1];
    *y = ((data[2] & 0x0f) << 8) | data[3];

    return FingerIndex;
}

uint8_t CST816D::i2c_read(uint8_t addr)
{
    uint8_t rdData;
    uint8_t rdDataCount;
    do
    {
        Wire1.beginTransmission(I2C_ADDR_CST816D);
        Wire1.write(addr);
        Wire1.endTransmission(false);
        rdDataCount = Wire1.requestFrom(I2C_ADDR_CST816D, 1);
    } while (rdDataCount == 0);
    while (Wire1.available())
    {
        rdData = Wire1.read();
    }
    return rdData;
}

uint8_t CST816D::i2c_read_continuous(uint8_t addr, uint8_t *data, uint32_t length)
{
    Wire1.beginTransmission(I2C_ADDR_CST816D);
    Wire1.write(addr);
    if (Wire1.endTransmission(true)) return -1;
    Wire1.requestFrom(I2C_ADDR_CST816D, length);
    for (int i = 0; i < length; i++) {
        *data++ = Wire1.read();
    }
    return 0;
}

void CST816D::i2c_write(uint8_t addr, uint8_t data)
{
    Wire1.beginTransmission(I2C_ADDR_CST816D);
    Wire1.write(addr);
    Wire1.write(data);
    Wire1.endTransmission();
}

uint8_t CST816D::i2c_write_continuous(uint8_t addr, const uint8_t *data, uint32_t length)
{
    Wire1.beginTransmission(I2C_ADDR_CST816D);
    Wire1.write(addr);
    for (int i = 0; i < length; i++) {
        Wire1.write(*data++);
    }
    if (Wire1.endTransmission(true)) return -1;
    return 0;
}
