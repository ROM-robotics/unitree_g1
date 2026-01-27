# 🖐️ Unitree G1 Dex3 Hand Control Example

## 📖 Overview / အကျဉ်းချုပ်

ဤ example သည် Unitree G1 Robot ၏ **Dex3 Dexterous Hand** (လက်ချောင်းများပါသော လက်) ကို control လုပ်နည်းကို ပြသထားသည်။

---

## 🤖 Dex3 Hand ဆိုတာ ဘာလဲ?

```
┌─────────────────────────────────────────┐
│          Dex3 Dexterous Hand            │
├─────────────────────────────────────────┤
│                                         │
│    🖐️ 7 Motors (လက်ချောင်း motors)      │
│    📊 9 Pressure Sensors                │
│    ↔️ Left / Right Hand ရွေးနိုင်        │
│                                         │
└─────────────────────────────────────────┘
```

| Specification | Value |
|---------------|-------|
| Motors | 7 (per hand) |
| Pressure Sensors | 9 (per hand) |
| Control Mode | Position control with PD gains |
| Hands | Left / Right selectable |

---

## 🔧 Dependencies / လိုအပ်သော Libraries

```cpp
#include <unitree/idl/hg/HandState_.hpp>    // Hand state message
#include <unitree/idl/hg/HandCmd_.hpp>      // Hand command message
#include <unitree/robot/channel/channel_publisher.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>
#include <eigen3/Eigen/Dense>               // Matrix operations
```

---

## 🎮 Keyboard Controls

| Key | Action | Description |
|-----|--------|-------------|
| `r` | **ROTATE** | လက်ချောင်းများ Sine wave pattern ဖြင့် လှုပ်ရှား |
| `g` | **GRIP** | လက်ချောင်းများ ဆုပ်ကိုင် (middle position) |
| `p` | **PRINT** | လက်ချောင်း positions ပြသ |
| `s` | **STOP** | Motors ရပ် |
| `q` | **QUIT** | Program ထွက် |

---

## 🔄 State Machine

```
┌────────┐
│  INIT  │ ──────→ ┌────────┐
└────────┘         │ ROTATE │ ←── 'r' key
                   └────────┘
                       ↕
    's' key ──→ ┌────────┐
                │  STOP  │
    'q' key ──→ └────────┘
                       ↕
    'g' key ──→ ┌────────┐
                │  GRIP  │
                └────────┘
                       ↕
    'p' key ──→ ┌────────┐
                │ PRINT  │
                └────────┘
```

---

## 📊 Hand Motor Configuration

### Left Hand Limits (Radians)

| Motor | Min | Max |
|-------|-----|-----|
| 0 | -1.05 | 1.05 |
| 1 | -0.724 | 1.05 |
| 2 | 0 | 1.75 |
| 3 | -1.57 | 0 |
| 4 | -1.75 | 0 |
| 5 | -1.57 | 0 |
| 6 | -1.75 | 0 |

### Right Hand Limits (Radians)

| Motor | Min | Max |
|-------|-----|-----|
| 0 | -1.05 | 1.05 |
| 1 | -1.05 | 0.742 |
| 2 | -1.75 | 0 |
| 3 | 0 | 1.57 |
| 4 | 0 | 1.75 |
| 5 | 0 | 1.57 |
| 6 | 0 | 1.75 |

---

## 📝 Code ရှင်းလင်းချက်

### 1. Motor Command Structure

```cpp
typedef struct {
    uint8_t id     : 4;    // Motor ID (0-6)
    uint8_t status : 3;    // Motor status
    uint8_t timeout: 1;    // Timeout flag
} RIS_Mode_t;
```

---

### 2. `rotateMotors()` - လက်ချောင်း လှုပ်ရှားခြင်း

```cpp
// Sine wave ဖြင့် ချောမွေ့စွာ ရွေ့လျား
float range = maxLimits[i] - minLimits[i];
float mid = (maxLimits[i] + minLimits[i]) / 2.0;
float amplitude = range / 2.0;
float q = mid + amplitude * sin(_count / 20000.0 * M_PI);

msg.motor_cmd()[i].q(q);      // Target position
msg.motor_cmd()[i].kp(0.5);   // Stiffness (position gain)
msg.motor_cmd()[i].kd(0.1);   // Damping (velocity gain)
```

**Motion Pattern:**
- Sine wave ဖြင့် min/max limits အတွင်း ရွေ့လျား
- ချောမွေ့သော periodic motion

---

### 3. `gripHand()` - ဆုပ်ကိုင်ခြင်း

```cpp
float mid = (maxLimits[i] + minLimits[i]) / 2.0;
msg.motor_cmd()[i].q(mid);    // Middle position
msg.motor_cmd()[i].kp(1.5);   // Higher stiffness for firm grip
msg.motor_cmd()[i].kd(0.1);   // Damping
```

**Grip Action:**
- လက်ချောင်းအားလုံး middle position သို့
- Higher kp = ပိုမိုခိုင်မာသော ဆုပ်ကိုင်မှု

---

### 4. `stopMotors()` - ရပ်တန့်ခြင်း

```cpp
msg.motor_cmd()[i].kp(0);     // No stiffness
msg.motor_cmd()[i].kd(0);     // No damping
msg.motor_cmd()[i].q(0);      // Zero position
msg.motor_cmd()[i].timeout = 0x01;  // Enable timeout
```

**Stop Action:**
- Motors အားလုံး control disable
- Passive mode (လွတ်လပ်စွာ ရွေ့နိုင်)

---

### 5. `printState()` - အခြေအနေပြသခြင်း

```cpp
for(int i = 0; i < 7; i++) {
    q(i) = state.motor_state()[i].q();  // Current position
    // Normalize to 0-1 range
    q(i) = (q(i) - minLimits[i]) / (maxLimits[i] - minLimits[i]);
    q(i) = std::clamp(q(i), 0.0f, 1.0f);
}
std::cout << "Position: " << q.transpose() << std::endl;
```

**Output:** 0.0 (fully open) to 1.0 (fully closed)

---

## 📡 DDS Topics

| Hand | Command Topic | State Topic |
|------|---------------|-------------|
| **Left** | `rt/dex3/left/cmd` | `rt/lf/dex3/left/state` |
| **Right** | `rt/dex3/right/cmd` | `rt/lf/dex3/right/state` |

---

## 🎛️ Motor Command Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `mode` | uint8_t | Motor mode (ID + status + timeout) |
| `q` | float | Target position (radians) |
| `dq` | float | Target velocity (rad/s) |
| `tau` | float | Target torque (Nm) |
| `kp` | float | Position gain (stiffness) |
| `kd` | float | Velocity gain (damping) |

### PD Control Formula

$$
\tau = k_p \cdot (q_{target} - q_{current}) + k_d \cdot (dq_{target} - dq_{current})
$$

---

## 🔨 Build Instructions

```bash
# Build directory သို့ သွား
cd /path/to/unitree_sdk2/build

# Compile
make g1_dex3_example
```

---

## 🚀 Run Instructions

```bash
# Run program
./g1_dex3_example eth0

# Then select hand:
# L - Left hand
# R - Right hand
```

### Example Session

```
 --- Unitree Robotics ---
     Dex3 Hand Example

Please input the hand id (L for left hand, R for right hand): L

--- Current State: INIT ---
Commands:
  r - Rotate
  g - Grip
  p - Print_state
  q - Quit
  s - Stop

Initializing...

--- Current State: ROTATE ---
```

---

## 📊 Visual Representation

### Hand Motor Layout (Approximate)

```
         [Thumb]
            │
    ┌───────┴───────┐
    │   Motor 0-1   │
    │               │
 [Index]  [Middle]  [Ring]
    │        │        │
  M2-3     M4-5     M6
```

---

## ⚠️ Safety Precautions

| ⚠️ Warning | Description |
|-----------|-------------|
| 🔌 Power | Hand power supply စစ်ဆေးပါ |
| 👆 Pinch | လက်ချောင်းကြား ညှပ်နိုင်သည် |
| 🔧 Limits | Joint limits ကျော်လွန်ခြင်း သတိပြုပါ |
| 🔄 Kp/Kd | Gains များ ရုတ်တရက် မပြောင်းပါနှင့် |

---

## 🔍 Troubleshooting

| ပြဿနာ | ဖြေရှင်းနည်း |
|--------|-------------|
| Hand not responding | DDS topic name စစ်ပါ၊ network interface စစ်ပါ |
| Motors not moving | kp, kd values စစ်ပါ (0 မဖြစ်ရ) |
| Jerky motion | kd value တိုးပါ (damping) |
| Position error | Joint limits ထဲတွင် ရှိမရှိ စစ်ပါ |

---

## 📚 API Reference

### HandCmd Message

```cpp
msg.motor_cmd()[i].mode(mode);   // Set motor mode
msg.motor_cmd()[i].q(position);  // Set target position
msg.motor_cmd()[i].dq(velocity); // Set target velocity
msg.motor_cmd()[i].tau(torque);  // Set target torque
msg.motor_cmd()[i].kp(gain);     // Set position gain
msg.motor_cmd()[i].kd(gain);     // Set velocity gain
```

### HandState Message

```cpp
state.motor_state()[i].q();      // Get current position
state.motor_state()[i].dq();     // Get current velocity
state.motor_state()[i].tau();    // Get current torque
state.press_sensor_state()[i];   // Get pressure sensor value
```

---

## 🎯 Summary

| Feature | Description |
|---------|-------------|
| **ဘာကို control လုပ်လဲ** | Dex3 Dexterous Hand (7 DOF) |
| **ဘယ်လက်/ညာလက်** | နှစ်ခုလုံး support |
| **Control Mode** | Position control with PD gains |
| **Sensors** | 9 pressure sensors per hand |
| **Use Case** | Object grasping, manipulation |

---

## � DDS vs ROS2 - Communication Protocol

### ဒီ Code သည် **DDS** ကို တိုက်ရိုက်သုံးထားသည်

```cpp
#include <unitree/idl/hg/HandState_.hpp>
#include <unitree/idl/hg/HandCmd_.hpp>
#include <unitree/robot/channel/channel_publisher.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>
```

### DDS vs ROS2 ကွာခြားချက်

| Feature | ဒီ Code (DDS Direct) | ROS2 |
|---------|---------------------|------|
| **Protocol** | ✅ DDS (Direct) | DDS (via ROS2 layer) |
| **Middleware** | Unitree SDK DDS | ROS2 RMW |
| **Topic Format** | `rt/dex3/left/cmd` | `/dex3/left/cmd` |
| **Message Type** | `unitree_hg::msg::dds_::HandCmd_` | `unitree_hg/msg/HandCmd` |
| **Extra Layer** | ❌ မလို | ✅ ROS2 runtime လို |

**Unitree SDK က DDS ကို တိုက်ရိုက်သုံးထားပြီး ROS2 layer မလိုပါ။**

### Architecture Comparison

```
┌─────────────────────────────────────────────────────────┐
│  Unitree SDK (DDS Direct) - ဒီ code သုံးတာ              │
├─────────────────────────────────────────────────────────┤
│   Your Code  →  Unitree SDK  →  DDS  →  Robot           │
│      ↓              ↓            ↓        ↓              │
│   Simple       Fast/Direct   UDP/TCP   Direct           │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│  ROS2 (Alternative - မသုံးထား)                          │
├─────────────────────────────────────────────────────────┤
│   Your Code → ROS2 Node → RMW → DDS → Robot             │
│      ↓           ↓         ↓     ↓      ↓               │
│   Complex    Extra Layer  Bridge      Same              │
└─────────────────────────────────────────────────────────┘
```

---

## 🖥️ Remote Control - PC ကနေ လှမ်းခိုင်းခြင်း

### ✅ သင့် PC ကနေ Control လုပ်လို့ ရပါတယ်!

```
┌─────────────────────────────────────────────────────────┐
│                    Network Setup                         │
├─────────────────────────────────────────────────────────┤
│                                                          │
│   ┌──────────┐         Ethernet        ┌──────────┐     │
│   │ Your PC  │ ←───────────────────→   │ G1 Robot │     │
│   │192.168.  │       Same Subnet       │192.168.  │     │
│   │123.100   │                         │123.161   │     │
│   └──────────┘                         └──────────┘     │
│        │                                     │          │
│        │              DDS                    │          │
│        └─────────── Domain 0 ───────────────┘          │
│                                                          │
└─────────────────────────────────────────────────────────┘
```

### Setup Steps

#### 1. Network Configuration

```bash
# PC IP ကို robot subnet တွင် သတ်မှတ်
sudo ifconfig eth0 192.168.123.100 netmask 255.255.255.0

# သို့မဟုတ် netplan/nmcli သုံးပါ
```

#### 2. Verify Connection

```bash
# Robot IP ကို ping ကြည့်ပါ
ping 192.168.123.161
```

#### 3. Run Program from PC

```bash
# eth0 = robot နှင့် ချိတ်ထားသော interface
./g1_dex3_example eth0
```

### Code ထဲတွင်

```cpp
// Domain ID 0 = Robot နှင့် တူညီရမည်
unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);
//                                           ↑      ↑
//                                      Domain   Network
//                                        ID    Interface
```

### Remote Control Requirements

| Requirement | Description |
|-------------|-------------|
| 🌐 **Network** | Robot နှင့် PC same subnet `192.168.123.x` |
| 🔢 **Domain ID** | `Init(0, ...)` - Robot နှင့် တူညီရမည် |
| 📦 **SDK** | PC တွင် Unitree SDK installed ရှိရမည် |
| 🔌 **Interface** | `eth0`, `enp0s3` စသည် - `ifconfig` ဖြင့် စစ်ပါ |

### Remote Control Q&A

| Question | Answer |
|----------|--------|
| DDS လား ROS2 လား? | **DDS** (တိုက်ရိုက်) |
| PC ကနေ control ရလား? | ✅ **ရပါတယ်** |
| ဘာလိုအပ်လဲ? | Same subnet + Unitree SDK installed |
| ROS2 လိုလား? | ❌ **မလိုပါ** (optional) |
| Latency ကောင်းလား? | ✅ DDS direct = Low latency |

---

## 📄 License

Unitree Robotics © 2024

