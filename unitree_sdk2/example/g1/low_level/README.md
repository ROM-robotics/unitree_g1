# G1 Low Level Control Examples

ဤ folder တွင် Unitree G1 စက်ရုပ်အတွက် low-level (အနိမ့်ဆုံးအဆင့်) motor control examples များပါဝင်ပါသည်။

---

## 📁 ဖိုင်များ အကျဥ်းချုပ်

### 1. `g1_ankle_swing_example.cpp`

**ရည်ရွယ်ချက်:** G1 ခြေကျင်း (ankle) joints များကို swing လုပ်ခြင်း

**အလုပ်လုပ်ပုံ:**
- **Stage 1 (0-3 စက္ကန့်):** စက်ရုပ်ကို zero posture (မူလအနေအထား) သို့ ပြန်သွားစေသည်
- **Stage 2 (3-6 စက္ကန့်):** PR Mode (Pitch/Roll) ဖြင့် ခြေကျင်းများကို swing လုပ်သည်
  - Pitch: ±30°
  - Roll: ±10°
- **Stage 3 (6 စက္ကန့်+):** AB Mode (Parallel Control) ဖြင့် ခြေကျင်းများကို swing လုပ်သည်

**အဓိက Features:**
- DDS (Data Distribution Service) ဖြင့် real-time communication
- Gamepad/Remote control support
- IMU (Inertial Measurement Unit) data ဖတ်ခြင်း
- Motor state monitoring (position, velocity, temperature, voltage)
- 2ms control loop frequency

**သုံးနည်း:**
```bash
./g1_ankle_swing_example <network_interface>
```

---

### 2. `g1_dual_arm_example.cpp`

**ရည်ရွယ်ချက်:** G1 စက်ရုပ်၏ လက်နှစ်ဖက် (dual arm) ကို offline trajectory အတိုင်း ထိန်းချုပ်ခြင်း

**အလုပ်လုပ်ပုံ:**
- **Stage 1 (0-3 စက္ကန့်):** စက်ရုပ်ကို zero posture သို့ ပြန်သွားစေသည်
- **Stage 2 (3 စက္ကန့်+):** `behavior_lib/motion.seq` ဖိုင်မှ ကြိုတင်သတ်မှတ်ထားသော trajectory ကို follow လုပ်သည်

**အဓိက Features:**
- YAML-based behavior library loading
- Left/Right arm joints (14 DOF) ထိန်းချုပ်ခြင်း:
  - Shoulder (Pitch, Roll, Yaw)
  - Elbow
  - Wrist (Roll, Pitch, Yaw)
- Motor type အလိုက် Kp/Kd gain သတ်မှတ်ခြင်း (GearboxS, GearboxM, GearboxL)
- Motion Switcher Client ဖြင့် service management

**သုံးနည်း:**
```bash
./g1_dual_arm_example <network_interface>
```

---

### 3. `terminations.cpp`

**ရည်ရွယ်ချက်:** Safety termination conditions များကို စစ်ဆေးခြင်း

**အလုပ်လုပ်ပုံ:**
- အဆက်မပြတ် loop ထဲတွင် အောက်ပါ conditions များကို စစ်ဆေးနေသည်:
  1. **Bad Orientation:** စက်ရုပ် လဲကျသွားခြင်း (threshold: 1.0 radian)
  2. **Lost Connection:** Network connection ပြတ်တောက်ခြင်း (timeout: 1000ms)

**အဓိက Features:**
- Real-time safety monitoring
- DDS lowstate subscription
- Command line arguments support (Boost program_options)

**သုံးနည်း:**
```bash
./terminations --network <network_interface>
# သို့မဟုတ်
./terminations -n <network_interface>
```

---

## 📂 အခြားဖိုင်များ

| ဖိုင် | ရှင်းလင်းချက် |
|------|---------------|
| `gamepad.hpp` | Wireless remote/gamepad data structures နှင့် parsing |
| `behavior_lib/motion.seq` | Dual arm example အတွက် trajectory data (YAML format) |

---

## ⚠️ သတိပြုရန်

1. **ဦးစွာ Motion Control Service ကို ပိတ်ရန်:** Low-level control မသုံးခင် sport mode/AI mode စသည်တို့ကို ReleaseMode() ဖြင့် ပိတ်ရပါမည်
2. **Network Interface:** `eth0`, `enp2s0` စသည်ဖြင့် G1 နှင့် ချိတ်ဆက်ထားသော network interface name ကို ထည့်ပေးရန်
3. **CRC Check:** Data integrity အတွက် CRC32 checksum သုံးထားပါသည်

---

## 🔧 Build လုပ်နည်း

```bash
cd /path/to/unitree_sdk2
mkdir build && cd build
cmake ..
make
```

---

## 📊 Motor/Joint များ

G1 စက်ရုပ်တွင် စုစုပေါင်း **29 motors** ရှိပါသည်:

| အစိတ်အပိုင်း | Motors |
|-------------|--------|
| ခြေထောက် (Legs) | 12 (6 per leg) |
| ခါး (Waist) | 3 |
| လက် (Arms) | 14 (7 per arm) |

