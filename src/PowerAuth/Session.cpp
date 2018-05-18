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

#include <PowerAuth/Session.h>
#include <PowerAuth/Encryptor.h>

#include <cc7/Base64.h>
#include "protocol/ProtocolUtils.h"
#include "protocol/Constants.h"
#include "crypto/CryptoUtils.h"
#include "utils/URLEncoding.h"
#include "utils/DataReader.h"
#include "utils/DataWriter.h"
#include <algorithm>

using namespace cc7;

namespace io
{
namespace getlime
{
namespace powerAuth
{
	
#define LOCK_GUARD() std::lock_guard<std::recursive_mutex> _lock_guard(_lock)
	
	// MARK: Construction / Destruction -
	
	Session::Session(const SessionSetup & setup) :
		_state(SS_Empty),
		_setup(setup),
		_pd(nullptr),
		_ad(nullptr)
	{
		if (protocol::ValidateSessionSetup(_setup, false)) {
			CC7_LOG("Session %p, %d: Object created.", this, sessionIdentifier());
		} else {
			_state = SS_Invalid;
			CC7_LOG("Session %p, %d: Object created, but SessionSetup is invalid!", this, sessionIdentifier());
		}
	}
	
	Session::~Session()
	{
		delete _pd;
		delete _ad;
		
		CC7_LOG("Session %p, %d: Object destroyed.", this, sessionIdentifier());
	}
	
	void Session::resetSession()
	{
		LOCK_GUARD();
		commitNewPersistentState(nullptr, SS_Empty);
	}
	
	const SessionSetup * Session::sessionSetup() const
	{
		LOCK_GUARD();
		return hasValidSetup() ? &_setup : nullptr;
	}
	
	cc7::U32 Session::sessionIdentifier() const
	{
		LOCK_GUARD();
		return hasValidSetup() ? _setup.sessionIdentifier : 0;
	}
	
	
	
	// MARK: - State probing -
	
	bool Session::hasValidSetup() const
	{
		LOCK_GUARD();
		return _state >= SS_Empty;
	}
	
	bool Session::canStartActivation() const
	{
		LOCK_GUARD();
		if (_state == SS_Empty) {
			if (CC7_CHECK(_pd == nullptr && _ad == nullptr, "Internal error. PD should be null when state is SS_Empty")) {
				return true;
			}
		}
		return false;
	}
	
	bool Session::hasPendingActivation() const
	{
		LOCK_GUARD();
		if (_state == SS_Activation1 || _state == SS_Activation2) {
			if (CC7_CHECK(_pd == nullptr && _ad != nullptr, "Internal error. Only AD should be valid during the pending activation.")) {
				return true;
			}
		}
		return false;
	}
	
	bool Session::hasValidActivation() const
	{
		LOCK_GUARD();
		if (_state == SS_Activated && hasValidSetup()) {
			if (CC7_CHECK(_pd != nullptr && _ad == nullptr, "Internal error. Only PD & setup should be valid when activated.")) {
				return true;
			}
		}
		return false;
	}
	
	
	
	// MARK: - Serialization -
	
	const cc7::byte HAS_PERSISTENT_DATA = 1 << 1;
	const cc7::byte DATA_TAG = 'P';
	const cc7::byte DATA_VER = 'A';
	
	
	cc7::ByteArray Session::saveSessionState() const
	{
		LOCK_GUARD();
		cc7:byte flags = 0;
		if (hasValidActivation()) {
			flags |= HAS_PERSISTENT_DATA;
		}
		utils::DataWriter writer;
		
		writer.openVersion(DATA_TAG, DATA_VER);
		writer.writeByte(flags);
		
		if (flags & HAS_PERSISTENT_DATA) {
			protocol::SerializePersistentData(*_pd, writer);
		}
		writer.closeVersion();
		
		return writer.serializedData();
	}
	
	ErrorCode Session::loadSessionState(const cc7::ByteRange & serialized_state)
	{
		LOCK_GUARD();
		utils::DataReader reader(serialized_state);
		cc7::byte flags = 0;
		
		bool has_data  = false;
		auto new_data = new protocol::PersistentData();
		
		bool result = reader.openVersion(DATA_TAG, DATA_VER) &&
					  reader.readByte(flags);
		
		if (result && (flags != 'M')) {
			if (flags & HAS_PERSISTENT_DATA) {
				result = result && protocol::DeserializePersistentData(*new_data, reader);
				has_data = result;
			}
		} else {
			// DATA_MIGRATION_TAG
			result = protocol::TryDeserializeOldPersistentData(*new_data, reader);
			has_data = result && !new_data->activationId.empty();
		}
		
		State new_state = has_data ? SS_Activated : SS_Empty;
		commitNewPersistentState(new_data, new_state);
		return result ? EC_Ok : EC_WrongParam;
	}
	
	
	
	// MARK: - Activation -
	
	std::string Session::activationIdentifier() const
	{
		LOCK_GUARD();
		if (hasValidActivation()) {
			return _pd->activationId;
		}
		return std::string();
	}
	
	std::string Session::activationFingerprint() const
	{
		LOCK_GUARD();
		std::string result;
		if (hasValidActivation()) {
			crypto::BNContext ctx;
			EC_KEY * public_key = crypto::ECC_ImportPublicKey(nullptr, _pd->devicePublicKey, ctx);
			auto coord_x = crypto::ECC_ExportPublicKeyToNormalizedForm(public_key, ctx);
			if (!coord_x.empty()) {
				result = protocol::CalculateDecimalizedSignature(crypto::SHA256(coord_x));
				if (result.size() != protocol::ACTIVATION_FINGERPRINT_SIZE) {
					result.clear();
				}
			}
			EC_KEY_free(public_key);
		}
		return result;
	}
	
	ErrorCode Session::startActivation(const ActivationStep1Param & param, ActivationStep1Result & result)
	{
		LOCK_GUARD();
		// Validate state & parameters
		if (!hasValidSetup()) {
			CC7_LOG("Session %p, %d: Step 1: Session has no valid setup.", this, sessionIdentifier());
			return EC_WrongState;
		}
		if (!canStartActivation()) {
			CC7_LOG("Session %p, %d: Step 1: Called in wrong state.", this, sessionIdentifier());
			return EC_WrongState;
		}
		if (param.activationOtp.empty() || param.activationIdShort.empty()) {
			CC7_LOG("Session %p, %d: Step 1: Wrong parameters.", this, sessionIdentifier());
			return EC_WrongParam;
		}
		
		auto error_code = EC_Encryption;
		auto ad = new protocol::ActivationData();
		
		do {
			crypto::BNContext ctx;
			
			// Import master server public key & try to validate OTP+ShortID signature
			ad->masterServerPublicKey = crypto::ECC_ImportPublicKeyFromB64(nullptr, _setup.masterServerPublicKey, ctx);
			if (nullptr == ad->masterServerPublicKey) {
				CC7_LOG("Session %p, %d: Step 1: Master server public key is invalid.", this, sessionIdentifier());
				break;
			}
			if (!protocol::ValidateShortIdAndOtpSignature(param.activationIdShort, param.activationOtp, param.activationSignature, ad->masterServerPublicKey)) {
				CC7_LOG("Session %p, %d: Step 1: Invalid OTP+ShortID signature.", this, sessionIdentifier());
				break;
			}
			
			// Generate device private & public key
			ad->devicePrivateKey = crypto::ECC_GenerateKeyPair();
			if (nullptr == ad->devicePrivateKey) {
				CC7_LOG("Session %p, %d: Step 1: Private key pair generator failed.", this, sessionIdentifier());
				break;
			}
			ad->devicePublicKeyData = crypto::ECC_ExportPublicKey(ad->devicePrivateKey, ctx);
			ad->devicePublicKeyCoordX = crypto::ECC_ExportPublicKeyToNormalizedForm(ad->devicePrivateKey, ctx);
			if (ad->devicePublicKeyData.empty() || ad->devicePublicKeyCoordX.empty()) {
				CC7_LOG("Session %p, %d: Step 1: Unable to export public key.", this, sessionIdentifier());
				break;
			}
			
			// Encrypt device public key with expanded OTP and ephemeral secret. The 'EncryptDevicePublicKey'
            // calculates expanded OTP key internally, ephemeral secret is calculated using server public key and
            // ephemeral private key
			ad->activationNonce = crypto::GetRandomData(protocol::ACTIVATION_NONCE_SIZE, true);
            ad->ephemeralDeviceKey = crypto::ECC_GenerateKeyPair();
			cc7::ByteArray c_device_public_key = protocol::EncryptDevicePublicKey(*ad, param.activationIdShort, param.activationOtp);
			if (c_device_public_key.empty()) {
				CC7_LOG("Session %p, %d: Step 1: Unable to encrypt device public key.", this, sessionIdentifier());
				break;
			}
			
			// Store output values in result structure, in Base64 format
            result.ephemeralPublicKey = crypto::ECC_ExportPublicKeyToB64(ad->ephemeralDeviceKey);
			result.activationNonce  = cc7::ToBase64String(ad->activationNonce);
			result.cDevicePublicKey = cc7::ToBase64String(c_device_public_key);
			
			// Last step, calculate signature of this application
			result.applicationSignature = protocol::CalculateApplicationSignature(param.activationIdShort, result.activationNonce, result.cDevicePublicKey,
																				  _setup.applicationKey, _setup.applicationSecret);
			if (result.applicationSignature.empty()) {
				CC7_LOG("Session %p, %d: Step 1: Unable to calculate application signature.", this, sessionIdentifier());
				break;
			}
			
			// Finally, everything is OK
			error_code = EC_Ok;
			
		} while (false);
		
		if (error_code == EC_Ok) {
			// Keep activation data for other steps
			_ad = ad;
			changeState(SS_Activation1);
		} else {
			// Activation failed, delete AD structure
			delete ad;
		}
		return error_code;
	}
	
	ErrorCode Session::validateActivationResponse(const ActivationStep2Param & param, ActivationStep2Result & result)
	{
		LOCK_GUARD();
		// Validate state & parameters
		if (!hasPendingActivation() || _state != SS_Activation1) {
			CC7_LOG("Session %p, %d: Step 2: Called in wrong state.", this, sessionIdentifier());
			return EC_WrongState;
		}
		if (param.activationId.empty() ||
			param.ephemeralNonce.empty() ||
			param.ephemeralPublicKey.empty() ||
			param.encryptedServerPublicKey.empty() ||
			param.serverDataSignature.empty()) {
			CC7_LOG("Session %p, %d: Step 2: Missing input parameter.", this, sessionIdentifier());
			return EC_WrongState;
		}
		
		auto error_code = EC_Encryption;
		do {
			crypto::BNContext ctx;
			
			cc7::ByteArray ephemeral_nonce, ephemeral_public_key;
			cc7::ByteArray c_server_public_key;
			cc7::ByteArray server_data_signature;
			
			// Try to import all B64 encoded input parameters.
			bool b_result;
			b_result = ephemeral_nonce.readFromBase64String(param.ephemeralNonce);
			b_result = b_result && ephemeral_public_key.readFromBase64String(param.ephemeralPublicKey);
			b_result = b_result && c_server_public_key.readFromBase64String(param.encryptedServerPublicKey);
			b_result = b_result && server_data_signature.readFromBase64String(param.serverDataSignature);
			if (!b_result) {
				// Note that we treat all B64 decode failures as an encryption error.
				CC7_LOG("Session %p, %d: Step 2: Base64 decode failed.", this, sessionIdentifier());
				break;
			}
			// Validate provided data
			if (!protocol::ValidateActivationDataSignature(param.activationId, param.encryptedServerPublicKey, server_data_signature, _ad->masterServerPublicKey)) {
				CC7_LOG("Session %p, %d: Step 2: Invalid signature.", this, sessionIdentifier());
				break;
			}
			// Decrypt server key
			if (!protocol::DecryptServerPublicKey(*_ad, ephemeral_public_key, c_server_public_key, ephemeral_nonce)) {
				CC7_LOG("Session %p, %d: Step 2: Unable to decrypt server public key.", this, sessionIdentifier());
				break;
			}
			// Now we have all required information and can calculate ECDH shared secret
			_ad->masterSharedSecret = protocol::ReduceSharedSecret(crypto::ECDH_SharedSecret(_ad->serverPublicKey, _ad->devicePrivateKey));
			if (_ad->masterSharedSecret.size() != protocol::SIGNATURE_KEY_SIZE) {
				// Shared secret calculation failed. Probably on an allocation failure.
				CC7_LOG("Session %p, %d: Step 2: Shared secret calculation failed.", this, sessionIdentifier());
				break;
			}
			// So far so good, the last step is decimalization of device's public key
			result.activationFingerprint = protocol::CalculateDecimalizedSignature(crypto::SHA256(_ad->devicePublicKeyCoordX));
			if (result.activationFingerprint.length() != protocol::ACTIVATION_FINGERPRINT_SIZE) {
				CC7_LOG("Session %p, %d: Step 2: Unable to calculate decimalized signature.", this, sessionIdentifier());
				break;
			}
			
			// Everything is OK, keep activation identifier
			_ad->activationId = param.activationId;
			error_code = EC_Ok;
			
		} while (false);
		
		if (error_code == EC_Ok) {
			// Everything is OK, switch to Activation2 state
			changeState(SS_Activation2);
		} else {
			// Activation failed, reset the session
			resetSession();
		}
		return error_code;
	}
	
	ErrorCode Session::completeActivation(const SignatureUnlockKeys & keys)
	{
		LOCK_GUARD();
		// Validate state & parameters
		if (!hasPendingActivation() || _state != SS_Activation2) {
			CC7_LOG("Session %p, %d: Step 3: Called in wrong state.", this, sessionIdentifier());
			return EC_WrongState;
		}
		if (!protocol::ValidateUnlockKeys(keys, eek(), protocol::SF_FirstLock)) {
			CC7_LOG("Session %p, %d: Step 3: Wrong signature protection keys.", this, sessionIdentifier());
			return EC_WrongParam;
		}
		auto error_code = EC_Encryption;
		auto pd = new protocol::PersistentData();
		do {
			// Keep all required information in the PD
			pd->signatureCounter	= 0;
			pd->activationId		= _ad->activationId;
			pd->passwordIterations	= protocol::PBKDF2_PASS_ITERATIONS;
			pd->passwordSalt		= crypto::GetRandomData(protocol::PBKDF2_SALT_SIZE, true);
			pd->devicePublicKey		= _ad->devicePublicKeyData;
			pd->serverPublicKey		= _ad->serverPublicKeyData;
			pd->flagsU32			= 0;
			// Keep information about external key usage in the flags
			pd->flags.usesExternalKey = eek() ? 1 : 0;
			
			// Derive all required keys from master shared secret.
			protocol::SignatureKeys plain_keys;
			cc7::ByteArray vault_key;
			if (!protocol::DeriveAllSecretKeys(plain_keys, vault_key, _ad->masterSharedSecret)) {
				CC7_LOG("Session %p, %d: Step 3: Unable to derive secret keys.", this, sessionIdentifier());
				break;
			}
			protocol::SignatureUnlockKeysReq lock_request(protocol::SF_FirstLock, &keys, eek(), &pd->passwordSalt, pd->passwordIterations);
			if (!protocol::LockSignatureKeys(pd->sk, plain_keys, lock_request)) {
				CC7_LOG("Session %p, %d: Step 3: Unable to protect secret keys.", this, sessionIdentifier());
				break;
			}
			
			cc7::ByteArray device_private_key_data = crypto::ECC_ExportPrivateKey(_ad->devicePrivateKey);
			if (device_private_key_data.empty()) {
				CC7_LOG("Session %p, %d: Step 3: Device private key export failed.", this, sessionIdentifier());
				break;
			}
			pd->cDevicePrivateKey = crypto::AES_CBC_Encrypt_Padding(vault_key, protocol::ZERO_IV, device_private_key_data);
			
			// Final step is PD validation. If this step fails, then there's an internal problem.
			if (!protocol::ValidatePersistentData(*pd)) {
				CC7_LOG("Session %p, %d: Step 3: Persistent data is invalid.", this, sessionIdentifier());
				break;
			}
			
			// Everything is OK.
			error_code = EC_Ok;
			
		} while (false);
		
		if (error_code == EC_Ok) {
			// Everything is OK, commit new persistent data with a Activated state.
			commitNewPersistentState(pd, SS_Activated);
		} else {
			// An error occured, rollback everything to state before the activation
			commitNewPersistentState(pd, SS_Empty);
		}
		return error_code;
	}
	
	
	
	// MARK: - Status -
	
	ErrorCode Session::decodeActivationStatus(const std::string & status_blob, const SignatureUnlockKeys & keys, ActivationStatus & status) const
	{
		LOCK_GUARD();
		if (!hasValidActivation()) {
			CC7_LOG("Session %p, %d: Status: Called in wrong state.", this, sessionIdentifier());
			return EC_WrongState;
		}
		if (status_blob.empty()) {
			CC7_LOG("Session %p, %d: Status: Missing status blob.", this, sessionIdentifier());
			return EC_WrongParam;
		}
		protocol::SignatureKeys signature_keys;
		protocol::SignatureUnlockKeysReq unlock_request(protocol::SF_Transport, &keys, eek(), nullptr, 0);
		if (!protocol::UnlockSignatureKeys(signature_keys, _pd->sk, unlock_request)) {
			CC7_LOG("Session %p, %d: Status: You have to provide valid possession key.", this, sessionIdentifier());
			return EC_WrongParam;
		}
		// Decode blob from B64 string
		cc7::ByteArray encrypted_status_blob;
		bool result = encrypted_status_blob.readFromBase64String(status_blob);
		if (encrypted_status_blob.size() != protocol::STATUS_BLOB_SIZE || !result) {
			// Considered as an attack on protocol
			return EC_Encryption;
		}
		// Decrypt blob and initialize reader for data parsing.
		utils::DataReader reader(crypto::AES_CBC_Decrypt(signature_keys.transportKey, protocol::ZERO_IV, encrypted_status_blob));
		cc7::ByteRange hdr;
		cc7::byte state = 0xdd, fail_ctr = 0xdd, max_fail_ctr = 0xdd;
		cc7::U64 counter = 0;
		result = reader.readMemoryRange(hdr, 4) &&
				 reader.readByte(state) &&
				 reader.readU64(counter) &&
				 reader.readByte(fail_ctr) &&
				 reader.readByte(max_fail_ctr);
		if (!result) {
			return EC_Encryption;
		}
		if (hdr[0] != 0xDE || hdr[1] != 0xC0 || hdr[2] != 0xDE || hdr[3] != 0xD1) {
			return EC_Encryption;
		}
		if (state < ActivationStatus::Created || state > ActivationStatus::Removed) {
			return EC_Encryption;
		}
		status.state        = static_cast<ActivationStatus::State>(state);
		status.failCount    = fail_ctr;
		status.maxFailCount = max_fail_ctr;
		status.counter      = counter;
		
		return EC_Ok;
	}
	
	
	// MARK: - Data signing -
	
	cc7::ByteArray Session::prepareKeyValueMapForDataSigning(const std::map<std::string, std::string> & map)
	{
		// Create a vector of keys
		std::vector<const std::string *> keys;
		keys.reserve(map.size());
		size_t expected_result_size = 0;
		for (auto && kvpair : map) {
			expected_result_size += 2 + kvpair.first.length() + kvpair.second.length();
			keys.push_back(&kvpair.first);
		}
		// Sort that keys
		std::sort(keys.begin(), keys.end(), [](const std::string * a, const std::string * b) {
			return a->compare(*b) < 0;
		});
		// Concat sorted keys & values into: 'key1=value1&keyN=valueN' byte blob
		cc7::ByteArray result;
		result.reserve(expected_result_size);
		for (auto && key_ptr : keys) {
			const std::string & key   = *key_ptr;
			const std::string & value = map.find(key)->second;
			if (!result.empty()) {
				result.append('&');
			}
			result.append(utils::ConvertStringToUrlEncodedData(key));
			result.append('=');
			result.append(utils::ConvertStringToUrlEncodedData(value));
		}
		return result;
	}
		
	ErrorCode Session::signHTTPRequestData(const HTTPRequestData & request,
										   const SignatureUnlockKeys & keys, SignatureFactor signature_factor,
										   HTTPRequestDataSignature & out)
	{
		LOCK_GUARD();
		// Validate session's state & parameters
		if (!hasValidActivation()) {
			CC7_LOG("Session %p, %d: Sign: There's no valid activation.", this, sessionIdentifier());
			return EC_WrongState;
		}
		if (!request.hasValidData()) {
			CC7_LOG("Session %p, %d: Sign: Wrong request data.", this, sessionIdentifier());
			return EC_WrongParam;
		}
		out.factor = protocol::ConvertSignatureFactorToString(signature_factor);
		if (out.factor.empty()) {
			CC7_LOG("Session %p, %d: Sign: Wrong signature factor 0x%04x.", this, sessionIdentifier(), signature_factor);
			return EC_WrongParam;
		}
		// Check combination of offlineNonce & vaultUnlock.
		bool vault_unlock = (signature_factor & SF_PrepareForVaultUnlock) != 0;
		if (vault_unlock && !request.offlineNonce.empty()) {
			CC7_LOG("Session %p, %d: Sign: Vault unlock should not be combined with offlineNonce.", this, sessionIdentifier());
			return EC_WrongParam;
		}
		
		// Re-seed OpenSSL's PRNG.
		crypto::ReseedPRNG();
		
		// Get NONCE from request structure, or generate a new one.
		cc7::ByteArray nonce;
		if (!request.isOfflineRequest()) {
			nonce = crypto::GetRandomData(protocol::SIGNATURE_KEY_SIZE, true);
			out.nonce = nonce.base64String();
		} else {
			if (!cc7::Base64_Decode(request.offlineNonce, 0, nonce)) {
				CC7_LOG("Session %p, %d: Sign: request.offlineNonce is invalid.", this, sessionIdentifier());
				return EC_Encryption;
			}
			out.nonce = request.offlineNonce;	// already in valid Base64 format
		}
		
		// Unlock keys. This also validates whether the provided unlock keys are present or not.
		protocol::SignatureKeys plain_keys;
		protocol::SignatureUnlockKeysReq unlock_request(signature_factor, &keys, eek(), &_pd->passwordSalt, _pd->passwordIterations);
		if (!protocol::UnlockSignatureKeys(plain_keys, _pd->sk, unlock_request)) {
			CC7_LOG("Session %p, %d: Sign: Unable to unlock signature keys.", this, sessionIdentifier());
			return EC_Encryption;
		}
		
		// Normalize data and calculate signature
		const std::string & app_secret = request.isOfflineRequest() ? protocol::PA_OFFLINE_APP_SECRET : _setup.applicationSecret;
		cc7::ByteArray data = protocol::NormalizeDataForSignature(request.method, request.uri, out.nonce, request.body, app_secret);
		out.signature = protocol::CalculateSignature(plain_keys, signature_factor, _pd->signatureCounter, data);
		if (out.signature.empty()) {
			CC7_LOG("Session %p, %d: Sign: Signature calculation failed.", this, sessionIdentifier());
			return EC_Encryption;
		}
		
		// Increase signing counter
		_pd->signatureCounter += 1;
		// Handle "Prepare for vault unlock"
		if (_pd->flags.waitingForVaultUnlock) {
			// This is just a warning. Your client probably did not receive response from a previous vault unlock HTTP request.
			CC7_LOG("Session %p, %d: Sign: Session is already waiting for a vault unlocking.");
		}
		if (vault_unlock) {
			// If we're signing vault unlock request then the counter must be increased for one more time
			_pd->signatureCounter += 1;
			_pd->flags.waitingForVaultUnlock = 1;
		} else {
			// Clear waiting flag.
			_pd->flags.waitingForVaultUnlock = 0;
		}
		
		// Fill the rest of values to out structure
		out.version			= protocol::PA_VERSION;
		out.activationId	= _pd->activationId;
		out.applicationKey	= request.isOfflineRequest() ? protocol::PA_OFFLINE_APP_SECRET : _setup.applicationKey;
		
		return EC_Ok;
	}
	
	const std::string & Session::httpAuthHeaderName() const
	{
		return protocol::PA_AUTH_HEADER_NAME;
	}
	
	ErrorCode Session::verifyServerSignedData(const SignedData & data) const
	{
		LOCK_GUARD();
		if (!hasValidSetup()) {
			CC7_LOG("Session %p, %d: ServerSig: Session has no valid setup.", this, sessionIdentifier());
			return EC_WrongState;
		}
		bool use_master_server_key = data.signingKey == SignedData::ECDSA_MasterServerKey;
		if (!use_master_server_key && !hasValidActivation()) {
			CC7_LOG("Session %p, %d: ServerSig: There's no valid activation.", this, sessionIdentifier());
			return EC_WrongState;
		}
		if (data.signature.empty()) {
			CC7_LOG("Session %p, %d: ServerSig: The signature is empty.", this, sessionIdentifier());
			return EC_WrongParam;
		}
		// Import public key
		bool success = false;
		crypto::BNContext ctx;
		EC_KEY * ec_public_key;
		if (use_master_server_key) {
			// Import master server public key
			ec_public_key = crypto::ECC_ImportPublicKeyFromB64(nullptr, _setup.masterServerPublicKey, ctx);
		} else {
			// Import server public key, which is personalized and associated with this session.
			ec_public_key = crypto::ECC_ImportPublicKey(nullptr, _pd->serverPublicKey);
		}
		if (nullptr != ec_public_key) {
			// validate signature
			success = crypto::ECDSA_ValidateSignature(data.data, data.signature, ec_public_key);
			//
		} else {
			CC7_LOG("Session %p, %d: ServerSig: %s public key is invalid.", this, sessionIdentifier(), use_master_server_key ? "Master server" : "Server");
		}
		// Free allocated OpenSSL resources
		EC_KEY_free(ec_public_key);
		
		return success ? EC_Ok : EC_Encryption;
	}
	
	// MARK: - Signature keys management -
	
	ErrorCode Session::changeUserPassword(const cc7::ByteRange & old_password, const cc7::ByteRange & new_password)
	{
		LOCK_GUARD();
		if (!hasValidActivation()) {
			CC7_LOG("Session %p, %d: PasswordChange: There's no valid activation.", this, sessionIdentifier());
			return EC_WrongState;
		}
		
		// Prepare lock / unlock structures. In this one particular case session keeps these
		// structures hidden in implementation and allows you to use password directly.
		
		SignatureUnlockKeys old_keys;
		old_keys.userPassword = old_password;
		SignatureUnlockKeys new_keys;
		new_keys.userPassword = new_password;
		
		// Unlock knowledge key with using old password
		protocol::SignatureKeys plain_keys;
		protocol::SignatureUnlockKeysReq unlock_request(SF_Knowledge, &old_keys, eek(), &_pd->passwordSalt, _pd->passwordIterations);
		if (false == protocol::UnlockSignatureKeys(plain_keys, _pd->sk, unlock_request)) {
			return EC_Encryption;
		}
		
		// Generate new salt and protect knowledge key with a new password
		const cc7::U32 new_iterations_count = protocol::PBKDF2_PASS_ITERATIONS;
		cc7::ByteArray new_salt = crypto::GetRandomData(protocol::PBKDF2_SALT_SIZE, true);
		protocol::SignatureKeys encrypted_keys;
		protocol::SignatureUnlockKeysReq lock_request(SF_Knowledge, &new_keys, eek(), &new_salt, new_iterations_count);
		if (false == protocol::LockSignatureKeys(encrypted_keys, plain_keys, lock_request)) {
			return EC_Encryption;
		}

		// Store change to the PD and return success
		_pd->sk.knowledgeKey    = encrypted_keys.knowledgeKey;
		_pd->passwordSalt       = new_salt;
		_pd->passwordIterations = new_iterations_count;
		
		return EC_Ok;
	}

	ErrorCode Session::addBiometryFactor(const std::string & c_vault_key, const SignatureUnlockKeys & keys)
	{
		LOCK_GUARD();
		if (keys.biometryUnlockKey.empty()) {
			CC7_LOG("Session %p, %d: addBiometryKey: The required biometryUnlockKey is missing.", this, sessionIdentifier());
			return EC_WrongParam;
		}
		
		cc7::ByteArray vault_key;
		ErrorCode code = decryptVaultKey(c_vault_key, keys, vault_key);
		if (code != EC_Ok) {
			return code;
		}
		if (!_pd->sk.biometryKey.empty()) {
			CC7_LOG("Session %p, %d: WARNING: There's already an existing biometry key.", this, sessionIdentifier());
		}

		// Ok, we have vault key and now we can decrypt stored device's private key.
		crypto::BNContext ctx;
		EC_KEY * device_private_key = nullptr;
		EC_KEY * server_public_key  = nullptr;
		code = EC_Encryption;
		
		do {
			// Decrypt device's private key
			cc7::ByteArray device_private_key_data = crypto::AES_CBC_Decrypt_Padding(vault_key, protocol::ZERO_IV, _pd->cDevicePrivateKey);
			if (device_private_key_data.empty()) {
				// Well, if the key decryption fails here then it seems that we have a problem in vault_key computation.
				// Error at this point means that we're not able to deduce KEY_ENCRYPTION_VAULT_TRANSPORT correctly.
				break;
			}
			// Import device's private & server's public key
			device_private_key = crypto::ECC_ImportPrivateKey(nullptr, device_private_key_data, ctx);
			server_public_key  = crypto::ECC_ImportPublicKey(nullptr, _pd->serverPublicKey, ctx);
			cc7::ByteArray master_secret = protocol::ReduceSharedSecret(crypto::ECDH_SharedSecret(server_public_key, device_private_key));
			if (master_secret.empty()) {
				break;
			}
			// ECDH operation succeeded and therefore we can derive a key for biometry signature factor.
			protocol::SignatureKeys plain;
			plain.usesExternalKey = eek() != nullptr;
			cc7::ByteArray test_vault_key;
			if (!protocol::DeriveAllSecretKeys(plain, test_vault_key, master_secret)) {
				break;
			}
			if (test_vault_key != vault_key) {
				// Strange, derived vault key is different to the decrypted one.
				break;
			}
			protocol::SignatureUnlockKeysReq lock_request(SF_Biometry, &keys, eek(), nullptr, 0);
			if (!protocol::LockSignatureKeys(_pd->sk, plain, lock_request)) {
				break;
			}
			// Everything looks fine
			code = EC_Ok;

		} while (false);

		EC_KEY_free(device_private_key);
		EC_KEY_free(server_public_key);

		return code;
	}
	
	ErrorCode Session::hasBiometryFactor(bool &hasBiometryFactor) const
	{
		LOCK_GUARD();
		if (!hasValidActivation()) {
			CC7_LOG("Session %p, %d: hasBiometryFactor: There's no valid activation.", this, sessionIdentifier());
			hasBiometryFactor = false;
			return EC_WrongState;
		}
		hasBiometryFactor = !_pd->sk.biometryKey.empty();
		return EC_Ok;
	}
	
	ErrorCode Session::removeBiometryFactor()
	{
		LOCK_GUARD();
		if (!hasValidActivation()) {
			CC7_LOG("Session %p, %d: removeBiometryKey: There's no valid activation.", this, sessionIdentifier());
			return EC_WrongState;
		}
		if (_pd->sk.biometryKey.empty()) {
			CC7_LOG("Session %p, %d: WARNING: The biometry key is not available.", this, sessionIdentifier());
		}

		// Clear encrypted biometry key and reset waiting for vault flag.
		_pd->sk.biometryKey.clear();
		_pd->flags.waitingForVaultUnlock = 0;
		return EC_Ok;
	}
	
	
	// MARK: - Vault operations -
	
	ErrorCode Session::deriveCryptographicKeyFromVaultKey(const std::string & c_vault_key, const SignatureUnlockKeys & keys,
														  cc7::U64 key_index, cc7::ByteArray & out_key)
	{
		LOCK_GUARD();
		cc7::ByteArray vault_key;
		ErrorCode code = decryptVaultKey(c_vault_key, keys, vault_key);
		if (code != EC_Ok) {
			return code;
		}
		out_key = protocol::DeriveSecretKey(vault_key, key_index);
		if (out_key.empty()) {
			return EC_Encryption;
		}
		return EC_Ok;
	}
	
	ErrorCode Session::signDataWithDevicePrivateKey(const std::string & c_vault_key, const SignatureUnlockKeys & keys,
													const cc7::ByteRange & in_data, cc7::ByteArray & out_signature)
	{
		LOCK_GUARD();
		cc7::ByteArray vault_key;
		ErrorCode code = decryptVaultKey(c_vault_key, keys, vault_key);
		if (code != EC_Ok) {
			return code;
		}
		
		// Ok, we have vault key and now we can decrypt stored device's private key.
		crypto::BNContext ctx;
		EC_KEY * device_private_key = nullptr;
		code = EC_Encryption;
		
		do {
			// Decrypt device's private key
			cc7::ByteArray device_private_key_data = crypto::AES_CBC_Decrypt_Padding(vault_key, protocol::ZERO_IV, _pd->cDevicePrivateKey);
			if (device_private_key_data.empty()) {
				// Well, if the key decryption fails here then it seems that we have a problem in vault_key computation.
				// Error at this point means that we're not able to deduce KEY_ENCRYPTION_VAULT_TRANSPORT correctly.
				break;
			}
			// Import device's private key & calculate signature
			device_private_key = crypto::ECC_ImportPrivateKey(nullptr, device_private_key_data, ctx);
			if (!crypto::ECDSA_ComputeSignature(in_data, device_private_key, out_signature)) {
				// Signature calculation failed.
				break;
			}
			code = EC_Ok;
		} while (false);
		
		EC_KEY_free(device_private_key);
		
		return code;
	}
	
	ErrorCode Session::decryptVaultKey(const std::string & c_vault_key, const SignatureUnlockKeys & keys, cc7::ByteArray & out_key)
	{
		LOCK_GUARD();
		if (!hasValidActivation()) {
			CC7_LOG("Session %p, %d: Vault: There's no valid activation.", this, sessionIdentifier());
			return EC_WrongState;
		}
		if (!_pd->flags.waitingForVaultUnlock) {
			CC7_LOG("Session %p, %d: Vault: The module is not waiting for a vault unlock.", this, sessionIdentifier());
			return EC_WrongState;
		}
		// Clear waiting flag
		_pd->flags.waitingForVaultUnlock = 0;
		
		// Check if there's encrypted vault key and if yes, try to decode from B64
		if (c_vault_key.empty()) {
			CC7_LOG("Session %p, %d: Vault: Missing encrypted vault key.", this, sessionIdentifier());
			return EC_WrongParam;
		}
		cc7::ByteArray encrypted_vault_key;
		bool bResult = encrypted_vault_key.readFromBase64String(c_vault_key);
		if (!bResult || encrypted_vault_key.empty()) {
			// Treat wrong B64 format as attack on the protocol.
			CC7_LOG("Session %p, %d: Vault: The provided vault key is wrong.", this, sessionIdentifier());
			return EC_Encryption;
		}
		// Unlock transport key
		protocol::SignatureKeys plain;
		protocol::SignatureUnlockKeysReq unlock_request(protocol::SF_Transport, &keys, eek(), nullptr, 0);
		if (false == protocol::UnlockSignatureKeys(plain, _pd->sk, unlock_request)) {
			CC7_LOG("Session %p, %d: Vault: You have to provide possession key.", this, sessionIdentifier());
			return EC_WrongParam;
		}
		
		// Calculates KEY_ENCRYPTION_VAULT_TRANSPORT.
		//
		// The counter is already incremented, because we had to use SF_PrepareForVaultUnlock
		// flag for the HTTP request signing. The current counter's value is equal to
		// value AFTER the whole operation and therefore we don't need to increment it.
		
		cc7::ByteArray vault_transport_key = protocol::DeriveSecretKey(plain.transportKey, _pd->signatureCounter - 1);
		if (vault_transport_key.empty()) {
			return EC_Encryption;
		}
		// Finally, decrypt received vault key
		out_key = crypto::AES_CBC_Decrypt_Padding(vault_transport_key, protocol::ZERO_IV, encrypted_vault_key);
		if (out_key.size() != protocol::VAULT_KEY_SIZE) {
			return EC_Encryption;
		}
		return EC_Ok;
	}
	

	
	// MARK: - Utilities for generic keys -
	
	cc7::ByteArray Session::normalizeSignatureUnlockKeyFromData(const cc7::ByteRange & any_data)
	{
		cc7::ByteArray key = crypto::SHA256(any_data);
		key.resize(protocol::SIGNATURE_KEY_SIZE);
		return key;
	}
	
	cc7::ByteArray Session::generateSignatureUnlockKey()
	{
		return crypto::GetRandomData(protocol::SIGNATURE_KEY_SIZE, true);
	}
	
	
	
	// MARK: - External encryption key -
	
	bool Session::hasExternalEncryptionKey() const
	{
		LOCK_GUARD();
		return eek() != nullptr;
	}
	
	ErrorCode Session::setExternalEncryptionKey(const cc7::ByteRange & eek)
	{
		LOCK_GUARD();
		if (hasExternalEncryptionKey()) {
			if (_setup.externalEncryptionKey == eek) {
				return EC_Ok;
			}
			CC7_LOG("Session %p, %d: EEK: Setting different EEK is not allowed.", this, sessionIdentifier());
		} else {
			if (hasValidActivation()) {
				// If session is activated, then we can check whether the EEK is really used or not.
				if (!_pd->flags.usesExternalKey) {
					// Setting EEK while session doesn't use it is invalid. You'll not able to sign data anymore.
					CC7_LOG("Session %p, %d: EEK: Activated session doesn't use EEK.", this, sessionIdentifier());
					return EC_WrongState;
				}
			}
			if (_setup.externalEncryptionKey.empty()) {
				if (eek.size() == protocol::SIGNATURE_KEY_SIZE) {
					_setup.externalEncryptionKey = eek;
					return EC_Ok;
				} else {
					CC7_LOG("Session %p, %d: EEK: Wrong size of EEK.", this, sessionIdentifier());
				}
			} else {
				CC7_LOG("Session %p, %d: EEK: Session has EEK but is already invalid.", this, sessionIdentifier());
			}
		}
		return EC_WrongParam;
	}
	
	ErrorCode Session::addExternalEncryptionKey(const cc7::ByteArray &eek)
	{
		LOCK_GUARD();
		if (!hasValidActivation()) {
			CC7_LOG("Session %p, %d: EEK: Session has no valid activation.", this, sessionIdentifier());
			return EC_WrongState;
		}
		if (_pd->flags.usesExternalKey) {
			CC7_LOG("Session %p, %d: EEK: Session is already using EEK.", this, sessionIdentifier());
			return EC_WrongState;
		}
		if (eek.size() != protocol::SIGNATURE_KEY_SIZE) {
			CC7_LOG("Session %p, %d: EEK: The provided key has wrong size.", this, sessionIdentifier());
			return EC_WrongParam;
		}
		// Add EEK protection
		if (!protocol::ProtectSignatureKeysWithEEK(_pd->sk, eek, true)) {
			return EC_Encryption;
		}
		_setup.externalEncryptionKey = eek;
		_pd->flags.usesExternalKey = true;
		return EC_Ok;
	}
	
	ErrorCode Session::removeExternalEncryptionKey()
	{
		LOCK_GUARD();
		if (!hasValidActivation()) {
			CC7_LOG("Session %p, %d: EEK: Session has no valid activation.", this, sessionIdentifier());
			return EC_WrongState;
		}
		if (!_pd->flags.usesExternalKey) {
			CC7_LOG("Session %p, %d: EEK: Session is not using EEK.", this, sessionIdentifier());
			return EC_WrongState;
		}
		if (!hasExternalEncryptionKey()) {
			CC7_LOG("Session %p, %d: EEK: The EEK is not set.", this, sessionIdentifier());
			return EC_WrongState;
		}
		// Remove EEK protection
		if (!protocol::ProtectSignatureKeysWithEEK(_pd->sk, _setup.externalEncryptionKey, false)) {
			return EC_Encryption;
		}
		_setup.externalEncryptionKey.clear();
		_pd->flags.usesExternalKey = false;
		return EC_Ok;
	}
	
	// Private EEK getter
	
	const cc7::ByteArray * Session::eek() const
	{
		if (hasValidSetup() && _setup.externalEncryptionKey.size() == protocol::SIGNATURE_KEY_SIZE) {
			return &_setup.externalEncryptionKey;
		}
		return nullptr;
	}

	
	
	// MARK: - End-To-End Encryption
	
	std::tuple<ErrorCode, Encryptor*> Session::createNonpersonalizedEncryptor(const cc7::ByteRange & session_index)
	{
		LOCK_GUARD();
		// Validate state & parameters
		if (!hasValidSetup()) {
			CC7_LOG("Session %p, %d: E2EE-NP: There's no valid setup.", this, sessionIdentifier());
			return { EC_WrongState, nullptr };
		}
		if (session_index.size() != protocol::SIGNATURE_KEY_SIZE || session_index == protocol::ZERO_IV) {
			CC7_LOG("Session %p, %d: E2EE-NP: Session index has wrong size or contains only zero bytes.", this, sessionIdentifier());
			return { EC_WrongParam, nullptr };
		}
		
		//
		// Encryptor in nonpersonalized mode is slightly more complex than the personalized one.
		// The next big sequence of code is creating an ephemeral key for ECDH. The shared secrted,
		// created from the ECDH is then reduced to 16B and used as a transport key for the encryptor.
		//
		bool error = true;
		EC_KEY * master_public_key = nullptr;
		EC_KEY * ephemeral_key     = nullptr;
		cc7::ByteArray transport_key;
		std::string ephemeral_public_key;
		do {
			crypto::BNContext ctx;
			
			// Import master server public key & try to validate OTP+ShortID signature
			master_public_key = crypto::ECC_ImportPublicKeyFromB64(nullptr, _setup.masterServerPublicKey, ctx);
			if (nullptr == master_public_key) {
				CC7_LOG("Session %p, %d: E2EE-NP: Master server public key is invalid.", this, sessionIdentifier());
				break;
			}
			ephemeral_key = crypto::ECC_GenerateKeyPair();
			if (nullptr == ephemeral_key) {
				CC7_LOG("Session %p, %d: E2EE-NP: Can't generate ephemeral key.", this, sessionIdentifier());
				break;
			}
			transport_key = protocol::ReduceSharedSecret(crypto::ECDH_SharedSecret(master_public_key, ephemeral_key));
			if (transport_key.empty()) {
				CC7_LOG("Session %p, %d: E2EE-NP: Can't calculate shared secret from ephemeral key.", this, sessionIdentifier());
				break;
			}
			// Export public par for ephemeral key to B64 format.
			ephemeral_public_key = crypto::ECC_ExportPublicKeyToB64(ephemeral_key);
			// So far, so good.
			error = false;
			
		} while (false);
		
		// Free allocated OpenSSL resources
		EC_KEY_free(master_public_key);
		EC_KEY_free(ephemeral_key);
		
		Encryptor * enc = nullptr;
		if (!error) {
			enc = new Encryptor(Encryptor::Nonpersonalized, session_index, transport_key);
			enc->setNonpersonalizedParams(ephemeral_public_key, _setup.applicationKey);
			error = !enc->validateConfiguration();
			if (error) {
				delete enc;
				enc = nullptr;
			}
		}
		return { enc == nullptr ? EC_Encryption : EC_Ok, enc };
	}
	
	std::tuple<ErrorCode, Encryptor*> Session::createPersonalizedEncryptor(const cc7::ByteRange & session_index, const SignatureUnlockKeys & keys)
	{
		LOCK_GUARD();
		// Validate state & parameters
		if (!hasValidActivation()) {
			CC7_LOG("Session %p, %d: E2EE-P: There's no valid activation.", this, sessionIdentifier());
			return { EC_WrongState, nullptr };
		}
		if (session_index.size() != protocol::SIGNATURE_KEY_SIZE || session_index == protocol::ZERO_IV) {
			CC7_LOG("Session %p, %d: E2EE-P: Session index has wrong length or contains only zero bytes.", this, sessionIdentifier());
			return { EC_WrongParam, nullptr };
		}
		// Unlock transport key
		protocol::SignatureKeys plain;
		protocol::SignatureUnlockKeysReq unlock_request(protocol::SF_Transport, &keys, eek(), nullptr, 0);
		if (false == protocol::UnlockSignatureKeys(plain, _pd->sk, unlock_request)) {
			CC7_LOG("Session %p, %d: E2EE-P: You have to provide possession key.", this, sessionIdentifier());
			return { EC_WrongParam, nullptr };
		}
		//
		// The personalized encryptor's implementation is way more simple than the nonpersonalized one.
		// The transport key is already present in the SignatureKeys structure, so we need to just
		// construct a proper Encryptor.
		//
		Encryptor * enc = new Encryptor(Encryptor::Personalized, session_index, plain.transportKey);
		enc->setPersonalizedParams(activationIdentifier());
		if (false == enc->validateConfiguration()) {
			delete enc;
			enc = nullptr;
		}
		return { enc == nullptr ? EC_Encryption : EC_Ok, enc };
	}
	
	
	
	// MARK: - Private methods -
	
	/*
	 The function deletes _ad and commits new persistent state, which is represented
	 by the combination of the parameters:
	 
	 | new_pd   | new_state    | Behavior
	 -----------------------------------------------------------------
	 | null     | any state    | Resets session to initial state
	 | not-null | SS_Empty     | Resets session to initial state
	 | not-null | SS_Activated | Keeps new PD and state
	 ------------------------------------------------------------------------------
	 
	 All other combination of parameters leads to fallback state.
	 */
	void Session::commitNewPersistentState(protocol::PersistentData *new_pd, Session::State new_state)
	{
		// At first, delete possible activation data. In all cases, commit must clear
		// any instance of activation data.
		delete _ad;
		_ad = nullptr;
		
		// The next structure is PersistentData. We have to delete possible previous instance
		// of PD and if state is correct, then keep the new one.
		delete _pd;
		if (new_pd != nullptr && new_state == SS_Activated) {
			// Ok, keep the new structure
			_pd = new_pd;
		} else {
			// Delete everything
			delete new_pd;
			_pd = new_pd = nullptr;
			// PD was not commited, so, we have to adjust new state.
			new_state = SS_Empty;
		}
		
		// Finally, change internal state of the session
		changeState(new_state);
	}
	
#ifdef ENABLE_CC7_LOG
	static const char * _StateName(Session::State st)
	{
		switch (st) {
			case Session::SS_Invalid:
				return "Invalid";
			case Session::SS_Empty:
				return "Empty";
			case Session::SS_Activated:
				return "Activated";
			case Session::SS_Activation1:
				return "Activation_1";
			case Session::SS_Activation2:
				return "Activation_2";
			default:
				return "Unknown!!";
		}
	}
#endif
	
	void Session::changeState(Session::State new_state)
	{
#ifdef ENABLE_CC7_LOG
		if (_state != new_state) {
			CC7_LOG("Session %p, %d: Changing state  %s  ->   %s", this, sessionIdentifier(), _StateName(_state), _StateName(new_state));
		}
#endif
		if (CC7_CHECK(new_state >= SS_Empty, "Internal error. Changing to SS_Invalid is not allowed!")) {
			_state = new_state;
		}
	}
	
	
} // io::getlime::powerAuth
} // io::getlime
} // io
