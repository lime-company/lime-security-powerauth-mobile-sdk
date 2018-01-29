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

#import <WatchConnectivity/WatchConnectivity.h>

/**
 The PA2WCSessionManager class is managing PowerAuth communication between
 Apple Watch and iPhone. The application, or watch extension has to use
 this interface to process messages received
 */
@interface PA2WCSessionManager : NSObject

/**
 Returns the default shared instance of PA2WCSessionManager.
 */
@property (class, nonnull, readonly) PA2WCSessionManager * sharedInstance;

/**
 Returns a valid WCSession. On IOS, the value  is valid only if session is supported
 on this device and Watch app is installed on the currently paired and active Apple Watch.
 On watchOS, property always contains session's singleton.
 */
@property (nonatomic, strong, nullable, readonly) WCSession * validSession __IOS_AVAILABLE(9.0);

// Messages from counterpart

/**
 Processes a received message data and returns YES if message has been consumed (e.g. PowerAuth
 is the data recipient), NO otherwise. If YES is returned and the replyHandler is available, then the handler
 is called with a valid reply data. The replyHandler nullability must match behavior of the message. That means
 that if WCSessionDelegate method is called with a valid reply handler (e.g. counterpart is expecing response),
 then you have to always provide the reply handler to this method.
 */
- (BOOL) processReceivedMessageData:(nonnull NSData *)data
					   replyHandler:(void (^ _Nullable)(NSData * _Nonnull reply))replyHandler;

/**
 Processes a received user info and returns YES if information has been consumed (e.g. PowerAuth
 is the recipient). You should call this method from your WCSessionDelegate code, handling communication
 between iPhone & Apple Watch.
 */
- (BOOL) processReceivedUserInfo:(nonnull NSDictionary<NSString *, id> *)userInfo;

@end
