# Unitree G1 EDU RL Framework အသုံးပြုနည်း 🤖

Unitree သည် **unitree_rl_gym** ဆိုသော official RL framework ကို ထုတ်ပေးထားပါသည်။ G1, H1, Go2 စသည့် robots များအတွက် RL training နှင့် deployment ကို support လုပ်ပါသည်။

---

## 🔁 Workflow Overview

```
┌─────────┐    ┌─────────┐    ┌───────────┐    ┌───────────┐
│  Train  │ →  │  Play   │ →  │  Sim2Sim  │ →  │ Sim2Real  │
│ (Isaac) │    │ (Isaac) │    │ (MuJoCo)  │    │ (Robot)   │
└─────────┘    └─────────┘    └───────────┘    └───────────┘
```

1. **Train**: Isaac Gym တွင် RL policy train
2. **Play**: Train ထားသော policy ကို verify
3. **Sim2Sim**: MuJoCo simulator တွင် test
4. **Sim2Real**: တကယ့် robot ပေါ်တွင် deploy

---

## 📦 Installation

### 1. Prerequisites

```bash
# NVIDIA GPU with CUDA support required
# Ubuntu 20.04 / 22.04 recommended

# Install Conda (if not installed)
wget https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
bash Miniconda3-latest-Linux-x86_64.sh
```

### 2. Clone Repository

```bash
git clone https://github.com/unitreerobotics/unitree_rl_gym.git
cd unitree_rl_gym
```

### 3. Create Environment

```bash
conda create -n unitree_rl python=3.8
conda activate unitree_rl
```

### 4. Install Isaac Gym

```bash
# Download Isaac Gym from NVIDIA
# https://developer.nvidia.com/isaac-gym
cd isaacgym/python
pip install -e .
```

### 5. Install Dependencies

```bash
cd unitree_rl_gym
pip install -e .

# Additional dependencies
pip install tensorboard
pip install mujoco
```

---

## 🛠️ Training (G1 Robot)

### Basic Training

```bash
python legged_gym/scripts/train.py --task=g1
```

### Training Parameters

| Parameter | Description | Example |
|-----------|-------------|---------|
| `--task` | Robot type | `g1`, `h1`, `go2` |
| `--headless` | No GUI (faster) | `--headless` |
| `--num_envs` | Parallel environments | `--num_envs=4096` |
| `--max_iterations` | Max training iterations | `--max_iterations=5000` |
| `--resume` | Resume from checkpoint | `--resume` |
| `--experiment_name` | Experiment name | `--experiment_name=g1_walk` |

### Example: Train G1 Walking

```bash
python legged_gym/scripts/train.py \
    --task=g1 \
    --headless \
    --num_envs=4096 \
    --max_iterations=3000 \
    --experiment_name=g1_custom_walk
```

**Training Output Directory:**
```
logs/<experiment_name>/<date_time>_<run_name>/model_<iteration>.pt
```

---

## 🎮 Play (Verify Training)

```bash
python legged_gym/scripts/play.py --task=g1
```

### Load Specific Checkpoint

```bash
python legged_gym/scripts/play.py \
    --task=g1 \
    --load_run=<run_name> \
    --checkpoint=<iteration>
```

**Exported Policy Location:**
```
logs/{experiment_name}/exported/policies/policy_lstm_1.pt
```

---

## 🔄 Sim2Sim (MuJoCo Testing)

MuJoCo simulator တွင် policy ကို test လုပ်ခြင်း:

```bash
python deploy/deploy_mujoco/deploy_mujoco.py g1.yaml
```

### Configuration File

`deploy/deploy_mujoco/configs/g1.yaml`:

```yaml
robot: g1
policy_path: "../../pre_train/g1/motion.pt"  # Pre-trained model
# Or use custom trained model:
# policy_path: "../../logs/g1/exported/policies/policy_lstm_1.pt"
```

---

## 🤖 Sim2Real (Physical Deployment)

### Prerequisites

1. G1 robot ကို **Debug Mode** သို့ ပြောင်းရန်
2. PC နှင့် Robot ကို Ethernet ဖြင့် ချိတ်ဆက်ရန်

### Python Deployment

```bash
python deploy/deploy_real/deploy_real.py eth0 g1.yaml
```

### C++ Deployment

```bash
cd deploy/deploy_real/cpp_g1

# Install LibTorch
wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-2.7.1%2Bcpu.zip
unzip libtorch-cxx11-abi-shared-with-deps-2.7.1+cpu.zip

# Build
mkdir build && cd build
cmake ..
make -j4

# Run
./g1_deploy_run eth0
```

---

## 📊 Project Structure

```
unitree_rl_gym/
├── legged_gym/
│   ├── envs/              # Environment definitions
│   │   └── g1/            # G1 specific config
│   ├── scripts/
│   │   ├── train.py       # Training script
│   │   └── play.py        # Visualization script
│   └── resources/
│       └── robots/        # Robot URDF/MJCF files
├── deploy/
│   ├── deploy_mujoco/     # MuJoCo sim2sim
│   ├── deploy_real/       # Physical deployment
│   └── pre_train/         # Pre-trained models
│       └── g1/
│           └── motion.pt  # G1 pre-trained policy
└── logs/                  # Training logs & checkpoints
```

---

## 🎯 Reward Function Customization

`legged_gym/envs/g1/g1_config.py`:

```python
class rewards:
    # Reward scales
    tracking_lin_vel = 1.0      # Velocity tracking
    tracking_ang_vel = 0.5      # Angular velocity tracking
    lin_vel_z = -2.0            # Penalize vertical velocity
    ang_vel_xy = -0.05          # Penalize roll/pitch rate
    orientation = -1.0          # Penalize non-upright orientation
    base_height = -1.0          # Penalize wrong height
    torques = -0.0001           # Penalize high torques
    dof_acc = -2.5e-7           # Penalize joint acceleration
    action_rate = -0.01         # Penalize action changes
    feet_air_time = 1.0         # Reward swing time
    collision = -1.0            # Penalize collisions
    stumble = -0.5              # Penalize stumbling
```

---

## 🔧 Domain Randomization

`legged_gym/envs/g1/g1_config.py`:

```python
class domain_rand:
    randomize_friction = True
    friction_range = [0.5, 1.25]
    
    randomize_base_mass = True
    added_mass_range = [-1.0, 1.0]
    
    push_robots = True
    push_interval_s = 15
    max_push_vel_xy = 1.0
```

---

## 📌 MuJoCo Simulator (unitree_mujoco)

Sim2Sim အတွက် MuJoCo simulator:

```bash
# Clone
git clone https://github.com/unitreerobotics/unitree_mujoco.git
cd unitree_mujoco

# Install MuJoCo
cd simulate
ln -s ~/.mujoco/mujoco-3.x.x mujoco

# Build
mkdir build && cd build
cmake ..
make -j4

# Run G1 simulation
./unitree_mujoco -r g1 -s scene.xml
```

---

## ⚠️ Important Notes

1. **GPU Required**: Training အတွက် NVIDIA GPU လိုအပ်ပါသည်
2. **Debug Mode**: Sim2Real မလုပ်ခင် robot ကို debug mode သို့ ပြောင်းရန်
3. **Pre-trained Models**: `deploy/pre_train/g1/motion.pt` တွင် ရှိသော pre-trained model ကို စတင် test လုပ်နိုင်ပါသည်
4. **Safety**: Physical deployment မလုပ်ခင် MuJoCo တွင် thoroughly test လုပ်ပါ

---

## 🔗 Related Repositories

| Repository | Description |
|------------|-------------|
| [unitree_rl_gym](https://github.com/unitreerobotics/unitree_rl_gym) | RL Training Framework |
| [unitree_mujoco](https://github.com/unitreerobotics/unitree_mujoco) | MuJoCo Simulator |
| [unitree_sdk2](https://github.com/unitreerobotics/unitree_sdk2) | Robot SDK |
| [unitree_sdk2_python](https://github.com/unitreerobotics/unitree_sdk2_python) | Python SDK |

---

## 📊 Quick Summary

| Step | Command | Output |
|------|---------|--------|
| Train | `train.py --task=g1` | `model_xxx.pt` |
| Play | `play.py --task=g1` | `policy_lstm_1.pt` |
| Sim2Sim | `deploy_mujoco.py g1.yaml` | MuJoCo test |
| Sim2Real | `deploy_real.py eth0 g1.yaml` | Robot walking |

---

## 🚀 Quick Start (Pre-trained Model)

Pre-trained model ဖြင့် စမ်းကြည့်ရန်:

```bash
# 1. MuJoCo တွင် test
python deploy/deploy_mujoco/deploy_mujoco.py g1.yaml

# 2. Real robot တွင် deploy (Debug mode ဖွင့်ထားရန်)
python deploy/deploy_real/deploy_real.py eth0 g1.yaml
```

---

## 📚 References

- [Unitree RL Gym Documentation](https://github.com/unitreerobotics/unitree_rl_gym)
- [Isaac Gym Documentation](https://developer.nvidia.com/isaac-gym)
- [MuJoCo Documentation](https://mujoco.readthedocs.io/)
- [Unitree Support](https://support.unitree.com/)

