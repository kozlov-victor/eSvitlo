
#ifndef V_AUTH_H
#define V_AUTH_H
#include <Arduino.h>
#include <base64.h>
#include "mbedtls/sha256.h"
#include "mbedtls/base64.h"
#include "../v_strings/v_strings.h"
#include "../v_request/v_request.h"


class VAuth {
    struct Creds {
        String login;
        String password;
        String secret;
    };

private:
    static Creds& defaultCreds() {
        static Creds creds = {
            .login = "admin",
            .password = "admin",
            .secret = "HARDWARE_ESP_KEY"
        };
        return creds;
    }

    static Creds& admin(){
        static Creds creds = {
            .login = defaultCreds().login,
            .password = defaultCreds().password,
            .secret = defaultCreds().secret
        };
        return creds;
    }

    static String sha256(const String &input) {
        uint8_t hash[32];
        // Ця функція все робить одним викликом
        mbedtls_sha256((const unsigned char*)input.c_str(), input.length(), hash, 0); // 0 = SHA256, 1 = SHA224

        char buf[65];
        for (int i = 0; i < 32; i++)
            sprintf(buf + i*2, "%02x", hash[i]);
        buf[64] = 0;
        return String(buf);
    }

    static String base64Decode(const String &input) {
        size_t outputLen = 0;
        // резервуємо буфер на максимально можливу довжину
        size_t bufLen = (input.length() * 3) / 4 + 1;
        uint8_t *buf = new uint8_t[bufLen];

        int ret = mbedtls_base64_decode(buf, bufLen, &outputLen,
                                        (const unsigned char *)input.c_str(), input.length());
        if (ret != 0) {
            delete[] buf;
            return "";
        }

        String result;
        for (size_t i = 0; i < outputLen; i++) {
            result += (char)buf[i];
        }

        delete[] buf;
        return result;
    }

    static String createHash(const String& login, const String &password, const String &expiration) {
        const Creds creds = admin();
        return sha256(login + ":" + password + ":" + expiration + ":" + creds.secret);
    }

public:

    static void setDefaultCreds(const String &login, const String &password, const String &secret = "") {
        defaultCreds().login = login;
        defaultCreds().password = password;
        if (!secret.isEmpty()) {
            defaultCreds().secret = secret;
        }
    }

    static void setCreds(const String &login, const String &password, const String &secret = "") {
        admin().login = login;
        admin().password = password;
        if (!secret.isEmpty()) {
            admin().secret = secret;
        } else {
            admin().secret = defaultCreds().secret;
        }
    }

    static void reset() {
        admin().login = defaultCreds().login;
        admin().password = defaultCreds().password;
        admin().secret = defaultCreds().secret;
    }

    static String createToken(const String& login, const String &password, const int ttlMinutes) {
        const Creds creds = admin();
        if (login!=creds.login || password!=creds.password) {
            return "";
        }
        const auto expiration = String(millis() + ttlMinutes * 60 * 1000);
        return base64::encode( login + ":" + expiration + ":" + createHash(login, password, expiration));
    }

    static boolean checkToken(const String& token) {
        if (token==nullptr || token.isEmpty()) {
            Serial.println("no token");
            return false;
        }
        const String decoded = base64Decode(token);
        if (!decoded.length()) {
            Serial.println("no decoded length");
            return false;
        }
        const auto parts = VStrings::splitBy(decoded,':');
        if (parts->size()!=3) {
            Serial.println("bad decoded parts length");
            return false;
        }
        const String login = parts->getAt(0).toString();
        const String expiration = parts->getAt(1).toString();
        const String hash = parts->getAt(2).toString();
        delete parts;
        if (hash!=createHash(login, admin().password, expiration)) {
            Serial.println("bad hash part");
            return false;
        }
        if (expiration.toInt()<millis()) {
            Serial.println("expired");
            return false;
        }
        return true;
    }

    static boolean checkToken(const VRequest *request) {
        const String token = request->headers->get("authorization");
        if (token.isEmpty()) {
            Serial.println("no Authorization header");
            return false;
        }
        const auto parts = VStrings::splitBy(token, ' ');
        boolean result = true;
        if (parts->size()!=2) {
            Serial.println("bad Authorization header length");
            result = false;
        }
        if (parts->getAt(0).toString()!="Bearer") {
            Serial.println("not a Bearer token");
            result = false;
        }
        if (!checkToken(parts->getAt(1).toString())) {
            Serial.println("bad token check");
            result = false;
        }
        delete parts;
        return result;
    }


};

#endif //V_AUTH_H