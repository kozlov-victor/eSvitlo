#ifndef POLL_ASYNC_H
#define POLL_ASYNC_H
#include <WiFi.h>

struct HttpResponse {
    int status;
    char body[512];
    size_t length;
};

class PollAsync {
public:
    using ResolveFn = void (*)(const HttpResponse&);
    using RejectFn  = void (*)(const char*);

    PollAsync() {}

    // ---------- public API ----------
    PollAsync& url(const char* urlStr) {
        parseUrl(urlStr);
        return *this;
    }

    PollAsync& then(ResolveFn fn) {
        onResolve = fn;
        return *this;
    }

    PollAsync& catchError(RejectFn fn) {
        onReject = fn;
        return *this;
    }

    PollAsync& every(unsigned long ms) {
        periodic = true;
        intervalMs = ms;
        return *this;
    }

    void start() {
        retryCount = 0;
        connectUsedThisInterval = false;
        nextRunAt = millis() + intervalMs;
        beginAttempt();
    }

    void tick() {
        unsigned long now = millis();

        // ----- periodic -----
        if (periodic && state == IDLE && now >= nextRunAt) {
            Serial.println("periodic tick");
            nextRunAt = now + intervalMs;
            retryCount = 0;
            connectUsedThisInterval = false;
            beginAttempt();
        }

        if (state == IDLE) return;

        // ----- wait retry -----
        if (state == WAIT_RETRY) {
            if (now >= nextRetryAt) {
                beginAttempt();
            }
            return;
        }

        // ----- timeout -----
        if (now > deadline) {
            scheduleRetry("timeout");
            return;
        }

        // ----- FSM -----
        switch (state) {
            case CONNECTING: tickConnect(); break;
            case SENDING:    tickSend();    break;
            case HEADERS:    tickHeaders(); break;
            case BODY:       tickBody();    break;
            default: break;
        }
    }

private:
    // ---------- state machine ----------
    enum State { IDLE, CONNECTING, SENDING, HEADERS, BODY, WAIT_RETRY } state = IDLE;

    // ---------- socket safety ----------
    unsigned long lastSocketClose = 0;
    bool connectUsedThisInterval = false;
    static constexpr unsigned long socketCooldownMs = 2000;

    bool canConnectNow() {
        unsigned long now = millis();

        if (connectUsedThisInterval) {
            Serial.println("connect blocked: already used in interval");
            return false;
        }

        if (now - lastSocketClose < socketCooldownMs) {
            Serial.println("connect blocked: socket cooldown");
            return false;
        }

        return true;
    }

    void beginAttempt() {
        if (!canConnectNow()) return;

        Serial.println("begin attempt");

        resetAttempt();
        deadline = millis() + timeoutMs;
        state = CONNECTING;
        connectUsedThisInterval = true;

        client.stop();
        lastSocketClose = millis();

        client.connect(host, port, timeoutMs);
    }

    void tickConnect() {
        if (client.connected()) {
            state = SENDING;
        }
    }

    void tickSend() {
        Serial.println(host);
        Serial.println(port);
        Serial.println(path);

        client.print(
            String("GET ") + path + " HTTP/1.1\r\n" +
            "Host: " + host + "\r\n" +
            "Connection: close\r\n\r\n"
        );

        state = HEADERS;
    }

    void tickHeaders() {
        while (client.available()) {
            char c = client.read();

            // ---- parse status line ----
            if (!statusParsed) {
                if (!statusStarted) {
                    if (c == ' ') statusStarted = true;
                } else {
                    if (c >= '0' && c <= '9' && statusPos < 3) {
                        statusBuf[statusPos++] = c;
                        if (statusPos == 3) {
                            statusBuf[3] = '\0';
                            response.status = atoi(statusBuf);
                            statusParsed = true;
                        }
                    }
                }
            }

            feedHeader(c);

            if (headersDone) {
                state = BODY;
                return;
            }
        }

        if (!client.connected()) {
            scheduleRetry("connection lost in headers");
        }
    }

    void tickBody() {
        while (client.available()) {
            char c = client.read();
            if (response.length < sizeof(response.body) - 1) {
                response.body[response.length++] = c;
            }
        }

        if (!client.connected()) {
            response.body[response.length] = '\0';

            client.stop();
            lastSocketClose = millis();

            state = IDLE;
            connectUsedThisInterval = false;

            Serial.println(response.body);
            if (onResolve) onResolve(response);
        }
    }

    // ---------- helpers ----------
    void feedHeader(char c) {
        static char last4[4] = {};
        last4[0]=last4[1];
        last4[1]=last4[2];
        last4[2]=last4[3];
        last4[3]=c;

        if (last4[0]=='\r' && last4[1]=='\n' &&
            last4[2]=='\r' && last4[3]=='\n') {
            headersDone = true;
        }
    }

    void scheduleRetry(const char* reason) {
        Serial.println("schedule retry");
        Serial.println(retryCount);

        client.stop();
        lastSocketClose = millis();

        if (retryCount >= maxRetries) {
            state = IDLE;
            connectUsedThisInterval = false;
            if (onReject) onReject(reason);
            return;
        }

        retryCount++;
        nextRetryAt = millis() + retryDelayMs;
        state = WAIT_RETRY;
    }

    void resetAttempt() {
        headersDone = false;
        response.status = -1;
        response.length = 0;
        statusParsed = false;
        statusStarted = false;
        statusPos = 0;
    }

    void parseUrl(const char* url) {
        port = 80;
        strcpy(path, "/");

        if (strncmp(url, "http://", 7) == 0) url += 7;

        const char* hostStart = url;
        const char* portPos = nullptr;
        const char* pathPos = nullptr;

        for (const char* p = url; *p; ++p) {
            if (*p == ':' && !portPos && !pathPos) portPos = p;
            else if (*p == '/') { pathPos = p; break; }
        }

        if (pathPos) {
            strncpy(path, pathPos, sizeof(path)-1);
            path[sizeof(path)-1] = 0;
        }

        if (portPos) {
            size_t hostLen = portPos - hostStart;
            strncpy(host, hostStart, hostLen);
            host[hostLen] = 0;
            port = atoi(portPos + 1);
        } else if (pathPos) {
            size_t hostLen = pathPos - hostStart;
            strncpy(host, hostStart, hostLen);
            host[hostLen] = 0;
        } else {
            strncpy(host, hostStart, sizeof(host)-1);
        }
    }

    // ---------- networking ----------
    WiFiClient client;
    char host[64];
    char path[96];
    uint16_t port;

    bool statusParsed = false;
    char statusBuf[4];
    uint8_t statusPos = 0;
    bool statusStarted = false;

    // ---------- callbacks ----------
    ResolveFn onResolve = nullptr;
    RejectFn  onReject  = nullptr;

    // ---------- state ----------
    bool headersDone = false;
    HttpResponse response;

    // ---------- timing ----------
    unsigned long deadline = 0;
    static constexpr unsigned long timeoutMs = 8000;

    // ---------- retry ----------
    uint8_t retryCount = 0;
    static constexpr uint8_t maxRetries = 3;
    static constexpr unsigned long retryDelayMs = 5000;
    unsigned long nextRetryAt = 0;

    // ---------- every ----------
    bool periodic = false;
    unsigned long intervalMs = 0;
    unsigned long nextRunAt = 0;
};

#endif // POLL_ASYNC_H