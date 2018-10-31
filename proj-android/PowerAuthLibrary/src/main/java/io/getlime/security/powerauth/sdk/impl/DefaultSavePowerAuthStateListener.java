/*
 * Copyright 2017 Wultra s.r.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package io.getlime.security.powerauth.sdk.impl;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import io.getlime.security.powerauth.keychain.PA2Keychain;

/**
 * Default implementation of PowerAuth 2.0 state listener.
 *
 * @author Petr Dvorak, petr@wultra.com
 */
public class DefaultSavePowerAuthStateListener implements ISavePowerAuthStateListener {

    private Context context;
    private PA2Keychain keychain;

    public DefaultSavePowerAuthStateListener(@NonNull Context context, @NonNull PA2Keychain keychain) {
        this.context = context.getApplicationContext();
        this.keychain = keychain;
    }

    @Override
    public byte[] serializedState(String instanceId) {
        return keychain.dataForKey(context, instanceId);
    }

    @Override
    public void onPowerAuthStateChanged(@Nullable String instanceId, byte[] serializedState) {
        keychain.putDataForKey(context, serializedState, instanceId);
    }

}
