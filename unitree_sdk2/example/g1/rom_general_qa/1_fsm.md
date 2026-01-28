# Unitree G1 FSM (Finite State Machine) Documentation

## FSM ဆိုတာ ဘာလဲ?

**FSM** သည် **Finite State Machine** (အခြေအနေ အကန့်အသတ်ရှိ စက်) ၏ အတိုကောက်ဖြစ်ပါသည်။ G1 စက်ရုပ်တွင် FSM သည် စက်ရုပ်၏ **လက်ရှိအခြေအနေ** (ရပ်နေ/ထိုင်နေ/လျှောက်နေ စသည်) ကို ကိုယ်စားပြုပါသည်။

FSM system သည် စက်ရုပ်၏ behavior ကို ထိန်းချုပ်ပြီး state တစ်ခုမှ အခြား state သို့ ပြောင်းရာတွင် safety နှင့် stability ကို သေချာစေပါသည်။

---

## 📊 G1 EDU U2 FSM ID များ (Complete List)

### အဓိက FSM States

| FSM ID | State Name | ရှင်းလင်းချက် (မြန်မာ) |
|--------|------------|------------------------|
| **0** | Zero Torque | Motor များအားလုံး torque မရှိ (limp mode) |
| **1** | Damping | Damping mode - motor များ soft brake |
| **2** | Squat | ထိုင်ခုံထိုင်နေသော အခြေအနေ |
| **3** | Sit | ထိုင်နေသော အခြေအနေ (ပိုနိမ့်) |
| **4** | Stand Up | ထရပ်ခြင်း (transition state) |
| **500** | Stand/Start | ရပ်နေသော အခြေအနေ (Normal Stand) |
| **501** | Walking | လမ်းလျှောက်နေသော အခြေအနေ |
| **801** | Advanced Mode | အဆင့်မြင့် ထိန်းချုပ်မှု mode |

---

## 🔄 FSM Mode (801 Advanced Mode အတွင်း)

FSM ID 801 တွင် sub-modes များရှိပါသည်:

| FSM Mode | ရှင်းလင်းချက် |
|----------|---------------|
| **0** | Default mode (arm actions အလုပ်လုပ်သည်) |
| **3** | Alternative mode (arm actions အလုပ်လုပ်သည်) |
| **အခြား** | Arm actions များ အလုပ်မလုပ်ပါ |

---

## 🎯 FSM ID နှင့် API Function Mapping

| Function | FSM ID သို့ ပြောင်းသည် |
|----------|----------------------|
| `ZeroTorque()` | 0 |
| `Damp()` | 1 |
| `Squat()` | 2 |
| `Sit()` | 3 |
| `StandUp()` | 4 |
| `Start()` | 500 |

---

## ⚡ Arm Actions အလုပ်လုပ်သော FSM States

Arm actions များသည် အောက်ပါ FSM states တွင်သာ အလုပ်လုပ်ပါသည်:

| FSM ID | State | Arm Actions |
|--------|-------|-------------|
| 500 | Stand | ✅ အလုပ်လုပ်သည် |
| 501 | Walking | ✅ အလုပ်လုပ်သည် |
| 801 (mode 0, 3) | Advanced | ✅ အလုပ်လုပ်သည် |
| အခြား | - | ❌ အလုပ်မလုပ်ပါ |

---

## 📈 State Transition Diagram

```
                    ┌─────────────┐
                    │  0: Zero    │
                    │   Torque    │
                    └──────┬──────┘
                           │
                           ▼
                    ┌─────────────┐
                    │  1: Damping │
                    └──────┬──────┘
                           │
              ┌────────────┼────────────┐
              ▼            ▼            ▼
       ┌──────────┐  ┌──────────┐  ┌──────────┐
       │ 2: Squat │  │  3: Sit  │  │4: StandUp│
       └────┬─────┘  └────┬─────┘  └────┬─────┘
            │             │             │
            └─────────────┼─────────────┘
                          ▼
                   ┌─────────────┐
                   │ 500: Stand  │◄──────────┐
                   │   (Start)   │           │
                   └──────┬──────┘           │
                          │                  │
                          ▼                  │
                   ┌─────────────┐           │
                   │501: Walking │───────────┘
                   └──────┬──────┘
                          │
                          ▼
                   ┌─────────────┐
                   │801: Advanced│
                   │    Mode     │
                   └─────────────┘
```

---

## 💡 အဘယ်ကြောင့် FSM အရေးကြီးသနည်း?

1. **Safety**: အချို့ commands များသည် သတ်မှတ်ထားသော states တွင်သာ အလုပ်လုပ်သည်
2. **State Transition**: State တစ်ခုမှ အခြား state သို့ ပြောင်းရာတွင် FSM က ထိန်းချုပ်သည်
3. **Error Prevention**: မမှန်ကန်သော state တွင် command ပေးလျှင် error ပြန်ပေးသည်
4. **Stability**: Smooth transition ဖြင့် စက်ရုပ် balance မပျက်စေ

---

## 🔧 FSM စစ်ဆေးနည်းနှင့် ပြောင်းနည်း

### Command Line မှ စစ်ဆေးခြင်း

```bash
# လက်ရှိ FSM ID ကြည့်ရန်
./g1_loco_client_example --network_interface=eth0 --get_fsm_id

# လက်ရှိ FSM Mode ကြည့်ရန်
./g1_loco_client_example --network_interface=eth0 --get_fsm_mode

# FSM ID ပြောင်းရန် (ဥပမာ - Stand state သို့)
./g1_loco_client_example --network_interface=eth0 --set_fsm_id=500
```

### DDS Topic မှ Subscribe လုပ်ခြင်း

```cpp
// rt/sportmodestate topic ကို subscribe လုပ်ပြီး FSM state ကို monitor လုပ်နိုင်သည်
// SportModeState_ message တွင် fsm_id နှင့် fsm_mode ပါဝင်သည်
```

---

## ⚠️ Error Codes

| Error Code | Error Name | ရှင်းလင်းချက် |
|------------|------------|---------------|
| 7301 | LOCOSTATE_NOT_AVAILABLE | LocoState data မရရှိပါ |
| 7302 | INVALID_FSM_ID | မမှန်ကန်သော FSM ID |
| 7303 | INVALID_TASK_ID | မမှန်ကန်သော Task ID |
| 7404 | INVALID_FSM_ID (Arm) | Arm action အတွက် FSM ID မမှန်ကန်ပါ |

---

## 📝 C++ Code Example

```cpp
#include <unitree/robot/g1/loco/g1_loco_client.hpp>

int main() {
    unitree::robot::ChannelFactory::Instance()->Init(0, "eth0");
    
    unitree::robot::g1::LocoClient client;
    client.Init();
    client.SetTimeout(10.f);
    
    // လက်ရှိ FSM ID ရယူခြင်း
    int fsm_id;
    client.GetFsmId(fsm_id);
    std::cout << "Current FSM ID: " << fsm_id << std::endl;
    
    // Stand state သို့ ပြောင်းခြင်း
    client.Start();  // FSM ID 500 သို့ ပြောင်းသည်
    
    // သို့မဟုတ် တိုက်ရိုက် FSM ID သတ်မှတ်ခြင်း
    client.SetFsmId(500);
    
    return 0;
}
```

---

## 📌 အကျဥ်းချုပ်

- **FSM ID** = စက်ရုပ်၏ main state (ဘာလုပ်နေလဲ)
- **FSM Mode** = FSM ID 801 အတွင်း sub-mode
- **မှန်ကန်သော state တွင်သာ** သက်ဆိုင်ရာ commands များ execute လုပ်နိုင်သည်
- **rt/sportmodestate** topic ကို subscribe လုပ်ပြီး real-time monitoring လုပ်နိုင်သည်

