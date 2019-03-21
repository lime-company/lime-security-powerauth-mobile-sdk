/*
 * Copyright 2019 Wultra s.r.o.
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
package io.getlime.security.powerauth.networking.model.response;

import java.util.Map;

/**
 * Response object for /pa/v3/activation/status end-point.
 *
 * @author Roman Strobl, roman.strobl@wultra.com
 *
 */
public class ActivationStatusResponse {

    private String activationId;
    private String encryptedStatusBlob;
    private Map<String, Object> customObject;

    /**
     * Get activation ID
     * @return Activation ID
     */
    public String getActivationId() {
        return activationId;
    }

    /**
     * Set activation ID
     * @param activationId Activation ID
     */
    public void setActivationId(String activationId) {
        this.activationId = activationId;
    }

    /**
     * Get encrypted activation status blob
     * @return Encrypted activation status blob
     */
    public String getEncryptedStatusBlob() {
        return encryptedStatusBlob;
    }

    /**
     * Set encrypted activation status blob
     * @param cStatusBlob encrypted activation status blob
     */
    public void setEncryptedStatusBlob(String cStatusBlob) {
        this.encryptedStatusBlob = cStatusBlob;
    }

    /**
     * Get custom associated object.
     * @return Custom associated object
     */
    public Map<String, Object> getCustomObject() {
        return customObject;
    }

    /**
     * Set custom associated object
     * @param customObject Custom associated object
     */
    public void setCustomObject(Map<String, Object> customObject) {
        this.customObject = customObject;
    }

}
