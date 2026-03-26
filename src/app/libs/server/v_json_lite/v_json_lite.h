#ifndef V_JSON_LITE_SAFE_H
#define V_JSON_LITE_SAFE_H

#include <Arduino.h>
#include "../v_array_list/v_array_list.h"
#include "../v_hash_table/v_hash_table.h"

// =====================================
// util
// =====================================
class JsonHelper {
public:
    static String escapeJsonString(const String &s) {
        String out;
        out.reserve(s.length() + 4);

        for (size_t i = 0; i < s.length(); i++) {
            const char c = s[i];
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
};

// =====================================
// TOKENIZER
// =====================================
enum TokenType {
    T_LBRACE, T_RBRACE, T_LBRACKET, T_RBRACKET,
    T_COMMA, T_COLON,
    T_STRING, T_INT, T_FLOAT,
    T_TRUE, T_FALSE, T_NULL,
    T_END, T_ERROR
};

struct Token {
    TokenType type;
    String value;
};

class Tokenizer {
private:
    const char* input;
    int pos = 0;

    char peek() const { return input[pos] ? input[pos] : '\0'; }
    char advance() { return input[pos] ? input[pos++] : '\0'; }

    static boolean isWsOrNewLine(const char c) {
        return c == '\n' || c == '\r' || c==' ';
    }

    static boolean isDigit(const char c) {
        return
            c=='0' || c=='1' || c=='2' || c=='3' || c=='4' || c=='5' ||
            c=='6' || c=='7' || c=='8' || c=='9';
    }

    void skipWS() { while (isWsOrNewLine(peek())) advance(); }

    bool match(const char* kw) {
        int i=0;
        while (kw[i]) {
            if (input[pos+i] != kw[i]) return false;
            i++;
        }
        pos += i;
        return true;
    }

    String parseString() {
        String s;
        advance(); // "
        while (true) {
            const char c = advance();
            if (c == '\0') return "";
            if (c == '"') break;
            if (c == '\\') {
                const char e = advance();
                if (e=='n') s+='\n';
                else if (e=='t') s+='\t';
                else s+=e;
            } else s += c;
        }
        return s;
    }

    String parseNumber() {
        String s;
        if (peek()=='-') s+=advance();
        while (isdigit(peek())) s+=advance();
        if (peek()=='.') {
            s+=advance();
            while (isdigit(peek())) s+=advance();
        }
        return s;
    }

public:
    explicit Tokenizer(const char* str): input(str) {}

    void tokenize(VArrayList<Token> &tokens) {
        while (true) {
            skipWS();
            const char c = peek();

            if (c=='\0') { tokens.add({T_END,""}); break; }

            if (c=='{') { advance(); tokens.add({T_LBRACE,"{"}); }
            else if (c=='}') { advance(); tokens.add({T_RBRACE,"}"}); }
            else if (c=='[') { advance(); tokens.add({T_LBRACKET,"["});}
            else if (c==']') { advance(); tokens.add({T_RBRACKET,"]"}); }
            else if (c==',') { advance(); tokens.add({T_COMMA,","}); }
            else if (c==':') { advance(); tokens.add({T_COLON,":"}); }
            else if (c=='"') tokens.add({T_STRING,parseString()});
            else if (isdigit(c)||c=='-') {
                String val = parseNumber();
                if (val.indexOf(".")>-1) {
                    tokens.add({T_FLOAT,val});
                }
                else {
                    tokens.add({T_INT,val});
                }
            }
            else if (match("true")) tokens.add({T_TRUE,"true"});
            else if (match("false")) tokens.add({T_FALSE,"false"});
            else if (match("null")) tokens.add({T_NULL,"null"});
            else { tokens.add({T_ERROR,String(c)});}
        }
    }
};

// =====================================
// JSON VALUE
// =====================================
enum JsonType { J_NULL, J_BOOL, J_INT, J_FLOAT, J_STRING, J_OBJECT, J_ARRAY };

struct JsonValue {
    JsonType type;

    bool boolVal;
    double floatVal;
    int intVal;
    String strVal;
    VHashTable<JsonValue*> object;
    VArrayList<JsonValue*> array;

    JsonValue() : type(J_NULL), boolVal(false), floatVal(0), intVal(0) {

    }

    JsonValue* get(const String& key) {
        if (type != J_OBJECT) return this;
        if (object.has(key)) return object.get(key);
        // якщо не знайдено — повертаємо null (this)
        return this;
    }

    JsonValue* get(const int index) {
        if (type != J_ARRAY) return this;
        if (index < 0 || index >= array.size()) return this;
        return array.getAt(index);
    }

    double asDouble() const {
        if (type == J_FLOAT) return floatVal;
        if (type == J_INT) return intVal;
        return 0;
    }

    int asInt() const {
        if (type == J_FLOAT) return floatVal;
        if (type == J_INT) return intVal;
        return 0;
    }

    bool asBool() const {
        if (type == J_BOOL) return boolVal;
        return false;
    }

    String asString() const {
        if (type == J_STRING) return strVal;
        return "";
    }

    boolean isNull() const {
        return type == J_NULL;
    }

    String toString() {
        switch(type) {
            case J_NULL: return "null";
            case J_BOOL: return boolVal?"true":"false";
            case J_FLOAT: return String(floatVal,6);
            case J_INT: return String(intVal);
            case J_STRING: return "\"" + JsonHelper::escapeJsonString(strVal) + "\"";

            case J_OBJECT: {
                String s="{";
                int i=0;
                object.forEach([&i,&s](const String &key, JsonValue *val) {
                    if(i>0)s+=",";
                    i++;
                    s += "\"" + key + "\":" + val->toString();
                });
                return s + "}";
            }

            case J_ARRAY: {
                String s="[";
                for(int i=0; i<array.size(); i++){
                    if(i>0) s+=",";
                    s += array.getAt(i)->toString();
                }
                return s + "]";
            }
        }
        return "";
    }
};

// =====================================
// PARSER
// =====================================
class JsonPool {
private:
    JsonValue* pool;
    int capacity;
    int used = 0;
    JsonValue fallbackJsonValue = {};
public:
    JsonPool(JsonValue* buffer, const int cap)
        : pool(buffer), capacity(cap) {
        fallbackJsonValue.type = J_NULL;
    }

    JsonValue* alloc() {
        if (used >= capacity) {
            Serial.println("ERROR: JsonPool is empty!!");
            return &fallbackJsonValue;
        }
        return &pool[used++];
    }

    void reset() { used = 0; }

    int free() const { return capacity - used; }
};

JsonValue defaultBuffer[128];
JsonPool defaultPool(defaultBuffer, 128);

class JsonParser {
private:
    VArrayList<Token> *tokens;
    int pos = 0;
    JsonPool* pool;
    JsonValue fallbackJsonValue = {};

    Token& lookNext() const { return tokens->getAt(pos); }
    Token& getNext(){ return tokens->getAt(pos++); }

    bool eat(TokenType t){
        if (lookNext().type==t){ pos++; return true; }
        return false;
    }

    JsonValue* parseValue() {
        const Token& t = lookNext();

        if (t.type == T_STRING) {
            JsonValue* v = pool->alloc();
            v->type = J_STRING;
            v->strVal = t.value;
            getNext();
            return v;
        }

        if (t.type == T_INT) {
            JsonValue* v = pool->alloc();
            v->type = J_INT;
            v->intVal = t.value.toInt();
            getNext();
            return v;
        }

        if (t.type == T_FLOAT) {
            JsonValue* v = pool->alloc();
            v->type = J_FLOAT;
            v->floatVal = t.value.toDouble();
            getNext();
            return v;
        }

        if (t.type == T_TRUE) {
            JsonValue* v = pool->alloc();
            v->type = J_BOOL;
            v->boolVal = true;
            getNext();
            return v;
        }

        if (t.type == T_FALSE) {
            JsonValue* v = pool->alloc();
            v->type = J_BOOL;
            v->boolVal = false;
            getNext();
            return v;
        }

        if (t.type == T_NULL) {
            JsonValue* v = pool->alloc();
            v->type = J_NULL;
            getNext();
            return v;
        }

        if (t.type == T_LBRACE) return parseObject();
        if (t.type == T_LBRACKET) return parseArray();

        return &fallbackJsonValue;
    }

    JsonValue* parseObject() {
        JsonValue* obj = pool->alloc();

        obj->type = J_OBJECT;

        getNext(); // {

        while (!eat(T_RBRACE)) {
            if (lookNext().type != T_STRING) return &fallbackJsonValue;

            String key = getNext().value;

            if (!eat(T_COLON)) return &fallbackJsonValue;

            JsonValue* val = parseValue();
            obj->object.put(key, val);

            if (!eat(T_COMMA)) {
                if (lookNext().type != T_RBRACE) {
                    return &fallbackJsonValue;
                }
            }
        }

        return obj;
    }

    JsonValue* parseArray() {
        JsonValue* arr = pool->alloc();
        if (!arr) return &fallbackJsonValue;

        arr->type = J_ARRAY;

        getNext(); // [

        while (lookNext().type != T_RBRACKET) {
            JsonValue* val = parseValue();
            if (!val) return &fallbackJsonValue;

            arr->array.add(val);

            if (!eat(T_COMMA)) break;
        }

        if (!eat(T_RBRACKET)) return &fallbackJsonValue;

        return arr;
    }

    explicit JsonParser(VArrayList<Token>* tokens, JsonPool* pool = &defaultPool) : tokens(tokens), pool(pool) {
        fallbackJsonValue.type = J_NULL;
    }

public:

    static JsonValue* parse(const char* str, JsonPool* pool = &defaultPool) {
        if (pool==&defaultPool) {
            pool->reset();
        }
        Tokenizer t(str);

        VArrayList<Token> tokens;
        t.tokenize(tokens);

        JsonParser p(&tokens, pool);
        return p.parseValue();
    }

    static JsonValue* parse(const String& str, JsonPool* pool = &defaultPool) {
        return parse(str.c_str(), pool);
    }

};

// =====================================
// BUILDER
// =====================================
class JsonObjectBuilder {
private:
    JsonValue root;
    JsonPool* pool;
    JsonValue fallbackJsonValue;
public:
    explicit JsonObjectBuilder(const JsonType type = J_OBJECT, JsonPool* pool = &defaultPool):pool(pool) {
        root.type = type;
        if (pool==&defaultPool) {
            pool->reset();
        }
        fallbackJsonValue.type = J_NULL;
    }

    void setInt(const String& key,const int v, JsonValue* target = nullptr){
        if (!target) target = &root;
        if (target->type!=J_OBJECT) return;
        JsonValue* j = pool->alloc();
        j->type = J_INT;
        j->intVal = v;
        target->object.put(key,j);
    }

    void setFloat(const String& key,const double v, JsonValue* target = nullptr){
        if (!target) target = &root;
        if (target->type!=J_OBJECT) return;
        JsonValue* j = pool->alloc();
        j->type = J_FLOAT;
        j->floatVal = v;
        target->object.put(key,j);
    }

    void putInt(const double v, JsonValue* target = nullptr){
        if (!target) target = &root;
        if (target->type!=J_ARRAY) return;
        JsonValue* j = pool->alloc();
        j->type = J_INT;
        j->intVal = v;
        target->array.add(j);
    }

    void putFloat(const double v, JsonValue* target = nullptr){
        if (!target) target = &root;
        if (target->type!=J_ARRAY) return;
        JsonValue* j = pool->alloc();
        j->type = J_FLOAT;
        j->floatVal = v;
        target->array.add(j);
    }

    void setBool(const String& key,const bool v, JsonValue* target = nullptr){
        if (!target) target = &root;
        if (target->type!=J_OBJECT) return;
        JsonValue* j = pool->alloc();
        j->type = J_BOOL;
        j->boolVal = v;
        target->object.put(key,j);
    }

    void putBool(const boolean v, JsonValue* target = nullptr){
        if (!target) target = &root;
        if (target->type!=J_ARRAY) return;
        JsonValue* j = pool->alloc();
        j->type = J_BOOL;
        j->boolVal = v;
        target->array.add(j);
    }

    void setString(const String& key,const String& v, JsonValue* target = nullptr){
        if (!target) target = &root;
        if (target->type!=J_OBJECT) return;
        JsonValue* j = pool->alloc();
        j->type = J_STRING;
        j->strVal = v;
        target->object.put(key,j);
    }

    void putString(const String& v, JsonValue* target = nullptr){
        if (!target) target = &root;
        if (target->type!=J_ARRAY) return;
        JsonValue* j = pool->alloc();
        j->type = J_STRING;
        j->strVal = v;
        target->array.add(j);
    }

    void setNull(const String& key, JsonValue* target = nullptr){
        if (!target) target = &root;
        if (target->type!=J_OBJECT) return;
        JsonValue* j = pool->alloc();
        j->type = J_NULL;
        target->object.put(key,j);
    }

    void putNull(JsonValue* target = nullptr){
        if (!target) target = &root;
        if (target->type!=J_ARRAY) return;
        JsonValue* j = pool->alloc();
        j->type = J_NULL;
        target->array.add(j);
    }

    JsonValue* putArray(JsonValue* target = nullptr){
        if (!target) target = &root;
        if (target->type!=J_ARRAY) return &fallbackJsonValue;
        JsonValue* j = pool->alloc();
        j->type = J_ARRAY;
        target->array.add(j);
        return j;
    }

    JsonValue* putObject(JsonValue* target = nullptr){
        if (!target) target = &root;
        if (target->type!=J_ARRAY) return &fallbackJsonValue;
        JsonValue* j = pool->alloc();
        j->type = J_OBJECT;
        target->array.add(j);
        return j;
    }

    JsonValue* setObject(const String& key, JsonValue* target = nullptr){
        if (!target) target = &root;
        if (target->type!=J_OBJECT) return &fallbackJsonValue;
        JsonValue* j = pool->alloc();
        j->type = J_OBJECT;
        target->object.put(key,j);
        return j;
    }

    JsonValue* setArray(const String& key, JsonValue* target = nullptr){
        if (!target) target = &root;
        if (target->type!=J_OBJECT) return &fallbackJsonValue;
        JsonValue* j = pool->alloc();
        j->type = J_ARRAY;
        target->object.put(key,j);
        return j;
    }

    String toString() { return root.toString(); }
};

#endif