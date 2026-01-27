# Unitree G1 AGV Client Example

## 📖 Overview / အကျဉ်းချုပ်

ဤ example သည် Unitree G1 humanoid robot ၏ AGV (Automated Guided Vehicle) mode ကို control လုပ်နည်းကို ပြသထားသည်။ Robot ကို ရှေ့/နောက် ရွေ့လျားခြင်း၊ လှည့်ခြင်းနှင့် အမြင့်ချိန်ညှိခြင်းများ ပြုလုပ်နိုင်သည်။

---

## 🔧 Dependencies / လိုအပ်သော Libraries

```cpp
#include <unitree/robot/g1/agv/g1_agv_client.hpp>  // AGV Client API
#include <iostream>                                  // Console output
#include <math.h>                                    // Math functions (M_PI, sinf)
#include <unistd.h>                                  // usleep function
```

| Library | ရည်ရွယ်ချက် |
|---------|------------|
| `g1_agv_client.hpp` | Unitree G1 AGV control API |
| `iostream` | Console output အတွက် |
| `math.h` | Sine function နှင့် PI constant အတွက် |
| `unistd.h` | `usleep()` - microsecond delay အတွက် |

---

## 📝 Code ရှင်းလင်းချက်

### 1. External C Function Declaration

```cpp
extern "C" {
    float sinf(float x);
}
```

C library မှ `sinf()` function ကို C++ code တွင် အသုံးပြုရန် declare လုပ်ထားသည်။

---

### 2. Channel Factory Initialization

```cpp
unitree::robot::ChannelFactory::Instance()->Init(0);
```

- **ChannelFactory** - Robot နှင့် communication channel တည်ဆောက်ပေးသည်
- **Init(0)** - Domain ID `0` ဖြင့် DDS (Data Distribution Service) ကို initialize လုပ်သည်
- Singleton pattern ဖြင့် တစ်ခုတည်းသော instance ကို အသုံးပြုသည်

---

### 3. AGV Client Setup

```cpp
unitree::robot::g1::AgvClient ac;
ac.SetTimeout(3.0f);  // 3 စက္ကန့် timeout
ac.Init();
```

| Method | ရှင်းလင်းချက် |
|--------|-------------|
| `AgvClient ac` | AGV client object ဖန်တီးခြင်း |
| `SetTimeout(3.0f)` | Command timeout ကို 3 စက္ကန့် သတ်မှတ်ခြင်း |
| `Init()` | Client ကို initialize လုပ်ပြီး robot နှင့် ချိတ်ဆက်ခြင်း |

---

### 4. Motion Parameters

```cpp
int cycle_count = 0;
const float cycle_period = 40.0f;      // 40 steps = 1 full cycle
const float vx_amplitude = 0.3f;       // Forward velocity amplitude (m/s)
const float vyaw_amplitude = 0.3f;     // Rotation velocity amplitude (rad/s)
const float height_amplitude = 1.0f;   // Height adjustment amplitude
```

| Parameter | တန်ဖိုး | ရှင်းလင်းချက် |
|-----------|--------|--------------|
| `cycle_period` | 40.0 | Sine wave တစ်ပတ်လည်ရန် steps အရေအတွက် |
| `vx_amplitude` | 0.3 | ရှေ့/နောက် အရှိန် အမြင့်ဆုံးတန်ဖိုး (m/s) |
| `vyaw_amplitude` | 0.3 | လှည့်ခြင်း အရှိန် အမြင့်ဆုံးတန်ဖိုး (rad/s) |
| `height_amplitude` | 1.0 | အမြင့်ချိန်ညှိမှု အများဆုံးတန်ဖိုး |

---

### 5. Main Control Loop

#### 5.1 Phase Calculation (Sine Wave)

```cpp
float time_phase = (cycle_count % (int)cycle_period) / cycle_period * 2.0f * M_PI;
```

**ဖော်မြူလာ ရှင်းလင်းချက်:**

$$
\text{time\_phase} = \frac{(\text{cycle\_count} \mod \text{cycle\_period})}{\text{cycle\_period}} \times 2\pi
$$

- `cycle_count % cycle_period` → 0 မှ 39 အထိ ပြန်လည်စက်ဝိုင်း
- `/cycle_period` → 0.0 မှ 1.0 အတွင်း normalize
- `* 2π` → 0 မှ 2π radian အဖြစ် ပြောင်းလဲ

---

#### 5.2 Velocity Control

```cpp
float vx = vx_amplitude * sinf(time_phase);
float vyaw = vyaw_amplitude * sinf(time_phase);
int32_t ret = ac.Move(vx, 0.0f, vyaw);
```

**Move Function Parameters:**

```cpp
ac.Move(vx, vy, vyaw);
```

| Parameter | အမျိုးအစား | ရှင်းလင်းချက် |
|-----------|-----------|--------------|
| `vx` | float | X-axis velocity (ရှေ့/နောက်) - positive = ရှေ့ |
| `vy` | float | Y-axis velocity (ဘယ်/ညာ) - ဤ example တွင် 0 |
| `vyaw` | float | Angular velocity (လှည့်ခြင်း) - positive = ဘယ်လှည့် |

**Return Value:** `0` = အောင်မြင်, အခြား = Error code

---

#### 5.3 Height Adjustment

```cpp
float height = height_amplitude * sinf(time_phase);
int32_t ret2 = ac.HeightAdjust(height);
```

Robot ၏ body height ကို ချိန်ညှိသည်။ Sine wave ပုံစံဖြင့် အပေါ်/အောက် ပြောင်းလဲသည်။

---

#### 5.4 Timing Control

```cpp
usleep(50000);  // 50ms = 50,000 microseconds
```

- Control loop frequency: **20 Hz** (1000ms / 50ms = 20)
- Move command နှင့် HeightAdjust command ကြားတွင် 50ms စီ delay

---

## 📊 Motion Pattern Visualization

```
Velocity/Height
    ^
    |     *       *
    |   *   *   *   *
    | *       *       *
----+-------------------> Time
    | *       *       *
    |   *   *   *   *
    |     *       *
```

Sine wave pattern ဖြင့် robot သည်:
1. ရှေ့သို့ တဖြည်းဖြည်း အရှိန်မြင့်
2. အရှိန်အမြင့်ဆုံးရောက်ပြီး ပြန်လျော့
3. နောက်သို့ ပြန်ရွေ့ (negative velocity)
4. ပြန်လည် စတင်

---

## 🔨 Build Instructions

### CMakeLists.txt Configuration

```cmake
add_executable(g1_agv_client_example agv/g1_agv_client_example.cpp)
target_link_libraries(g1_agv_client_example unitree_sdk2)
```

### Compile Command

```bash
# Project root directory သို့ သွားပါ
cd /path/to/unitree_sdk2

# Build directory ဖန်တီးပါ
mkdir -p build && cd build

# CMake configure
cmake ..

# Compile
make g1_agv_client_example
```

---

## 🚀 Run Instructions

```bash
# Build directory မှ run ပါ
./g1_agv_client_example
```

**⚠️ သတိပြုရန်:**
- Robot ကို safe area တွင် ထားပါ
- Emergency stop button ကို လက်လှမ်းမီသည့်နေရာတွင် ထားပါ
- Robot ၏ battery level ကို စစ်ဆေးပါ

---

## 📚 API Reference

### AgvClient Class Methods

| Method | Parameters | Return | ရှင်းလင်းချက် |
|--------|------------|--------|--------------|
| `Init()` | - | void | Client initialize |
| `SetTimeout(float)` | timeout (seconds) | void | Command timeout သတ်မှတ် |
| `Move(vx, vy, vyaw)` | float, float, float | int32_t | Velocity command ပို့ |
| `HeightAdjust(height)` | float | int32_t | Body height ချိန်ညှိ |

---

## 🔍 Troubleshooting

| ပြဿနာ | ဖြေရှင်းနည်း |
|--------|-------------|
| Connection failed | Network connection စစ်ဆေးပါ၊ Domain ID မှန်မမှန် စစ်ပါ |
| Timeout error | Robot ကို ဖွင့်ထားမှု စစ်ဆေးပါ၊ timeout တန်ဖိုး တိုးပါ |
| No movement | AGV mode enable ဖြစ်မဖြစ် စစ်ဆေးပါ |

---

## 📄 License

Unitree Robotics © 2024
