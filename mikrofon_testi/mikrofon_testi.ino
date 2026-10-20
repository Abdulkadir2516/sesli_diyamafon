#include <Arduino.h>
#include <driver/i2s.h>

#define I2S_WS   19
#define I2S_SCK  14
#define I2S_SD   32
#define I2S_PORT I2S_NUM_0

void setup() {
  Serial.begin(115200);

  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // INMP441 için 32-bit şart
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 256,
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

void loop() {
  int32_t sampleBuffer[64];
  size_t bytesRead = 0;

  // Mikrofondan 32-bit veri oku
  i2s_read(I2S_PORT, &sampleBuffer, sizeof(sampleBuffer), &bytesRead, portMAX_DELAY);

  int samplesRead = bytesRead / 4; // 32-bit = 4 byte
  for (int i = 0; i < samplesRead; i++) {
    // 24-bitlik gerçek veriyi almak için 14 bit sağa kaydırıyoruz
    int32_t pcm = sampleBuffer[i] >> 14; 
    Serial.println(pcm);
  }
}