/**
 * @file g1_audio_openai_example.cpp
 * @brief Unitree G1 Audio Client with OpenAI Integration - Closed Loop Conversation
 * 
 * ဤ program သည် Robot ၏ ASR (Speech-to-Text) ကို နားထောင်ပြီး
 * OpenAI API သို့ ပို့ကာ response ကို TTS ဖြင့် ပြန်ပြောသည်။
 * 
 * Features:
 * - ASR: လူပြောသံ → Text
 * - OpenAI: Text → AI Response
 * - TTS: AI Response → Robot Speaker
 * 
 * Dependencies:
 * - libcurl (for HTTP requests)
 * - nlohmann/json (for JSON parsing)
 * 
 * Build:
 *   g++ -o g1_audio_openai g1_audio_openai_example.cpp -lunitree_sdk2 -lcurl -lpthread
 */

#include <fstream>
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <sstream>
#include <curl/curl.h>

#include <unitree/common/time/time_tool.hpp>
#include <unitree/idl/ros2/String_.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>
#include <unitree/robot/g1/audio/g1_audio_client.hpp>

// ============================================================================
// Configuration
// ============================================================================

#define AUDIO_SUBSCRIBE_TOPIC "rt/audio_msg"
#define OPENAI_API_URL "https://api.openai.com/v1/chat/completions"

// ⚠️ သင့် OpenAI API Key ကို ဒီမှာ ထည့်ပါ သို့မဟုတ် environment variable သုံးပါ
// export OPENAI_API_KEY="sk-your-api-key-here"
std::string OPENAI_API_KEY = "";

// System prompt for the AI assistant
const std::string SYSTEM_PROMPT = R"(
You are a helpful AI assistant running on a Unitree G1 humanoid robot.
You can understand and respond in both English and Chinese.
Keep your responses concise and natural for spoken conversation.
Respond in the same language the user speaks to you.
)";

// ============================================================================
// Global Variables
// ============================================================================

unitree::robot::g1::AudioClient* g_audio_client = nullptr;
std::mutex g_mutex;
std::queue<std::string> g_message_queue;
std::atomic<bool> g_running(true);

// Conversation history for context
std::vector<std::pair<std::string, std::string>> g_conversation_history;
const size_t MAX_HISTORY = 10;  // Keep last 10 exchanges

// ============================================================================
// CURL Callback for Response
// ============================================================================

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// ============================================================================
// OpenAI API Call
// ============================================================================

std::string callOpenAI(const std::string& user_message) {
    if (OPENAI_API_KEY.empty()) {
        // Try to get from environment variable
        const char* env_key = std::getenv("OPENAI_API_KEY");
        if (env_key) {
            OPENAI_API_KEY = env_key;
        } else {
            return "Error: OpenAI API key not configured. Please set OPENAI_API_KEY environment variable.";
        }
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        return "Error: Failed to initialize CURL";
    }

    // Build messages array with conversation history
    std::ostringstream messages_json;
    messages_json << "[";
    messages_json << R"({"role": "system", "content": ")" << SYSTEM_PROMPT << R"("})";
    
    // Add conversation history
    for (const auto& exchange : g_conversation_history) {
        messages_json << R"(,{"role": "user", "content": ")" << exchange.first << R"("})";
        messages_json << R"(,{"role": "assistant", "content": ")" << exchange.second << R"("})";
    }
    
    // Add current user message
    messages_json << R"(,{"role": "user", "content": ")" << user_message << R"("})";
    messages_json << "]";

    // Build request body
    std::ostringstream request_body;
    request_body << R"({)";
    request_body << R"("model": "gpt-4o-mini",)";  // သို့မဟုတ် "gpt-4" သုံးနိုင်သည်
    request_body << R"("messages": )" << messages_json.str() << ",";
    request_body << R"("max_tokens": 150,)";  // Keep responses short for TTS
    request_body << R"("temperature": 0.7)";
    request_body << R"(})";

    std::string response_string;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + OPENAI_API_KEY).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, OPENAI_API_URL);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.str().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);  // 30 seconds timeout

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return "Error: Network request failed - " + std::string(curl_easy_strerror(res));
    }

    // Parse JSON response (simple parsing without external library)
    // In production, use nlohmann/json or similar
    std::string content_marker = R"("content":")";
    size_t content_pos = response_string.find(content_marker);
    if (content_pos == std::string::npos) {
        // Check for error
        if (response_string.find("error") != std::string::npos) {
            return "Error: API returned an error. Please check your API key.";
        }
        return "Error: Could not parse response";
    }

    content_pos += content_marker.length();
    size_t end_pos = response_string.find(R"(")", content_pos);
    
    // Handle escaped quotes and newlines
    std::string content;
    for (size_t i = content_pos; i < response_string.length() && response_string[i] != '"'; ++i) {
        if (response_string[i] == '\\' && i + 1 < response_string.length()) {
            char next = response_string[i + 1];
            if (next == 'n') {
                content += ' ';  // Replace newline with space for TTS
                ++i;
            } else if (next == '"') {
                content += '"';
                ++i;
            } else if (next == '\\') {
                content += '\\';
                ++i;
            } else {
                content += response_string[i];
            }
        } else {
            content += response_string[i];
        }
    }

    // Update conversation history
    g_conversation_history.push_back({user_message, content});
    if (g_conversation_history.size() > MAX_HISTORY) {
        g_conversation_history.erase(g_conversation_history.begin());
    }

    return content;
}

// ============================================================================
// ASR Message Handler
// ============================================================================

void asr_handler(const void* msg) {
    std_msgs::msg::dds_::String_* resMsg = (std_msgs::msg::dds_::String_*)msg;
    std::string user_text = resMsg->data();
    
    if (user_text.empty()) {
        return;
    }

    std::cout << "\n🎤 [ASR] User said: " << user_text << std::endl;
    
    // Add to message queue for processing
    std::lock_guard<std::mutex> lock(g_mutex);
    g_message_queue.push(user_text);
}

// ============================================================================
// Detect Language (Simple heuristic)
// ============================================================================

int detectLanguage(const std::string& text) {
    // Check for Chinese characters (Unicode range)
    for (unsigned char c : text) {
        if (c > 127) {  // Non-ASCII, likely Chinese
            return 0;   // Chinese
        }
    }
    return 1;  // English
}

// ============================================================================
// Conversation Processing Thread
// ============================================================================

void conversation_thread() {
    std::cout << "🤖 Conversation thread started. Waiting for user input..." << std::endl;
    
    while (g_running) {
        std::string user_message;
        
        // Check for new message
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            if (!g_message_queue.empty()) {
                user_message = g_message_queue.front();
                g_message_queue.pop();
            }
        }
        
        if (!user_message.empty()) {
            // LED: Processing (Yellow/Orange)
            g_audio_client->LedControl(255, 165, 0);
            
            std::cout << "🧠 [OpenAI] Processing..." << std::endl;
            
            // Call OpenAI API
            std::string ai_response = callOpenAI(user_message);
            
            std::cout << "💬 [AI Response] " << ai_response << std::endl;
            
            // Detect language for TTS
            int language = detectLanguage(ai_response);
            
            // LED: Speaking (Green)
            g_audio_client->LedControl(0, 255, 0);
            
            // TTS: Speak the response
            int32_t ret = g_audio_client->TtsMaker(ai_response, language);
            if (ret != 0) {
                std::cerr << "❌ TTS Error: " << ret << std::endl;
            }
            
            // Wait for TTS to finish (estimate based on text length)
            int wait_time = std::max(3, (int)(ai_response.length() / 10));
            unitree::common::Sleep(wait_time);
            
            // LED: Listening (Blue)
            g_audio_client->LedControl(0, 0, 255);
        }
        
        // Small delay to prevent busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// ============================================================================
// Signal Handler for Graceful Shutdown
// ============================================================================

void signalHandler(int signum) {
    std::cout << "\n\n🛑 Shutting down..." << std::endl;
    g_running = false;
}

// ============================================================================
// Main Function
// ============================================================================

int main(int argc, char const* argv[]) {
    std::cout << "╔════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║     Unitree G1 - OpenAI Conversational AI Example          ║" << std::endl;
    std::cout << "║                    Closed Loop Version                      ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════╝" << std::endl;
    std::cout << std::endl;

    // Check arguments
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <NetworkInterface> [OpenAI_API_Key]" << std::endl;
        std::cout << "Example: " << argv[0] << " eth0" << std::endl;
        std::cout << "         " << argv[0] << " eth0 sk-your-api-key" << std::endl;
        std::cout << std::endl;
        std::cout << "Or set environment variable: export OPENAI_API_KEY=sk-your-api-key" << std::endl;
        return 1;
    }

    // Get API key from argument or environment
    if (argc >= 3) {
        OPENAI_API_KEY = argv[2];
    }

    // Initialize CURL globally
    curl_global_init(CURL_GLOBAL_ALL);

    // Setup signal handler
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Initialize ChannelFactory
    std::cout << "🔌 Initializing connection on interface: " << argv[1] << std::endl;
    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);

    // Initialize AudioClient
    unitree::robot::g1::AudioClient client;
    client.Init();
    client.SetTimeout(10.0f);
    g_audio_client = &client;

    // Set volume to 100%
    client.SetVolume(100);
    std::cout << "🔊 Volume set to 100%" << std::endl;

    // Initialize ASR Subscriber
    unitree::robot::ChannelSubscriber<std_msgs::msg::dds_::String_> subscriber(
        AUDIO_SUBSCRIBE_TOPIC);
    subscriber.InitChannel(asr_handler);
    std::cout << "🎤 ASR subscriber initialized on topic: " << AUDIO_SUBSCRIBE_TOPIC << std::endl;

    // LED: Ready (Blue)
    client.LedControl(0, 0, 255);

    // Greeting
    std::cout << "🗣️ Speaking greeting..." << std::endl;
    client.TtsMaker("Hello! I am your AI assistant. You can talk to me now.", 1);
    unitree::common::Sleep(4);

    // Start conversation thread
    std::thread conv_thread(conversation_thread);

    std::cout << std::endl;
    std::cout << "═══════════════════════════════════════════════════════════════" << std::endl;
    std::cout << "  🎯 System Ready! Speak to the robot to start conversation.  " << std::endl;
    std::cout << "  📌 Press Ctrl+C to exit.                                    " << std::endl;
    std::cout << "═══════════════════════════════════════════════════════════════" << std::endl;
    std::cout << std::endl;

    // Main loop - wait for shutdown
    while (g_running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Cleanup
    conv_thread.join();
    client.LedControl(0, 0, 0);  // LED off
    curl_global_cleanup();

    std::cout << "👋 Goodbye!" << std::endl;
    return 0;
}
