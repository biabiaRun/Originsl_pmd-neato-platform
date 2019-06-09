/*
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

package com.pmdtec.jroyale.listener;

import com.pmdtec.jroyale.data.*;

/**
 * Provides the listener interface for consuming depth data from Royale. A listener needs
 * to implement this interface and register itself as a listener to the ICameraDevice.
 */
@FunctionalInterface
public interface DepthDataListener
{
    /**
     * Will be called on every frame update by the Royale framework
     * <p>
     * NOTICE: Calling other framework functions within the data callback
     * can lead to undefined behavior and is therefore unsupported.
     * Call these framework functions from another thread to avoid problems.
     */
    void onNewData(DepthData depthData);
}
