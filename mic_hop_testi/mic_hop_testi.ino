#include <Arduino.h>
#include <driver/i2s.h>
#include <driver/dac.h>

#define DAC_CHANNEL DAC_CHANNEL_2 // GPIO 26 (Amfi Girişi)

// INMP441 Pinleri
#define I2S_WS   19
#define I2S_SCK  14
#define I2S_SD   32
#define I2S_PORT I2S_NUM_0

#define SAMPLE_RATE 16000 

// Yazılımsal Ses Kazancı (Ses az gelirse 8 veya 16 yapabilirsin)
#define GAIN 8 

void setupI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = 128,
    .use_apll = false
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
}

void setup() {
  dac_output_enable(DAC_CHANNEL);
  setupI2S();
}

void loop() {
  int32_t sampleBuffer[64];
  size_t bytesRead = 0;

  i2s_read(I2S_PORT, &sampleBuffer, sizeof(sampleBuffer), &bytesRead, portMAX_DELAY);

  int samplesRead = bytesRead / 4;
  for (int i = 0; i < samplesRead; i++) {
    // 32-bit paketten 24-bit ses verisini hizala
    int32_t pcm = sampleBuffer[i] >> 8; 

    // Ses genliğini DAC (0-255) ölçeğine oturtup GAIN ile çarpıyoruz
    int32_t val = ((pcm >> 12) * GAIN) + 128;

    // Sınırlandırma (Clamping)
    if (val > 255) val = 255;
    if (val < 0) val = 0;

    dac_output_voltage(DAC_CHANNEL, (uint8_t)val);
  }
}