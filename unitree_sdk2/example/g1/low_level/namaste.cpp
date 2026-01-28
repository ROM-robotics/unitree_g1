/**
 * @file namaste.cpp
 * @brief G1 Robot Namaste/Prayer Pose Example (လက်အုပ်ချီ ရှိခိုးပုံစံ)
 * 
 * This example demonstrates how to control both arms to perform
 * a Namaste (prayer) greeting pose using low-level arm SDK control.
 */

#include <array>
#include <chrono>
#include <iostream>
#include <thread>

#include <unitree/idl/hg/LowCmd_.hpp>
#include <unitree/idl/hg/LowState_.hpp>
#include <unitree/robot/channel/channel_publisher.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>

static const std::string kTopicArmSDK = "rt/arm_sdk";
static const std::string kTopicState = "rt/lowstate";

constexpr float kPi = 3.141592654f;
constexpr int G1_NUM_MOTOR = 29;

// Joint Indices for G1 29-DOF model
enum JointIndex {
    // Left leg
    kLeftHipPitch = 0,
    kLeftHipRoll = 1,
    kLeftHipYaw = 2,
    kLeftKnee = 3,
    kLeftAnkle = 4,
    kLeftAnkleRoll = 5,

    // Right leg
    kRightHipPitch = 6,
    kRightHipRoll = 7,
    kRightHipYaw = 8,
    kRightKnee = 9,
    kRightAnkle = 10,
    kRightAnkleRoll = 11,

    // Waist
    kWaistYaw = 12,
    kWaistRoll = 13,
    kWaistPitch = 14,

    // Left arm
    kLeftShoulderPitch = 15,
    kLeftShoulderRoll = 16,
    kLeftShoulderYaw = 17,
    kLeftElbow = 18,
    kLeftWristRoll = 19,
    kLeftWristPitch = 20,
    kLeftWristYaw = 21,

    // Right arm
    kRightShoulderPitch = 22,
    kRightShoulderRoll = 23,
    kRightShoulderYaw = 24,
    kRightElbow = 25,
    kRightWristRoll = 26,
    kRightWristPitch = 27,
    kRightWristYaw = 28,

    // Not used (for weight control)
    kNotUsedJoint = 29
};

// Namaste pose joint angles (radians)
struct NamastePose {
    // Left arm - ရှေ့ကို ကွေး၊ အတွင်းသို့ ပိတ်
    float left_shoulder_pitch  = 0.8f;    // ရှေ့သို့ ကြွ (~45°)
    float left_shoulder_roll   = -0.3f;   // အတွင်းသို့
    float left_shoulder_yaw    = 0.4f;    // လှည့်
    float left_elbow           = 1.2f;    // ကွေး (~70°)
    float left_wrist_roll      = 0.0f;
    float left_wrist_pitch     = 0.3f;    // လက်ဖဝါး အတွင်းသို့
    float left_wrist_yaw       = 0.0f;
    
    // Right arm - mirror of left arm
    float right_shoulder_pitch = 0.8f;    // ရှေ့သို့ ကြွ
    float right_shoulder_roll  = 0.3f;    // အတွင်းသို့ (opposite sign)
    float right_shoulder_yaw   = -0.4f;   // လှည့် (opposite sign)
    float right_elbow          = 1.2f;    // ကွေး
    float right_wrist_roll     = 0.0f;
    float right_wrist_pitch    = 0.3f;    // လက်ဖဝါး အတွင်းသို့
    float right_wrist_yaw      = 0.0f;
};

// Arm joints to control
std::array<JointIndex, 14> arm_joints = {
    kLeftShoulderPitch, kLeftShoulderRoll, kLeftShoulderYaw, kLeftElbow,
    kLeftWristRoll, kLeftWristPitch, kLeftWristYaw,
    kRightShoulderPitch, kRightShoulderRoll, kRightShoulderYaw, kRightElbow,
    kRightWristRoll, kRightWristPitch, kRightWristYaw
};

// Current motor state
std::array<float, G1_NUM_MOTOR> current_pos{};

void LowStateHandler(const void *message) {
    auto low_state = (const unitree_hg::msg::dds_::LowState_ *)message;
    for (int i = 0; i < G1_NUM_MOTOR; ++i) {
        current_pos[i] = low_state->motor_state()[i].q();
    }
}

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <network_interface>" << std::endl;
        std::cout << "Example: " << argv[0] << " eth0" << std::endl;
        return -1;
    }

    std::cout << " --- Unitree G1 Namaste Pose Example --- " << std::endl;
    std::cout << "        လက်အုပ်ချီ ရှိခိုးပုံစံ 🙏        " << std::endl << std::endl;

    // Initialize DDS
    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);

    // Publisher for arm commands
    unitree::robot::ChannelPublisherPtr<unitree_hg::msg::dds_::LowCmd_> publisher;
    publisher.reset(new unitree::robot::ChannelPublisher<unitree_hg::msg::dds_::LowCmd_>(kTopicArmSDK));
    publisher->InitChannel();

    // Subscriber for current state
    unitree::robot::ChannelSubscriberPtr<unitree_hg::msg::dds_::LowState_> subscriber;
    subscriber.reset(new unitree::robot::ChannelSubscriber<unitree_hg::msg::dds_::LowState_>(kTopicState));
    subscriber->InitChannel(LowStateHandler, 1);

    // Wait for state data
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    unitree_hg::msg::dds_::LowCmd_ msg;
    NamastePose namaste;

    // Control parameters
    float kp = 60.0f;           // Stiffness
    float kd = 1.5f;            // Damping
    float control_dt = 0.02f;   // 20ms control loop
    float transition_time = 3.0f;  // 3 seconds to reach pose
    int steps = static_cast<int>(transition_time / control_dt);

    auto sleep_time = std::chrono::milliseconds(static_cast<int>(control_dt * 1000));

    // Get Namaste target positions as array
    std::array<float, 14> namaste_pos = {
        namaste.left_shoulder_pitch, namaste.left_shoulder_roll,
        namaste.left_shoulder_yaw, namaste.left_elbow,
        namaste.left_wrist_roll, namaste.left_wrist_pitch, namaste.left_wrist_yaw,
        namaste.right_shoulder_pitch, namaste.right_shoulder_roll,
        namaste.right_shoulder_yaw, namaste.right_elbow,
        namaste.right_wrist_roll, namaste.right_wrist_pitch, namaste.right_wrist_yaw
    };

    // Store initial positions
    std::array<float, 14> init_pos{};
    for (size_t i = 0; i < arm_joints.size(); ++i) {
        init_pos[i] = current_pos[arm_joints[i]];
    }

    std::cout << "Current arm positions recorded." << std::endl;
    std::cout << "Press ENTER to perform Namaste pose... 🙏" << std::endl;
    std::cin.get();

    // Enable arm SDK control (weight = 1.0)
    msg.motor_cmd().at(kNotUsedJoint).q(1.0f);

    std::cout << "Moving to Namaste pose..." << std::endl;

    // ===== Phase 1: Move to Namaste pose =====
    for (int i = 0; i <= steps; ++i) {
        float ratio = static_cast<float>(i) / steps;
        
        // Smooth interpolation using cubic easing
        float smooth_ratio = ratio * ratio * (3.0f - 2.0f * ratio);

        for (size_t j = 0; j < arm_joints.size(); ++j) {
            float target = init_pos[j] + (namaste_pos[j] - init_pos[j]) * smooth_ratio;
            
            msg.motor_cmd().at(arm_joints[j]).q(target);
            msg.motor_cmd().at(arm_joints[j]).dq(0.0f);
            msg.motor_cmd().at(arm_joints[j]).kp(kp);
            msg.motor_cmd().at(arm_joints[j]).kd(kd);
            msg.motor_cmd().at(arm_joints[j]).tau(0.0f);
        }

        publisher->Write(msg);
        std::this_thread::sleep_for(sleep_time);
    }

    std::cout << "Namaste pose complete! 🙏" << std::endl;
    std::cout << std::endl;
    std::cout << "Press ENTER to return to initial position..." << std::endl;
    std::cin.get();

    std::cout << "Returning to initial position..." << std::endl;

    // ===== Phase 2: Return to initial position =====
    for (int i = 0; i <= steps; ++i) {
        float ratio = static_cast<float>(i) / steps;
        float smooth_ratio = ratio * ratio * (3.0f - 2.0f * ratio);

        for (size_t j = 0; j < arm_joints.size(); ++j) {
            float target = namaste_pos[j] + (init_pos[j] - namaste_pos[j]) * smooth_ratio;
            
            msg.motor_cmd().at(arm_joints[j]).q(target);
            msg.motor_cmd().at(arm_joints[j]).dq(0.0f);
            msg.motor_cmd().at(arm_joints[j]).kp(kp);
            msg.motor_cmd().at(arm_joints[j]).kd(kd);
            msg.motor_cmd().at(arm_joints[j]).tau(0.0f);
        }

        publisher->Write(msg);
        std::this_thread::sleep_for(sleep_time);
    }

    std::cout << "Done! Returned to initial position." << std::endl;

    return 0;
}
