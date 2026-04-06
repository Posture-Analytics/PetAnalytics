#include <M5Unified.h>
#include <M5_IMU_PRO.h>
#include <SPI.h>
#include <SD.h>

// ===== CONFIGURAÇÕES IMU EXTERNA =====
#define IMU1_ADDR 0x68
#define IMU2_ADDR 0x69
BMI270::BMI270 ext_imu1;
BMI270::BMI270 ext_imu2;

// ===== CONFIGURAÇÕES ÁUDIO (PADRÃO ML) =====
static const int SAMPLE_RATE = 16000;
static const int BUFFER_SIZE = 256; 
int16_t mic_buffer[BUFFER_SIZE];

// ===== PINOS DO MICROSD (CONFORME SUA TABELA) =====
const int chipSelect = 11; // G11 -> CS
const int mosiPin = 12;    // G12 -> MOSI
const int clkPin = 14;     // G14 -> CLK
const int misoPin = 39;    // G39 -> MISO

// ===== ESTRUTURA DO CABEÇALHO WAV (44 BYTES) =====
struct wav_header_t {
    char chunkID[4] = {'R', 'I', 'F', 'F'};
    uint32_t chunkSize = 0; 
    char format[4] = {'W', 'A', 'V', 'E'};
    char subchunk1ID[4] = {'f', 'm', 't', ' '};
    uint32_t subchunk1Size = 16;
    uint16_t audioFormat = 1; // PCM
    uint16_t numChannels = 1; // Mono
    uint32_t sampleRate = 16000;
    uint32_t byteRate = 16000 * 1 * 2; 
    uint16_t blockAlign = 2; 
    uint16_t bitsPerSample = 16;
    char subchunk2ID[4] = {'d', 'a', 't', 'a'};
    uint32_t subchunk2Size = 0; 
};

// ===== ARQUIVOS E CONTROLE =====
File imuFile;
File audioFile;
uint32_t audioDataSize = 0;
unsigned long frame_count = 0;
bool sd_ready = false;
bool recording = true;

// Função para atualizar o header WAV no SD
void updateWavHeader(File file, uint32_t dataSize) {
    wav_header_t header;
    header.sampleRate = SAMPLE_RATE;
    header.byteRate = SAMPLE_RATE * 1 * 2;
    header.subchunk2Size = dataSize;
    header.chunkSize = dataSize + 36;

    file.seek(0);
    file.write((uint8_t*)&header, sizeof(wav_header_t));
}

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    Serial.begin(921600);

    // --- INICIALIZAÇÃO DO MICROSD ---
    Serial.println(">>> Iniciando SD Card com pinos customizados...");
    SPI.begin(clkPin, misoPin, mosiPin, chipSelect);

    if (SD.begin(chipSelect)) {
        sd_ready = true;
        Serial.println("SD Card OK!");

        if (!SD.exists("/coleta")) SD.mkdir("/coleta");

        imuFile = SD.open("/coleta/imu.csv", FILE_WRITE);
        audioFile = SD.open("/coleta/audio.wav", FILE_WRITE);

        if (!imuFile || !audioFile) {
            Serial.println("Erro ao criar arquivos.");
            sd_ready = false;
        } else {
            // Escreve cabeçalho CSV
            imuFile.println("frame,ax,ay,az,gx,gy,gz,ax1,ay1,az1,gx1,gy1,gz1,ax2,ay2,az2,gx2,gy2,gz2");
            
            // Reserva espaço para o cabeçalho WAV
            wav_header_t header;
            audioFile.write((uint8_t*)&header, sizeof(wav_header_t));
        }
    } else {
        Serial.println("ERRO: Cartao SD nao montado.");
    }

    // --- INICIALIZAÇÃO IMUS E MIC ---
    M5.Ex_I2C.begin();
    ext_imu1.init(I2C_NUM_0, IMU1_ADDR);
    ext_imu2.init(I2C_NUM_0, IMU2_ADDR);

    M5.Speaker.end(); 
    M5.Mic.begin();

    Serial.println(">>> Gravando... Pressione Botao A para parar.");
}

void loop() {
    M5.update();
    
    // Parar gravação se pressionar botão
    if (M5.BtnA.wasPressed()) {
        recording = false;
        if (sd_ready) {
            updateWavHeader(audioFile, audioDataSize);
            audioFile.close();
            imuFile.close();
            Serial.println(">>> Gravacao finalizada e arquivos salvos!");
        }
    }

    if (!recording) return;

    // 1. CAPTURA DE ÁUDIO
    bool mic_ok = M5.Mic.record(mic_buffer, BUFFER_SIZE, SAMPLE_RATE);

    // 2. LEITURA DAS IMUs
    M5.Imu.update();
    auto data = M5.Imu.getImuData();

    float ax1=0, ay1=0, az1=0, gx1=0, gy1=0, gz1=0;
    if (ext_imu1.accelerationAvailable()) {
        ext_imu1.readAcceleration(ax1, ay1, az1);
        ext_imu1.readGyroscope(gx1, gy1, gz1);
    }

    float ax2=0, ay2=0, az2=0, gx2=0, gy2=0, gz2=0;
    if (ext_imu2.accelerationAvailable()) {
        ext_imu2.readAcceleration(ax2, ay2, az2);
        ext_imu2.readGyroscope(gx2, gy2, gz2);
    }

    // 3. ESCRITA NO SD
    if (sd_ready) {
        // IMU CSV
        imuFile.printf("%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                       frame_count, data.accel.x, data.accel.y, data.accel.z,
                       data.gyro.x, data.gyro.y, data.gyro.z,
                       ax1, ay1, az1, gx1, gy1, gz1,
                       ax2, ay2, az2, gx2, gy2, gz2);

        // ÁUDIO WAV
        if (mic_ok) {
            size_t bw = audioFile.write((uint8_t*)mic_buffer, BUFFER_SIZE * sizeof(int16_t));
            audioDataSize += bw;
        }

        // Flush e Update Header a cada 5 segundos (Segurança)
        static unsigned long lastFlush = 0;
        if (millis() - lastFlush > 5000) {
            updateWavHeader(audioFile, audioDataSize);
            audioFile.seek(audioDataSize + sizeof(wav_header_t)); 
            
            audioFile.flush();
            imuFile.flush();
            lastFlush = millis();
            Serial.printf("Frame: %lu | Audio Size: %u bytes\n", frame_count, audioDataSize);
        }
    }

    frame_count++;
}