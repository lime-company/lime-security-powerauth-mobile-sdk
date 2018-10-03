/**
 * Copyright 2016 Wultra s.r.o.
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

#import "PA2CreateActivationRequest.h"
#import "PA2PrivateMacros.h"

@implementation PA2CreateActivationRequest

- (NSDictionary*) toDictionary
{
    NSMutableDictionary *dictionary = [NSMutableDictionary dictionaryWithCapacity:8];
    if (_activationIdShort) {
        [dictionary setObject:_activationIdShort forKey:@"activationIdShort"];
    }
    if (_activationName) {
        [dictionary setObject:_activationName forKey:@"activationName"];
    }
    if (_applicationKey) {
        [dictionary setObject:_applicationKey forKey:@"applicationKey"];
    }
    if (_activationNonce) {
        [dictionary setObject:_activationNonce forKey:@"activationNonce"];
    }
    if (_applicationSignature) {
        [dictionary setObject:_applicationSignature forKey:@"applicationSignature"];
    }
    if (_encryptedDevicePublicKey) {
        [dictionary setObject:_encryptedDevicePublicKey forKey:@"encryptedDevicePublicKey"];
    }
    if (_ephemeralPublicKey) {
        [dictionary setObject:_ephemeralPublicKey forKey:@"ephemeralPublicKey"];
    }
    if (_extras) {
        [dictionary setObject:_extras forKey:@"extras"];
    }
    return dictionary;
}

@end
