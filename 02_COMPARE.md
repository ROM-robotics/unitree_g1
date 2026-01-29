# 🤖 Unitree Robots - AI Features နှိုင်းယှဥ်ချက်

## 📊 Robot အလိုက် AI Features Summary

| Robot | Audio/TTS/ASR | Video/Camera | LLM Integration | RL Locomotion |
|-------|---------------|--------------|-----------------|---------------|
| **G1** | ✅ Full (`AudioClient`) | ❌ | ✅ OpenAI, Google Cloud | ✅ |
| **A2** | ✅ Full (`AudioClient`) | ❌ | ❌ (Demo only) | ✅ |
| **Go2** | ⚠️ VUI only (`VuiClient`) | ✅ (`VideoClient`) | ❌ | ✅ |
| **H1** | ❌ | ❌ | ❌ | ✅ |
| **B2** | ❌ | ❌ | ❌ | ✅ |
| **B2W** | ❌ | ❌ | ❌ | ✅ |
| **Go2W** | ❌ | ❌ | ❌ | ✅ |

---

## 📁 SDK Include Structure

```
include/unitree/robot/
├── a2/
│   ├── audio/          ✅ AudioClient (TTS, ASR, LED)
│   └── sport/          ✅ SportClient
├── g1/
│   ├── audio/          ✅ AudioClient (TTS, ASR, LED)
│   ├── agv/            ✅ AGV control
│   ├── arm/            ✅ Arm control
│   ├── loco/           ✅ LocoClient
│   └── common/
├── go2/
│   ├── video/          ✅ VideoClient (Camera)
│   ├── vui/            ✅ VuiClient (Display/LED)
│   ├── sport/          ✅ SportClient
│   ├── robot_state/    ✅ RobotStateClient
│   ├── obstacles_avoid/
│   ├── utrack/
│   └── config/
├── h1/
│   └── loco/           ✅ LocoClient (locomotion only)
└── b2/
    └── (sport only)
```

---

## 🔊 1. A2 Robot - Audio Features

### Code Example (`a2_audio_client_example.cpp`)

```cpp
#include <unitree/robot/a2/audio/audio_client.hpp>

// G1 နှင့် တူညီသော API structure
unitree::robot::a2::AudioClient client;
client.Init();
client.SetTimeout(10.0f);

// TTS - Chinese
client.TtsMaker("你好。我是宇树科技的机器人。", 0);

// TTS - English  
client.TtsMaker("Hello. I'm a robot from Unitree Robotics.", 1);

// Volume Control
client.SetVolume(100);
client.GetVolume(volume);

// LED Control
client.LedControl(0, 255, 0);  // Green

// ASR Subscriber
unitree::robot::ChannelSubscriber<std_msgs::msg::dds_::String_> subscriber("rt/audio_msg");
subscriber.InitChannel(asr_handler);

// Microphone Recording (Multicast UDP)
// Group IP: 239.168.123.161, Port: 5555
```

### A2 vs G1 Audio API Comparison

| Feature | G1 | A2 | ကွာခြားချက် |
|---------|----|----|------------|
| TtsMaker | ✅ | ✅ | တူညီ |
| SetVolume/GetVolume | ✅ | ✅ | တူညီ |
| LedControl | ✅ | ✅ | တူညီ |
| PlayStream | ✅ | ✅ | တူညီ |
| ASR Topic | `rt/audio_msg` | `rt/audio_msg` | တူညီ |
| Multicast IP | `239.168.123.161` | `239.168.123.161` | တူညီ |
| OpenAI Example | ✅ | ❌ | G1 တွင်သာ ရှိ |
| Burmese Example | ✅ | ❌ | G1 တွင်သာ ရှိ |

**နိဂုံး:** A2 နှင့် G1 ၏ Audio API သည် **100% တူညီ**သည်။ G1 အတွက် ရေးထားသော OpenAI integration code ကို A2 တွင် တိုက်ရိုက် အသုံးပြုနိုင်သည်။

---

## 📹 2. Go2 Robot - Video & VUI Features

### Video Client (`go2_video_client.cpp`)

```cpp
#include <unitree/robot/go2/video/video_client.hpp>

unitree::robot::go2::VideoClient video_client;
video_client.SetTimeout(1.0f);
video_client.Init();

// Get camera image
std::vector<uint8_t> image_sample;
int ret = video_client.GetImageSample(image_sample);

if (ret == 0) {
    // Save as JPEG
    std::ofstream image_file("captured.jpg", std::ios::binary);
    image_file.write(reinterpret_cast<const char*>(image_sample.data()), 
                     image_sample.size());
}
```

### VUI Client (`go2_vui_client.cpp`)

```cpp
#include <unitree/robot/go2/vui/vui_client.hpp>

unitree::robot::go2::VuiClient vc;
vc.SetTimeout(1.0f);
vc.Init();

// Brightness Control (0-10 levels)
vc.SetBrightness(level);  // level = 0 to 10
vc.GetBrightness(value);
```

### Go2 Unique Features

| Feature | API | Description |
|---------|-----|-------------|
| Camera Capture | `VideoClient::GetImageSample()` | JPEG image capture |
| Display Brightness | `VuiClient::SetBrightness()` | 0-10 levels |
| Robot State | `RobotStateClient` | Battery, temperature, etc. |
| Obstacle Avoidance | `obstacles_avoid/` | Built-in obstacle detection |
| UTrack | `utrack/` | Tracking features |

---

## 🦿 3. H1 Robot - Locomotion Only

H1 သည် Audio/Video features မပါဘဲ **Locomotion control** ကိုသာ focus လုပ်ထားသည်။

### LocoClient API (`h1_loco_client_example.cpp`)

```cpp
#include <unitree/robot/h1/loco/h1_loco_client.hpp>

unitree::robot::h1::LocoClient client;
client.Init();
client.SetTimeout(10.f);

// FSM Control
client.GetFsmId(fsm_id);
client.SetFsmId(fsm_id);

// Balance Mode
client.GetBalanceMode(balance_mode);
client.SetBalanceMode(balance_mode);

// Walking Parameters
client.GetSwingHeight(swing_height);
client.SetSwingHeight(swing_height);

client.GetStandHeight(stand_height);
client.SetStandHeight(stand_height);

// Velocity Control
client.SetVelocity(vx, vy, omega, duration);

// Odometry
client.EnableOdom();
client.GetOdom(x, y, yaw);
client.DisableOdom();

// Phase (gait)
client.GetPhase(phase);
```

### H1 Parallel Mechanism (Ankle Control)

H1_2 model တွင် ankle joints များအတွက် **PR Mode** (Pitch-Roll mode) ရှိသည်:

```cpp
// PR Mode for ankle control
mode_ = PR;

// Sinusoidal tracking
double L_P_des = max_P * std::cos(2.0 * M_PI * t);  // Pitch
double L_R_des = max_R * std::sin(2.0 * M_PI * t);  // Roll

// Joint commands
dds_low_command.motor_cmd().at(4).q() = L_P_des;   // LeftAnklePitch
dds_low_command.motor_cmd().at(5).q() = L_R_des;   // LeftAnkleRoll
dds_low_command.motor_cmd().at(10).q() = R_P_des;  // RightAnklePitch
dds_low_command.motor_cmd().at(11).q() = R_R_des;  // RightAnkleRoll
```

---

## 🐕 4. B2/B2W Robot - Basic Sport Control

B2 သည် quadruped robot ဖြစ်ပြီး **sport client** သာပါဝင်သည်။

### Available Examples

| File | Description |
|------|-------------|
| `b2_sport_client.cpp` | Sport mode control |
| `b2_stand_example.cpp` | Stand up/down |

---

## 🎯 5. AI Integration Potential

### ✅ Can Add LLM Integration

| Robot | Effort | Notes |
|-------|--------|-------|
| **A2** | 🟢 Easy | Same Audio API as G1, copy code directly |
| **Go2** | 🟡 Medium | Has camera, can add vision AI |
| **H1** | 🔴 Hard | No audio hardware, needs external mic/speaker |
| **B2** | 🔴 Hard | No audio hardware |

### A2 OpenAI Integration (Recommended Approach)

G1 ၏ `g1_audio_openai_example.cpp` ကို A2 အတွက် ပြောင်းလဲနည်း:

```cpp
// Change this:
#include <unitree/robot/g1/audio/g1_audio_client.hpp>
unitree::robot::g1::AudioClient client;

// To this:
#include <unitree/robot/a2/audio/audio_client.hpp>
unitree::robot::a2::AudioClient client;

// Everything else remains the same!
```

### Go2 Vision AI Potential

Go2 တွင် Camera ရှိသောကြောင့် Vision AI ထည့်နိုင်သည်:

```cpp
// Capture image
video_client.GetImageSample(image_sample);

// Send to Vision API (OpenAI GPT-4 Vision, Google Cloud Vision, etc.)
std::string description = callVisionAPI(image_sample);

// Optional: Text-to-Speech (needs external TTS service)
```

---

## 📌 6. Summary Table

| Robot | Type | DOF | Audio | Video | Best AI Use Case |
|-------|------|-----|-------|-------|------------------|
| **G1** | Humanoid | 23/29 | ✅ | ❌ | Voice Assistant, Conversation |
| **A2** | Quadruped | 12 | ✅ | ❌ | Voice Commands, Announcements |
| **Go2** | Quadruped | 12 | ⚠️ | ✅ | Vision AI, Object Detection |
| **H1** | Humanoid | 19/27 | ❌ | ❌ | Custom RL Locomotion |
| **B2** | Quadruped | 12 | ❌ | ❌ | Industrial Automation |

---

## 🔗 7. Related Files

| Robot | Example Directory |
|-------|-------------------|
| G1 | `unitree_sdk2/example/g1/` |
| A2 | `unitree_sdk2/example/a2/` |
| Go2 | `unitree_sdk2/example/go2/` |
| H1 | `unitree_sdk2/example/h1/` |
| B2 | `unitree_sdk2/example/b2/` |

---

*Last Updated: January 29, 2026*
