#include "LinuxAlsaAudioDriver.h"
#include "log/Logging.h"

namespace audio
{

bool LinuxAlsaAudioDriver::start()
{
    int rc;
    snd_pcm_hw_params_t *params;
    unsigned int val;
    int dir;
    snd_pcm_uframes_t frames;

    /* Open PCM device for recording (capture). */
    rc = snd_pcm_open(&handle, "default",
                      SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        qCCritical(jtAudio) <<
                               "unable to open pcm device: " << snd_strerror(rc) << endl;
        return false;
    }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&params);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(handle, params);

    /* Set the desired hardware parameters. */

    /* Interleaved mode */
    snd_pcm_hw_params_set_access(handle, params,
                                 SND_PCM_ACCESS_RW_INTERLEAVED);

    /* Signed 16-bit little-endian format */
    snd_pcm_hw_params_set_format(handle, params,
                                 SND_PCM_FORMAT_S16_LE);

    /* Two channels (stereo) */
    snd_pcm_hw_params_set_channels(handle, params, 2);

    /* 44100 bits/second sampling rate (CD quality) */
    val = 44100;
    snd_pcm_hw_params_set_rate_near(handle, params,
                                    &val, &dir);

    /* Set period size to 32 frames. */
    frames = 32;
    snd_pcm_hw_params_set_period_size_near(handle,
                                           params, &frames, &dir);

    /* Write the parameters to the driver */
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        qCCritical(jtAudio) <<
                               "unable to set hw paameters: " << snd_strerror(rc) << endl;
        return false;
    }

    qCDebug(jtAudio) << "PCM state = " <<
           snd_pcm_state_name(snd_pcm_state(handle)) << endl;

    return true;
}

void LinuxAlsaAudioDriver::stop(bool)
{
    if(!handle) return;
    //snd_pcm_drain(handle);
    snd_pcm_close(handle);
    handle = nullptr;
}

void LinuxAlsaAudioDriver::release()
{
    qCDebug(jtAudio) << "releasing alsa resources...";
    stop(false);
    qCDebug(jtAudio) << "alsa terminated!";
}

QList<int> LinuxAlsaAudioDriver::getValidBufferSizes(int) const
{
    snd_pcm_t *handle = this->handle;
    snd_pcm_hw_params_t *params;
    unsigned int val;

    if (!handle) {
        int rc = snd_pcm_open(&handle, "default",
                          SND_PCM_STREAM_PLAYBACK, 0);
        if (rc < 0) {
          qWarning() <<
                  "unable to open pcm device: " <<
                  snd_strerror(rc)<< endl;
          return QList<int>();
        }
        /* Allocate a hardware parameters object. */
        snd_pcm_hw_params_alloca(&params);

        /* Fill it in with default values. */
        snd_pcm_hw_params_any(handle, params);

        /* Set the desired hardware parameters. */

        /* Interleaved mode */
        snd_pcm_hw_params_set_access(handle, params,
                            SND_PCM_ACCESS_RW_INTERLEAVED);

        /* Signed 16-bit little-endian format */
        snd_pcm_hw_params_set_format(handle, params,
                                    SND_PCM_FORMAT_S16_LE);

        /* Two channels (stereo) */
        snd_pcm_hw_params_set_channels(handle, params, 2);

        /* 44100 bits/second sampling rate (CD quality) */
        val = 44100;
        int dir;
        snd_pcm_hw_params_set_rate_near(handle,
                                       params, &val, &dir);

        /* Write the parameters to the driver */
        rc = snd_pcm_hw_params(handle, params);
        if (rc < 0) {
           qWarning() <<
                  "unable to set hw parameters: "<<
                  snd_strerror(rc)<<endl;
          return QList<int>();
        }
    }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&params);

    int rc = snd_pcm_hw_params_current(handle, params);
    if (rc < 0) {
        qWarning("snd_pcm_hw_params_current: %s", snd_strerror(rc));
        qWarning() <<
                              "unable to get hw parameters: " << snd_strerror(rc) << endl;
        return QList<int>();
    }
    snd_pcm_hw_params_get_buffer_size(params,
                                      (snd_pcm_uframes_t *) &val);
    printf("buffer size = %d frames\n", val);
    return QList<int>() << val;
}

QList<int> LinuxAlsaAudioDriver::getValidSampleRates(int) const
{
    return QList<int>() << 44100;
}

int LinuxAlsaAudioDriver::getMaxInputs() const
{
    return 1;
}

int LinuxAlsaAudioDriver::getMaxOutputs() const
{
    return 2;
}

QString LinuxAlsaAudioDriver::getInputChannelName(const unsigned int) const
{
    return "in_chan";
}

QString LinuxAlsaAudioDriver::getOutputChannelName(const unsigned int) const
{
    return "out_chan";
}

QString LinuxAlsaAudioDriver::getAudioInputDeviceName(int index) const
{
    return index == CurrentAudioDeviceSelection ? getAudioInputDeviceName(0) : "AlsaInputDevice";
}

QString LinuxAlsaAudioDriver::getAudioOutputDeviceName(int index) const
{
    return index == CurrentAudioDeviceSelection ? getAudioOutputDeviceName(0) : "AlsaOutputDevice";
}

QString LinuxAlsaAudioDriver::getAudioDeviceInfo(int index,unsigned& nIn, unsigned& nOut ) const
{
    nIn = nOut = 1;
    return index==0 ? "AlsaInputDevice" : "AlsaOutputDevice";
}

int LinuxAlsaAudioDriver::getAudioInputDeviceIndex() const
{
    return 0;
}

void LinuxAlsaAudioDriver::setAudioInputDeviceIndex(int)
{
    //
}

int LinuxAlsaAudioDriver::getAudioOutputDeviceIndex() const
{
    return 1;
}

void LinuxAlsaAudioDriver::setAudioOutputDeviceIndex(int)
{
    //
}

int LinuxAlsaAudioDriver::getDevicesCount() const
{
    return 2;
}

bool LinuxAlsaAudioDriver::canBeStarted() const
{
    return true;
}

bool LinuxAlsaAudioDriver::hasControlPanel() const
{
    return false;
}

void LinuxAlsaAudioDriver::openControlPanel(void *)
{
    //
}

} // namespace
