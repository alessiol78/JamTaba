#ifndef LINUXALSAAUDIODRIVER_H
#define LINUXALSAAUDIODRIVER_H

#include "AudioDriver.h"

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>
#include <QThread>

namespace audio {

class AlsaWorker : public QObject
{
    Q_OBJECT

public:
    AlsaWorker(QObject *parent = 0) :
        QObject(parent),
        p_inpbuff(2),
        p_outbuff(2),
        p_run(false)
    { p_buffSize = 0; }
    bool init();
    int getBufferSize() { return p_buffSize; }

public slots:
    void doWork();
    void stop() { p_run = false; }

private:
    SamplesBuffer p_inpbuff;
    SamplesBuffer p_outbuff;

    snd_pcm_t *p_handle;
    int p_buffSize;
    bool p_run;
};

class LinuxAlsaAudioDriver : public AudioDriver
{
    Q_OBJECT
    QThread alsaThread;

public:

    LinuxAlsaAudioDriver(controller::MainController *mainController) :
        AudioDriver(nullptr),
        worker(nullptr)
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

signals:
      void startAlsaWorker();

private:
    AlsaWorker *worker;
};

} // namespace

#endif // LINUXALSAAUDIODRIVER_H
