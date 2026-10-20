#include <Arduino.h>
#include <driver/dac.h>

// Basit bir test melodisinin frekansları (Hz)
int melody[] = {262, 294, 330, 349, 392, 440, 494, 523}; // Do-Re-Mi-Fa-Sol-La-Si-Do
int noteDuration = 250; // Her notanın süresi (milisaniye)

// Belirtilen frekansta kare dalga ses üretir
void playTone(int frequency, int durationMs) {
  if (frequency == 0) {
    dac_output_voltage(DAC_CHANNEL_1, 0);
    delay(durationMs);
    return;
  }

  long periodUs = 1000000L / frequency;
  long halfPeriodUs = periodUs / 2;
  long startTime = millis();

  while (millis() - startTime < durationMs) {
    dac_output_voltage(DAC_CHANNEL_1, 100); // Yüksek seviye
    delayMicroseconds(halfPeriodUs);
    dac_output_voltage(DAC_CHANNEL_1, 0);   // Düşük seviye
    delayMicroseconds(halfPeriodUs);

    dac_output_voltage(DAC_CHANNEL_2, 100); // Yüksek seviye
    delayMicroseconds(halfPeriodUs);
    dac_output_voltage(DAC_CHANNEL_2, 0);   // Düşük seviye
    delayMicroseconds(halfPeriodUs);
  }
}

void setup() {
  // GPIO 25 pinini DAC1 olarak aktifleştir
  dac_output_enable(DAC_CHANNEL_1);
    dac_output_enable(DAC_CHANNEL_2);

}

void loop() {
  // Melodideki notaları sırayla çal
  for (int i = 0; i < 8; i++) {
    playTone(melody[i], noteDuration);
    delay(50); // Notalar arası kısa boşluk
  }

  delay(2000); // Melodi bitince 2 saniye bekle ve tekrar çal
}