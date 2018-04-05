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

#import "PA2ErrorResponse.h"

@implementation PA2ErrorResponse

- (instancetype)initWithError:(PA2Error*)error {
	self = [super init];
	if (self) {
		self.status = PA2RestResponseStatus_ERROR;
		self.responseObject = error;
	}
	return self;
}

- (instancetype)initWithDictionary:(NSDictionary *)dictionary {
	self = [super init];
	if (self) {
		self.status = PA2RestResponseStatus_ERROR;
		NSDictionary *errorDict = [dictionary objectForKey:@"responseObject"];
		if (errorDict != nil) {
			self.responseObject = [[PA2Error alloc] initWithDictionary:errorDict];
		}
	}
	return self;
}

#ifdef DEBUG
- (NSString*) description
{
	NSString * status_str = _status == PA2RestResponseStatus_OK ? @"OK" : @"ERROR";
	NSString * code_str = [@(_httpStatusCode) stringValue];
	NSString * ro = _responseObject ? [_responseObject description] : @"<null>";
	return [NSString stringWithFormat:@"<PA2ErrorResponse status=%@, httpStatusCode=%@, responseObject=%@>", status_str, code_str, ro];
}
#endif

@end
