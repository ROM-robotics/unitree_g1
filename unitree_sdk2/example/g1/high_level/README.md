# G1 High Level Control Examples

ဤ folder တွင် Unitree G1 စက်ရုပ်အတွက် high-level (အမြင့်ဆုံးအဆင့်) control examples များပါဝင်ပါသည်။ Low-level နှင့်မတူဘဲ motor တစ်ခုချင်းစီကို တိုက်ရိုက်မထိန်းဘဲ API client များမှတစ်ဆင့် အဆင့်မြင့် commands များပေးပို့ပါသည်။

---

## 📁 ဖိုင်များ အကျဥ်းချုပ်

### 1. `g1_arm_action_example.cpp`

**ရည်ရွယ်ချက်:** ကြိုတင်သတ်မှတ်ထားသော arm actions များကို execute လုပ်ခြင်း

**အလုပ်လုပ်ပုံ:**
- `G1ArmActionClient` API ကိုသုံးပြီး predefined arm actions များကို ခေါ်သည်
- Action ID သို့မဟုတ် Action Name ဖြင့် execute လုပ်နိုင်သည်

**Command Line Options:**
| Option | ရှင်းလင်းချက် |
|--------|---------------|
| `-l, --list` | ရရှိနိုင်သော actions အားလုံးကို list လုပ်သည် |
| `-i, --id <id>` | Action ID ဖြင့် execute လုပ်သည် |
| `--name <name>` | Custom action name ဖြင့် execute လုပ်သည် |
| `--stop` | လက်ရှိ custom action ကို ရပ်သည် |
| `-n, --network` | Network interface သတ်မှတ်သည် |

**သုံးနည်း:**
```bash
# Actions list ကြည့်ရန်
./g1_arm_action_example -n eth0 --list

# Action ID 1 ကို execute လုပ်ရန်
./g1_arm_action_example -n eth0 -i 1
```

**မှတ်ချက်:** FSM ID {500, 501, 801} states တွင်သာ actions များ အလုပ်လုပ်ပါသည်။

---

### 2. `g1_arm5_sdk_dds_example.cpp`

**ရည်ရွယ်ချက်:** G1 23-DOF model (5-DOF arms) အတွက် arm control demo

**အလုပ်လုပ်ပုံ:**
1. **Initialization Phase (2 စက္ကန့်):** လက်များကို zero position သို့ ပြန်ယူသည်
2. **Control Phase (5 စက္ကန့်):** လက်များကို target position သို့ ရွှေ့သည်
   - Left arm: Shoulder Roll = 90°, Elbow = 90°
   - Right arm: Shoulder Roll = -90°, Elbow = 90°

**ထိန်းချုပ်သော Joints (13 ခု):**
- Left Arm: Shoulder (Pitch, Roll, Yaw), Elbow (Pitch, Roll)
- Right Arm: Shoulder (Pitch, Roll, Yaw), Elbow (Pitch, Roll)
- Waist: Yaw, Roll, Pitch

**Parameters:**
| Parameter | တန်ဖိုး |
|-----------|--------|
| Kp (stiffness) | 60 |
| Kd (damping) | 1.5 |
| Control dt | 20ms |
| Max joint velocity | 0.5 rad/s |

**သုံးနည်း:**
```bash
./g1_arm5_sdk_dds_example <network_interface>
```

---

### 3. `g1_arm7_sdk_dds_example.cpp`

**ရည်ရွယ်ချက်:** G1 29-DOF model (7-DOF arms) အတွက် arm control demo

**အလုပ်လုပ်ပုံ:**
- `g1_arm5_sdk_dds_example` နှင့် အတူတူပင်ဖြစ်သော်လည်း wrist joints (3 ခု) ပိုပါဝင်သည်

**ထိန်းချုပ်သော Joints (17 ခု):**
- Left Arm: Shoulder (Pitch, Roll, Yaw), Elbow, Wrist (Roll, Pitch, Yaw)
- Right Arm: Shoulder (Pitch, Roll, Yaw), Elbow, Wrist (Roll, Pitch, Yaw)
- Waist: Yaw, Roll, Pitch

**G1 23-DOF vs 29-DOF ကွာခြားချက်:**
| Model | Arm DOF | Wrist Joints |
|-------|---------|--------------|
| 23-DOF | 5 | Roll only |
| 29-DOF | 7 | Roll, Pitch, Yaw |

**သုံးနည်း:**
```bash
./g1_arm7_sdk_dds_example <network_interface>
```

---

### 4. `g1_loco_client_example.cpp`

**ရည်ရွယ်ချက်:** G1 locomotion (လမ်းလျှောက်ခြင်း/ရပ်ခြင်း) ကို ထိန်းချုပ်ခြင်း

**အလုပ်လုပ်ပုံ:**
- `G1LocoClient` API ကိုသုံးပြီး စက်ရုပ်၏ locomotion state များကို ထိန်းချုပ်သည်
- Command line arguments ဖြင့် အမျိုးမျိုးသော commands များ ပေးပို့နိုင်သည်

**ရရှိနိုင်သော Commands:**

| Command | ရှင်းလင်းချက် |
|---------|---------------|
| `--get_fsm_id` | လက်ရှိ FSM state ID ကိုရယူသည် |
| `--get_fsm_mode` | လက်ရှိ FSM mode ကိုရယူသည် |
| `--get_balance_mode` | Balance mode ကိုရယူသည် |
| `--get_swing_height` | ခြေလှမ်း အမြင့်ကိုရယူသည် |
| `--get_stand_height` | ရပ်နေသော အမြင့်ကိုရယူသည် |
| `--get_phase` | လက်ရှိ gait phase ကိုရယူသည် |
| `--set_fsm_id=<id>` | FSM state ကိုပြောင်းသည် |
| `--set_balance_mode=<mode>` | Balance mode သတ်မှတ်သည် |
| `--set_swing_height=<height>` | ခြေလှမ်း အမြင့်သတ်မှတ်သည် |
| `--set_stand_height=<height>` | ရပ်နေသော အမြင့်သတ်မှတ်သည် |
| `--set_velocity="vx vy omega [duration]"` | Velocity command ပေးသည် |
| `--move="vx vy omega"` | ရွေ့ရန် command ပေးသည် |
| `--damp` | Damp mode သို့ပြောင်းသည် |
| `--start` | စတင်သည် |
| `--squat` | ထိုင်ခုံထိုင်သည် |
| `--sit` | ထိုင်သည် |
| `--stand_up` | ထရပ်သည် |
| `--zero_torque` | Torque များကို zero လုပ်သည် |
| `--stop_move` | ရွေ့ခြင်းရပ်သည် |
| `--high_stand` | မြင့်မြင့်ရပ်သည် |
| `--low_stand` | နိမ့်နိမ့်ရပ်သည် |
| `--balance_stand` | Balance ယူပြီးရပ်သည် |
| `--continous_gait=<true/false>` | Continuous gait on/off |
| `--switch_move_mode=<true/false>` | Move mode ပြောင်းသည် |
| `--shake_hand` | လက်ဆွဲနှုတ်ဆက်သည် |
| `--wave_hand` | လက်ဝှေ့သည် |
| `--wave_hand_with_turn` | လှည့်ပြီး လက်ဝှေ့သည် |
| `--set_speed_mode=<mode>` | Speed mode သတ်မှတ်သည် |

**သုံးနည်း:**
```bash
# FSM ID ကြည့်ရန်
./g1_loco_client_example --network_interface=eth0 --get_fsm_id

# ရှေ့ကို လျှောက်ရန် (vx=0.5 m/s)
./g1_loco_client_example --network_interface=eth0 --set_velocity="0.5 0 0"

# ထရပ်ရန်
./g1_loco_client_example --network_interface=eth0 --stand_up

# လက်ဝှေ့ရန်
./g1_loco_client_example --network_interface=eth0 --wave_hand
```

---

## 📊 High-Level vs Low-Level ကွာခြားချက်

| အချက် | High-Level | Low-Level |
|-------|------------|-----------|
| ထိန်းချုပ်မှု | API Client မှတစ်ဆင့် | Motor တစ်ခုချင်းစီ |
| Complexity | ရိုးရှင်း | ရှုပ်ထွေး |
| Flexibility | ကန့်သတ်ချက်ရှိ | လွတ်လပ်စွာထိန်းချုပ်နိုင် |
| Safety | Built-in | Manual implementation |
| Use case | Standard motions | Custom motions |

---

## ⚠️ သတိပြုရန်

1. **Network Interface:** G1 နှင့် ချိတ်ဆက်ထားသော network interface name ကို မှန်ကန်စွာထည့်ပေးရန်
2. **FSM State:** အချို့ commands များသည် သတ်မှတ်ထားသော FSM states တွင်သာ အလုပ်လုပ်ပါသည်
3. **Arm SDK Topic:** Arm control examples များသည် `rt/arm_sdk` topic ကို publish လုပ်သည်

