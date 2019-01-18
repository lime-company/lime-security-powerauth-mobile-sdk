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

package io.getlime.security.powerauth.exception;

/**
 * Created by miroslavmichalec on 20/10/2016.
 */

public class PowerAuthErrorCodes {
    
    public static final int PA2Succeed = 0;
    public static final int PA2ErrorCodeNetworkError = 1;                       // Future SDK compatibility
    public static final int PA2ErrorCodeSignatureError = 2;
    public static final int PA2ErrorCodeInvalidActivationState = 3;
    public static final int PA2ErrorCodeInvalidActivationData = 4;
    public static final int PA2ErrorCodeMissingActivation = 5;
    public static final @Deprecated int PA2ErrorCodeAuthenticationFailed = 6;   // Never returned from SDK
    public static final int PA2ErrorCodeActivationPending = 7;
    public static final @Deprecated int PA2ErrorCodeKeychain = 8;               // Never returned from SDK
    public static final @Deprecated int PA2ErrorCodeTouchIDNotAvailable = 9;    // Never returned from SDK
    public static final @Deprecated int PA2ErrorCodeTouchIDCancel = 10;         // Please use "PA2ErrorCodeBiometryCancel"
    public static final int PA2ErrorCodeBiometryCancel = 10;                    // Replacement for "PA2ErrorCodeTouchIDCancel"
    public static final int PA2ErrorCodeOperationCancelled = 11;                // Future SDK compatibility
    public static final int PA2ErrorCodeInvalidActivationCode = 12;
    public static final int PA2ErrorCodeInvalidToken = 13;
    public static final int PA2ErrorCodeEncryptionError = 14;
    public static final int PA2ErrorCodeWrongParameter = 15;
    public static final int PA2ErrorCodeProtocolUpgrade = 16;                   // Future SDK compatibility
    public static final int PA2ErrorCodePendingProtocolUpgrade = 17;            // Future SDK compatibility
}
