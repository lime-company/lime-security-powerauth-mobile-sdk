/*
 * Copyright 2017 Wultra s.r.o.
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

package io.getlime.security.powerauth.util.otp;

/**
 * Class for parsing and validation the activation code.
 *
 * Current format of code:
 * <pre>
 * code without signature:  CCCCC-CCCCC-CCCCC-CCCCC
 * code with signature:     CCCCC-CCCCC-CCCCC-CCCCC#BASE64_STRING_WITH_SIGNATURE
 * </pre>
 *
 * Where the 'C' is Base32 sequence of characters, fully decodable into the sequence of bytes.
 * The validator then compares CRC-16 checksum calculated for the first 10 bytes and compares
 * it to last two bytes (in big endian order).
 */
public class OtpUtil {

    static {
        System.loadLibrary("PowerAuth2Module");
    }

    /**
     * Parses an input |activationCode| (which may or may not contain an optional signature) and
     * returns Otp object filled with valid data. The method doesn't perform an auto-correction,
     * so the provided code must be valid.
     *
     * @return Otp object if code is valid, or null
     * */
    public native static Otp parseFromActivationCode(String activationCode);

    /**
     *  Returns true if |utfCodepoint| is a valid character allowed in the activation code.
     *  The method strictly checks whether the character is from [A-Z2-7] characters range.
     */
    public native static boolean validateTypedCharacter(int utfCodepoint);

    /**
     * Validates an input |utfCodepoint| and returns 0 if it's not valid or cannot be corrected.
     * The non-zero returned value contains the same input character, or the corrected
     * one. You can use this method for validation &amp; auto-correction of just typed characters.
     * <p>
     * The function performs following auto-corrections:
     * <ul>
     * <li>lowercase characters are corrected to uppercase (e.g. 'a' will be corrected to 'A')</li>
     * <li>'0' is corrected to 'O' (zero to capital O)</li>
     * <li>'1' is corrected to 'I' (one to capital I)</li>
     * </ul>
     */
    public native static int validateAndCorrectTypedCharacter(int utfCodepoint);

    /**
     *  Returns true if |activationCode| is a valid activation code. The input code must not contain
     *  a signature part. You can use this method to validate a whole user-typed activation code
     *  at once.
     */
    public native static boolean validateActivationCode(String activationCode);

}
