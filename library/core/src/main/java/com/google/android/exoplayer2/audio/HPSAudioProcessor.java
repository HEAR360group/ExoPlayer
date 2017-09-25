package com.google.android.exoplayer2.audio;

import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.Format;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Created by Richard on 9/21/17.
 */

/**
 * An {@link AudioProcessor} that converts audio data to {@link C#ENCODING_PCM_16BIT}.
 */
/* package */ final class HPSAudioProcessor implements AudioProcessor {

    private int sampleRateHz;
    private int channelCount;

    @C.PcmEncoding
    private int encoding;
    private ByteBuffer buffer;
    private ByteBuffer outputBuffer;
    private boolean inputEnded;

    private float volumeFront;
    private float volumeLeft;
    private float volumeBack;
    private float volumeRight;

    /**
     * Creates a new audio processor that converts audio data to {@link C#ENCODING_PCM_16BIT}.
     */
    public HPSAudioProcessor() {
        sampleRateHz = Format.NO_VALUE;
        channelCount = Format.NO_VALUE;
        encoding = C.ENCODING_INVALID;
        buffer = EMPTY_BUFFER;
        outputBuffer = EMPTY_BUFFER;
    }

    @Override
    public boolean configure(int sampleRateHz, int channelCount, @C.Encoding int encoding)
            throws AudioProcessor.UnhandledFormatException {
    /*
    boolean outputChannelsChanged = !Arrays.equals(pendingOutputChannels, outputChannels);
    outputChannels = pendingOutputChannels;
    if (outputChannels == null) {
      //active = false;
      return outputChannelsChanged;
    }
*/
        if (encoding != C.ENCODING_PCM_16BIT) {
            throw new AudioProcessor.UnhandledFormatException(sampleRateHz, channelCount, encoding);
        }
        if (this.sampleRateHz == sampleRateHz && this.channelCount == channelCount
                && this.encoding == encoding) {
            return false;
        }
        this.sampleRateHz = sampleRateHz;
        this.channelCount = channelCount;
        this.encoding = encoding;

        return true;
    }

    @Override
    public boolean isActive() {
        //return false;
        return encoding != C.ENCODING_INVALID && channelCount == 6;
    }

    @Override
    public int getOutputChannelCount() {
        return channelCount;
    }

    @Override
    public int getOutputEncoding() {
        return C.ENCODING_PCM_16BIT;
    }

    @Override
    public void queueInput(ByteBuffer inputBuffer) {
        // Prepare the output buffer.
        int position = inputBuffer.position();
        int limit = inputBuffer.limit();
        int frameCount = (limit - position) / (2 * channelCount);
        //6 in 6 out
        int outputSize = frameCount * 6 * 2;
        //int outputSize = frameCount * outputChannels.length * 2;
        if (buffer.capacity() < outputSize) {
            buffer = ByteBuffer.allocateDirect(outputSize).order(ByteOrder.nativeOrder());
        } else {
            buffer.clear();
        }
        while (position < limit) {
            //The channel order of Opus (FL, C, FR, SL, SR, RL, RR, LFE) is different than Mp4 (L, R, C, LFE, RL, RR, SL, SR)
            //Front Perspective
            short inputFrontL = inputBuffer.getShort(position + 2 * 0);
            short inputFrontR = inputBuffer.getShort(position + 2 * 1);

            //Left Perspective
            short inputLeftL = inputBuffer.getShort(position + 2 * 2);
            short inputLeftR = inputBuffer.getShort(position + 2 * 3);

            //Back Perspective
            short inputBackL = inputBuffer.getShort(position + 2 * 4);
            short inputBackR = inputBuffer.getShort(position + 2 * 5);

            //Write the mixed stereo to the first 2 channels as the output
            buffer.putShort(inputFrontL);
            buffer.putShort(inputFrontR);

            //Pad with 0 for all the other channels
            buffer.putShort((short)0);
            buffer.putShort((short)0);
            buffer.putShort((short)0);
            buffer.putShort((short)0);

            //(8byte per 8bits)16bit in total, multiple by 8 channels
            position += channelCount * 2;
        }
        inputBuffer.position(limit);
        buffer.flip();
        outputBuffer = buffer;
    }

    @Override
    public void queueEndOfStream() {
        inputEnded = true;
    }

    @Override
    public ByteBuffer getOutput() {
        ByteBuffer outputBuffer = this.outputBuffer;
        this.outputBuffer = EMPTY_BUFFER;
        return outputBuffer;
    }

    @SuppressWarnings("ReferenceEquality")
    @Override
    public boolean isEnded() {
        return inputEnded && outputBuffer == EMPTY_BUFFER;
    }

    @Override
    public void flush() {
        outputBuffer = EMPTY_BUFFER;
        inputEnded = false;
    }

    @Override
    public void reset() {
        flush();
        buffer = EMPTY_BUFFER;
        sampleRateHz = Format.NO_VALUE;
        channelCount = Format.NO_VALUE;
        encoding = C.ENCODING_INVALID;
    }
}
