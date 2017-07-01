/**
 * Copyright 2017 Lime - HighTech Solutions s.r.o.
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

#import <Foundation/Foundation.h>

/**
 The PA2ActivationResult object represents successfull result from the activation
 process.
 */
@interface PA2ActivationResult : NSObject

/**
 Decimalized fingerprint calculated from device's public key.
 */
@property (nonatomic, strong, nonnull) NSString * activationFingerprint;

/**
 Custom attributes received from the server. The value may be nil in case that there 
 are no custom attributes available.
 */
@property (nonatomic, strong, nullable) NSDictionary<NSString*, NSObject*>* customAttributes;

@end
