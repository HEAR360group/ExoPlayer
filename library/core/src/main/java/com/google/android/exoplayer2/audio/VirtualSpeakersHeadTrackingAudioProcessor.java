package com.google.android.exoplayer2.audio;

import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.Format;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Created by Richard on 9/21/17.
 */

/**
 * An {@link AudioProcessor} that converts audio data to {@link C#ENCODING_PCM_16BIT}.
 */
/* package */ final class VirtualSpeakersHeadTrackingAudioProcessor implements AudioProcessor {

    private static final int MAX_CHANNEL_COUNT = 8;
    private final int DEFAULT_CHANNEL_COUNT = 8;

    private int sampleRateHz;
    private int channelCount;

    @C.PcmEncoding
    private int encoding;
    private ByteBuffer buffer;
    private ByteBuffer outputBuffer;
    private boolean inputEnded;

    private static double azimuth;
    private static final Object azimuthLock = new Object();

    private static double[][] volumeMatrix = new double[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];
//    private double[] speakerPos = new double[MAX_CHANNEL_COUNT];
//    private Vector3d[] speakerVec = new Vector3d[MAX_CHANNEL_COUNT];
//    private Vector3d[] rotatedSpeakerVec = new Vector3d[MAX_CHANNEL_COUNT];
    private static final Object rotatedSpeakerVecLock = new Object();
    /**
     * Creates a new audio processor that converts audio data to {@link C#ENCODING_PCM_16BIT}.
     */
    public VirtualSpeakersHeadTrackingAudioProcessor() {
        sampleRateHz = Format.NO_VALUE;
        channelCount = Format.NO_VALUE;
        encoding = C.ENCODING_INVALID;
        buffer = EMPTY_BUFFER;
        outputBuffer = EMPTY_BUFFER;

        //azimuth = Math.toRadians(45);
    }

    @Override
    public boolean configure(int sampleRateHz, int channelCount, @C.Encoding int encoding)
            throws UnhandledFormatException {
    /*
    boolean outputChannelsChanged = !Arrays.equals(pendingOutputChannels, outputChannels);
    outputChannels = pendingOutputChannels;
    if (outputChannels == null) {
      //active = false;
      return outputChannelsChanged;
    }
*/
        if (encoding != C.ENCODING_PCM_16BIT) {
            throw new UnhandledFormatException(sampleRateHz, channelCount, encoding);
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
        return encoding != C.ENCODING_INVALID && channelCount == DEFAULT_CHANNEL_COUNT;
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
        double fuck = Math.toDegrees(azimuth);
        //double[][] volumeMatrix = new double[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];

        //updateVolumeMatrix(volumeMatrix);

        // Prepare the output buffer.
        int position = inputBuffer.position();
        int limit = inputBuffer.limit();
        int frameCount = (limit - position) / (2 * channelCount);
        //8 in 8 out
        int outputSize = frameCount * DEFAULT_CHANNEL_COUNT * 2;
        //int outputSize = frameCount * outputChannels.length * 2;
        if (buffer.capacity() < outputSize) {
            buffer = ByteBuffer.allocateDirect(outputSize).order(ByteOrder.nativeOrder());
        } else {
            buffer.clear();
        }
        while (position < limit) {
            //The channel order of Opus (FL, C, FR, SL, SR, RL, RR, LFE) is different than Mp4 (L, R, C, LFE, RL, RR, SL, SR)
            for(int outIndex = 0; outIndex < channelCount; outIndex++) {
                short output = 0;
                for(int inIndex = 0; inIndex < channelCount; inIndex++) {
                    short input = inputBuffer.getShort(position + 2 * inIndex);
                    short mixedInput = (short)(input * volumeMatrix[inIndex][outIndex]);
                    output += mixedInput;
                }
                buffer.putShort(output);
            }

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

    public void setAzimuth(double azimuth) {
        /*
        synchronized (azimuthLock) {
            this.azimuth = -azimuth;
        }
        */
    }

    public void setVolumeMatrix(double[][] volumeMatrix) {
        //synchronized (azimuthLock) {
            //this.volumeMatrix = volumeMatrix;
            for(int i = 0; i < volumeMatrix.length; i++) {
                for(int j = 0; j < volumeMatrix[i].length; j++) {
                    this.volumeMatrix[i][j] = volumeMatrix[i][j];
                }
            }
        //}
    }
}
