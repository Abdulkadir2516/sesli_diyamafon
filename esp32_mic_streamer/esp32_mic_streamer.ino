#include <WiFi.h>
#include <WiFiUdp.h>
#include "driver/i2s.h"

const char* ssid = "TurkTelekom_Z433V";
const char* password = "c2bE7E4BBEf77";
const char* udpAddress = "192.168.1.38"; // Alıcı IP Adresiniz
const int udpPort = 12345;

WiFiUDP udp;

#define I2S_WS   15
#define I2S_SD   32
#define I2S_SCK  14
#define I2S_PORT I2S_NUM_0

// Buton Pini (INPUT_PULLUP kullanıldığı için butonun diğer bacağını GND'ye bağlamalısınız)
#define BUTTON_PIN 12

// Alıcı ile aynı sample rate
#define SAMPLE_RATE 22050
#define BUFFER_LEN  512

int16_t sBuffer[BUFFER_LEN];

void i2s_install() {
  const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 256,
    .use_apll = false
  };

  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
}

void setup() {
  Serial.begin(115200);

  // Buton pinini dahili pull-up direnci ile ayarla
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[Verici] WiFi Baglandi!");

  i2s_install();
  i2s_start(I2S_PORT);
}

void loop() {
  // Butona basılıp basılmadığını kontrol et (INPUT_PULLUP nedeniyle basıldığında LOW olur)
  if (digitalRead(BUTTON_PIN) == LOW) {
    size_t bytesRead = 0;
    i2s_read(I2S_PORT, (char*)sBuffer, sizeof(sBuffer), &bytesRead, portMAX_DELAY);

    if (bytesRead > 0) {
      udp.beginPacket(udpAddress, udpPort);
      udp.write((uint8_t*)sBuffer, bytesRead);
      udp.endPacket();
    }
  } else {
    // Butona basılmıyorsa I2S buffer'ını boşta okuyup şişmesini önle veya kısa bir gecikme ver
    size_t bytesRead = 0;
    i2s_read(I2S_PORT, (char*)sBuffer, sizeof(sBuffer), &bytesRead, 10);
  }
}