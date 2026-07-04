/*
 * ESP32 Audio Player with Spotify
 * Spotify-connected music player with touchscreen and voice control
 * 
 * Components:
 * - MAX98357 (I2S Audio Amplifier)
 * - 20W Speaker
 * - 2.8" Touch Screen
 * - Microphone
 * - SD Card Module
 * 
 * Libraries Required:
 * - WiFi Library
 * - HTTPClient Library
 * - ArduinoJson Library
 * - I2S Library
 * - SD Library
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <driver/i2s.h>
#include <SD.h>

// ===== WiFi Credentials =====
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ===== Spotify API =====
const char* spotifyToken = "YOUR_SPOTIFY_ACCESS_TOKEN";
const char* spotifyBase = "https://api.spotify.com/v1";

// ===== Pin Definitions =====
#define I2S_BCK 26
#define I2S_WS 25
#define I2S_DOUT 22
#define SD_CS 5
#define MIC_PIN 34

// ===== Variables =====
String currentTrack = "No track playing";
String currentArtist = "";
String currentAlbum = "";
int volume = 50;
bool isPlaying = false;
unsigned long lastHeartbeat = 0;

// ===== Setup =====
void setup() {
    Serial.begin(115200);
    
    // Initialize SD Card
    if (!SD.begin(SD_CS)) {
        Serial.println("SD Card initialization failed!");
    }
    
    // Initialize I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_DOUT,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_set_sample_rates(I2S_NUM_0, 44100);
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("WiFi Connected!");
    
    Serial.println("Audio Player Ready!");
}

// ===== Main Loop =====
void loop() {
    if (millis() - lastHeartbeat > 10000) {
        getCurrentTrack();
        lastHeartbeat = millis();
    }
    
    // Check for voice command (simplified)
    if (analogRead(MIC_PIN) > 2000) {
        processVoiceCommand();
    }
    
    delay(100);
}

// ===== Get Current Track from Spotify =====
void getCurrentTrack() {
    if (WiFi.status() != WL_CONNECTED) return;
    
    HTTPClient http;
    http.begin(spotifyBase + String("/me/player/currently-playing"));
    http.addHeader("Authorization", "Bearer " + String(spotifyToken));
    
    int httpCode = http.GET();
    if (httpCode == 200) {
        String payload = http.getString();
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, payload);
        
        if (doc["item"]["name"].is<String>()) {
            currentTrack = doc["item"]["name"].as<String>();
            currentArtist = doc["item"]["artists"][0]["name"].as<String>();
            currentAlbum = doc["item"]["album"]["name"].as<String>();
            isPlaying = doc["is_playing"];
            
            Serial.println("Now Playing: " + currentTrack + " by " + currentArtist);
        }
    }
    http.end();
}

// ===== Process Voice Command =====
void processVoiceCommand() {
    Serial.println("Voice command detected!");
    // In real implementation, use speech recognition
    playNextTrack();
}

// ===== Play Track =====
void playTrack(String trackId) {
    if (WiFi.status() != WL_CONNECTED) return;
    
    HTTPClient http;
    http.begin(spotifyBase + String("/me/player/play"));
    http.addHeader("Authorization", "Bearer " + String(spotifyToken));
    http.addHeader("Content-Type", "application/json");
    
    String body = "{\"uris\":[\"spotify:track:" + trackId + "\"]}";
    int httpCode = http.PUT(body);
    http.end();
}

// ===== Play Next Track =====
void playNextTrack() {
    if (WiFi.status() != WL_CONNECTED) return;
    
    HTTPClient http;
    http.begin(spotifyBase + String("/me/player/next"));
    http.addHeader("Authorization", "Bearer " + String(spotifyToken));
    http.POST("");
    http.end();
}

// ===== Set Volume =====
void setVolume(int vol) {
    volume = constrain(vol, 0, 100);
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(spotifyBase + String("/me/player/volume?volume_percent=") + String(volume));
        http.addHeader("Authorization", "Bearer " + String(spotifyToken));
        http.PUT("");
        http.end();
    }
}

// ===== Play/Pause =====
void togglePlayback() {
    if (WiFi.status() != WL_CONNECTED) return;
    
    HTTPClient http;
    if (isPlaying) {
        http.begin(spotifyBase + String("/me/player/pause"));
    } else {
        http.begin(spotifyBase + String("/me/player/play"));
    }
    http.addHeader("Authorization", "Bearer " + String(spotifyToken));
    http.PUT("");
    http.end();
    
    isPlaying = !isPlaying;
}
