# 🔊 Unitree G1 Audio Client Example

## 📖 Overview / အကျဉ်းချုပ်

ဤ example သည် Unitree G1 humanoid robot ၏ audio system ကို control လုပ်နည်းကို ပြသထားသည်။ အောက်ပါ features များ ပါဝင်သည်:

- **Text-to-Speech (TTS)** - စာသားကို အသံထွက်ပြောခြင်း
- **Audio Playback** - WAV ဖိုင် ဖွင့်ခြင်း
- **Volume Control** - အသံအတိုးအလျော့ ထိန်းချုပ်ခြင်း
- **Microphone Recording** - အသံသွင်းခြင်း
- **ASR (Automatic Speech Recognition)** - အသံမှတ်သားခြင်း
- **LED Control** - LED မီး ထိန်းချုပ်ခြင်း

---

## 🔧 Dependencies / လိုအပ်သော Libraries

```cpp
#include <fstream>                                      // File operations
#include <iostream>                                     // Console I/O
#include <thread>                                       // Multi-threading
#include <unitree/common/time/time_tool.hpp>           // Time utilities
#include <unitree/idl/ros2/String_.hpp>                // ROS2 String message
#include <unitree/robot/channel/channel_subscriber.hpp> // DDS Subscriber
#include <unitree/robot/g1/audio/g1_audio_client.hpp>  // Audio Client API
#include "wav.hpp"                                      // WAV file handling
```

---

## 📝 Constants / သတ်မှတ်ချက်များ

```cpp
#define AUDIO_FILE_PATH "../example/g1/audio/test.wav"  // ဖွင့်မည့် audio file
#define AUDIO_SUBSCRIBE_TOPIC "rt/audio_msg"            // ASR message topic
#define GROUP_IP "239.168.123.161"                      // Multicast group IP
#define PORT 5555                                        // UDP port

#define WAV_SECOND 5                    // Record duration (seconds)
#define WAV_LEN (16000 * 2 * WAV_SECOND) // Total bytes (16kHz, 16-bit, 5s)
#define WAV_LEN_ONCE (16000 * 2 * 160 / 1000)  // Bytes per chunk
#define CHUNK_SIZE 96000                // 3 seconds per chunk for playback
```

| Constant | တန်ဖိုး | ရှင်းလင်းချက် |
|----------|--------|--------------|
| `WAV_LEN` | 160,000 bytes | 16kHz × 2 bytes × 5s |
| `CHUNK_SIZE` | 96,000 bytes | 16kHz × 2 bytes × 3s |

---

## 🎯 Code Structure / Code ဖွဲ့စည်းပုံ

```
main()
├── ChannelFactory Init
├── AudioClient Init
├── ASR Subscriber Setup
├── Volume Control Demo
├── TTS Demo (Chinese + English)
├── Audio Playback Demo
├── LED Control Demo
└── Microphone Recording Thread
```

---

## 📝 Code ရှင်းလင်းချက်

### 1. ASR Message Handler

```cpp
void asr_handler(const void *msg) {
  std_msgs::msg::dds_::String_ *resMsg = (std_msgs::msg::dds_::String_ *)msg;
  std::cout << "Topic:\"rt/audio_msg\" recv: " << resMsg->data() << std::endl;
}
```

- Robot ၏ speech recognition result ကို receive လုပ်သည်
- `rt/audio_msg` topic မှ message များကို နားထောင်သည်

---

### 2. Local IP Detection

```cpp
std::string get_local_ip_for_multicast() {
  // Network interfaces များကို scan
  // "192.168.123.x" subnet ရှိ IP ကို return
}
```

- Multicast audio stream အတွက် local IP ကို ရှာသည်
- Robot network subnet `192.168.123.x` ကို ရှာဖွေသည်

---

### 3. Microphone Recording Thread

```cpp
void thread_mic(void) {
  // UDP socket ဖွင့်ခြင်း
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  
  // Multicast group join
  inet_pton(AF_INET, GROUP_IP, &mreq.imr_multiaddr);
  setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
  
  // Audio data receive
  while (total_bytes < WAV_LEN) {
    ssize_t len = recvfrom(sock, buffer, sizeof(buffer), 0, nullptr, nullptr);
    // PCM data ကို collect
  }
  
  // WAV file save
  WriteWave("record.wav", 16000, pcm_data.data(), pcm_data.size(), 1);
}
```

| Step | Description |
|------|-------------|
| 1 | UDP socket ဖန်တီး |
| 2 | Multicast group `239.168.123.161` သို့ join |
| 3 | Audio data ကို 5 seconds ကြာ receive |
| 4 | `record.wav` အဖြစ် save |

---

### 4. Main Function - Initialization

```cpp
unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);
unitree::robot::g1::AudioClient client;
client.Init();
client.SetTimeout(10.0f);
```

| Method | ရှင်းလင်းချက် |
|--------|-------------|
| `Init(0, argv[1])` | Domain ID 0 + Network interface (eth0) |
| `SetTimeout(10.0f)` | 10 seconds timeout |

---

### 5. ASR Subscriber Setup

```cpp
unitree::robot::ChannelSubscriber<std_msgs::msg::dds_::String_> subscriber(
    AUDIO_SUBSCRIBE_TOPIC);
subscriber.InitChannel(asr_handler);
```

- Robot ပြောသော စကားကို recognize လုပ်ပြီး result ကို receive လုပ်ရန်
- Callback function `asr_handler` ကို register

---

### 6. Volume Control

```cpp
uint8_t volume;
ret = client.GetVolume(volume);    // လက်ရှိ volume ဖတ်ခြင်း
ret = client.SetVolume(100);       // Volume ကို 100% သတ်မှတ်ခြင်း
```

| Method | Parameter | Return |
|--------|-----------|--------|
| `GetVolume(volume)` | uint8_t& (0-100) | int32_t |
| `SetVolume(value)` | uint8_t (0-100) | int32_t |

---

### 7. Text-to-Speech (TTS)

```cpp
// Chinese TTS
ret = client.TtsMaker("你好。我是宇树科技的机器人。", 0);

// English TTS
ret = client.TtsMaker("Hello. I'm a robot from Unitree Robotics.", 1);
```

| Parameter | Value | Language |
|-----------|-------|----------|
| Language ID | 0 | Chinese (中文) |
| Language ID | 1 | English |

---

### 8. Audio Playback (WAV File)

```cpp
// WAV file ဖတ်ခြင်း
std::vector<uint8_t> pcm = ReadWave(AUDIO_FILE_PATH, &sample_rate, &num_channels, &filestate);

// Streaming playback (3 seconds per chunk)
while (offset < total_size) {
    std::vector<uint8_t> chunk(pcm.begin() + offset,
                               pcm.begin() + offset + current_chunk_size);
    client.PlayStream("example", stream_id, chunk);
    offset += current_chunk_size;
}

// Playback stop
client.PlayStop(stream_id);
```

**WAV File Requirements:**
| Parameter | Required Value |
|-----------|---------------|
| Sample Rate | 16000 Hz |
| Channels | 1 (Mono) |
| Bits per Sample | 16-bit |

---

### 9. LED Control

```cpp
client.LedControl(0, 255, 0);   // Green
client.LedControl(0, 0, 0);     // Off
client.LedControl(0, 0, 255);   // Blue
```

| Parameter | Description |
|-----------|-------------|
| R | Red (0-255) |
| G | Green (0-255) |
| B | Blue (0-255) |

---

## 🔨 Build Instructions

```bash
# Build directory သို့ သွား
cd /path/to/unitree_sdk2/build

# Compile
make g1_audio_client_example
```

---

## 🚀 Run Instructions

```bash
# Network interface ကို argument အဖြစ် ပေးရမည်
./g1_audio_client_example eth0
```

**⚠️ Arguments:**
| Argument | Description | Example |
|----------|-------------|---------|
| Network Interface | Robot နှင့် ချိတ်ဆက်ထားသော interface | eth0, enp0s3 |

---

## 📊 Program Flow

```
┌─────────────────────────────────────────────────┐
│                  Program Start                   │
└─────────────────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────┐
│          Initialize ChannelFactory               │
│          Initialize AudioClient                  │
│          Setup ASR Subscriber                    │
└─────────────────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────┐
│              Volume Control Demo                 │
│         GetVolume → SetVolume(100)              │
└─────────────────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────┐
│                  TTS Demo                        │
│         Chinese → Wait 5s → English → Wait 8s   │
└─────────────────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────┐
│             Audio Playback Demo                  │
│         Load test.wav → Stream Play             │
└─────────────────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────┐
│              LED Control Demo                    │
│         Green → Off → Blue                      │
└─────────────────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────┐
│        Start Microphone Recording Thread         │
│         Record 5s → Save record.wav             │
└─────────────────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────┐
│            Wait for ASR Messages                 │
│               (Infinite Loop)                    │
└─────────────────────────────────────────────────┘
```

---

## 📚 API Reference

### AudioClient Class Methods

| Method | Parameters | Return | Description |
|--------|------------|--------|-------------|
| `Init()` | - | void | Client initialize |
| `SetTimeout(float)` | seconds | void | Timeout သတ်မှတ် |
| `GetVolume(uint8_t&)` | volume ref | int32_t | Volume ဖတ် |
| `SetVolume(uint8_t)` | 0-100 | int32_t | Volume သတ်မှတ် |
| `TtsMaker(string, int)` | text, lang | int32_t | Text-to-Speech |
| `PlayStream(string, string, vector)` | name, id, data | int32_t | Audio stream |
| `PlayStop(string)` | stream_id | int32_t | Playback stop |
| `LedControl(uint8_t, uint8_t, uint8_t)` | R, G, B | int32_t | LED control |

---

## 🎤 wav.hpp Helper Functions

```cpp
// WAV file ဖတ်ခြင်း
std::vector<uint8_t> ReadWave(
    const char* filename,
    int32_t* sampling_rate,
    int8_t* channelCount,
    bool* is_ok
);

// WAV file ရေးခြင်း
void WriteWave(
    const char* filename,
    int32_t sample_rate,
    const int16_t* samples,
    size_t num_samples,
    int num_channels
);
```

---

## 🔍 Troubleshooting

| ပြဿနာ | ဖြေရှင်းနည်း |
|--------|-------------|
| Network interface error | `ifconfig` ဖြင့် interface name စစ်ပါ |
| WAV file format error | 16kHz, Mono, 16-bit ဖြစ်ရမည် |
| No ASR message | Microphone enable ဖြစ်မဖြစ် စစ်ပါ |
| Multicast fail | Firewall settings စစ်ပါ |
| Volume not changing | Robot audio system စစ်ပါ |

---

## ⚠️ Requirements

| Requirement | Value |
|-------------|-------|
| Audio Format | WAV (PCM) |
| Sample Rate | 16000 Hz |
| Channels | 1 (Mono) |
| Bit Depth | 16-bit |
| Network | Same subnet as robot |

---

## 📄 License

Unitree Robotics © 2024
