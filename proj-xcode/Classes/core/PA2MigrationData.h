/**
 * Copyright 2018 Wultra s.r.o.
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
 The `PA2MigrationData` protocol defines abstract interface for providing
 data for protocol migration.
 */
@protocol PA2MigrationData <NSObject>
@end

/**
 The `PA2MigrationDataV3` object contains data required for protocol migration
 from version 2 to version 3.
 */
@interface PA2MigrationDataV3 : NSObject<PA2MigrationData>

/**
 Contains initial value for hash-based counter. The Base64 string is expected
 with exact 16 bytes long encoded data.
 */
@property (nonatomic, strong) NSString * ctrData;

@end
