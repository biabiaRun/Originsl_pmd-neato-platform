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
 * This interface allows observers to receive events.
 */
@FunctionalInterface
public interface CameraEventListener
{
    /**
     * Will be called when an cameraEvent occurs.
     * <p>
     * Note there are some constraints on what the user is allowed to do
     * in the callback.
     * - Actually the royale API does not claim to be reentrant (and probably isn't),
     * so the user is not supposed to call any API function from this callback.
     * - Deleting the ICameraDevice from the callback will most certainly
     * lead to a deadlock.
     * This has the interesting side effect that calling exit() or equivalent
     * from the callback may cause issues.
     * <p>
     * @param cameraEvent The cameraEvent.
     */
    void onEvent(CameraEvent cameraEvent);
}
