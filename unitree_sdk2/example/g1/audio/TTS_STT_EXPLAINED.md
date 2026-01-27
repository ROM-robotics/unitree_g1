# 🔊 G1 Audio System - TTS & STT/ASR ရှင်းလင်းချက်

## 📖 Overview

ဤ document တွင် Unitree G1 Robot ၏ Audio System (TTS, STT/ASR) အကြောင်း အသေးစိတ် ရှင်းလင်းထားသည်။

---

## 🎯 ဒီ Code က ဘာလုပ်လဲ?

### ✅ ပါဝင်သော Features

| Feature | ပါ/မပါ | Description |
|---------|--------|-------------|
| **TTS (Text-to-Speech)** | ✅ ပါ | စာသား → အသံထွက်ပြော |
| **STT (Speech-to-Text)** | ✅ ပါ | အသံ → စာသား (ASR) |
| **Audio Playback** | ✅ ပါ | WAV ဖိုင်ဖွင့် |
| **Recording** | ✅ ပါ | Mic မှ အသံသွင်း |
| **Volume Control** | ✅ ပါ | အသံအတိုးအလျော့ |
| **LED Control** | ✅ ပါ | မီးရောင်ထိန်းချုပ် |

---

## 🔊 TTS (Text-to-Speech) Data Flow

### ဘယ်ကလာ ဘယ်ကိုသွားလဲ?

```
┌─────────────────┐      ┌─────────────────┐      ┌─────────────────┐
│   Your Program  │  →   │   Robot CPU     │  →   │  Robot Speaker  │
│   (Text String) │      │   (TTS Engine)  │      │   (Audio Out)   │
└─────────────────┘      └─────────────────┘      └─────────────────┘
```

### Code Example

```cpp
// Chinese TTS - "你好。我是宇树科技的机器人。"
ret = client.TtsMaker("你好。我是宇树科技的机器人。例程启动成功", 0);

// English TTS - "Hello. I'm a robot..."
ret = client.TtsMaker("Hello. I'm a robot from Unitree Robotics.", 1);
```

### TTS Flow အဆင့်ဆင့်

| အဆင့် | ဘယ်ကနေ | ဘယ်ကိုသွား |
|-------|---------|-----------|
| 1 | **သင့် Code** (Text String) | Robot သို့ DDS ဖြင့် ပို့ |
| 2 | Robot ၏ TTS Engine | Text ကို Audio ပြောင်း |
| 3 | TTS Engine | **Robot Speaker** (အသံထွက်) |

---

## 🎤 STT/ASR (Speech-to-Text) Data Flow

### ဘယ်ကလာ ဘယ်ကိုသွားလဲ?

```
┌─────────────┐      ┌─────────────┐      ┌──────────────────┐
│  လူပြောသံ   │  →   │  Robot CPU  │  →   │  DDS Topic       │
│ (Microphone) │      │  (ASR Engine)│      │ "rt/audio_msg"   │
└─────────────┘      └─────────────┘      └──────────────────┘
                                                   │
                                                   ▼
                                          ┌──────────────────┐
                                          │  Your Program    │
                                          │  (Subscriber)    │
                                          └──────────────────┘
```

### Code Example

```cpp
// DDS Topic "rt/audio_msg" ကို subscribe လုပ်ထားသည်
unitree::robot::ChannelSubscriber<std_msgs::msg::dds_::String_> subscriber(
    "rt/audio_msg");
subscriber.InitChannel(asr_handler);

// ASR result ရောက်လာရင် ဒီ function ကို ခေါ်သည်
void asr_handler(const void *msg) {
  std_msgs::msg::dds_::String_ *resMsg = (std_msgs::msg::dds_::String_ *)msg;
  std::cout << resMsg->data() << std::endl;  // Console မှာ print
}
```

### STT/ASR Flow အဆင့်ဆင့်

| အဆင့် | ဘယ်ကနေ | ဘယ်ကိုသွား |
|-------|---------|-----------|
| 1 | Microphone | Robot ၏ ASR Engine |
| 2 | ASR Engine | DDS Topic `"rt/audio_msg"` |
| 3 | DDS Topic | **သင့် Program** (Subscriber) |
| 4 | Program | **Console output** (std::cout) |

---

## 🔄 TTS vs STT ယှဉ်ကြည့်ခြင်း

```
TTS (Text-to-Speech):
┌──────────┐     ┌───────────┐     ┌───────────┐
│ Your Code │ →  │ Robot TTS │ →   │  Speaker  │
│  (Text)   │     │  Engine   │     │ (အသံထွက်) │
└──────────┘     └───────────┘     └───────────┘

STT (Speech-to-Text):
┌───────────┐     ┌───────────┐     ┌──────────┐
│ Microphone │ →  │ Robot ASR │ →   │ Your Code │
│ (လူပြောသံ) │     │  Engine   │     │  (Text)   │
└───────────┘     └───────────┘     └──────────┘
```

### Summary Table

| Feature | Input (ဘယ်ကလာ) | Output (ဘယ်ကိုသွား) |
|---------|----------------|-------------------|
| **TTS** | သင့် code ထဲက text string | Robot ၏ Speaker (အသံထွက်ပြော) |
| **STT** | Robot ၏ Microphone (လူပြောသံ) | သင့် code ထဲ text string |

---

## ⚠️ လက်ရှိ Code ၏ ကန့်သတ်ချက်များ

### Current State (Open Loop)

```
┌─────────────────────────────────────────────────────────────┐
│                    CURRENT STATE (Open Loop)                 │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│   ASR: လူပြော → Robot ASR → Console Print (ဒါပဲ)            │
│                              ↓                               │
│                           ❌ Process မလုပ်                   │
│                           ❌ Response မပြန်                  │
│                                                              │
│   TTS: Hardcoded Text → Robot TTS → Speaker                 │
│         (ကြိုရေးထားတာပဲ)                                      │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### ❌ မပါသေးတာများ

| Feature | Status | Description |
|---------|--------|-------------|
| Online LLM API | ❌ မပါ | ChatGPT, Claude, Gemini စသည် |
| Closed Loop | ❌ မပါ | ASR → Think → TTS ပြန်ပြော |
| Conversation | ❌ မပါ | အပြန်အလှန် စကားပြော |

---

## ✅ Closed Loop ဖြစ်ဖို့ လိုအပ်တာ

### Desired State (Closed Loop)

```
┌──────────────────────────────────────────────────────────────┐
│                  CLOSED LOOP (လိုချင်တာ)                      │
├──────────────────────────────────────────────────────────────┤
│                                                               │
│   ┌─────────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐   │
│   │ ASR     │ →  │ Online  │ →  │ LLM     │ →  │ TTS     │   │
│   │ (နားထောင်)│    │   API   │    │ Response│    │ (ပြန်ပြော)│   │
│   └─────────┘    └─────────┘    └─────────┘    └─────────┘   │
│        ↑                                            │        │
│        └────────────────────────────────────────────┘        │
│                     (Continuous Loop)                         │
│                                                               │
└──────────────────────────────────────────────────────────────┘
```

### Closed Loop Implementation Concept

```cpp
void asr_handler(const void *msg) {
  std_msgs::msg::dds_::String_ *resMsg = (std_msgs::msg::dds_::String_ *)msg;
  std::string user_text = resMsg->data();
  
  // 1️⃣ ASR result ရပြီ
  std::cout << "User said: " << user_text << std::endl;
  
  // 2️⃣ Online API သို့ ပို့ (ဥပမာ - OpenAI)
  std::string ai_response = callOpenAI(user_text);  // ❌ Original code မှာ မပါ
  
  // 3️⃣ TTS ဖြင့် ပြန်ပြော
  client.TtsMaker(ai_response, 1);  // ❌ Original code မှာ မပါ
}
```

---

## 🎯 နိဂုံးချုပ်

| Question | Answer |
|----------|--------|
| Robot built-in TTS/ASR သုံးထားလား? | ✅ ဟုတ်ကဲ့ |
| Online API ချိတ်ထားလား? | ❌ မချိတ်ထားပါ |
| Closed loop ဖြစ်လား? | ❌ မဖြစ်သေးပါ |
| ဒီ code က ဘာလဲ? | **Demo/Example သာဖြစ်သည်** |

**ဒီ code က Robot ၏ Audio API တွေကို သီးခြားစီ test/demo လုပ်ပြထားတာသာ ဖြစ်ပါတယ်။ Real conversational AI robot ဖြစ်ဖို့ Online LLM API integration ထပ်ထည့်ရပါမည်!** 🤖

---

## 📚 Related Files

| File | Description |
|------|-------------|
| `g1_audio_client_example.cpp` | Original demo code |
| `g1_audio_openai_example.cpp` | OpenAI integrated closed loop version |
| `README.md` | API documentation |
| `wav.hpp` | WAV file utilities |
