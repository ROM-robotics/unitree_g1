# G1 Namaste Pose Example (လက်အုပ်ချီ ရှိခိုးပုံစံ) 🙏

ဤ example သည် G1 စက်ရုပ်၏ လက်နှစ်ဖက်ကို Namaste (Prayer) pose လုပ်စေသည်။

---

## 📖 ရှင်းလင်းချက်

Namaste pose သည် built-in action မဟုတ်သောကြောင့် **low-level arm SDK control** ဖြင့် joint angles များကို တိုက်ရိုက် ထိန်းချုပ်ရပါသည်။

```
        ┌─────────────┐
        │    Head     │
        └──────┬──────┘
               │
    ┌──────────┴──────────┐
    │       Torso         │
    └──────────┬──────────┘
         ┌─────┴─────┐
         │    🙏     │  ← လက်နှစ်ဖက် ရင်ဘတ်ရှေ့မှာ ဆုံ
         │  Hands    │
         │  Meet     │
         └───────────┘
```

---

## 🔧 Compile လုပ်နည်း

```bash
cd /path/to/unitree_sdk2
mkdir -p build && cd build

# Manual compile (CMakeLists.txt မထည့်ထားလျှင်)
g++ -std=c++17 \
    -I../include \
    -I../thirdparty/include \
    ../example/g1/low_level/namaste.cpp \
    -L../lib \
    -lunitree_sdk2 \
    -lpthread \
    -o namaste
```

---

## 🚀 Run လုပ်နည်း

```bash
./namaste <network_interface>

# Example
./namaste eth0
```

---

## 📊 Joint Angles (Radians)

### Namaste Pose Target Positions

| Joint | Left Arm | Right Arm | ရှင်းလင်းချက် |
|-------|----------|-----------|---------------|
| Shoulder Pitch | +0.8 | +0.8 | ရှေ့သို့ ကြွ (~45°) |
| Shoulder Roll | -0.3 | +0.3 | အတွင်းသို့ ပိတ် |
| Shoulder Yaw | +0.4 | -0.4 | လှည့် |
| Elbow | +1.2 | +1.2 | ကွေး (~70°) |
| Wrist Roll | 0.0 | 0.0 | - |
| Wrist Pitch | +0.3 | +0.3 | လက်ဖဝါး အတွင်းသို့ |
| Wrist Yaw | 0.0 | 0.0 | - |

> **Note**: Right arm သည် Left arm ၏ mirror ဖြစ်သောကြောင့် Roll နှင့် Yaw joints များ၏ signs ပြောင်းပြန်ဖြစ်သည်။

---

## 🎯 Control Parameters

| Parameter | Value | ရှင်းလင်းချက် |
|-----------|-------|---------------|
| Kp (Stiffness) | 60.0 | Joint stiffness |
| Kd (Damping) | 1.5 | Joint damping |
| Control dt | 20ms | Control loop frequency |
| Transition time | 3.0s | Pose သို့ ရောက်ရန် ကြာချိန် |

---

## 📝 Code အလုပ်လုပ်ပုံ

### 1. Initialization
```cpp
// DDS channel များ ဖန်တီး
publisher.reset(new ChannelPublisher<LowCmd_>(kTopicArmSDK));
subscriber.reset(new ChannelSubscriber<LowState_>(kTopicState));
```

### 2. Enable Arm SDK Control
```cpp
// Weight = 1.0 ဖြင့် arm SDK control ဖွင့်
msg.motor_cmd().at(kNotUsedJoint).q(1.0f);
```

### 3. Smooth Interpolation
```cpp
// Cubic easing for smooth motion
float smooth_ratio = ratio * ratio * (3.0f - 2.0f * ratio);
float target = init_pos + (namaste_pos - init_pos) * smooth_ratio;
```

### 4. Send Command
```cpp
msg.motor_cmd().at(joint).q(target);    // Position
msg.motor_cmd().at(joint).kp(kp);       // Stiffness
msg.motor_cmd().at(joint).kd(kd);       // Damping
publisher->Write(msg);
```

---

## ⚠️ သတိပြုရန်

1. **FSM State**: စက်ရုပ်သည် **Stand state (FSM ID 500)** တွင် ရှိနေရပါမည်
2. **Joint Limits**: Joint angles များသည် စက်ရုပ်၏ physical limits အတွင်း ရှိရပါမည်
3. **Collision**: လက်နှစ်ဖက် တိုက်မိခြင်း မရှိစေရန် angles များကို သေချာစွာ ချိန်ညှိပါ
4. **Smooth Motion**: ရုတ်တရက် ရွေ့ခြင်း မရှိစေရန် interpolation သုံးထားပါသည်
5. **Tuning**: Joint angles များသည် ခန့်မှန်းချက်ဖြစ်ပြီး တကယ့်စက်ရုပ်ပေါ်တွင် adjust လုပ်ရန် လိုအပ်နိုင်ပါသည်

---

## 🔄 Pose Variations

### Higher Namaste (ဦးခေါင်းအထက်)
```cpp
float left_shoulder_pitch  = 2.5f;   // ပိုမြင့်မြင့် ကြွ
float right_shoulder_pitch = 2.5f;
```

### Lower Namaste (ရင်ဘတ်အောက်)
```cpp
float left_shoulder_pitch  = 0.3f;   // နိမ့်နိမ့် ကြွ
float right_shoulder_pitch = 0.3f;
```

### Bow with Namaste (ကိုင်းပြီး ရှိခိုး)
```cpp
// Waist pitch ကိုလည်း ထည့်သုံး
msg.motor_cmd().at(kWaistPitch).q(0.3f);  // ရှေ့သို့ ကိုင်း
```

---

## 📁 Related Files

| File | Description |
|------|-------------|
| [g1_dual_arm_example.cpp](g1_dual_arm_example.cpp) | Dual arm trajectory control |
| [g1_arm7_sdk_dds_example.cpp](../high_level/g1_arm7_sdk_dds_example.cpp) | 7-DOF arm control |
| [greeting.md](../greeting.md) | Built-in greeting actions |

---

## 🙏 Usage Example

```bash
# 1. Stand state သို့ ပြောင်း (high-level)
./g1_loco_client_example --network_interface=eth0 --start

# 2. Namaste pose လုပ်
./namaste eth0
```

