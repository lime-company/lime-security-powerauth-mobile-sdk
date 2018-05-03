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

#include "PasswordJNI.h"
#include "EncryptorJNI.h"
#include <PowerAuth/Session.h>
#include <PowerAuth/Debug.h>
#include <map>

// Package: io.getlime.security.powerauth.core
#define CC7_JNI_CLASS_PATH	    	"io/getlime/security/powerauth/core"
#define CC7_JNI_CLASS_PACKAGE	    io_getlime_security_powerauth_core
#define CC7_JNI_JAVA_CLASS  		Session
#define CC7_JNI_CPP_CLASS		    Session
#include <cc7/jni/JniModule.inl>

using namespace io::getlime::powerAuth;

CC7_JNI_MODULE_CLASS_BEGIN()

// ----------------------------------------------------------------------------
// Local helper functions
// ----------------------------------------------------------------------------

static bool LoadSignatureUnlockKeys(SignatureUnlockKeys & out, JNIEnv * env, jobject unlockKeys)
{
	if (!unlockKeys) {
		CC7_ASSERT(false, "SignatureUnlockKeys java object should not be null.");
		return false;
	}
	jclass keysClazz  = CC7_JNI_MODULE_FIND_CLASS("SignatureUnlockKeys");
	out.possessionUnlockKey	= cc7::jni::CopyFromJavaByteArray(env, CC7_JNI_GET_FIELD_BYTEARRAY(unlockKeys, keysClazz, "possessionUnlockKey"));
	out.biometryUnlockKey	= cc7::jni::CopyFromJavaByteArray(env, CC7_JNI_GET_FIELD_BYTEARRAY(unlockKeys, keysClazz, "biometryUnlockKey"));
	jobject userPasswordObject = CC7_JNI_GET_FIELD_OBJECT(unlockKeys, keysClazz, "userPassword", CC7_JNI_MODULE_CLASS_SIGNATURE("Password"));
	if (userPasswordObject != NULL) {
		auto cppPassword = GetCppPasswordFromJavaObject(env, userPasswordObject);
		if (!cppPassword) {
			return false;
		}
		out.userPassword = cppPassword->passwordData();
	}
	return true;
}


// ----------------------------------------------------------------------------
// Init & Destroy
// ----------------------------------------------------------------------------

//
// private native long init(SessionSetup setup)
//
CC7_JNI_METHOD_PARAMS(jlong, init, jobject setup)
{
	if (!setup) {
		CC7_ASSERT(false, "You have to provide SessionSetup object.");
		return 0;
	}
	// Copy data from java SessionSetup to backing C++ structure
	jclass setupClazz  = CC7_JNI_MODULE_FIND_CLASS("SessionSetup");
	SessionSetup cppSetup;
	cppSetup.applicationKey			= cc7::jni::CopyFromJavaString(env, CC7_JNI_GET_FIELD_STRING(setup, setupClazz, "applicationKey"));
	cppSetup.applicationSecret		= cc7::jni::CopyFromJavaString(env, CC7_JNI_GET_FIELD_STRING(setup, setupClazz, "applicationSecret"));
	cppSetup.masterServerPublicKey	= cc7::jni::CopyFromJavaString(env, CC7_JNI_GET_FIELD_STRING(setup, setupClazz, "masterServerPublicKey"));
	cppSetup.sessionIdentifier		= CC7_JNI_GET_FIELD_INT(setup, setupClazz, "sessionIdentifier");
	cppSetup.externalEncryptionKey	= cc7::jni::CopyFromJavaByteArray(env, CC7_JNI_GET_FIELD_BYTEARRAY(setup, setupClazz, "externalEncryptionKey"));

	auto session = new Session(cppSetup);
	return (jlong)session;
}

//
// private native void destroy(long handle)
//
CC7_JNI_METHOD_PARAMS(void, destroy, jlong handle)
{
	auto session = CC7_THIS_OBJ();
	if (!session || (jlong)session != handle) {
		CC7_ASSERT(false, "Internal object is already destroyed, or provided handle is not ours.");
		return;
	}
	delete session;
}

//
// public native SessionSetup getSessionSetup()
//
CC7_JNI_METHOD(jobject, getSessionSetup)
{
	auto session = CC7_THIS_OBJ();
	if (!session) {
		CC7_ASSERT(false, "Missing internal handle.");
		return NULL;
	}
	auto cppSetup = session->sessionSetup();
	if (!cppSetup) {
		return NULL;
	}
	// Copy cppResult into java result object
	jclass  resultClazz  = CC7_JNI_MODULE_FIND_CLASS("SessionSetup");
	jobject resultObject = cc7::jni::CreateJavaObject(env, CC7_JNI_MODULE_CLASS_PATH("SessionSetup"), "()V");
	CC7_JNI_SET_FIELD_STRING(resultObject, resultClazz, "applicationKey",  			cc7::jni::CopyToJavaString(env, cppSetup->applicationKey));
	CC7_JNI_SET_FIELD_STRING(resultObject, resultClazz, "applicationSecret", 		cc7::jni::CopyToJavaString(env, cppSetup->applicationSecret));
	CC7_JNI_SET_FIELD_STRING(resultObject, resultClazz, "masterServerPublicKey",	cc7::jni::CopyToJavaString(env, cppSetup->masterServerPublicKey));
	CC7_JNI_SET_FIELD_INT   (resultObject, resultClazz, "sessionIdentifier", 		cppSetup->sessionIdentifier);
	if (session->hasExternalEncryptionKey()) {
		CC7_JNI_SET_FIELD_BYTEARRAY(resultObject, resultClazz, "externalEncryptionKey",	cc7::jni::CopyToJavaByteArray(env, cppSetup->externalEncryptionKey));
	}
	return resultObject;
}

//
// public native void resetSession()
//
CC7_JNI_METHOD(void, resetSession)
{
	auto session = CC7_THIS_OBJ();
	if (session) {
		session->resetSession();
	}
}

//
// public native boolean hasDebugFeatures()
//
CC7_JNI_METHOD(jboolean, hasDebugFeatures)
{
	return HasDebugFeaturesTurnedOn();
}

//
// public native boolean hasValidSetup();
//
CC7_JNI_METHOD(jboolean, hasValidSetup)
{
	auto session = CC7_THIS_OBJ();
	return session ? session->hasValidSetup() : false;
}


// ----------------------------------------------------------------------------
// Serialization
// ----------------------------------------------------------------------------

//
// public native byte[] serializedState();
//
CC7_JNI_METHOD(jbyteArray, serializedState)
{
	auto session = CC7_THIS_OBJ();
	if (!session) {
		return NULL;
	}
	cc7::ByteArray state = session->saveSessionState();
	return cc7::jni::CopyToJavaByteArray(env, state);
}

//
// public native int deserializeState(byte[] state);
//
CC7_JNI_METHOD_PARAMS(jint, deserializeState, jbyteArray state)
{
	auto session = CC7_THIS_OBJ();
	if (!session) {
		return EC_WrongParam;
	}
	cc7::ByteArray cppState = cc7::jni::CopyFromJavaByteArray(env, state);
	return session->loadSessionState(cppState);
}


// ----------------------------------------------------------------------------
// Activation
// ----------------------------------------------------------------------------

//
// public native boolean canStartActivation();
//
CC7_JNI_METHOD(jboolean, canStartActivation)
{
	auto session = CC7_THIS_OBJ();
	return session ? session->canStartActivation() : false;
}

//
// public native boolean hasPendingActivation();
//
CC7_JNI_METHOD(jboolean, hasPendingActivation)
{
	auto session = CC7_THIS_OBJ();
	return session ? session->hasPendingActivation() : false;
}

//
// public native boolean hasValidActivation();
//
CC7_JNI_METHOD(jboolean, hasValidActivation)
{
	auto session = CC7_THIS_OBJ();
	return session ? session->hasValidActivation() : false;
}

//
// public native String getActivationIdentifier()
//
CC7_JNI_METHOD(jstring, getActivationIdentifier)
{
	auto session = CC7_THIS_OBJ();
	if (!session || !session->hasValidActivation()) {
		return NULL;
	}
	return cc7::jni::CopyToJavaString(env, session->activationIdentifier());
}

//
// public native ActivationStep1Result startActivation(ActivationStep1Param param);
//
CC7_JNI_METHOD_PARAMS(jobject, startActivation, jobject param)
{
	auto session = CC7_THIS_OBJ();
	if (!session || !param) {
		CC7_ASSERT(false, "Missing param or internal handle.");
		return NULL;
	}
	// Copy params to C++ struct
	ActivationStep1Param cppParam;
	jclass paramClazz  = CC7_JNI_MODULE_FIND_CLASS("ActivationStep1Param");
	cppParam.activationIdShort		= cc7::jni::CopyFromJavaString(env, CC7_JNI_GET_FIELD_STRING(param, paramClazz, "activationIdShort"));
	cppParam.activationOtp			= cc7::jni::CopyFromJavaString(env, CC7_JNI_GET_FIELD_STRING(param, paramClazz, "activationOtp"));
	cppParam.activationSignature	= cc7::jni::CopyFromJavaString(env, CC7_JNI_GET_FIELD_STRING(param, paramClazz, "activationSignature"));
	// Call session
	ActivationStep1Result cppResult;
	ErrorCode code = session->startActivation(cppParam, cppResult);
	// Copy cppResult into java result object
	jclass  resultClazz  = CC7_JNI_MODULE_FIND_CLASS("ActivationStep1Result");
	jobject resultObject = cc7::jni::CreateJavaObject(env, CC7_JNI_MODULE_CLASS_PATH("ActivationStep1Result"), "()V");
	CC7_JNI_SET_FIELD_INT(resultObject, resultClazz, "errorCode", code);
	if (code == EC_Ok) {
		CC7_JNI_SET_FIELD_STRING(resultObject, resultClazz, "activationNonce",  	cc7::jni::CopyToJavaString(env, cppResult.activationNonce));
		CC7_JNI_SET_FIELD_STRING(resultObject, resultClazz, "cDevicePublicKey", 	cc7::jni::CopyToJavaString(env, cppResult.cDevicePublicKey));
		CC7_JNI_SET_FIELD_STRING(resultObject, resultClazz, "applicationSignature",	cc7::jni::CopyToJavaString(env, cppResult.applicationSignature));
		CC7_JNI_SET_FIELD_STRING(resultObject, resultClazz, "ephemeralPublicKey",	cc7::jni::CopyToJavaString(env, cppResult.ephemeralPublicKey));
	}
	return resultObject;
}

//
// public native ActivationStep2Result validateActivationResponse(ActivationStep2Param param);
//
CC7_JNI_METHOD_PARAMS(jobject, validateActivationResponse, jobject param)
{
	auto session = CC7_THIS_OBJ();
	if (!session || !param) {
		CC7_ASSERT(false, "Missing param or internal handle.");
		return nullptr;
	}
	// Copy data from param jobject into cppParam.
	ActivationStep2Param cppParam;	
	jclass paramClazz  = CC7_JNI_MODULE_FIND_CLASS("ActivationStep2Param");
	cppParam.activationId				= cc7::jni::CopyFromJavaString(env, CC7_JNI_GET_FIELD_STRING(param, paramClazz, "activationId"));
	cppParam.ephemeralNonce				= cc7::jni::CopyFromJavaString(env, CC7_JNI_GET_FIELD_STRING(param, paramClazz, "ephemeralNonce"));
	cppParam.ephemeralPublicKey			= cc7::jni::CopyFromJavaString(env, CC7_JNI_GET_FIELD_STRING(param, paramClazz, "ephemeralPublicKey"));
	cppParam.encryptedServerPublicKey	= cc7::jni::CopyFromJavaString(env, CC7_JNI_GET_FIELD_STRING(param, paramClazz, "encryptedServerPublicKey"));
	cppParam.serverDataSignature		= cc7::jni::CopyFromJavaString(env, CC7_JNI_GET_FIELD_STRING(param, paramClazz, "serverDataSignature"));
	// Call C++ session
	ActivationStep2Result cppResult;
	ErrorCode code = session->validateActivationResponse(cppParam, cppResult);
	// Copy cppResult into java result object
	jclass  resultClazz  = CC7_JNI_MODULE_FIND_CLASS("ActivationStep2Result");
	jobject resultObject = cc7::jni::CreateJavaObject(env, CC7_JNI_MODULE_CLASS_PATH("ActivationStep2Result"), "()V");
	CC7_JNI_SET_FIELD_INT(resultObject, resultClazz, "errorCode", code);
	if (code == EC_Ok) {
		CC7_JNI_SET_FIELD_STRING(resultObject, resultClazz, "hkDevicePublicKey",  cc7::jni::CopyToJavaString(env, cppResult.hkDevicePublicKey));
	}
	return resultObject;
}

//
// public native int completeActivation(SignatureUnlockKeys lockKeys);
//
CC7_JNI_METHOD_PARAMS(jint, completeActivation, jobject lockKeys)
{
	auto session = CC7_THIS_OBJ();
	if (!session || !lockKeys) {
		CC7_ASSERT(false, "Missing param or internal handle.");
		return EC_WrongParam;
	}
	// Load keys into C++ structures
	SignatureUnlockKeys cppLockKeys;
	if (false == LoadSignatureUnlockKeys(cppLockKeys, env, lockKeys)) {
		return EC_WrongParam;
	}
	// Call C++ session
	ErrorCode code = session->completeActivation(cppLockKeys);
	return code;
}


// ----------------------------------------------------------------------------
// Activation status
// ----------------------------------------------------------------------------

//
// public native ActivationStatus decodeActivationStatus(String statusBlob, SignatureUnlockKeys unlockKeys);
//
CC7_JNI_METHOD_PARAMS(jobject, decodeActivationStatus, jstring statusBlob, jobject unlockKeys)
{
	auto session = CC7_THIS_OBJ();
	if (!session || !unlockKeys || !statusBlob) {
		CC7_ASSERT(false, "Missing param or internal handle.");
		return NULL;
	}
	// Load parameters into C++ structures
	std::string cppStatusBlob = cc7::jni::CopyFromJavaString(env, statusBlob);
	SignatureUnlockKeys cppUnlockKeys;
	if (false == LoadSignatureUnlockKeys(cppUnlockKeys, env, unlockKeys)) {
		return NULL;
	}
	// Call C++ session
	ActivationStatus cppStatus;
	ErrorCode code = session->decodeActivationStatus(cppStatusBlob, cppUnlockKeys, cppStatus);
	// Copy result to java object
	jclass  resultClazz  = CC7_JNI_MODULE_FIND_CLASS("ActivationStatus");
	jobject resultObject = cc7::jni::CreateJavaObject(env, CC7_JNI_MODULE_CLASS_PATH("ActivationStatus"), "()V");
	CC7_JNI_SET_FIELD_INT(resultObject, resultClazz, "errorCode", code);
	if (code == EC_Ok) {
		CC7_JNI_SET_FIELD_INT	(resultObject, resultClazz, "state", 	 	cppStatus.state);
		CC7_JNI_SET_FIELD_INT	(resultObject, resultClazz, "failCount", 	cppStatus.failCount);
		CC7_JNI_SET_FIELD_INT	(resultObject, resultClazz, "maxFailCount",	cppStatus.maxFailCount);
		CC7_JNI_SET_FIELD_LONG	(resultObject, resultClazz, "counter",	 	cppStatus.counter);
	}
	return resultObject;
}


// ----------------------------------------------------------------------------
// Data signing
// ----------------------------------------------------------------------------

//
// private native byte[] prepareKeyValueDictionaryForDataSigning(String[] keys, String[] values);
//
CC7_JNI_METHOD_PARAMS(jbyteArray, prepareKeyValueDictionaryForDataSigning, jobjectArray keys, jobjectArray values)
{
	auto session = CC7_THIS_OBJ();
	if (!session || !keys || !values) {
		CC7_ASSERT(false, "Missing param or internal handle.");
		return NULL;
	}
	// Copy java keys and values into std::map<string,string>
	jsize keysCount = env->GetArrayLength(keys);
	if (keysCount != env->GetArrayLength(values)) {
		CC7_ASSERT(false, "Different number of keys and values.");
		return NULL;
	}
	std::map<std::string, std::string> cppMap;
	for (jsize index = 0; index < keysCount; index++) {
		jstring javaKey      = (jstring) env->GetObjectArrayElement(keys, index);
		jstring javaValue    = (jstring) env->GetObjectArrayElement(values, index);
		std::string cppKey   = cc7::jni::CopyFromJavaString(env, javaKey);
		std::string cppValue = cc7::jni::CopyFromJavaString(env, javaValue);
		cppMap[cppKey] = cppValue;
	}
	// Call C++ session and return byte[]
	cc7::ByteArray cppResult = Session::prepareKeyValueMapForDataSigning(cppMap);
	return cc7::jni::CopyToJavaByteArray(env, cppResult);
}

//
// public native SignatureResult signHTTPRequest(SignatureRequest request, SignatureUnlockKeys unlockKeys, int signatureFactor);
//
CC7_JNI_METHOD_PARAMS(jobject, signHTTPRequest, jobject request, jobject unlockKeys, jint signatureFactor)
{
	auto session = CC7_THIS_OBJ();
	if (!session || !request || !unlockKeys) {
		CC7_ASSERT(false, "Missing param or internal handle.");
		return NULL;
	}	
	// Load parameters into C++ objects 
	HTTPRequestData cppRequest;
	jclass requestClazz		= CC7_JNI_MODULE_FIND_CLASS("SignatureRequest");
	cppRequest.body			= cc7::jni::CopyFromJavaByteArray(env, CC7_JNI_GET_FIELD_BYTEARRAY(request, requestClazz, "body"));
	cppRequest.method		= cc7::jni::CopyFromJavaString(env, CC7_JNI_GET_FIELD_STRING(request, requestClazz, "method"));
	cppRequest.uri			= cc7::jni::CopyFromJavaString(env, CC7_JNI_GET_FIELD_STRING(request, requestClazz, "uri"));
	cppRequest.offlineNonce	= cc7::jni::CopyFromJavaString(env, CC7_JNI_GET_FIELD_STRING(request, requestClazz, "nonce"));
	SignatureFactor cppSignatureFactor = (SignatureFactor)signatureFactor;
	SignatureUnlockKeys cppUnlockKeys;
	if (false == LoadSignatureUnlockKeys(cppUnlockKeys, env, unlockKeys)) {
		return NULL;
	}
	// Call C++ session
	HTTPRequestDataSignature cppSignature;
	ErrorCode code = session->signHTTPRequestData(cppRequest, cppUnlockKeys, cppSignatureFactor, cppSignature);
	// Copy result to java object
	jclass  resultClazz  = CC7_JNI_MODULE_FIND_CLASS("SignatureResult");
	jobject resultObject = cc7::jni::CreateJavaObject(env, CC7_JNI_MODULE_CLASS_PATH("SignatureResult"), "()V");
	CC7_JNI_SET_FIELD_INT(resultObject, resultClazz, "errorCode", code);
	if (code == EC_Ok) {
		CC7_JNI_SET_FIELD_STRING(resultObject, resultClazz, "authHeaderValue",  cc7::jni::CopyToJavaString(env, cppSignature.buildAuthHeaderValue()));
		CC7_JNI_SET_FIELD_STRING(resultObject, resultClazz, "signatureCode",  	cc7::jni::CopyToJavaString(env, cppSignature.signature));
	}
	return resultObject;
}

//
// public native String getHttpAuthHeaderName();
//
CC7_JNI_METHOD(jstring, getHttpAuthHeaderName)
{
	auto session = CC7_THIS_OBJ();
	if (!session) {
		return NULL;
	}
	return cc7::jni::CopyToJavaString(env, session->httpAuthHeaderName());
}

//
// public native int verifyServerSignedData(SignedData signedData);
//
CC7_JNI_METHOD_PARAMS(jint, verifyServerSignedData, jobject signedData)
{
	auto session = CC7_THIS_OBJ();
	if (!session || !signedData) {
		CC7_ASSERT(false, "Missing signedData or internal handle.");
		return EC_WrongParam;
	}
	// Load parameters into C++ objects
	jclass requestClazz		    = CC7_JNI_MODULE_FIND_CLASS("SignedData");
	// Get type of key
	bool useMasterKey           = CC7_JNI_GET_FIELD_BOOL(signedData, requestClazz, "useMasterKey");
	// Prepare cpp structure
	SignedData cppSignedData;
	cppSignedData.signingKey    = useMasterKey ? SignedData::ECDSA_MasterServerKey : SignedData::ECDSA_PersonalizedKey;
	cppSignedData.data		    = cc7::jni::CopyFromJavaByteArray(env, CC7_JNI_GET_FIELD_BYTEARRAY(signedData, requestClazz, "data"));
	cppSignedData.signature	    = cc7::jni::CopyFromJavaByteArray(env, CC7_JNI_GET_FIELD_BYTEARRAY(signedData, requestClazz, "signature"));
	return (jint) session->verifyServerSignedData(cppSignedData);
}

// ----------------------------------------------------------------------------
// Signature keys management
// ----------------------------------------------------------------------------

//
// public native int changeUserPassword(Password oldPassword, Password newPassword);
//
CC7_JNI_METHOD_PARAMS(jint, changeUserPassword, jobject oldPassword, jobject newPassword)
{
	auto session = CC7_THIS_OBJ();
	if (!session || !oldPassword || !newPassword) {
		CC7_ASSERT(false, "Missing param or internal handle.");
		return EC_WrongParam;
	}
	Password * oldPasswordObj = GetCppPasswordFromJavaObject(env, oldPassword);
	Password * newPasswordObj = GetCppPasswordFromJavaObject(env, newPassword);
	if (!oldPasswordObj || !newPasswordObj) {
		return EC_WrongParam;
	}
	// Call C++ session
	return session->changeUserPassword(oldPasswordObj->passwordData(), newPasswordObj->passwordData());
}

//
// public native int addBiometryFactor(String cVaultKey, SignatureUnlockKeys unlockKeys);
//
CC7_JNI_METHOD_PARAMS(jint, addBiometryFactor, jstring cVaultKey, jobject unlockKeys)
{
	auto session = CC7_THIS_OBJ();
	if (!session || !cVaultKey || !unlockKeys) {
		CC7_ASSERT(false, "Missing param or internal handle.");
		return EC_WrongParam;
	}
	// Load parameters into C++ objects 
	std::string cppCVaultKey = cc7::jni::CopyFromJavaString(env, cVaultKey);
	SignatureUnlockKeys cppUnlockKeys;
	if (false == LoadSignatureUnlockKeys(cppUnlockKeys, env, unlockKeys)) {
		return EC_WrongParam;
	}
	// Call C++ session
	return session->addBiometryFactor(cppCVaultKey, cppUnlockKeys);
}

//
// public native boolean hasBiometryFactor();
//
CC7_JNI_METHOD(jboolean, hasBiometryFactor)
{
	auto session = CC7_THIS_OBJ();
	if (!session) {
		CC7_ASSERT(false, "Missing internal handle.");
		return false;
	}
	// Get the status. We don't care about error returned from the method,
	// because the problem is already dumped to the debug log and
	// the result is always false in case of error.
	bool result;
	session->hasBiometryFactor(result);
	return result;
}

//
// public native int removeBiometryFactor();
//
CC7_JNI_METHOD(jint, removeBiometryFactor)
{
	auto session = CC7_THIS_OBJ();
	if (!session) {
		CC7_ASSERT(false, "Missing internal handle.");
		return EC_WrongParam;
	}
	return session->removeBiometryFactor();
}


// ----------------------------------------------------------------------------
// Vault operations
// ----------------------------------------------------------------------------

//
// public native byte[] deriveCryptographicKeyFromVaultKey(String cVaultKey, SignatureUnlockKeys unlockKeys, long keyIndex);
//
CC7_JNI_METHOD_PARAMS(jbyteArray, deriveCryptographicKeyFromVaultKey, jstring cVaultKey, jobject unlockKeys, jlong keyIndex)
{
	auto session = CC7_THIS_OBJ();
	if (!session || !cVaultKey || !unlockKeys) {
		CC7_ASSERT(false, "Missing param or internal handle.");
		return NULL;
	}
	// Load parameters into C++ objects 
	std::string cppCVaultKey = cc7::jni::CopyFromJavaString(env, cVaultKey);
	SignatureUnlockKeys cppUnlockKeys;
	if (false == LoadSignatureUnlockKeys(cppUnlockKeys, env, unlockKeys)) {
		return NULL;
	}
	cc7::ByteArray derivedKey;
	ErrorCode code = session->deriveCryptographicKeyFromVaultKey(cppCVaultKey, cppUnlockKeys, (cc7::U64)keyIndex, derivedKey);
	if (code != EC_Ok) {
		return NULL;
	}
	return cc7::jni::CopyToJavaByteArray(env, derivedKey);
}

//
// public native byte[] signDataWithDevicePrivateKey(String cVaultKey, SignatureUnlockKeys unlockKeys, byte[] data);
//
CC7_JNI_METHOD_PARAMS(jbyteArray, signDataWithDevicePrivateKey, jstring cVaultKey, jobject unlockKeys, jbyteArray data)
{
	auto session = CC7_THIS_OBJ();
	if (!session || !cVaultKey || !unlockKeys || !data) {
		CC7_ASSERT(false, "Missing param or internal handle.");
		return NULL;
	}
	// Load parameters into C++ objects 
	std::string cppCVaultKey = cc7::jni::CopyFromJavaString(env, cVaultKey);
	cc7::ByteArray cppData   = cc7::jni::CopyFromJavaByteArray(env, data);
	SignatureUnlockKeys cppUnlockKeys;
	if (false == LoadSignatureUnlockKeys(cppUnlockKeys, env, unlockKeys)) {
		return NULL;
	}
	cc7::ByteArray signature;
	ErrorCode code = session->signDataWithDevicePrivateKey(cppCVaultKey, cppUnlockKeys, cppData, signature);
	if (code != EC_Ok) {
		return NULL;
	}
	return cc7::jni::CopyToJavaByteArray(env, signature);
}


// ----------------------------------------------------------------------------
// External Encryption Key
// ----------------------------------------------------------------------------

//
// public native bool hasExternalEncryptionKey();
//
CC7_JNI_METHOD(jboolean, hasExternalEncryptionKey)
{
	auto session = CC7_THIS_OBJ();
	if (!session) {
		CC7_ASSERT(false, "Missing internal handle.");
		return false;
	}
	return session->hasExternalEncryptionKey();
}

//
// public native int setExternalEncryptionKey(byte[] externalEncryptionKey);
//
CC7_JNI_METHOD_PARAMS(jint, setExternalEncryptionKey, jbyteArray eek)
{
	auto session = CC7_THIS_OBJ();
	if (!session) {
		CC7_ASSERT(false, "Missing internal handle.");
		return EC_WrongParam;
	}
	cc7::ByteArray cppEEK   = cc7::jni::CopyFromJavaByteArray(env, eek);
	ErrorCode code = session->setExternalEncryptionKey(cppEEK);
	return code;
}

//
// public native int addExternalEncryptionKey(byte[] externalEncryptionKey);
//
CC7_JNI_METHOD_PARAMS(jint, addExternalEncryptionKey, jbyteArray eek)
{
	auto session = CC7_THIS_OBJ();
	if (!session) {
		CC7_ASSERT(false, "Missing internal handle.");
		return EC_WrongParam;
	}
	cc7::ByteArray cppEEK   = cc7::jni::CopyFromJavaByteArray(env, eek);
	ErrorCode code = session->addExternalEncryptionKey(cppEEK);
	return code;
}

// 
// public native int removeExternalEncryptionKey();
//
CC7_JNI_METHOD(jboolean, removeExternalEncryptionKey)
{
	auto session = CC7_THIS_OBJ();
	if (!session) {
		CC7_ASSERT(false, "Missing internal handle.");
		return false;
	}
	return session->removeExternalEncryptionKey();
}

// ----------------------------------------------------------------------------
// E2EE
// ----------------------------------------------------------------------------

//
// public native Encryptor createNonpersonalizedEncryptor(byte[] sessionIndex);
//
CC7_JNI_METHOD_PARAMS(jobject, createNonpersonalizedEncryptor, jbyteArray sessionIndex)
{
	auto session = CC7_THIS_OBJ();
	ErrorCode code = EC_WrongParam;
	Encryptor * encryptor = NULL;
	if (CC7_CHECK(session != NULL, "Missing internal handle.")) {
		auto cpp_sessionIndex = cc7::jni::CopyFromJavaByteArray(env, sessionIndex);
		std::tie(code, encryptor) = session->createNonpersonalizedEncryptor(cpp_sessionIndex);		
	}
	return CreateJavaEncryptorForCppClass(env, encryptor, code);
}

//
// public native Encryptor createPersonalizedEncryptor(byte[] sessionIndex, SignatureUnlockKeys unlockKeys);
//
CC7_JNI_METHOD_PARAMS(jobject, createPersonalizedEncryptor, jbyteArray sessionIndex, jobject unlockKeys)
{
	auto session = CC7_THIS_OBJ();
	ErrorCode code = EC_WrongParam;
	Encryptor * encryptor = NULL;
	if (CC7_CHECK(session != NULL, "Missing internal handle.")) {
		auto cpp_sessionIndex = cc7::jni::CopyFromJavaByteArray(env, sessionIndex);
		SignatureUnlockKeys cpp_unlockKeys;
		if (LoadSignatureUnlockKeys(cpp_unlockKeys, env, unlockKeys)) {
			std::tie(code, encryptor) = session->createPersonalizedEncryptor(cpp_sessionIndex, cpp_unlockKeys);
		}		
	}
	return CreateJavaEncryptorForCppClass(env, encryptor, code);
}

// ----------------------------------------------------------------------------
// Utilities
// ----------------------------------------------------------------------------

//
// public native byte[] normalizeSignatureUnlockKeyFromData(byte[] arbitraryData);
//
CC7_JNI_METHOD_PARAMS(jbyteArray, normalizeSignatureUnlockKeyFromData, jbyteArray arbitraryData)
{
	cc7::ByteArray cppData = cc7::jni::CopyFromJavaByteArray(env, arbitraryData);
	return cc7::jni::CopyToJavaByteArray(env, Session::normalizeSignatureUnlockKeyFromData(cppData));
}

//
// public native byte[] generateSignatureUnlockKey();
//
CC7_JNI_METHOD(jbyteArray, generateSignatureUnlockKey)
{
	return cc7::jni::CopyToJavaByteArray(env, Session::generateSignatureUnlockKey());
}

CC7_JNI_MODULE_CLASS_END()
