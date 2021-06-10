#include <driver/gpio.h>
#include <driver/i2c.h>

// SPI
#define PIN_NUM_MISO    GPIO_NUM_19
#define PIN_NUM_MOSI    GPIO_NUM_23
#define PIN_NUM_CLK     GPIO_NUM_18
#define PIN_NUM_CS      GPIO_NUM_5

// I2C
#define I2C_PORT        I2C_NUM_0
#define PIN_NUM_SCL     GPIO_NUM_22
#define PIN_NUM_SDA     GPIO_NUM_21

// Lift
#define PIN_NUM_ENA        GPIO_NUM_26
#define PIN_NUM_DIR        GPIO_NUM_21
#define PIN_NUM_PUL        GPIO_NUM_22
#define PIN_NUM_END_DOWN   GPIO_NUM_16
#define PIN_NUM_END_UP     GPIO_NUM_17