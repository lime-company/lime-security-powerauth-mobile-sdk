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

// PA2_SHARED_SOURCE PowerAuth2ForWatch .
// PA2_SHARED_SOURCE PowerAuth2ForExtensions .

#import <PowerAuth2ForWatch/PowerAuthMacros.h>

#pragma mark - Error codes

/**
 PowerAuth SDK error domain
 */
PA2_EXTERN_C NSString * __nonnull const PowerAuthErrorDomain;

/**
 A key to NSError.userInfo dicionary where the optional NSDictionary with additional
 information about error is stored.
 */
PA2_EXTERN_C NSString * __nonnull const PowerAuthErrorInfoKey_AdditionalInfo;

/**
 A key to NSError.userInfo dicionary where the optional NSData with error response
 is stored.
 */
PA2_EXTERN_C NSString * __nonnull const PowerAuthErrorInfoKey_ResponseData;

/**
 Error codes returned for PowerAuthErrorDomain errors
 */
typedef NS_ENUM(NSInteger, PowerAuthErrorCode) {
	
	/**
	 Error code is not determined. This constant is returned in `NSError.powerAuthErrorCode`
	 when the NSError object has different than `PowerAuthErrorDomain` domain.
	 */
	PowerAuthErrorCode_NA							= 0,
	/**
	 Error code for error with network connectivity or download
	 */
	PowerAuthErrorCode_NetworkError					= 1,
	/**
	 Error code for error in signature calculation
	 */
	PowerAuthErrorCode_SignatureError				= 2,
	/**
	 Error code for error that occurs when activation state is invalid
	 */
	PowerAuthErrorCode_InvalidActivationState		= 3,
	/**
	 Error code for error that occurs when activation data is invalid
	 */
	PowerAuthErrorCode_InvalidActivationData		= 4,
	/**
	 Error code for error that occurs when activation is required but missing
	 */
	PowerAuthErrorCode_MissingActivation			= 5,
	/**
	 Error code for error that occurs when pending activation is present and work
	 with completed activation is required
	 */
	PowerAuthErrorCode_ActivationPending			= 6,
	/**
	 Error code for TouchID or FaceID not available error
	 */
	PowerAuthErrorCode_BiometryNotAvailable			= 7,
	/**
	 Error code for TouchID or FaceID action cancel error
	 */
	PowerAuthErrorCode_BiometryCancel				= 8,
	/**
	 Error code for biometric authentication failure. This can happen when biometric
	 authentication is requested and is not configured, or when failed to acquire
	 biometry key from the keychain.
	 */
	PowerAuthErrorCode_BiometryFailed				= 9,
	/**
	 Error code for canceled operation. This kind of error may occur in situations, when SDK
	 needs to cancel an asynchronous operation, but the cancel is not initiated by the application
	 itself. For example, if you reset the state of `PowerAuthSDK` during the pending
	 fetch for activation status, then the application gets an exception, with this error code.
	 */
	PowerAuthErrorCode_OperationCancelled			= 10,
	/**
	 Error code for errors related to end-to-end encryption or general encryption failure.
	 */
	PowerAuthErrorCode_Encryption					= 11,
	/**
	 Error code for a general API misuse
	 */
	PowerAuthErrorCode_WrongParameter				= 12,
	/**
	 Error code for accessing an unknown token
	 */
	PowerAuthErrorCode_InvalidToken					= 13,
	/**
	 Error code for a general error related to WatchConnectivity
	 */
	PowerAuthErrorCode_WatchConnectivity			= 14,
	/**
	 Error code for protocol upgrade failure.
	 The recommended action is to retry the status fetch operation, or locally remove the activation.
	 */
	PowerAuthErrorCode_ProtocolUpgrade				= 15,
	/**
	 The requested function is not available during the protocol upgrade. You can retry the operation,
	 after the upgrade is finished.
	 */
	PowerAuthErrorCode_PendingProtocolUpgrade		= 16,
};

@interface NSError (PowerAuthErrorCode)
/**
 Contains `PowerAuthErrorCode` in case that NSError object has `PowerAuthErrorDomain`. If error object
 has different domain, then property contains `PowerAuthErrorCode_NA`.
 */
@property (nonatomic, readonly) PowerAuthErrorCode powerAuthErrorCode;

@end
