#ifndef LINUXALSAAUDIODRIVER_H
#define LINUXALSAAUDIODRIVER_H

#include "AudioDriver.h"

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>

namespace audio {

class LinuxAlsaAudioDriver : public AudioDriver
{
    Q_OBJECT

public:

    LinuxAlsaAudioDriver(controller::MainController *mainController) :
        AudioDriver(nullptr),
        handle(nullptr)
    {

    }

    bool start() override;
    void stop(bool) override;
    void release() override;

    QList<int> getValidSampleRates(int) const override;
    QList<int> getValidBufferSizes(int) const override;


    int getMaxInputs() const  override;
    int getMaxOutputs() const  override;

    QString getInputChannelName(const unsigned int) const override;
    QString getOutputChannelName(const unsigned int) const override;

    QString getAudioInputDeviceName(int index = CurrentAudioDeviceSelection) const override;
    QString getAudioOutputDeviceName(int index = CurrentAudioDeviceSelection) const override;
    QString getAudioDeviceInfo(int index, unsigned& nIn, unsigned& nOut) const override;

    int getAudioInputDeviceIndex() const override;
    void setAudioInputDeviceIndex(int) override;

    int getAudioOutputDeviceIndex() const override;
    void setAudioOutputDeviceIndex(int) override;

    int getDevicesCount() const override;

    bool canBeStarted() const override;

    bool hasControlPanel() const override;

    void openControlPanel(void *) override;

private:
    snd_pcm_t *handle;
};

} // namespace

#endif // LINUXALSAAUDIODRIVER_H
