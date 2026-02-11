// NOTE: This simulation was built before the physical hardware arrived.
// Finger percentages and gesture detection are functional in Wokwi.

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/i2c.h"
#include "esp_log.h"

static const char *TAG = "AuraLink";

// --- Finger Sensor Pins (ADC1 channels) ---
// GPIO32 = ADC1_CH4, GPIO33 = ADC1_CH5, GPIO34 = ADC1_CH6
// GPIO35 = ADC1_CH7, GPIO36 = ADC1_CH0
static const adc1_channel_t fingerChannels[] = {
    ADC1_CHANNEL_4,  // Thumb   - GPIO32
    ADC1_CHANNEL_5,  // Index   - GPIO33
    ADC1_CHANNEL_6,  // Middle  - GPIO34
    ADC1_CHANNEL_7,  // Ring    - GPIO35
    ADC1_CHANNEL_0   // Pinky   - GPIO36
};
static const char* fingerNames[] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
static int fingerPercent[5];

// --- MPU6050 ---
#define I2C_MASTER_SCL_IO       22
#define I2C_MASTER_SDA_IO       21
#define I2C_MASTER_NUM          I2C_NUM_0
#define I2C_MASTER_FREQ_HZ      400000
#define MPU6050_ADDR            0x68
#define MPU6050_PWR_MGMT_1      0x6B
#define MPU6050_ACCEL_XOUT_H    0x3B

// --- Thresholds ---
#define BENT     60
#define STRAIGHT 30

// --- I2C Init ---
static void i2c_master_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

// --- Wake up MPU6050 ---
static void mpu6050_init(void) {
    uint8_t data[2] = {MPU6050_PWR_MGMT_1, 0x00};
    i2c_master_write_to_device(I2C_MASTER_NUM, MPU6050_ADDR, data, 2, 1000 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "MPU6050 initialized");
}

// --- Read MPU6050 Accel ---
static void mpu6050_read_accel(int16_t *ax, int16_t *ay, int16_t *az) {
    uint8_t reg = MPU6050_ACCEL_XOUT_H;
    uint8_t buf[6];
    i2c_master_write_read_device(I2C_MASTER_NUM, MPU6050_ADDR, &reg, 1, buf, 6, 1000 / portTICK_PERIOD_MS);
    *ax = (int16_t)((buf[0] << 8) | buf[1]);
    *ay = (int16_t)((buf[2] << 8) | buf[3]);
    *az = (int16_t)((buf[4] << 8) | buf[5]);
}

// --- Read Fingers ---
static void read_fingers(void) {
    for (int i = 0; i < 5; i++) {
        int raw = adc1_get_raw(fingerChannels[i]);
        // Map 0-4095 to 0-100%
        fingerPercent[i] = (raw * 100) / 4095;
    }
}

// --- Gesture Detection ---
static int is_fist(void) {
    for (int i = 0; i < 5; i++)
        if (fingerPercent[i] < BENT) return 0;
    return 1;
}

static int is_open(void) {
    for (int i = 0; i < 5; i++)
        if (fingerPercent[i] > STRAIGHT) return 0;
    return 1;
}

static int is_two_fingers(void) {
    return fingerPercent[0] > BENT &&
           fingerPercent[1] < STRAIGHT &&
           fingerPercent[2] < STRAIGHT &&
           fingerPercent[3] > BENT &&
           fingerPercent[4] > BENT;
}

static int is_thumbs_up(void) {
    return fingerPercent[0] < STRAIGHT &&
           fingerPercent[1] > BENT &&
           fingerPercent[2] > BENT &&
           fingerPercent[3] > BENT &&
           fingerPercent[4] > BENT;
}

static const char* classify_gesture(void) {
    if (is_fist())       return "FIST";
    if (is_open())       return "OPEN";
    if (is_two_fingers()) return "TWO_FINGERS";
    if (is_thumbs_up())  return "THUMBS_UP";
    return "UNKNOWN";
}

// --- Main Task ---
static void auralink_task(void *arg) {
    int16_t ax, ay, az;

    while (1) {
        read_fingers();

        // Print finger data
        printf(">>> ");
        for (int i = 0; i < 5; i++) {
            printf("%s: %d%% | ", fingerNames[i], fingerPercent[i]);
        }

        // Read and print MPU6050
        mpu6050_read_accel(&ax, &ay, &az);
        printf("Tilt X: %d | Y: %d | Z: %d\n", ax / 1000, ay / 1000, az / 1000);

        // Classify gesture
        const char* gesture = classify_gesture();
        printf("Gesture: %s\n", gesture);

        // Log what action WOULD happen (no BLE in this version)
        if (strcmp(gesture, "FIST") == 0)
            ESP_LOGI(TAG, "Would send: Ctrl+C (Copy)");
        else if (strcmp(gesture, "TWO_FINGERS") == 0)
            ESP_LOGI(TAG, "Would send: Ctrl+V (Paste)");
        else if (strcmp(gesture, "THUMBS_UP") == 0)
            ESP_LOGI(TAG, "Would send: Ctrl+Z (Undo)");
        else if (strcmp(gesture, "OPEN") == 0)
            ESP_LOGI(TAG, "Would send: Win Key (Launcher)");

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "AuraLink Wokwi Simulation Starting...");

    // ADC setup
    adc1_config_width(ADC_WIDTH_BIT_12);
    for (int i = 0; i < 5; i++) {
        adc1_config_channel_atten(fingerChannels[i], ADC_ATTEN_DB_11);
    }

    // I2C + MPU6050
    i2c_master_init();
    mpu6050_init();

    // Start main task
    xTaskCreate(auralink_task, "auralink_task", 4096, NULL, 5, NULL);
}
