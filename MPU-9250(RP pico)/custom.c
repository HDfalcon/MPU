#include "pico/stdlib.h"
#include "stdio.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "string.h"

void gyro_offset();

#define LED PICO_DEFAULT_LED_PIN

// required registers
#define MPU_ADDR 0x68
#define PWR_MGMT 0x6B
#define GYRO_CNFG 0x1B
#define GYRO_XOUT_H 0x43

//  required values
int16_t gyro_raw;
uint8_t buffer[2], c_buffer[2];
float gyro_cal = 0;

int main() {
    stdio_init_all();

    gpio_init(LED);
    gpio_set_dir(LED, 1);

    i2c_init(i2c_default, 400*1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    uint8_t PWR_CNFG[] = {PWR_MGMT, 0x00};
    i2c_write_blocking(i2c_default, MPU_ADDR, PWR_CNFG, 2, false);
    
    uint8_t gyro_config[] = {GYRO_CNFG, 0x10}; // Assuming you want ±1000 degrees/sec
    i2c_write_blocking(i2c_default, MPU_ADDR, gyro_config, sizeof(gyro_config), false);

    gyro_offset();

    float elapsed_time;
    uint32_t previous_time = time_us_32(), current_time;
    float gyro_x; 
    float angle_x = 0;
    while(1){
        c_buffer[0] = 0x43;
        i2c_write_blocking(i2c_default, MPU_ADDR, c_buffer, 1, true); // true to keep master control of bus
        i2c_read_blocking(i2c_default, MPU_ADDR, c_buffer, 2, false);   // false finished with bus
        gyro_raw = ((c_buffer[0] << 8 | c_buffer[1]));
        gyro_raw -= gyro_cal;

        gyro_x = (gyro_raw) / 32.8;

        current_time = time_us_32();
        elapsed_time = (current_time - previous_time) / 1000000.0;
        previous_time = current_time;

        angle_x += gyro_x * elapsed_time;
        printf("%f %f\n", angle_x, gyro_x);
        
        sleep_ms(100);
    }
}


void gyro_offset(){
    for(uint16_t x = 0; x < 2000; x++){
        c_buffer[0] = 0x43;
        i2c_write_blocking(i2c_default, MPU_ADDR, c_buffer, 1, true); // true to keep master control of bus
        i2c_read_blocking(i2c_default, MPU_ADDR, c_buffer, 2, false);   // false finished with bus
        gyro_raw = (c_buffer[0] << 8 | c_buffer[1]);
        gyro_cal += gyro_raw;
        sleep_ms(3); 
    }
    gyro_cal /= 2000;

    printf("%f\n", gyro_cal);
    gpio_put(LED, 1);
    sleep_ms(500);
    gpio_put(LED, 0);
}