# Ultimate Reverb VST3 Plugin

A state-of-the-art reverb plugin built with JUCE, featuring an advanced Feedback Delay Network (FDN) algorithm with modulation and damping for superior sound quality.

## Features

- **Advanced FDN Algorithm**: 8-delay FDN with Hadamard feedback matrix for rich, dense reverb.
- **Modulation**: Subtle LFO modulation on delays for added diffusion and realism.
- **Damping Control**: Adjustable high-frequency damping for natural decay.
- **Parameters**:
  - Decay: Controls reverb tail length (0-1)
  - Pre-Delay: Initial delay before reverb onset (0-100ms)
  - Wet/Dry: Mix between dry and wet signals (0-1)
  - Size: Room size affecting delay lengths (0-1)
  - Damping: High-frequency decay control (0-1)

## Building on macOS

### Prerequisites

1. Install JUCE: Download from https://juce.com/ and follow installation instructions, or use Homebrew:
   ```
   brew install juce
   ```

2. Install CMake:
   ```
   brew install cmake
   ```

3. Ensure Xcode is installed with command line tools:
   ```
   xcode-select --install
   ```

### Build Steps

1. Clone or navigate to the project directory:
   ```
   cd /Users/fudegeng/Programe\ of\ AI\ -\ Ryan\ Fu/VST3ReverbPlugin
   ```

2. Create a build directory:
   ```
   mkdir build
   cd build
   ```

3. Configure with CMake:
   ```
   cmake .. -DCMAKE_BUILD_TYPE=Release
   ```

4. Build the plugin:
   ```
   make -j$(sysctl -n hw.ncpu)
   ```

5. The VST3 plugin will be built in `build/VST3/`.

### Installation

Copy the `.vst3` bundle to your VST3 plugins directory:
- System: `/Library/Audio/Plug-Ins/VST3/`
- User: `~/Library/Audio/Plug-Ins/VST3/`

### Usage

Load the plugin in your DAW (e.g., Logic Pro, Ableton Live) as a VST3 effect. Adjust parameters for desired reverb characteristics.

## Algorithm Details

This plugin uses a Feedback Delay Network (FDN) with:
- 8 parallel delays with prime-based lengths for minimal coloration.
- Hadamard matrix for orthogonal feedback, ensuring even distribution.
- Per-delay LFO modulation for enhanced diffusion.
- IIR low-pass filters for frequency-dependent damping.
- Pre-delay buffer for early reflections simulation.

The implementation draws inspiration from high-end reverbs like Valhalla Room and iZotope Ozone, with optimizations for low CPU usage and high audio quality.