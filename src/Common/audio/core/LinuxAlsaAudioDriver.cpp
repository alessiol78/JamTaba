#include "LinuxAlsaAudioDriver.h"
#include "log/Logging.h"

namespace audio
{

bool AlsaWorker::init()
{
    int rc;
    snd_pcm_hw_params_t *params;
    unsigned int val;
    int dir;
    snd_pcm_uframes_t frames;

    /* Open PCM device for recording (capture). */
    rc = snd_pcm_open(&p_handle, "default",
                      SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        qCCritical(jtAudio) <<
                               "unable to open pcm device: " << snd_strerror(rc) << endl;
        return false;
    }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&params);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(p_handle, params);

    /* Set the desired hardware parameters. */

    /* Interleaved mode */
    snd_pcm_hw_params_set_access(p_handle, params,
                                 SND_PCM_ACCESS_RW_INTERLEAVED);

    /* Signed 16-bit little-endian format */
    snd_pcm_hw_params_set_format(p_handle, params,
                                 SND_PCM_FORMAT_S16_LE);

    /* Two channels (stereo) */
    snd_pcm_hw_params_set_channels(p_handle, params, 2);

    /* 44100 bits/second sampling rate (CD quality) */
    val = 44100;
    snd_pcm_hw_params_set_rate_near(p_handle, params,
                                    &val, &dir);

    /* Set period size to 32 frames. */
    frames = 32;
    snd_pcm_hw_params_set_period_size_near(p_handle,
                                           params, &frames, &dir);

    /* Write the parameters to the driver */
    rc = snd_pcm_hw_params(p_handle, params);
    if (rc < 0) {
        qCCritical(jtAudio) <<
                               "unable to set hw paameters: " << snd_strerror(rc) << endl;
        return false;
    }

    qCDebug(jtAudio) << "PCM state = " <<
           snd_pcm_state_name(snd_pcm_state(p_handle)) << endl;

    rc = snd_pcm_hw_params_get_buffer_size(params, (snd_pcm_uframes_t *)&val);
    if (rc < 0) {
        qCCritical(jtAudio) << "unable to get buffer size: " << snd_strerror(rc) << endl;
        return false;
    }
    p_buffSize = val;
    qCDebug(jtAudio) << "buffer size =" << p_buffSize << "frames";

    p_run = true;
    return true;
}

void AlsaWorker::doWork()
{
    if(!p_handle) return;
    int rc;
    uint frames;
    qDebug() << "alsa thread started";
    while(p_run) {
        frames = p_outbuff.getFrameLenght();
        if(!frames) { continue; }
        rc = snd_pcm_writei(p_handle, p_outbuff.getInterleavedSamples(), frames);
        if (rc == -EPIPE) {
          /* EPIPE means underrun */
          fprintf(stderr, "underrun occurred\n");
          snd_pcm_prepare(p_handle);
        } else if (rc < 0) {
          fprintf(stderr,
                  "error from writei: %s\n",
                  snd_strerror(rc));
        }  else if (rc != (int)frames) {
          fprintf(stderr,
                  "short write, write %d frames\n", rc);
        }
    }
    qDebug() << "alsa thread terminated";

    //snd_pcm_drain(handle);
    snd_pcm_close(p_handle);
    p_handle = nullptr;
}

bool LinuxAlsaAudioDriver::start()
{
    if(!worker)
    {
        worker = new AlsaWorker();
        if(!worker->init()) return false;
        worker->moveToThread(&alsaThread);
    }
    connect(this, &LinuxAlsaAudioDriver::startAlsaWorker, worker, &AlsaWorker::doWork, Qt::QueuedConnection);
    alsaThread.start();
    emit startAlsaWorker();
    return true;
}

void LinuxAlsaAudioDriver::stop(bool)
{
    if(alsaThread.isRunning()) {
        worker->stop();
        alsaThread.quit();
        alsaThread.wait();
    }
}

void LinuxAlsaAudioDriver::release()
{
    qCDebug(jtAudio) << "releasing alsa resources...";
    stop(false);
    qCDebug(jtAudio) << "alsa terminated!";
}

QList<int> LinuxAlsaAudioDriver::getValidBufferSizes(int) const
{
    if (!worker) {
        return  QList<int>();
    }
    return QList<int>() << worker->getBufferSize();
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
