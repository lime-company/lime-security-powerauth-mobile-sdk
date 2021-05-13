/*
 * Copyright 2021 Wultra s.r.o.
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

#import <PowerAuth2/PowerAuthDeprecated.h>
#import <PowerAuth2/PowerAuthLog.h>
#import "PA2PrivateConstants.h"

// PA2Error*...

NSString *const PA2ErrorDomain					= PA2Def_PowerAuthErrorDomain;
NSString *const PA2ErrorInfoKey_AdditionalInfo	= PA2Def_PowerAuthErrorInfoKey_AdditionalInfo;
NSString *const PA2ErrorInfoKey_ResponseData	= PA2Def_PowerAuthErrorInfoKey_ResponseData;

NSInteger const PA2ErrorCodeNetworkError				= PowerAuthErrorCode_NetworkError;
NSInteger const PA2ErrorCodeSignatureError				= PowerAuthErrorCode_SignatureError;
NSInteger const PA2ErrorCodeInvalidActivationState		= PowerAuthErrorCode_InvalidActivationState;
NSInteger const PA2ErrorCodeInvalidActivationData		= PowerAuthErrorCode_InvalidActivationData;
NSInteger const PA2ErrorCodeMissingActivation			= PowerAuthErrorCode_MissingActivation;
NSInteger const PA2ErrorCodeActivationPending			= PowerAuthErrorCode_ActivationPending;
NSInteger const PA2ErrorCodeBiometryNotAvailable		= PowerAuthErrorCode_BiometryNotAvailable;
NSInteger const PA2ErrorCodeBiometryFailed				= PowerAuthErrorCode_BiometryFailed;
NSInteger const PA2ErrorCodeBiometryCancel				= PowerAuthErrorCode_BiometryCancel;
NSInteger const PA2ErrorCodeOperationCancelled			= PowerAuthErrorCode_OperationCancelled;
NSInteger const PA2ErrorCodeEncryption					= PowerAuthErrorCode_Encryption;
NSInteger const PA2ErrorCodeWrongParameter				= PowerAuthErrorCode_WrongParameter;
NSInteger const PA2ErrorCodeInvalidToken				= PowerAuthErrorCode_InvalidToken;
NSInteger const PA2ErrorCodeWatchConnectivity			= PowerAuthErrorCode_WatchConnectivity;
NSInteger const PA2ErrorCodeProtocolUpgrade				= PowerAuthErrorCode_ProtocolUpgrade;
NSInteger const PA2ErrorCodePendingProtocolUpgrade		= PowerAuthErrorCode_PendingProtocolUpgrade;

// PA2Keychain*

NSString *const PA2Keychain_Initialized			= PA2Def_PowerAuthKeychain_Initialized;
NSString *const PA2Keychain_Status				= PA2Def_PowerAuthKeychain_Status;
NSString *const PA2Keychain_Possession			= PA2Def_PowerAuthKeychain_Possession;
NSString *const PA2Keychain_Biometry			= PA2Def_PowerAuthKeychain_Biometry;
NSString *const PA2Keychain_TokenStore			= PA2Def_PowerAuthKeychain_TokenStore;
NSString *const PA2KeychainKey_Possession		= PA2Def_PowerAuthKeychainKey_Possession;

// PA2Log

void PA2LogSetEnabled(BOOL enabled)
{
	PowerAuthLogSetEnabled(enabled);
}
BOOL PA2LogIsEnabled(void)
{
	return PowerAuthLogIsEnabled();
}
void PA2LogSetVerbose(BOOL verbose)
{
	PowerAuthLogSetVerbose(verbose);
}
BOOL PA2LogIsVerbose(void)
{
	return PowerAuthLogIsVerbose();
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-implementations"

PA2_DEPRECATED_CLASS_IMPL(1.6.0, PA2ClientConfiguration, PowerAuthClientConfiguration)
PA2_DEPRECATED_CLASS_IMPL(1.6.0, PA2AuthorizationHttpHeader, PowerAuthAuthorizationHttpHeader)
PA2_DEPRECATED_CLASS_IMPL(1.6.0, PA2Otp, PowerAuthActivationCode)
PA2_DEPRECATED_CLASS_IMPL(1.6.0, PA2OtpUtil, PowerAuthActivationCodeUtil)
PA2_DEPRECATED_CLASS_IMPL(1.6.0, PA2ActivationStatus, PowerAuthActivationStatus)
PA2_DEPRECATED_CLASS_IMPL(1.6.0, PA2ActivationRecoveryData, PowerAuthActivationRecoveryData)
PA2_DEPRECATED_CLASS_IMPL(1.6.0, PA2ActivationResult, PowerAuthActivationResult)
PA2_DEPRECATED_CLASS_IMPL(1.6.0, PA2CustomHeaderRequestInterceptor, PowerAuthCustomHeaderRequestInterceptor)
PA2_DEPRECATED_CLASS_IMPL(1.6.0, PA2BasicHttpAuthenticationRequestInterceptor, PowerAuthBasicHttpAuthenticationRequestInterceptor)
PA2_DEPRECATED_CLASS_IMPL(1.6.0, PA2ClientSslNoValidationStrategy, PowerAuthClientSslNoValidationStrategy)
PA2_DEPRECATED_CLASS_IMPL(1.6.0, PA2ErrorResponse, PowerAuthRestApiErrorResponse)
PA2_DEPRECATED_CLASS_IMPL(1.6.0, PA2Error, PowerAuthRestApiError)
PA2_DEPRECATED_CLASS_IMPL(1.6.0, PA2Keychain, PowerAuthKeychain)
PA2_DEPRECATED_CLASS_IMPL(1.6.0, PA2KeychainConfiguration, PowerAuthKeychainConfiguration)
PA2_DEPRECATED_CLASS_IMPL(1.6.0, PA2System, PowerAuthSystem)

#if defined(PA2_WATCH_SUPPORT)
PA2_DEPRECATED_CLASS_IMPL(1.6.0, PA2WCSessionManager, PowerAuthWCSessionManager)
#endif

#pragma clang diagnostic pop
