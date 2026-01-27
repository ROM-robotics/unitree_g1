/**
 * @file g1_audio_burmese_example.cpp
 * @brief Unitree G1 Audio Client with Burmese Language Support
 * 
 * ဤ program သည် Google Cloud APIs ကို အသုံးပြုပြီး မြန်မာဘာသာဖြင့်
 * Robot နှင့် စကားပြောနိုင်အောင် ဖန်တီးထားသည်။
 * 
 * Features:
 * - Google Cloud STT: မြန်မာစကားပြောသံ → Text
 * - OpenAI GPT: Text → AI Response (မြန်မာ)
 * - Google Cloud TTS: မြန်မာ Text → Audio → Robot Speaker
 * 
 * Dependencies:
 * - libcurl (for HTTP requests)
 * - Google Cloud API Key
 * - OpenAI API Key
 * 
 * Build:
 *   g++ -o g1_audio_burmese g1_audio_burmese_example.cpp -lunitree_sdk2 -lcurl -lpthread
 */

#include <fstream>
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <sstream>
#include <cstring>
#include <vector>
#include <curl/curl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/socket.h>

#include <unitree/common/time/time_tool.hpp>
#include <unitree/idl/ros2/String_.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>
#include <unitree/robot/g1/audio/g1_audio_client.hpp>

#include "wav.hpp"

// ============================================================================
// Configuration
// ============================================================================

// API URLs
#define GOOGLE_STT_URL "https://speech.googleapis.com/v1/speech:recognize"
#define GOOGLE_TTS_URL "https://texttospeech.googleapis.com/v1/text:synthesize"
#define OPENAI_API_URL "https://api.openai.com/v1/chat/completions"

// Multicast settings for robot microphone
#define GROUP_IP "239.168.123.161"
#define MIC_PORT 5555
#define RECORD_SECONDS 5
#define SAMPLE_RATE 16000

// ⚠️ API Keys - Set via environment variables
// export GOOGLE_API_KEY="your-google-api-key"
// export OPENAI_API_KEY="your-openai-api-key"
std::string GOOGLE_API_KEY = "";
std::string OPENAI_API_KEY = "";

// System prompt for Burmese AI assistant
const std::string SYSTEM_PROMPT = R"(
You are a friendly AI assistant running on a Unitree G1 humanoid robot in Myanmar.
You MUST respond ONLY in Burmese (Myanmar) language using Myanmar script.
Keep your responses short and natural for spoken conversation.
Be helpful, polite, and culturally appropriate for Myanmar.
Always use Myanmar script (Unicode), not Zawgyi.
)";

// ============================================================================
// Global Variables
// ============================================================================

unitree::robot::g1::AudioClient* g_audio_client = nullptr;
std::mutex g_mutex;
std::atomic<bool> g_running(true);
std::atomic<bool> g_is_listening(false);

// Conversation history
std::vector<std::pair<std::string, std::string>> g_conversation_history;
const size_t MAX_HISTORY = 5;

// ============================================================================
// CURL Callback
// ============================================================================

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// ============================================================================
// Base64 Encoding (for audio data)
// ============================================================================

static const std::string base64_chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

std::string base64_encode(const unsigned char* data, size_t len) {
    std::string ret;
    int i = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (len--) {
        char_array_3[i++] = *(data++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (int j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (int j = 0; j < i + 1; j++)
            ret += base64_chars[char_array_4[j]];

        while (i++ < 3)
            ret += '=';
    }

    return ret;
}

std::vector<unsigned char> base64_decode(const std::string& encoded) {
    std::vector<unsigned char> ret;
    int i = 0;
    unsigned char char_array_4[4], char_array_3[3];
    
    for (char c : encoded) {
        if (c == '=') break;
        size_t pos = base64_chars.find(c);
        if (pos == std::string::npos) continue;
        
        char_array_4[i++] = pos;
        if (i == 4) {
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            
            for (i = 0; i < 3; i++)
                ret.push_back(char_array_3[i]);
            i = 0;
        }
    }
    
    if (i) {
        for (int j = i; j < 4; j++)
            char_array_4[j] = 0;
        
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        
        for (int j = 0; j < i - 1; j++)
            ret.push_back(char_array_3[j]);
    }
    
    return ret;
}

// ============================================================================
// Get Local IP for Multicast
// ============================================================================

std::string get_local_ip_for_multicast() {
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];
    std::string result = "";

    getifaddrs(&ifaddr);
    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) continue;
        getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST,
                    NULL, 0, NI_NUMERICHOST);
        std::string ip(host);
        if (ip.find("192.168.123.") == 0) {
            result = ip;
            break;
        }
    }
    freeifaddrs(ifaddr);
    return result;
}

// ============================================================================
// Record Audio from Robot Microphone (Multicast)
// ============================================================================

std::vector<int16_t> recordAudioFromRobot(int seconds) {
    std::cout << "🎤 မြန်မာလို ပြောပါ... (" << seconds << " စက္ကန့်)" << std::endl;
    
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "❌ Socket creation failed" << std::endl;
        return {};
    }

    sockaddr_in local_addr{};
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(MIC_PORT);
    local_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sock, (sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        std::cerr << "❌ Bind failed" << std::endl;
        close(sock);
        return {};
    }

    ip_mreq mreq{};
    inet_pton(AF_INET, GROUP_IP, &mreq.imr_multiaddr);
    std::string local_ip = get_local_ip_for_multicast();
    mreq.imr_interface.s_addr = inet_addr(local_ip.c_str());
    setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));

    // Set timeout
    struct timeval tv;
    tv.tv_sec = seconds + 2;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    int target_bytes = SAMPLE_RATE * 2 * seconds;  // 16-bit = 2 bytes
    int total_bytes = 0;
    std::vector<int16_t> pcm_data;
    pcm_data.reserve(target_bytes / 2);

    char buffer[1024];
    while (total_bytes < target_bytes && g_running) {
        ssize_t len = recvfrom(sock, buffer, sizeof(buffer), 0, nullptr, nullptr);
        if (len > 0) {
            size_t sample_count = len / 2;
            const int16_t* samples = reinterpret_cast<const int16_t*>(buffer);
            pcm_data.insert(pcm_data.end(), samples, samples + sample_count);
            total_bytes += len;
        } else {
            break;
        }
    }

    close(sock);
    std::cout << "✅ အသံဖမ်းပြီး (" << pcm_data.size() << " samples)" << std::endl;
    return pcm_data;
}

// ============================================================================
// Google Cloud Speech-to-Text (Burmese)
// ============================================================================

std::string googleSTT(const std::vector<int16_t>& audio_data) {
    if (audio_data.empty()) {
        return "";
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        return "Error: CURL init failed";
    }

    // Convert audio to base64
    std::string audio_base64 = base64_encode(
        reinterpret_cast<const unsigned char*>(audio_data.data()),
        audio_data.size() * sizeof(int16_t)
    );

    // Build request JSON
    std::ostringstream request_body;
    request_body << R"({
        "config": {
            "encoding": "LINEAR16",
            "sampleRateHertz": 16000,
            "languageCode": "my-MM",
            "alternativeLanguageCodes": ["en-US"],
            "enableAutomaticPunctuation": true
        },
        "audio": {
            "content": ")" << audio_base64 << R"("
        }
    })";

    std::string url = std::string(GOOGLE_STT_URL) + "?key=" + GOOGLE_API_KEY;
    std::string response_string;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.str().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return "Error: STT request failed";
    }

    // Parse response - find transcript
    std::string marker = R"("transcript": ")";
    size_t pos = response_string.find(marker);
    if (pos == std::string::npos) {
        std::cout << "⚠️ No speech detected" << std::endl;
        return "";
    }

    pos += marker.length();
    std::string transcript;
    for (size_t i = pos; i < response_string.length() && response_string[i] != '"'; ++i) {
        if (response_string[i] == '\\' && i + 1 < response_string.length()) {
            char next = response_string[i + 1];
            if (next == 'u' && i + 5 < response_string.length()) {
                // Unicode escape sequence
                std::string hex = response_string.substr(i + 2, 4);
                unsigned int codepoint = std::stoul(hex, nullptr, 16);
                // Convert to UTF-8
                if (codepoint < 0x80) {
                    transcript += static_cast<char>(codepoint);
                } else if (codepoint < 0x800) {
                    transcript += static_cast<char>(0xC0 | (codepoint >> 6));
                    transcript += static_cast<char>(0x80 | (codepoint & 0x3F));
                } else {
                    transcript += static_cast<char>(0xE0 | (codepoint >> 12));
                    transcript += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                    transcript += static_cast<char>(0x80 | (codepoint & 0x3F));
                }
                i += 5;
            } else {
                ++i;
                transcript += response_string[i];
            }
        } else {
            transcript += response_string[i];
        }
    }

    return transcript;
}

// ============================================================================
// OpenAI Chat (Burmese Response)
// ============================================================================

std::string callOpenAI(const std::string& user_message) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "Error: CURL init failed";
    }

    // Escape special characters in user message
    std::string escaped_message;
    for (char c : user_message) {
        if (c == '"') escaped_message += "\\\"";
        else if (c == '\\') escaped_message += "\\\\";
        else if (c == '\n') escaped_message += "\\n";
        else escaped_message += c;
    }

    // Build messages with history
    std::ostringstream messages_json;
    messages_json << "[";
    messages_json << R"({"role": "system", "content": ")" << SYSTEM_PROMPT << R"("})";
    
    for (const auto& exchange : g_conversation_history) {
        std::string escaped_user = exchange.first;
        std::string escaped_assistant = exchange.second;
        // Simple escape for quotes
        for (size_t i = 0; i < escaped_user.length(); ++i) {
            if (escaped_user[i] == '"') escaped_user.insert(i++, "\\");
        }
        for (size_t i = 0; i < escaped_assistant.length(); ++i) {
            if (escaped_assistant[i] == '"') escaped_assistant.insert(i++, "\\");
        }
        messages_json << R"(,{"role": "user", "content": ")" << escaped_user << R"("})";
        messages_json << R"(,{"role": "assistant", "content": ")" << escaped_assistant << R"("})";
    }
    
    messages_json << R"(,{"role": "user", "content": ")" << escaped_message << R"("})";
    messages_json << "]";

    std::ostringstream request_body;
    request_body << R"({"model": "gpt-4o-mini", "messages": )" << messages_json.str();
    request_body << R"(, "max_tokens": 150, "temperature": 0.7})";

    std::string response_string;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + OPENAI_API_KEY).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, OPENAI_API_URL);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.str().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return "တောင်းပန်ပါတယ်။ Network error ဖြစ်နေပါတယ်။";
    }

    // Parse response
    std::string marker = R"("content": ")";
    size_t pos = response_string.find(marker);
    if (pos == std::string::npos) {
        return "တောင်းပန်ပါတယ်။ Response error ဖြစ်နေပါတယ်။";
    }

    pos += marker.length();
    std::string content;
    for (size_t i = pos; i < response_string.length(); ++i) {
        if (response_string[i] == '"' && (i == 0 || response_string[i-1] != '\\')) {
            break;
        }
        if (response_string[i] == '\\' && i + 1 < response_string.length()) {
            char next = response_string[i + 1];
            if (next == 'n') {
                content += ' ';
                ++i;
            } else if (next == '"') {
                content += '"';
                ++i;
            } else if (next == 'u' && i + 5 < response_string.length()) {
                std::string hex = response_string.substr(i + 2, 4);
                unsigned int codepoint = std::stoul(hex, nullptr, 16);
                if (codepoint < 0x80) {
                    content += static_cast<char>(codepoint);
                } else if (codepoint < 0x800) {
                    content += static_cast<char>(0xC0 | (codepoint >> 6));
                    content += static_cast<char>(0x80 | (codepoint & 0x3F));
                } else {
                    content += static_cast<char>(0xE0 | (codepoint >> 12));
                    content += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                    content += static_cast<char>(0x80 | (codepoint & 0x3F));
                }
                i += 5;
            } else {
                content += response_string[i];
            }
        } else {
            content += response_string[i];
        }
    }

    // Update history
    g_conversation_history.push_back({user_message, content});
    if (g_conversation_history.size() > MAX_HISTORY) {
        g_conversation_history.erase(g_conversation_history.begin());
    }

    return content;
}

// ============================================================================
// Google Cloud Text-to-Speech (Burmese)
// ============================================================================

std::vector<uint8_t> googleTTS(const std::string& text) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return {};
    }

    // Escape text for JSON
    std::string escaped_text;
    for (char c : text) {
        if (c == '"') escaped_text += "\\\"";
        else if (c == '\\') escaped_text += "\\\\";
        else if (c == '\n') escaped_text += " ";
        else escaped_text += c;
    }

    // Request body for Burmese TTS
    std::ostringstream request_body;
    request_body << R"({
        "input": {
            "text": ")" << escaped_text << R"("
        },
        "voice": {
            "languageCode": "my-MM",
            "name": "my-MM-Standard-A",
            "ssmlGender": "FEMALE"
        },
        "audioConfig": {
            "audioEncoding": "LINEAR16",
            "sampleRateHertz": 16000
        }
    })";

    std::string url = std::string(GOOGLE_TTS_URL) + "?key=" + GOOGLE_API_KEY;
    std::string response_string;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.str().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "❌ TTS request failed" << std::endl;
        return {};
    }

    // Parse response - find audioContent
    std::string marker = R"("audioContent": ")";
    size_t pos = response_string.find(marker);
    if (pos == std::string::npos) {
        std::cerr << "❌ No audio content in response" << std::endl;
        return {};
    }

    pos += marker.length();
    size_t end_pos = response_string.find('"', pos);
    std::string audio_base64 = response_string.substr(pos, end_pos - pos);

    // Decode base64 to audio bytes
    std::vector<unsigned char> audio_data = base64_decode(audio_base64);
    
    // Convert to uint8_t vector
    std::vector<uint8_t> result(audio_data.begin(), audio_data.end());
    return result;
}

// ============================================================================
// Play Audio through Robot Speaker
// ============================================================================

void playAudioOnRobot(const std::vector<uint8_t>& audio_data) {
    if (audio_data.empty() || !g_audio_client) {
        return;
    }

    std::string stream_id = std::to_string(unitree::common::GetCurrentTimeMillisecond());
    
    const size_t CHUNK_SIZE = 96000;  // 3 seconds
    size_t offset = 0;
    
    while (offset < audio_data.size()) {
        size_t remaining = audio_data.size() - offset;
        size_t chunk_size = std::min(CHUNK_SIZE, remaining);
        
        std::vector<uint8_t> chunk(
            audio_data.begin() + offset,
            audio_data.begin() + offset + chunk_size
        );
        
        g_audio_client->PlayStream("burmese", stream_id, chunk);
        unitree::common::Sleep(1);
        offset += chunk_size;
    }
    
    g_audio_client->PlayStop(stream_id);
}

// ============================================================================
// Main Conversation Loop
// ============================================================================

void conversationLoop() {
    while (g_running) {
        // LED: Listening (Blue)
        g_audio_client->LedControl(0, 0, 255);
        
        // 1. Record audio from robot microphone
        std::vector<int16_t> audio = recordAudioFromRobot(RECORD_SECONDS);
        
        if (audio.empty() || !g_running) {
            continue;
        }
        
        // LED: Processing (Orange)
        g_audio_client->LedControl(255, 165, 0);
        
        // 2. Google STT - Convert speech to text (Burmese)
        std::cout << "🔄 Google STT processing..." << std::endl;
        std::string user_text = googleSTT(audio);
        
        if (user_text.empty()) {
            std::cout << "⚠️ စကားမကြားရပါ။ ထပ်ပြောပါ။" << std::endl;
            continue;
        }
        
        std::cout << "🎤 User: " << user_text << std::endl;
        
        // 3. OpenAI - Get AI response in Burmese
        std::cout << "🧠 OpenAI processing..." << std::endl;
        std::string ai_response = callOpenAI(user_text);
        
        std::cout << "🤖 Robot: " << ai_response << std::endl;
        
        // LED: Speaking (Green)
        g_audio_client->LedControl(0, 255, 0);
        
        // 4. Google TTS - Convert text to speech (Burmese)
        std::cout << "🔊 Google TTS processing..." << std::endl;
        std::vector<uint8_t> tts_audio = googleTTS(ai_response);
        
        // 5. Play audio through robot speaker
        if (!tts_audio.empty()) {
            playAudioOnRobot(tts_audio);
        } else {
            // Fallback to English TTS if Google TTS fails
            std::cout << "⚠️ Using fallback English TTS" << std::endl;
            g_audio_client->TtsMaker("Sorry, Burmese voice is not available.", 1);
            unitree::common::Sleep(3);
        }
        
        std::cout << std::endl;
    }
}

// ============================================================================
// Signal Handler
// ============================================================================

void signalHandler(int signum) {
    std::cout << "\n🛑 ပိတ်နေပါပြီ..." << std::endl;
    g_running = false;
}

// ============================================================================
// Main Function
// ============================================================================

int main(int argc, char const* argv[]) {
    std::cout << "╔════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║     Unitree G1 - မြန်မာဘာသာ AI Assistant                    ║" << std::endl;
    std::cout << "║     Burmese Conversational AI with Google Cloud            ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════╝" << std::endl;
    std::cout << std::endl;

    // Check arguments
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <NetworkInterface>" << std::endl;
        std::cout << "Example: " << argv[0] << " eth0" << std::endl;
        std::cout << std::endl;
        std::cout << "Required environment variables:" << std::endl;
        std::cout << "  export GOOGLE_API_KEY=\"your-google-api-key\"" << std::endl;
        std::cout << "  export OPENAI_API_KEY=\"your-openai-api-key\"" << std::endl;
        return 1;
    }

    // Get API keys from environment
    const char* google_key = std::getenv("GOOGLE_API_KEY");
    const char* openai_key = std::getenv("OPENAI_API_KEY");
    
    if (!google_key || !openai_key) {
        std::cerr << "❌ Error: API keys not set!" << std::endl;
        std::cerr << "Please set GOOGLE_API_KEY and OPENAI_API_KEY environment variables." << std::endl;
        return 1;
    }
    
    GOOGLE_API_KEY = google_key;
    OPENAI_API_KEY = openai_key;

    // Initialize CURL
    curl_global_init(CURL_GLOBAL_ALL);

    // Signal handler
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Initialize robot connection
    std::cout << "🔌 Connecting on interface: " << argv[1] << std::endl;
    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);

    // Initialize Audio Client
    unitree::robot::g1::AudioClient client;
    client.Init();
    client.SetTimeout(10.0f);
    client.SetVolume(100);
    g_audio_client = &client;

    std::cout << "✅ Robot connected!" << std::endl;

    // Greeting
    client.LedControl(0, 255, 0);
    std::cout << "🗣️ Greeting..." << std::endl;
    
    // Use English for greeting since Burmese TTS might not be immediately available
    client.TtsMaker("Hello! I am your Burmese AI assistant. Please speak to me in Burmese.", 1);
    unitree::common::Sleep(5);

    std::cout << std::endl;
    std::cout << "═══════════════════════════════════════════════════════════════" << std::endl;
    std::cout << "  🎯 မြန်မာလို စကားပြောနိုင်ပါပြီ!                              " << std::endl;
    std::cout << "  📌 ရပ်ရန် Ctrl+C နှိပ်ပါ။                                    " << std::endl;
    std::cout << "═══════════════════════════════════════════════════════════════" << std::endl;
    std::cout << std::endl;

    // Main conversation loop
    conversationLoop();

    // Cleanup
    client.LedControl(0, 0, 0);
    curl_global_cleanup();

    std::cout << "👋 နောက်မှတွေ့မယ်!" << std::endl;
    return 0;
}
