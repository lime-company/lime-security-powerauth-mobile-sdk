/*
 * Copyright 2016-2017 Lime - HighTech Solutions s.r.o.
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

#import "PA2Types.h"
#import "PA2Password.h"
#import "PA2Encryptor.h"

#include <PowerAuth/PublicTypes.h>
#include <PowerAuth/Password.h>
#include <PowerAuth/Encryptor.h>

#include <cc7/ByteArray.h>
#include <cc7/objc/ObjcHelper.h>


/*
 This header contains various private interfaces, internally used
 in the PA2's Objective-C wrappper.
 */

@interface PA2Password (Private)

- (io::getlime::powerAuth::Password &) passObjRef;

@end


@interface PA2Encryptor (Private)

- (id) initWithEncryptorPtr:(io::getlime::powerAuth::Encryptor*)encryptor;

@end


@interface PA2HTTPRequestDataSignature (Private)

- (io::getlime::powerAuth::HTTPRequestDataSignature&) signatureStructRef;

@end


/**
 Converts PA2SessionSetup object into SessionSetup C++ structure.
 */
CC7_EXTERN_C void PA2SessionSetupToStruct(PA2SessionSetup * setup, io::getlime::powerAuth::SessionSetup & cpp_setup);
/**
 Returns new instance of PA2SessionSetup object, with content copied from SessionSetup C++ structure.
 */
CC7_EXTERN_C PA2SessionSetup * PA2SessionSetupToObject(const io::getlime::powerAuth::SessionSetup & cpp_setup);

/**
 Converts PA2SignatureUnlockKeys object into SignatureUnlockKeys C++ structure.
 */
CC7_EXTERN_C void PA2SignatureUnlockKeysToStruct(PA2SignatureUnlockKeys * keys, io::getlime::powerAuth::SignatureUnlockKeys & cpp_keys);
/**
 Returns new instance of PA2ActivationStatus object, with content copied from ActivationStatus C++ structure.
 */
CC7_EXTERN_C PA2ActivationStatus * PA2ActivationStatusToObject(const io::getlime::powerAuth::ActivationStatus& cpp_status);

/**
 Converts PA2HTTPRequestData object into HTTPRequestData C++ structure.
 */
CC7_EXTERN_C void PA2HTTPRequestDataToStruct(PA2HTTPRequestData * req, io::getlime::powerAuth::HTTPRequestData & cpp_req);

/**
 Converts PA2ActivationStep1Param object into ActivationStep1Param C++ structure.
 */
CC7_EXTERN_C void PA2ActivationStep1ParamToStruct(PA2ActivationStep1Param * p1, io::getlime::powerAuth::ActivationStep1Param & cpp_p1);
/**
 Returns new instance of PA2ActivationStep1Result object, with content copied from ActivationStep1Result C++ structure.
 */
CC7_EXTERN_C PA2ActivationStep1Result * PA2ActivationStep1ResultToObject(const io::getlime::powerAuth::ActivationStep1Result& cpp_r1);

/**
 Converts PA2ActivationStep2Param object into ActivationStep2Param C++ structure.
 */
CC7_EXTERN_C void PA2ActivationStep2ParamToStruct(PA2ActivationStep2Param * p2, io::getlime::powerAuth::ActivationStep2Param & cpp_p2);
/**
 Returns new instance of PA2ActivationStep2Result object, with content copied from ActivationStep2Result C++ structure.
 */
CC7_EXTERN_C PA2ActivationStep2Result * PA2ActivationStep2ResultToObject(const io::getlime::powerAuth::ActivationStep2Result& cpp_r2);

/**
 Converts PA2EncryptedMessage object into EncryptedMessage C++ structure.
 */
CC7_EXTERN_C void PA2EncryptedMessageToStruct(PA2EncryptedMessage * msg, io::getlime::powerAuth::EncryptedMessage& cpp_msg);
/**
 Returns new instance of PA2EncryptedMessage object, with content copied from EncryptedMessage C++ structure.
 */
CC7_EXTERN_C PA2EncryptedMessage * PA2EncryptedMessageToObject(const io::getlime::powerAuth::EncryptedMessage& cpp_msg);
