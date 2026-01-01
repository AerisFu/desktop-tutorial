#include "PluginProcessor.h"

//==============================================================================
class FDNReverb
{
public:
    FDNReverb() = default;
    ~FDNReverb() = default;

    void prepare(double sampleRate, int numChannels)
    {
        this->sampleRate = sampleRate;
        numDelays = 16;  // Increased for better quality
        delayBuffers.resize(numDelays);
        writePos.assign(numDelays, 0);
        delayLengths.resize(numDelays);
        feedbackMatrix = juce::dsp::Matrix<float>(numDelays, numDelays);
        inputGains.assign(numDelays, 1.0f / std::sqrt(numDelays));
        outputGains.assign(numDelays, 1.0f / std::sqrt(numDelays));

        // Generate random orthogonal matrix for better diffusion
        juce::Random random;
        for (int i = 0; i < numDelays; ++i)
        {
            for (int j = 0; j < numDelays; ++j)
            {
                feedbackMatrix(i, j) = (random.nextFloat() - 0.5f) * 2.0f;
            }
        }
        // Orthogonalize using Gram-Schmidt (simplified)
        for (int i = 0; i < numDelays; ++i)
        {
            for (int j = 0; j < i; ++j)
            {
                float dot = 0.0f;
                for (int k = 0; k < numDelays; ++k)
                    dot += feedbackMatrix(k, i) * feedbackMatrix(k, j);
                for (int k = 0; k < numDelays; ++k)
                    feedbackMatrix(k, i) -= dot * feedbackMatrix(k, j);
            }
            float norm = 0.0f;
            for (int k = 0; k < numDelays; ++k)
                norm += feedbackMatrix(k, i) * feedbackMatrix(k, i);
            norm = std::sqrt(norm);
            for (int k = 0; k < numDelays; ++k)
                feedbackMatrix(k, i) /= norm;
        }

        dampingFilters.resize(numDelays);
        lfos.resize(numDelays);
        for (int i = 0; i < numDelays; ++i)
        {
            dampingFilters[i].reset();
            lfos[i].initialise([](float x) { return std::sin(x); }, 128);
            lfos[i].setFrequency(0.05f + random.nextFloat() * 0.1f);  // Slower modulation
        }

        // Early reflections
        earlyReflections.resize(8);
        for (int i = 0; i < 8; ++i)
        {
            earlyReflections[i].resize(1024, 0.0f);  // 23ms at 44.1kHz
            earlyWritePos[i] = 0;
            earlyDelays[i] = 100 + i * 50;  // Varying delays
            earlyGains[i] = 0.3f / (i + 1);  // Decreasing gains
        }

        updateParameters();
    }

    void process(float* left, float* right, int numSamples)
    {
        for (int s = 0; s < numSamples; ++s)
        {
            // Pre-delay
            float inL = left[s];
            float inR = right[s];
            int readPos = (preDelayWritePos - preDelayLength + (int)preDelayBufferL.size()) % preDelayBufferL.size();
            float delayedL = preDelayBufferL[readPos];
            float delayedR = preDelayBufferR[readPos];
            preDelayBufferL[preDelayWritePos] = inL;
            preDelayBufferR[preDelayWritePos] = inR;
            preDelayWritePos = (preDelayWritePos + 1) % preDelayBufferL.size();

            // Early reflections
            float earlyL = delayedL;
            float earlyR = delayedR;
            for (int i = 0; i < 8; ++i)
            {
                int erReadPos = (earlyWritePos[i] - earlyDelays[i] + (int)earlyReflections[i].size()) % earlyReflections[i].size();
                earlyL += earlyReflections[i][erReadPos] * earlyGains[i];
                earlyR += earlyReflections[i][erReadPos] * earlyGains[i];
                earlyReflections[i][earlyWritePos[i]] = delayedL * earlyGains[i];
                earlyWritePos[i] = (earlyWritePos[i] + 1) % earlyReflections[i].size();
            }

            // Inputs to FDN
            std::vector<float> inputs(numDelays, 0.0f);
            for (int i = 0; i < numDelays; ++i)
                inputs[i] = (earlyL + earlyR) * inputGains[i];

            std::vector<float> outputs(numDelays, 0.0f);
            for (int i = 0; i < numDelays; ++i)
            {
                // Read with modulation
                int pos = writePos[i];
                float mod = lfos[i].processSample(0.0f) * 0.01f * delayLengths[i];  // Increased modulation depth
                int modPos = (pos - (int)mod + delayLengths[i]) % delayLengths[i];
                float delayOut = delayBuffers[i][modPos];

                // Damping
                delayOut = dampingFilters[i].processSample(delayOut);

                // Feedback
                float fb = 0.0f;
                for (int j = 0; j < numDelays; ++j)
                    fb += feedbackMatrix(i, j) * delayOut;
                fb *= decay;

                // Write
                delayBuffers[i][pos] = inputs[i] + fb;
                writePos[i] = (pos + 1) % delayLengths[i];
                outputs[i] = delayOut * outputGains[i];
            }

            // Mix to stereo with enhanced spatialization
            float wetL = 0.0f, wetR = 0.0f;
            for (int i = 0; i < numDelays; ++i)
            {
                float pan = (float)i / (numDelays - 1) * 2.0f - 1.0f;  // -1 to 1
                float leftGain = (1.0f - pan) * 0.5f;
                float rightGain = (1.0f + pan) * 0.5f;
                wetL += outputs[i] * leftGain;
                wetR += outputs[i] * rightGain;
            }

            // Wet/dry
            left[s] = inL * (1.0f - wetDry) + wetL * wetDry;
            right[s] = inR * (1.0f - wetDry) + wetR * wetDry;
        }
    }

    void setParameters(float dec, float pd, float wd, float sz, float damp)
    {
        decay = dec;
        preDelay = pd;
        wetDry = wd;
        size = sz;
        damping = damp;
        updateParameters();
    }

private:
    double sampleRate = 44100.0;
    int numDelays = 16;
    std::vector<std::vector<float>> delayBuffers;
    std::vector<int> writePos;
    std::vector<int> delayLengths;
    juce::dsp::Matrix<float> feedbackMatrix;
    std::vector<float> inputGains, outputGains;
    std::vector<juce::dsp::IIR::Filter<float>> dampingFilters;
    std::vector<juce::dsp::Oscillator<float>> lfos;

    float decay = 0.5f;
    float preDelay = 0.01f;
    float wetDry = 0.5f;
    float size = 0.5f;
    float damping = 0.5f;

    std::vector<float> preDelayBufferL, preDelayBufferR;
    int preDelayWritePos = 0;
    int preDelayLength = 441;

    // Early reflections
    std::vector<std::vector<float>> earlyReflections;
    std::vector<int> earlyWritePos;
    std::vector<int> earlyDelays;
    std::vector<float> earlyGains;

    void updateParameters()
    {
        // Pre-delay
        preDelayLength = std::max(1, (int)(preDelay * sampleRate));
        if ((int)preDelayBufferL.size() != preDelayLength)
        {
            preDelayBufferL.assign(preDelayLength, 0.0f);
            preDelayBufferR.assign(preDelayLength, 0.0f);
            preDelayWritePos = 0;
        }

        // Damping
        float cutoff = 2000.0f + damping * 8000.0f;
        for (auto& filter : dampingFilters)
            *filter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, cutoff, 0.707f);

        // Delay lengths
        float baseDelays[16] = {29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97};
        for (int i = 0; i < numDelays; ++i)
        {
            int newLen = std::max(1, (int)(baseDelays[i] * (0.5f + size * 2.0f) * sampleRate / 44100.0));
            if (newLen != delayLengths[i])
            {
                delayLengths[i] = newLen;
                delayBuffers[i].assign(newLen, 0.0f);
                writePos[i] = 0;
            }
        }
    }
};

//==============================================================================
PluginProcessor::PluginProcessor()
    : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     ),
      parameters(*this, nullptr, "parameters", createParameterLayout()),
      reverb(std::make_unique<FDNReverb>())
{
}

PluginProcessor::~PluginProcessor()
{
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
    return false;
}

bool PluginProcessor::producesMidi() const
{
    return false;
}

bool PluginProcessor::isMidiEffect() const
{
    return false;
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 10.0; // Long tail for reverb
}

int PluginProcessor::getNumPrograms()
{
    return 1;
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
}

const juce::String PluginProcessor::getProgramName (int index)
{
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    reverb->prepare(sampleRate, getTotalNumOutputChannels());
}

void PluginProcessor::releaseResources()
{
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDeniesAudioChange noDenies(*this);

    auto dec = parameters.getRawParameterValue("decay")->load();
    auto pd = parameters.getRawParameterValue("preDelay")->load();
    auto wd = parameters.getRawParameterValue("wetDry")->load();
    auto sz = parameters.getRawParameterValue("size")->load();
    auto damp = parameters.getRawParameterValue("damping")->load();

    reverb->setParameters(dec, pd, wd, sz, damp);

    if (buffer.getNumChannels() >= 2)
        reverb->process(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>("decay", "Decay", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f, "Decay"));
    layout.add(std::make_unique<juce::AudioParameterFloat>("preDelay", "Pre-Delay", juce::NormalisableRange<float>(0.0f, 0.1f, 0.001f), 0.01f, "Pre-Delay"));
    layout.add(std::make_unique<juce::AudioParameterFloat>("wetDry", "Wet/Dry", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f, "Wet/Dry"));
    layout.add(std::make_unique<juce::AudioParameterFloat>("size", "Size", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f, "Size"));
    layout.add(std::make_unique<juce::AudioParameterFloat>("damping", "Damping", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f, "Damping"));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}