#include <WiFi.h>
#include <WiFiUdp.h>
#include "driver/i2s.h"

// --- Wi-Fi ve UDP Ayarları ---
const char* ssid = "Xiaomi_FBD7";
const char* password = "M251677y+";

const uint16_t LOCAL_UDP_PORT = 12345;        
const char* targetUdpAddress = "192.168.1.25"; 
const uint16_t TARGET_UDP_PORT = 12346;       

WiFiUDP udp;

// --- BUTON AYARI ---
#define BUTTON_PIN      19   // Bas-Konuş (PTT) Butonu (Pin 19 ve GND arasına bağlayın)

// --- GÖNDERİCİ (Mikrofon - RX) Ayarları ---
#define I2S_MIC_NUM     I2S_NUM_0
#define I2S_MIC_WS      15
#define I2S_MIC_SD      32
#define I2S_MIC_SCK     14
#define SAMPLE_RATE     22050 

// --- ALICI (Hoparlör - TX) Ayarları ---
#define I2S_SPK_NUM     I2S_NUM_1
#define I2S_SPK_BCLK    26
#define I2S_SPK_LRCK    25
#define I2S_SPK_DATA    22

// Gönderici ile aynı boyutta olmalı (512 * 2 = 1024 bayt)
#define BUFFER_LEN      512
#define BUFFER_SIZE     (BUFFER_LEN * sizeof(int16_t))

int16_t micBuffer[BUFFER_LEN];
uint8_t spkBuffer[BUFFER_SIZE];

// Mikrofon (RX) I2S Kurulumu
void setupMicI2S() {
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
    I2S_PIN_NO_CHANGE,  // mck_io_num
    I2S_MIC_SCK,        // bck_io_num
    I2S_MIC_WS,         // ws_io_num
    I2S_PIN_NO_CHANGE,  // data_out_num
    I2S_MIC_SD          // data_in_num
  };

  i2s_driver_install(I2S_MIC_NUM, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_MIC_NUM, &pin_config);
  i2s_start(I2S_MIC_NUM);
}

// Hoparlör (TX) I2S Kurulumu
void setupSpeakerI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 256,
    .use_apll = false,
    .tx_desc_auto_clear = true
  };

  const i2s_pin_config_t pin_config = {
    I2S_PIN_NO_CHANGE,  // mck_io_num
    I2S_SPK_BCLK,       // bck_io_num
    I2S_SPK_LRCK,       // ws_io_num
    I2S_SPK_DATA,       // data_out_num
    I2S_PIN_NO_CHANGE   // data_in_num
  };

  i2s_driver_install(I2S_SPK_NUM, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_SPK_NUM, &pin_config);
  i2s_start(I2S_SPK_NUM);
}

void setup() {
  Serial.begin(115200);

  // Buton pini dahili PULLUP ile başlatılıyor
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Wi-Fi Bağlantısı
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Wi-Fi'ya bağlanılıyor");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWi-Fi Bağlandı!");
  Serial.print("ESP32 IP Adresi: ");
  Serial.println(WiFi.localIP());

  // I2S Donanımlarını Başlat
  setupMicI2S();
  setupSpeakerI2S();

  // UDP Portunu Dinlemeye Başla
  udp.begin(LOCAL_UDP_PORT);
  Serial.printf("UDP Dinleniyor (Port: %d), Gönderilen IP: %s\n", LOCAL_UDP_PORT, targetUdpAddress);
}

void loop() {
  // 1. İŞLEM: Sadece butona basıldığında (LOW) mikrofondan oku ve karşı tarafa gönder
  if (digitalRead(BUTTON_PIN) == LOW) {
    size_t bytesRead = 0;
    // Butona basıldığı sürece I2S buffer'ını okuyoruz
    i2s_read(I2S_MIC_NUM, (char*)micBuffer, sizeof(micBuffer), &bytesRead, 0);

    if (bytesRead > 0) {
      udp.beginPacket(targetUdpAddress, TARGET_UDP_PORT);
      udp.write((uint8_t*)micBuffer, bytesRead);
      udp.endPacket();
    }
  }

  // 2. İŞLEM: Ağdan gelen ses paketini dinle ve hoparlörde çal
  int packetSize = udp.parsePacket();
  if (packetSize > 0) {
    int bytesReceived = udp.read(spkBuffer, BUFFER_SIZE);

    if (bytesReceived > 0) {
      size_t bytesWritten;
      i2s_write(I2S_SPK_NUM, spkBuffer, bytesReceived, &bytesWritten, portMAX_DELAY);
    }
  }
}