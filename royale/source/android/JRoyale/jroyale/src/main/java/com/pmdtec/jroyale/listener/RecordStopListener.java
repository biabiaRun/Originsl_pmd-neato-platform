/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

package com.pmdtec.jroyale.listener;

/**
 *  This interface needs to be implemented if the client wants to get notified when recording stopped after
 *  the specified number of frames.
 */
@FunctionalInterface
public interface RecordStopListener
{
    /**
     * Will be called if the recording is stopped.
     * @param numFrames Number of frames that have been recorded
     */
    void onRecordingStopped (long numFrames);
}
