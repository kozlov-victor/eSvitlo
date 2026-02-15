#ifndef TABLE_MULTI_TYPE_H
#define TABLE_MULTI_TYPE_H

#include "../v_hash_table/v_hash_table.h"

class VTableMultitype {
private:
    enum Type {
        INT, STRING, BOOLEAN
    };
    struct MultiValue {
        Type type;
        int nValue;
        String sValue;
        boolean bValue;
    };
    VHashTable<MultiValue> *table;

    static String escapeJsonString(const String &s) {
        String out;
        out.reserve(s.length() + 4);

        for (size_t i = 0; i < s.length(); i++) {
            char c = s[i];
            switch (c) {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n";  break;
                case '\r': out += "\\r";  break;
                case '\t': out += "\\t";  break;
                default:
                    if ((uint8_t)c < 0x20) {
                        // control chars — пропускаємо або замінюємо
                    } else {
                        out += c;
                    }
            }
        }
        return out;
    }

    static String stringifyItem(const MultiValue &value) {
        if (value.type == INT) {
            return String(value.nValue);
        }
        if (value.type == STRING) {
            return "\"" + escapeJsonString(value.sValue) + "\"";
        }
        return value.bValue?"true":"false";
    }

public:
    VTableMultitype() {
        table = new VHashTable<MultiValue>();
    }
    ~VTableMultitype() {
        delete table;
    }
    void putAll(VTableMultitype& value) const {
        value.table->forEach([this](const String key, const MultiValue val) {
            this->table->put(key,val);
        });
    }
    void putString(const String key, String value) const {
        table->put(key,{STRING,0,value,false});
    }
    String getString(const String key) const{
        return table->get(key).sValue;
    }
    void putBoolean(const String key, boolean value) const{
        table->put(key,{BOOLEAN, 0,"",value});
    }
    boolean getBoolean(const String key) const {
        return table->get(key).bValue;
    }
    void putInt(const String key, int value) const{
        table->put(key,{INT, value,"",false});
    }
    int getInt(const String key) const {
        return table->get(key).nValue;
    }
    String stringify() {
        String result = "{";
        int cnt = 0;
        size_t size = this->table->size();
        this->table->forEach([&result, &cnt, size](const String key, const MultiValue value) {
            result += "\"" + key + "\":" + stringifyItem(value);
            if (cnt < size - 1) {
                result += ",";
            }
            cnt++;
        });
        result += "}";
        return result;
    }

    static VTableMultitype parseJson(const String &source) {
        bool colonAppeared = false;
        bool quoteAppeared = false;
        bool valueWasQuoted = false;

        String currentKey = "";
        String currentValue = "";

        VTableMultitype result;

        for (int i = 0; i < source.length(); i++) {
            char c = source[i];

            // ігноруємо пробіли поза рядком
            if (!quoteAppeared &&
                (c == ' ' || c == '\n' || c == '\r' || c == '\t')) {
                continue;
            }

            // escape-послідовності всередині рядка
            if (quoteAppeared && c == '\\') {
                if (i + 1 < source.length()) {
                    char next = source[++i];
                    char decoded;
                    switch (next) {
                        case '"':  decoded = '"';  break;
                        case '\\': decoded = '\\'; break;
                        case 'n':  decoded = '\n'; break;
                        case 'r':  decoded = '\r'; break;
                        case 't':  decoded = '\t'; break;
                        default:   decoded = next; break;
                    }

                    if (colonAppeared) {
                        currentValue += decoded;
                    } else {
                        currentKey += decoded;
                    }
                }
                continue;
            }

            // лапки
            if (c == '"') {
                quoteAppeared = !quoteAppeared;

                // якщо відкрилась лапка значення — запамʼятовуємо
                if (quoteAppeared && colonAppeared) {
                    valueWasQuoted = true;
                }
                continue;
            }

            // двокрапка
            if (c == ':' && !quoteAppeared) {
                colonAppeared = true;
                continue;
            }

            // кінець пари
            if ((c == ',' || c == '}') && !quoteAppeared) {
                if (currentKey.length() > 0) {

                    if (valueWasQuoted) {
                        result.putString(currentKey, currentValue);
                    }
                    else if (currentValue == "true") {
                        result.putBoolean(currentKey, true);
                    }
                    else if (currentValue == "false") {
                        result.putBoolean(currentKey, false);
                    }
                    else {
                        // int (JSON без лапок)
                        result.putInt(currentKey, currentValue.toInt());
                    }
                }

                currentKey = "";
                currentValue = "";
                colonAppeared = false;
                valueWasQuoted = false;
                continue;
            }

            // накопичення
            if (quoteAppeared) {
                if (colonAppeared) {
                    currentValue += c;
                } else {
                    currentKey += c;
                }
            } else {
                // поза рядком → число або boolean
                if (colonAppeared) {
                    currentValue += c;
                }
            }
        }

        return result;
    }

};

#endif //TABLE_MULTI_TYPE_H