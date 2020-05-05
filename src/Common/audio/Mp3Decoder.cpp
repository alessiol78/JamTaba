#include "Mp3Decoder.h"
#include "core/AudioDriver.h"
#include "core/SamplesBuffer.h"

#include <QDebug>
#include <cmath>
#include <climits>

using audio::Mp3DecoderMiniMp3;
using audio::SamplesBuffer;

const int Mp3DecoderMiniMp3::MINIMUM_SIZE_TO_DECODE = 1024 + 256;
const int Mp3DecoderMiniMp3::AUDIO_SAMPLES_BUFFER_MAX_SIZE = 4096 * 2;

Mp3DecoderMiniMp3::Mp3DecoderMiniMp3() :
    mp3Decoder(nullptr),
    buffer(SamplesBuffer(2, INTERNAL_SHORT_BUFFER_SIZE))
{
#ifdef __LINUX_LAME__
    mp3Decoder = hip_decode_init();
#else
    mp3Decoder = mp3_create();
#endif
    reset();
}

Mp3DecoderMiniMp3::~Mp3DecoderMiniMp3()
{
    // leaking here to avoid a crash, minimp3 has a litle problem in mp3_done described in author blog comments: http://keyj.emphy.de/minimp3/
#ifdef __LINUX_LAME__
    if(mp3Decoder) hip_decode_exit(mp3Decoder);
#else
    //if (mp3Decoder)
        //mp3_done(&mp3Decoder);
#endif
}

void Mp3DecoderMiniMp3::reset()
{
    array.clear();

#ifdef __LINUX_LAME__
    memset(internalShortBuffer_l,0,sizeof(internalShortBuffer_l));
    memset(internalShortBuffer_r,0,sizeof(internalShortBuffer_r));
#else
    for (int i = 0; i < INTERNAL_SHORT_BUFFER_SIZE; ++i)
    {
        internalShortBuffer[i] = 0;
    }
#endif
    buffer.zero();
}

int Mp3DecoderMiniMp3::getSampleRate() const
{
#ifdef __LINUX_LAME__
    if (mp3Info.samplerate <= 0)
        return 44100;

    return  mp3Info.samplerate;
#else
    if (mp3Info.sample_rate <= 0)
        return 44100;

    return mp3Info.sample_rate;
#endif
}

const SamplesBuffer Mp3DecoderMiniMp3::decode(char *inputBuffer, int inputBufferLenght)
{
    array.append(inputBuffer, inputBufferLenght);
    if (array.size() < MINIMUM_SIZE_TO_DECODE)
        return SamplesBuffer::ZERO_BUFFER;

    int totalBytesDecoded = 0;
    int bytesDecoded = 0;
    int totalSamplesDecoded = 0;
#ifdef __LINUX_LAME__
    short *out_l = internalShortBuffer_l;
    short *out_r = internalShortBuffer_r;
#else
    signed short *out = internalShortBuffer;
#endif
    char *in = array.data();
    int bytesLeft = array.size() - totalBytesDecoded;
#ifdef __LINUX_LAME__
    do {
        bytesDecoded = (int)hip_decode_headers(mp3Decoder, (uchar *)in, bytesLeft, out_l, out_r, &mp3Info);
        if (bytesDecoded > 0) {
            bytesLeft -= mp3Info.framesize;
            in += bytesDecoded;
            int samplesDecoded = bytesDecoded;
            out_l += samplesDecoded;
            out_r += samplesDecoded;
            totalSamplesDecoded += samplesDecoded;
            totalBytesDecoded += bytesDecoded;
        }
    } while (bytesDecoded > 0 && bytesLeft > 0);

    if(totalBytesDecoded <= 0) return SamplesBuffer::ZERO_BUFFER;

    qDebug() << "mp3 size:" << array.size() << ", decoded:" << totalBytesDecoded << ", frame size:" << mp3Info.framesize;

    array = array.right(array.size() - totalBytesDecoded); // keep just the undecoded bytes to the next call for decode
    int framesDecoded = mp3Info.framesize;

    if (mp3Info.stereo)
        buffer.setToStereo();
    else
        buffer.setToMono();
#else
    do {
        bytesDecoded = mp3_decode((void **)mp3Decoder, in, bytesLeft, out, &mp3Info);
        if (bytesDecoded > 0) {
            bytesLeft -= bytesDecoded;
            in += bytesDecoded;
            int samplesDecoded = mp3Info.audio_bytes/2;
            out += samplesDecoded;
            totalSamplesDecoded += samplesDecoded;
            totalBytesDecoded += bytesDecoded;
        }
    } while (bytesDecoded > 0 && bytesLeft > 0);

    array = array.right(array.size() - totalBytesDecoded); // keep just the undecoded bytes to the next call for decode
    if (totalBytesDecoded <= 0)
        return SamplesBuffer::ZERO_BUFFER;

    // +++++++++++++++++++++++++++
    int framesDecoded = totalSamplesDecoded/mp3Info.channels;

    if (mp3Info.channels >= 2)
        buffer.setToStereo();
    else
        buffer.setToMono();

    if (framesDecoded > AUDIO_SAMPLES_BUFFER_MAX_SIZE)
        framesDecoded = AUDIO_SAMPLES_BUFFER_MAX_SIZE;
#endif

    buffer.setFrameLenght(framesDecoded);
    int internalIndex = 0;
    for (int i = 0; i < framesDecoded; ++i) {
#ifdef __LINUX_LAME__
        buffer.set(0, i, (float)internalShortBuffer_l[internalIndex] / SHRT_MAX);
        buffer.set(1, i, (float)internalShortBuffer_r[internalIndex] / SHRT_MAX);
        internalIndex++;
#else
        for (int c = 0; c < buffer.getChannels(); ++c)
            buffer.set(c, i, (float)internalShortBuffer[internalIndex++] / SHRT_MAX);
#endif
    }

    return buffer;
}


