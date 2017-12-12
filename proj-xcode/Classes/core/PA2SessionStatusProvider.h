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
 The `PA2SessionStatusProvider` protocol defines an abstract interface for getting instant
 information about PowerAuth session.
 */
@protocol PA2SessionStatusProvider <NSObject>

/**
 Check if it is possible to start an activation process
 
 @return YES if activation process can be started, NO otherwise.
 @exception NSException thrown in case configuration is not present.
 */
- (BOOL) canStartActivation;

/**
 Checks if there is a pending activation (activation in progress).
 
 @return YES if there is a pending activation, NO otherwise.
 @exception NSException thrown in case configuration is not present.
 */
- (BOOL) hasPendingActivation;

/**
 Checks if there is a valid activation.
 
 @return YES if there is a valid activation, NO otherwise.
 @exception NSException thrown in case configuration is not present.
 */
- (BOOL) hasValidActivation;

@end
