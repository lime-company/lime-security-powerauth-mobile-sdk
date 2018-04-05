/**
 * Copyright 2018 Lime - HighTech Solutions s.r.o.
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

#import "PA2NetworkObject.h"

typedef NS_ENUM(int, PA2VaultUnlockReason) {
	// If occured, then it's internal SDK error. Don't use as parameter to fetch function
	PA2VaultUnlockReason_Unknown = 0,
	
	PA2VaultUnlockReason_PASSWORD_VALIDATE,
	PA2VaultUnlockReason_PASSWORD_CHANGE,
	PA2VaultUnlockReason_ADD_BIOMETRY,
	PA2VaultUnlockReason_FETCH_ENCRYPTION_KEY,
	PA2VaultUnlockReason_SIGN_WITH_DEVICE_PRIVATE_KEY
};

@interface PA2VaultUnlockRequest : NSObject <PA2NetworkObject>

- (id) initWithReason:(PA2VaultUnlockReason)reason;

@property (nonatomic, assign) PA2VaultUnlockReason reason;

@end
