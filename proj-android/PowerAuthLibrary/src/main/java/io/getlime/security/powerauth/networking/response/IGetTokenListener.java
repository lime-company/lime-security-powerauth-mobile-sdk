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

package io.getlime.security.powerauth.networking.response;

import androidx.annotation.MainThread;
import androidx.annotation.NonNull;

import io.getlime.security.powerauth.sdk.PowerAuthToken;

/**
 * Listener for getting access token.
 */
public interface IGetTokenListener {

    /**
     * Callen when access token retrieval succeeds with a valid token.
     *
     * @param token valid token object
     */
    @MainThread
    void onGetTokenSucceeded(@NonNull PowerAuthToken token);

    /**
     * Called when getting token fails with an error.
     *
     * @param t error occurred during the operation
     */
    @MainThread
    void onGetTokenFailed(@NonNull Throwable t);
}
