#ifndef V_STRINGS_H
#define V_STRINGS_H

#include <Arduino.h>
#include "../v_array_list/v_array_list.h"

class VStrings {
public:
    static VArrayList<String> splitBy(String source, const char delimiter) {
        VArrayList<String> result;
        String current;
        for (size_t i = 0;i<source.length();i++) {
            if (source[i]==delimiter) {
                if (current.length()>0) {
                    result.add(current);
                }
                current = "";
            }
            else {
                current+=source[i];
            }
        }
        // Don't forget to add the last part
        if (current.length() > 0) {
            result.add(current);
        }
        return result;
    }

    static String join(VArrayList<String> source, const String &delimiter) {
        String result;
        for (int i = 0; i < source.size(); i++) {
            result+=source.getAt(i);
            if (i < source.size() - 1) {
                result += delimiter;
            }
        }
        return result;
    }

    static String replaceAll(const String &source, const char delimiter, const String &replace) {
        return join(splitBy(source, delimiter),replace);
    }

    static String uriEncode(String str) {
        String encoded = "";

        for (int i = 0; i < str.length(); i++) {
            const unsigned char c = str[i];

            // дозволені символи
            if ( ('a' <= c && c <= 'z') ||
                 ('A' <= c && c <= 'Z') ||
                 ('0' <= c && c <= '9') ||
                 c == '-' || c == '_' || c == '.' || c == '!' ||
                 c == '~' || c == '*' || c == '\'' || c == '(' || c == ')') {
                encoded += (char)c;
                 } else {
                     // кодуємо у UTF-8
                     // Arduino String вже у UTF-8, тому просто беремо байт
                     encoded += '%';
                     char hex[3];
                     sprintf(hex, "%02X", c);
                     encoded += hex;
                 }
        }

        return encoded;
    }
};

#endif