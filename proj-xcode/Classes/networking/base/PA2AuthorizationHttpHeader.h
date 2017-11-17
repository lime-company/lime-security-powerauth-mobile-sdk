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

#import <Foundation/Foundation.h>

/**
 Class representing authorization HTTP header with the PowerAuth-Authorization
 or PowerAuth-Token signature.
 */
@interface PA2AuthorizationHttpHeader : NSObject

/**
 Property representing PowerAuth 2.0 HTTP Authorization Header. The current implementation
 contains value "X-PowerAuth-Authorization" for standard authorization and "X-PowerAuth-Token" for
 token-based authorization.
 */
@property (nonatomic, strong, readonly, nonnull) NSString *key;

/**
 Computed value of the PowerAuth 2.0 HTTP Authorization Header, to be used in HTTP requests "as is".
 */
@property (nonatomic, strong, readonly, nonnull) NSString *value;

/**
 Returns a new header object created for standard authorization header.
 If the value parameter contains nil, then returns nil object.
 */
+ (nullable PA2AuthorizationHttpHeader*) authorizationHeaderWithValue:(nullable NSString*)value;

/**
 Returns a new header object created for token based authorization header.
 If the value parameter contains nil, then returns nil object.
 */
+ (nullable PA2AuthorizationHttpHeader*) tokenHeaderWithValue:(nullable NSString*)value;

@end
