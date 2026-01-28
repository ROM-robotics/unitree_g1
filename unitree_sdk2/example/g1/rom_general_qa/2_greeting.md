# G1 Robot Greeting Actions 👋

G1 စက်ရုပ်တွင် greeting actions နှစ်မျိုးရှိပါသည်။

---

## 1. လက်ဝှေ့ခြင်း (Wave Hand) 🙋

### Command Line

```bash
# ရိုးရိုး လက်ဝှေ့ရန်
./g1_loco_client_example --network_interface=eth0 --wave_hand

# ခန္ဓာကိုယ် လှည့်ပြီး လက်ဝှေ့ရန်
./g1_loco_client_example --network_interface=eth0 --wave_hand_with_turn
```

### C++ API

```cpp
client.WaveHand();       // ရိုးရိုး wave
client.WaveHand(true);   // လှည့်ပြီး wave
```

---

## 2. လက်ဆွဲနှုတ်ဆက်ခြင်း (Shake Hand) 🤝

### Command Line

```bash
./g1_loco_client_example --network_interface=eth0 --shake_hand
```

ဒီ command သည် 10 စက္ကန့် ကြာပြီး အလိုအလျောက် ရပ်သွားပါမည်။

### C++ API

```cpp
client.ShakeHand(0);   // လက်ဆွဲ စတင်
sleep(10);             // 10 စက္ကန့် စောင့်
client.ShakeHand(1);   // လက်ဆွဲ ရပ်
```

---

## 📝 Complete C++ Example

```cpp
#include <unitree/robot/g1/loco/g1_loco_client.hpp>
#include <unistd.h>

int main() {
    // DDS Init
    unitree::robot::ChannelFactory::Instance()->Init(0, "eth0");
    
    // LocoClient ဖန်တီးခြင်း
    unitree::robot::g1::LocoClient client;
    client.Init();
    client.SetTimeout(10.f);
    
    // ဦးစွာ Stand state သို့ ပြောင်းရန် (FSM ID 500)
    client.Start();
    sleep(2);
    
    // လက်ဝှေ့ရန်
    client.WaveHand();           // ရိုးရိုး wave
    // client.WaveHand(true);    // လှည့်ပြီး wave
    
    // သို့မဟုတ် လက်ဆွဲနှုတ်ဆက်ရန်
    // client.ShakeHand(0);      // စတင်
    // sleep(10);
    // client.ShakeHand(1);      // ရပ်
    
    return 0;
}
```

---

## ⚠️ သတိပြုရန်

1. **FSM State**: Greeting actions များသည် **FSM ID 500, 501, 801** states တွင်သာ အလုပ်လုပ်ပါသည်
2. **ဦးစွာ Stand state** သို့ ရောက်နေရပါမည် (`--start` သို့မဟုတ် `client.Start()`)
3. **network_interface** ကို G1 နှင့် ချိတ်ဆက်ထားသော interface name ဖြင့် အစားထိုးပါ (ဥပမာ: `eth0`, `enp2s0`)

---

## 📊 Greeting Actions အကျဥ်းချုပ်

| Action | Command | C++ Function | Task ID |
|--------|---------|--------------|---------|
| လက်ဝှေ့ | `--wave_hand` | `WaveHand()` | 0 |
| လှည့်ပြီး လက်ဝှေ့ | `--wave_hand_with_turn` | `WaveHand(true)` | 1 |
| လက်ဆွဲ စတင် | `--shake_hand` | `ShakeHand(0)` | 2 |
| လက်ဆွဲ ရပ် | - | `ShakeHand(1)` | 3 |

---

## 🔧 Internal Implementation

```cpp
// WaveHand သည် SetTaskId ကို ခေါ်သည်
int32_t WaveHand(bool turn_flag = false) { 
    return SetTaskId(turn_flag ? 1 : 0); 
}

// ShakeHand သည်လည်း SetTaskId ကို ခေါ်သည်
int32_t ShakeHand(int stage = -1) {
    switch (stage) {
        case 0:  return SetTaskId(2);  // စတင်
        case 1:  return SetTaskId(3);  // ရပ်
        default: // toggle
    }
}
```

---

## 🎯 Quick Start

```bash
# 1. Stand state သို့ ပြောင်းရန်
./g1_loco_client_example --network_interface=eth0 --start

# 2. လက်ဝှေ့ရန်
./g1_loco_client_example --network_interface=eth0 --wave_hand
```

