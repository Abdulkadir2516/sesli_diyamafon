import socket
import pyaudio
import numpy as np

# UDP Dinleme Ayarları
UDP_IP = "0.0.0.0" 
UDP_PORT = 12346

# Ses (PyAudio) Ayarları
FORMAT = pyaudio.paInt16
CHANNELS = 1          
RATE = 22050          
CHUNK = 1024          

# Ses Yükseltme Katsayısı (Gain)
# 1.0 orijinal boyuttur. Sesi iki katına çıkarmak için 2.0 yapabilirsin, 
# cızırtı yaparsa 1.5 veya 1.8 deneyebilirsin.
VOLUME_MULTIPLIER = 2.5

# UDP Soketi Oluşturma
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

# PyAudio Başlatma
p = pyaudio.PyAudio()
stream = p.open(
    format=FORMAT,
    channels=CHANNELS,
    rate=RATE,
    output=True,
    frames_per_buffer=CHUNK
)

print(f"UDP Port {UDP_PORT} dinleniyor... Sesi durdurmak için Ctrl+C yapın.")

try:
    while True:
        # UDP Paketini al
        data, addr = sock.recvfrom(2048)
        
        if data:
            # Gelen byte verisini NumPy dizisine (int16) çevir
            audio_data = np.frombuffer(data, dtype=np.int16).astype(np.float32)
            
            # Sesi ses katsayısı ile çarp
            audio_data = audio_data * VOLUME_MULTIPLIER
            
            # 16-bit ses sınırlarının (clipping olmaması için -32768 ile 32767 arası) dışına taşmayı engelle
            audio_data = np.clip(audio_data, -32768, 32767)
            
            # Tekrar byte formatına dönüştür
            processed_data = audio_data.astype(np.int16).tobytes()
            
            # Hoparlörde çal
            stream.write(processed_data)

except KeyboardInterrupt:
    print("\nSonlandırılıyor...")
finally:
    stream.stop_stream()
    stream.close()
    p.terminate()
    sock.close()