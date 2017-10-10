package com.google.android.exoplayer2.audio;

import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.Format;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import javax.vecmath.Quat4d;
import javax.vecmath.Vector3d;

/**
 * Created by Richard on 9/21/17.
 */

/**
 * An {@link AudioProcessor} that converts audio data to {@link C#ENCODING_PCM_16BIT}.
 */
/* package */ final class VirtualSpeakersHeadTrackingAudioProcessor implements AudioProcessor {

    private final int MAX_CHANNEL_COUNT = 8;
    private final int DEFAULT_CHANNEL_COUNT = 8;
    private final int LFE_CHANNEL_ID = 3;
    private final boolean HAS_LFE = true;
    private final Vector3d FRONT_VEC = new Vector3d(0, 0, 1);

    private int sampleRateHz;
    private int channelCount;

    @C.PcmEncoding
    private int encoding;
    private ByteBuffer buffer;
    private ByteBuffer outputBuffer;
    private boolean inputEnded;

    public class DegreeDiff {
        public double dotValue;
        public double crossValue;
        public double angleValue;
        public int speakerIndex;
    }

    private static double azimuth;
    private static final Object azimuthLock = new Object();

//    private double[][] volumeMatrix = new double[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];
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
        double[][] volumeMatrix = new double[MAX_CHANNEL_COUNT][MAX_CHANNEL_COUNT];

        updateVolumeMatrix(volumeMatrix);

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
        synchronized (azimuthLock) {
            this.azimuth = -azimuth;
        }
    }

    private void updateVolumeMatrix(double[][] volumeMatrix) {
        double[] speakerPos = new double[MAX_CHANNEL_COUNT];
        Vector3d[] speakerVec = new Vector3d[MAX_CHANNEL_COUNT];
        Vector3d[] rotatedSpeakerVec = new Vector3d[MAX_CHANNEL_COUNT];

        Vector3d rotatedFrontVec = rotate(azimuth, FRONT_VEC);
/*
        speakerPos[0] = Math.toRadians(-30);
        speakerPos[1] = Math.toRadians(30);
        speakerPos[2] = Math.toRadians(0);
        speakerPos[3] = Math.toRadians(0);
        speakerPos[4] = Math.toRadians(-120);
        speakerPos[5] = Math.toRadians(120);
*/

        speakerPos[0] = Math.toRadians(-45);
        speakerPos[1] = Math.toRadians(45);
        speakerPos[2] = Math.toRadians(0);
        speakerPos[3] = Math.toRadians(0);
        speakerPos[4] = Math.toRadians(-135);
        speakerPos[5] = Math.toRadians(135);
        speakerPos[6] = Math.toRadians(-90);
        speakerPos[7] = Math.toRadians(90);

        for(int i = 0; i < DEFAULT_CHANNEL_COUNT; i++) {
            speakerVec[i] = rotate(speakerPos[i], FRONT_VEC);
        }

        for(int i = 0; i < DEFAULT_CHANNEL_COUNT; i++) {
            if(HAS_LFE && i == LFE_CHANNEL_ID)
                continue;

            rotatedSpeakerVec[i] = rotate(speakerPos[i], rotatedFrontVec);
        }




        for(int i = 0; i < DEFAULT_CHANNEL_COUNT; i++) {
            if(HAS_LFE && i == LFE_CHANNEL_ID)
                continue;

            DegreeDiff[] degreeDiffArrayL = new DegreeDiff[MAX_CHANNEL_COUNT];
            DegreeDiff[] degreeDiffArrayR = new DegreeDiff[MAX_CHANNEL_COUNT];
            int degreeDiffArrayCountL = 0;
            int degreeDiffArrayCountR = 0;

            for (int speakerPosID = 0; speakerPosID < DEFAULT_CHANNEL_COUNT; speakerPosID++) {
                if(HAS_LFE && speakerPosID == LFE_CHANNEL_ID)
                    continue;

                //double dotValue = rotatedSpeakerVec[i].dot(speakerVec[speakerPosID]);
                Vector3d crossVec = new Vector3d();
                crossVec.cross(rotatedSpeakerVec[i], speakerVec[speakerPosID]);
                double angleValue = rotatedSpeakerVec[i].angle(speakerVec[speakerPosID]);
                //if(dotValue <= 0)
                    //continue;

                DegreeDiff degreeDiff = new DegreeDiff();
                //degreeDiff.dotValue = dotValue;
                degreeDiff.crossValue = crossVec.y;
                degreeDiff.angleValue = angleValue;
                degreeDiff.speakerIndex = speakerPosID;

                if(degreeDiff.angleValue == 0 /*|| Math.abs(degreeDiff.dotValue) < 0.001*/) {
                    degreeDiffArrayCountL = 0;
                    degreeDiffArrayCountR = 0;
                    break;
                }
                //Select the nearest speaker from left
                else if(degreeDiff.crossValue < 0) {
                    orderInsert(degreeDiffArrayL, 0, degreeDiffArrayCountL, degreeDiff);
                    degreeDiffArrayCountL++;
                }
                //Select the nearest speaker from right
                else {
                        orderInsert(degreeDiffArrayR, 0, degreeDiffArrayCountR, degreeDiff);
                    degreeDiffArrayCountR++;
                }
            }

            if(degreeDiffArrayCountL != 0 && degreeDiffArrayCountR != 0) {
                //Calculate volume distribution
                int speakerIndex0 = degreeDiffArrayL[0].speakerIndex;
                double angle0 = degreeDiffArrayL[0].angleValue;
                //double dot0 = degreeDiffArrayL[0].dotValue;
                int speakerIndex1 = degreeDiffArrayR[0].speakerIndex;
                double angle1 = degreeDiffArrayR[0].angleValue;
                //double dot1 = degreeDiffArrayR[0].dotValue;

                //double actAngle0 = Math.sin(angle0);
                //double actAngle1 = Math.cos(angle1);
                double speaker0Vol = angle1 / (angle0 + angle1);
                double speaker1Vol = angle0 / (angle0 + angle1);
                //double speaker0ActVol = Math.sin(speaker0Vol * Math.PI / 2);
                //double speaker1ActVol = Math.cos(speaker0Vol * Math.PI / 2);

                //double speaker0Vol = dot0 / (dot0 + dot1);
                //double speaker1Vol = dot1 / (dot0 + dot1);

                volumeMatrix[i][speakerIndex0] = speaker0Vol;
                volumeMatrix[i][speakerIndex1] = speaker1Vol;

                for(int j = 0; j < DEFAULT_CHANNEL_COUNT; j++) {
                    if(j == speakerIndex0 || j == speakerIndex1)
                        continue;

                    if(HAS_LFE && j == LFE_CHANNEL_ID)
                        continue;

                    volumeMatrix[i][j] = 0;
                }
            }
            else {
                volumeMatrix[i][i] = 1;

                for(int j = 0; j < DEFAULT_CHANNEL_COUNT; j++) {
                    if(j == i)
                        continue;

                    if(HAS_LFE && j == LFE_CHANNEL_ID)
                        continue;

                    volumeMatrix[i][j] = 0;
                }
            }
        }

        if(HAS_LFE)
            volumeMatrix[LFE_CHANNEL_ID][LFE_CHANNEL_ID] = 1;
    }

    private Vector3d rotate(double theta, Vector3d in) {
        double cosTheta = Math.cos(theta);
        double sinTheta = Math.sin(theta);
        Vector3d out = new Vector3d();
        out.x = in.x * cosTheta + in.z * sinTheta;
        out.y = in.y;
        out.z = -in.x * sinTheta + in.z * cosTheta;
        out.normalize();
        return out;
    }

    private int orderInsert(DegreeDiff[] arr, int first, int last, DegreeDiff target) {
        int i = last;
        while((i > first) && (target.angleValue < arr[i - 1].angleValue)) {
        //while((i > first) && (target.dotValue > arr[i - 1].dotValue)) {
            arr[i] = arr[i - 1];
            i = i - 1;
        }
        arr[i] = target;
        return i;
    }
}
