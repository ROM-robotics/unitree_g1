# 🧠 Unitree SDK AI Integration - Design Pattern Analysis

## 📐 1. Unitree SDK သုံးထားသော Design Patterns

### 1.1 Publish-Subscribe Pattern (DDS-based)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    DDS (Data Distribution Service)                   │
│                  Cyclone DDS / Eclipse Iceoryx                       │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  Publisher                    Topic                    Subscriber    │
│  ┌─────────┐              ┌──────────┐              ┌─────────┐     │
│  │ Robot   │ ─────────→   │rt/lowcmd │ ─────────→   │ User    │     │
│  │ Sensors │              │rt/lowstate│              │ Code    │     │
│  └─────────┘              │rt/audio_msg│             └─────────┘     │
│                           └──────────┘                               │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

**Code Example:**
```cpp
// Subscriber Pattern
ChannelSubscriber<std_msgs::msg::dds_::String_> subscriber("rt/audio_msg");
subscriber.InitChannel(asr_handler);  // Callback function

// Publisher Pattern
ChannelPublisher<unitree_hg::msg::dds_::LowCmd_> publisher("rt/lowcmd");
publisher.Write(cmd);
```

### 1.2 Client-Service Pattern (RPC-like)

```cpp
// High-level API Client Pattern
unitree::robot::g1::AudioClient client;
client.Init();
client.SetTimeout(10.0f);

// Synchronous Request-Response
int32_t ret = client.TtsMaker("Hello", 1);
client.GetVolume(volume);
client.SetVolume(100);
```

### 1.3 Factory Pattern (Singleton)

```cpp
// ChannelFactory - Singleton Pattern
unitree::robot::ChannelFactory::Instance()->Init(0, "eth0");

// Creates DDS channels internally
ChannelPtr<MSG> channel = mDdsFactoryPtr->CreateTopicChannel<MSG>(name);
```

### 1.4 Observer Pattern (Callback-based)

```cpp
// Register callback for incoming messages
void asr_handler(const void* msg) {
    // Process incoming ASR message
    std_msgs::msg::dds_::String_* resMsg = (std_msgs::msg::dds_::String_*)msg;
    std::cout << resMsg->data() << std::endl;
}

// Attach observer
subscriber.InitChannel(asr_handler);
```

---

## 📊 2. AI Integration Architecture (Current Implementation)

```
┌─────────────────────────────────────────────────────────────────────┐
│                     CURRENT AI ARCHITECTURE                          │
│                      (Direct HTTP Calls)                             │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ┌─────────────┐      ┌─────────────┐      ┌─────────────┐          │
│  │   Robot     │      │  User App   │      │  Cloud API  │          │
│  │   (DDS)     │ ←──→ │  (C++ code) │ ───→ │  (OpenAI)   │          │
│  │             │      │             │ ←─── │             │          │
│  └─────────────┘      └─────────────┘      └─────────────┘          │
│                                                                      │
│  Pattern: Pub-Sub     Pattern: Direct     Pattern: REST API         │
│                       HTTP Client                                    │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

**Code Flow:**
```cpp
void asr_handler(const void* msg) {
    // 1. DDS Pub-Sub: Receive ASR text
    std::string user_text = resMsg->data();
    
    // 2. Direct HTTP: Call OpenAI API
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, OPENAI_API_URL);
    // ... direct HTTP request
    
    // 3. DDS Client: Send TTS response
    g_audio_client->TtsMaker(ai_response, lang);
}
```

---

## 🔄 3. MCP (Model Context Protocol) နှင့် နှိုင်းယှဥ်ချက်

### MCP Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                        MCP ARCHITECTURE                              │
│                   (Standardized AI Tool Protocol)                    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ┌───────────┐      ┌───────────┐      ┌───────────┐                │
│  │   Host    │      │    MCP    │      │   MCP     │                │
│  │  (Claude) │ ←──→ │  Client   │ ←──→ │  Server   │                │
│  │           │      │           │      │ (Tools)   │                │
│  └───────────┘      └───────────┘      └───────────┘                │
│                                                                      │
│  Protocol: JSON-RPC 2.0 over stdio/HTTP                             │
│  Features: Tools, Resources, Prompts, Sampling                       │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Comparison Table

| Feature | Unitree SDK (Current) | MCP |
|---------|----------------------|-----|
| **Protocol** | DDS + Direct HTTP | JSON-RPC 2.0 |
| **Transport** | UDP Multicast / TCP | stdio / SSE / HTTP |
| **Focus** | Robot Control | AI Tool Integration |
| **Latency** | Real-time (< 5ms) | Higher (HTTP overhead) |
| **Discovery** | Static Topics | Dynamic Tool Discovery |
| **State Management** | DDS Topics | Resources + Context |
| **AI Integration** | Direct API Calls | Standardized Interface |
| **Language Support** | C++ (primary) | Python, TypeScript, etc. |
| **Ecosystem** | Unitree specific | Universal AI tools |

---

## ❓ 4. MCP သုံးသင့်လား? (Detailed Analysis)

### ✅ MCP သုံးသင့်သော Cases

| Use Case | ဘာလို့? |
|----------|---------|
| **Multiple AI Models** | Claude, GPT, Gemini အားလုံးကို standardize လုပ်နိုင် |
| **Tool Discovery** | Robot capabilities ကို AI မှ auto-discover လုပ်နိုင် |
| **Complex Workflows** | Multi-step reasoning, planning |
| **Development Experience** | Easier debugging, logging |
| **Extensibility** | New tools ထည့်ရလွယ် |
| **Context Management** | Conversation history, resources |

**MCP Server for Unitree (Conceptual Design):**
```typescript
// MCP Server Tool Definitions
{
  "tools": [
    {
      "name": "robot_speak",
      "description": "Make the robot speak text using TTS",
      "inputSchema": {
        "type": "object",
        "properties": {
          "text": { "type": "string", "description": "Text to speak" },
          "language": { "enum": ["en", "zh", "my"], "description": "Language code" }
        },
        "required": ["text"]
      }
    },
    {
      "name": "robot_move",
      "description": "Move the robot with velocity commands",
      "inputSchema": {
        "type": "object",
        "properties": {
          "vx": { "type": "number", "description": "Forward velocity (m/s)" },
          "vy": { "type": "number", "description": "Lateral velocity (m/s)" },
          "omega": { "type": "number", "description": "Angular velocity (rad/s)" },
          "duration": { "type": "number", "description": "Duration in seconds" }
        },
        "required": ["vx", "vy", "omega"]
      }
    },
    {
      "name": "robot_arm_action",
      "description": "Execute predefined arm action",
      "inputSchema": {
        "type": "object",
        "properties": {
          "action_id": { "type": "integer", "description": "Action ID to execute" },
          "action_name": { "type": "string", "description": "Or action name" }
        }
      }
    },
    {
      "name": "robot_get_state",
      "description": "Get current robot state",
      "inputSchema": {
        "type": "object",
        "properties": {
          "state_type": { 
            "enum": ["fsm", "battery", "joints", "imu"],
            "description": "Type of state to retrieve"
          }
        }
      }
    }
  ],
  "resources": [
    {
      "uri": "robot://state/joints",
      "name": "Joint States",
      "description": "Current joint positions and velocities"
    },
    {
      "uri": "robot://state/imu",
      "name": "IMU Data",
      "description": "Orientation and angular velocity"
    }
  ]
}
```

### ❌ MCP မသုံးသင့်သော Cases

| Use Case | ဘာလို့? |
|----------|---------|
| **Real-time Control** | MCP latency က robot control အတွက် မြင့်လွန်း (>50ms) |
| **Low-level Motor Control** | 500Hz-1kHz loop မှာ JSON-RPC overhead ကြီးလွန်း |
| **Existing DDS Infrastructure** | Unitree SDK က DDS optimized ဖြစ်ပြီးသား |
| **Embedded Systems** | Memory/CPU constraints on robot |
| **Safety-critical Operations** | JSON parsing errors can cause issues |

---

## 🎯 5. Recommended Architecture (Hybrid Approach)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    RECOMMENDED HYBRID ARCHITECTURE                   │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │                    HIGH-LEVEL LAYER                          │    │
│  │              (MCP Server - AI Interface)                     │    │
│  │                                                              │    │
│  │  ┌─────────┐    ┌─────────┐    ┌─────────┐                  │    │
│  │  │ Claude  │    │  GPT    │    │ Gemini  │                  │    │
│  │  └────┬────┘    └────┬────┘    └────┬────┘                  │    │
│  │       └──────────────┴──────────────┘                        │    │
│  │                      │                                       │    │
│  │              ┌───────▼───────┐                               │    │
│  │              │  MCP Server   │ ← JSON-RPC (High-level cmds)  │    │
│  │              │ (Python/Node) │                               │    │
│  │              └───────┬───────┘                               │    │
│  └──────────────────────┼───────────────────────────────────────┘    │
│                         │                                            │
│  ┌──────────────────────┼───────────────────────────────────────┐    │
│  │                      ▼                                       │    │
│  │              ┌───────────────┐                               │    │
│  │              │   Bridge      │ ← Translation Layer           │    │
│  │              │  (C++/Python) │                               │    │
│  │              └───────┬───────┘                               │    │
│  │                      │                                       │    │
│  │                LOW-LEVEL LAYER                               │    │
│  │              (DDS - Robot Control)                           │    │
│  │                      │                                       │    │
│  │  ┌─────────┬─────────┴─────────┬─────────┐                  │    │
│  │  ▼         ▼                   ▼         ▼                  │    │
│  │ ┌────┐  ┌────┐            ┌────┐     ┌────┐                 │    │
│  │ │TTS │  │ASR │            │Loco│     │Arm │                 │    │
│  │ └────┘  └────┘            └────┘     └────┘                 │    │
│  │                                                              │    │
│  │           ← DDS Topics (Real-time, <5ms) →                   │    │
│  │                                                              │    │
│  └──────────────────────────────────────────────────────────────┘    │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Layer Responsibilities

| Layer | Technology | Latency | Responsibility |
|-------|------------|---------|----------------|
| **AI Interface** | MCP Server | ~100-500ms | AI reasoning, planning, NLU |
| **Bridge** | Python/C++ | ~10-50ms | Command translation, validation |
| **Robot Control** | DDS | <5ms | Real-time motor control |

---

## 🛠️ 6. Implementation Options

### Option A: Pure DDS (Current)
```
Pros: Low latency, proven, stable
Cons: Hard-coded AI integration, difficult to switch AI providers
```

### Option B: MCP + DDS Bridge (Recommended)
```
Pros: Flexible AI integration, standardized, extensible
Cons: Additional complexity, slightly higher latency for commands
```

### Option C: Full MCP (Not Recommended for Robots)
```
Pros: Full standardization
Cons: Too slow for real-time control, not suitable for robotics
```

---

## 📝 7. Summary / နိဂုံးချုပ်

| Question | Answer |
|----------|--------|
| **Unitree SDK က ဘာ pattern သုံးလဲ?** | DDS Pub-Sub + Client-Service + Factory (Singleton) + Observer |
| **AI integration ဘယ်လိုလုပ်ထားလဲ?** | Direct HTTP calls (libcurl) to OpenAI/Google APIs |
| **MCP သုံးထားလား?** | ❌ မသုံးထားပါ |
| **MCP ဆိုင်သလား?** | ⚠️ High-level AI tasks အတွက်သာ ဆိုင်သည် |
| **MCP သုံးသင့်လား?** | **Hybrid approach** - MCP for AI planning, DDS for real-time control |

### Final Recommendation

```
┌─────────────────────────────────────────────────────┐
│              RECOMMENDED APPROACH                    │
├─────────────────────────────────────────────────────┤
│                                                      │
│  1. Real-time control (legs, arms)                  │
│     → DDS ကိုပဲ ဆက်သုံးပါ                           │
│                                                      │
│  2. AI conversation/planning                        │
│     → MCP server တစ်ခု ထည့်နိုင်သည်                 │
│                                                      │
│  3. Best approach                                   │
│     → MCP → Bridge → DDS (layered architecture)    │
│                                                      │
│  4. Use MCP for:                                    │
│     • Voice commands interpretation                 │
│     • Task planning                                 │
│     • Multi-model AI switching                      │
│     • Tool discovery                                │
│                                                      │
│  5. Keep DDS for:                                   │
│     • Motor commands (500Hz-1kHz)                   │
│     • Sensor data streaming                         │
│     • Low-level safety controls                     │
│                                                      │
└─────────────────────────────────────────────────────┘
```

---

*Last Updated: January 29, 2026*
