/**
 * Copyright 2016 Lime - HighTech Solutions s.r.o.
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

#import "PA2Macros.h"

/** Class representing a multi-factor authentication object.
 */
@interface PowerAuthAuthentication : NSObject<NSCopying>

/** Indicates if a possession factor should be used.
 */
@property (nonatomic, assign) BOOL usePossession;

/** Indicates if a biometry factor should be used.
 */
@property (nonatomic, assign) BOOL useBiometry;

/** Password to be used for knowledge factor, or nil of knowledge factor should not be used.
 */
@property (nonatomic, strong, nullable) NSString *usePassword;

/** (optional) Specifies the text displayed on Touch or Face ID prompt in case biometry is required to obtain data.
 
 Use this value to give user a hint on what is biometric authentication used for in this specific authentication.
 For example, include a name of the account user uses to log in.
 */
@property (nonatomic, strong, nullable) NSString *biometryPrompt;

/** (optional) If 'usePossession' is set to YES, this value may specify possession key data. If no custom data is specified, default possession key is used.
 */
@property (nonatomic, strong, nullable) NSData *overridenPossessionKey;

/** (optional) If 'useBiometry' is set to YES, this value may specify biometry key data. If no custom data is specified, default biometry key is used for the PowerAuthSDK instance, based on the keychain configuration and SDK instance configuration.
 */
@property (nonatomic, strong, nullable) NSData *overridenBiometryKey;

@end
