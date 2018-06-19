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

@class PA2ECIESCryptogram;

#pragma mark - ECIES Encryptor -

/**
 The PA2ECIESEncryptor class implements a request encryption and response decryption for our custom ECIES scheme.
 For more details about our ECIES implementation, please check documentation available at the beginning of
 <PowerAuth/ECIES.h> C++ header.
 */
@interface PA2ECIESEncryptor : NSObject

#pragma mark Initialization

/**
 Initializes an ecnryptor with server's |publicKey| and optional |sharedInfo2|.
 The initialized instance can be used for both encryption and decryption tasks.
 */
- (nullable instancetype) initWithPublicKey:(nonnull NSData*)publicKey sharedInfo2:(nullable NSData*)sharedInfo;

/**
 Initializes an encryptor with previously calculated |envelopeKey| and optional |sharedInfo|.
 The initialized instance can be used only for decryption task.
 */
- (nullable instancetype) initWithEnvelopeKey:(nonnull NSData*)envelopeKey sharedInfo2:(nullable NSData*)sharedInfo;

/**
 Returns a new instance of PA2ECIESEncryptor, suitable only for data decryption or nil if current encryptor is not
 able to decrypt response (this happens typically if you did not call `encryptRequest` or instnace contains invalid keys).
 
 Discussion
 
 The returned copy will not be able to encrypt a new requests, but will be able to decrypt a received response.
 This behavior is helpful when processing of simultaneous encrypted requests and resonses is required.
 Due to fact, that our ECIES scheme is generating an unique key for each request-response roundtrip, you need to
 capture that key for later safe decryption. As you can see, that might be problematic, because you don't know when
 exactly the response will be received. To help with this, you can make a copy of the object and use that copy
 only for response decryption.
 
 The `-encryptRequest:completion:` method is an one example of safe approach, but you can implement your own
 processing, if the thread safety is not a problem.
 */
- (nullable PA2ECIESEncryptor*) copyForDecryption;

#pragma mark Properties

/**
 Contains server's public key, provided during the object initialization. The value is optional, because
 you can create an encryptor object without the public key.
 */
@property (nonatomic, strong, readonly, nullable) NSData * publicKey;
/**
 Contains a value for optional shared info.
 */
@property (nonatomic, strong, nullable) NSData * sharedInfo2;
/**
 Contains YES if this instnace of encryptor can encrypt request data.
 */
@property (nonatomic, readonly) BOOL canEncryptRequest;
/**
 Contains YES if this instnace of encryptor can decrypt a cryptogram with response data.
 The property typically contains NO only when the instance is not properly initialized, or
 you did not call `encryptRequest` at least once.
 */
@property (nonatomic, readonly) BOOL canDecryptResponse;

#pragma mark Encrypt & Decrypt

/**
 Encrypts an input |data| into PA2ECIESCryptogram object or nil in case of failure.
 Note that each call for this method will regenerate an internal envelope key, so you should use
 the method only in pair with subsequent call to `decryptResponse:`. If you plan to reuse one
 encryptor for multiple simultaneous requests, then you should make a copy of the object after
 every successful encryption. Check `-copyForDecryption` or  `-encryptRequest:completion:` methods
 for details.
 
 The DEBUG version of the SDK prints detailed error about the failure reason into the log.
 */
- (nullable PA2ECIESCryptogram *) encryptRequest:(nullable NSData *)data;

/**
 Decrypts a |cryptogram| received from the server and returns decrypted data or nil in case of failure.
 
 The DEBUG version of the SDK prints detailed error about the failure reason into the log.
 */
- (nullable NSData *) decryptResponse:(nonnull PA2ECIESCryptogram *)cryptogram;

/**
 This is a special, thread-safe version of request encryption. The method encrypts provided data
 and makes a copy of itself in thread synchronized block. Then the completion block is called with
 generated cryptogram and copied instance, which is suitable only for response decryption.
 The completion is called from outside of the synchronization block.
 
 Note that the rest of the encryptor's interface is not thread safe. So, once the shared instance
 for encryption is created, then you should not change its parameters or call other methods.
 
 Returns YES if encryption succeeded or NO in case of error.
 */
- (BOOL) encryptRequest:(nullable NSData *)data
			 completion:(void (NS_NOESCAPE ^_Nonnull)(PA2ECIESCryptogram * _Nullable cryptogram, PA2ECIESEncryptor * _Nullable decryptor))completion;

@end


#pragma mark - ECIES cryptogram -

/**
 The PA2ECIESCryptogram object represents cryptogram transmitted
 over the network.
 */
@interface PA2ECIESCryptogram : NSObject

/**
 Encrypted data
 */
@property (nonatomic, strong, nullable) NSData * body;
/**
 A MAC computed for key & data
 */
@property (nonatomic, strong, nullable) NSData * mac;
/**
 An ephemeral EC public key. The value is optional for response data.
 */
@property (nonatomic, strong, nullable) NSData * key;

/**
 Encrypted data in Base64 format. The value is mapped
 to the `base` property.
 */
@property (nonatomic, strong, nullable) NSString * bodyBase64;
/**
 A MAC computed for key & data in Base64 format. The value is mapped
 to the `mac` property.
 */
@property (nonatomic, strong, nullable) NSString * macBase64;
/**
 An ephemeral EC public key in Base64 format. The value is mapped
 to the `mac` property.
 */
@property (nonatomic, strong, nullable) NSString * keyBase64;

@end
