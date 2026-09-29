#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>

#ifdef _WIN32
#define CURL_STATICLIB 
#pragma comment(lib, "auth/libcurl.lib")
#pragma comment(lib, "auth/zlib.lib") 
#pragma comment(lib, "Normaliz.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Wldap32.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "User32.lib")
#endif

#include "curl/curl.h"
#include "json.hpp" 
#include "skStr.h" 

#ifdef _WIN32
#include <windows.h>
#include <sddl.h>
#endif

using json = nlohmann::json;

// CONFIGURATION - EDIT THESE
#define STONIXY_API_BASE    "https://stonixyauth.qzz.io"
#define STONIXY_APP_ID      "bfQRPL0M3ER4"
#define STONIXY_APP_SECRET  "OdxaszqTEZUjBFhz_lz_yUVQoIz3b4NQ"
#define STONIXY_APP_NAME    "Test"

namespace Stonixy {

    struct UserInfo {
        std::string username;
        std::string email;
        std::string expires_at;
        std::string hwid;
        std::string user_var;
    };

    struct AuthResponse {
        bool success;
        std::string message;
        std::string token;
        UserInfo info;
        std::map<std::string, std::string> variables;
    };

    struct AppInfo {
        std::string id;
        std::string name;
        std::string version;
        bool hwid_lock;
        std::string status;
    };

    struct AppDetailsResponse {
        bool success;
        AppInfo app;
        std::string message;
    };

    class api {
    private:
        std::string baseUrl;
        std::string appId;
        std::string appSecret;
        std::string appName;

    public:
        std::map<std::string, std::string> variables;

        static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
            ((std::string*)userp)->append((char*)contents, size * nmemb);
            return size * nmemb;
        }

        std::string postRequest(const std::string& endpoint, const json& payload) {
            std::string readBuffer;
            CURL* curl = curl_easy_init();
            if (!curl) return skCrypt("{\"success\":false,\"message\":\"Failed to initialize CURL\"}").decrypt();

            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, skCrypt("Content-Type: application/json").decrypt());

            std::string url = baseUrl + endpoint;
            std::string data = payload.dump();

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "StonixyAuth/2.0");
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

            CURLcode res = curl_easy_perform(curl);
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);

            if (res != CURLE_OK) {
                readBuffer = std::string("{\"success\":false,\"message\":\"CURL Error ") + std::to_string((int)res) + ": " + curl_easy_strerror(res) + "\"}";
            }
            return readBuffer;
        }

        std::string getRequest(const std::string& endpoint) {
            std::string readBuffer;
            CURL* curl = curl_easy_init();
            if (!curl) return skCrypt("{\"success\":false,\"message\":\"Failed to initialize CURL\"}").decrypt();

            std::string url = baseUrl + endpoint;
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "StonixyAuth/2.0");
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            if (res != CURLE_OK) return skCrypt("{\"success\":false,\"message\":\"Network connection error\"}").decrypt();
            return readBuffer;
        }

        std::string safeGet(const json& j, const std::string& key, const std::string& def = "") {
            if (j.contains(key)) {
                if (j[key].is_string()) {
                    std::string s = j[key].get<std::string>();
                    if (!s.empty()) return s;
                }
            }
            return def;
        }

    public:
        api(std::string baseUrl, std::string appId, std::string appSecret, std::string appName)
            : baseUrl(baseUrl), appId(appId), appSecret(appSecret), appName(appName) {
            if (this->baseUrl.back() == '/') this->baseUrl.pop_back();
            curl_global_init(CURL_GLOBAL_DEFAULT);
        }

        ~api() {
            curl_global_cleanup();
        }

        // --- Auth SDK Methods ---
        AppDetailsResponse checkApp();
        bool init(const std::string& currentVersion = "2.0");

        std::string getHWID();
        AuthResponse login(const std::string& user, const std::string& pass);
        AuthResponse registerUser(const std::string& user, const std::string& pass, const std::string& key);
        AuthResponse license(const std::string& key);

        AuthResponse response;

        std::string getVar(const std::string& key, const std::string& def = "") {
            if (variables.count(key)) return variables.at(key);
            return def;
        }
    };

    inline bool api::init(const std::string& currentVersion) {
        auto res = this->checkApp();
        if (!res.success) {
            std::cout << (skCrypt("[!] Connection to StonixyAuth failed!").decrypt()) << std::endl;
            return false;
        }

        if (!currentVersion.empty() && res.app.version != currentVersion) {
            std::cout << (skCrypt("[!] Version mismatch detected!").decrypt()) << std::endl;
            std::cout << (skCrypt("    Local:  ").decrypt()) << currentVersion << std::endl;
            std::cout << (skCrypt("    Server: ").decrypt()) << res.app.version << std::endl;
            return false;
        }

        if (res.app.status != (skCrypt("active").decrypt())) {
            std::cout << (skCrypt("[!] Application is currently disabled by developer.").decrypt()) << std::endl;
            return false;
        }

        return true;
    }

    inline AppDetailsResponse api::checkApp() {
        AppDetailsResponse res; res.success = false;
        try {
            std::string response = getRequest(skCrypt("/api/client/check?id=").decrypt() + appId);
            if (response.empty()) return res;
            json j = json::parse(response);
            res.success = j.value(skCrypt("success").decrypt(), false);
            res.message = safeGet(j, skCrypt("message").decrypt(), safeGet(j, skCrypt("error").decrypt(), ""));
            if (res.success && j.contains((skCrypt("app").decrypt())) && j[(skCrypt("app").decrypt())].is_object()) {
                auto app = j[(skCrypt("app").decrypt())];
                res.app = { safeGet(app, skCrypt("id").decrypt()), safeGet(app, skCrypt("name").decrypt()), safeGet(app, skCrypt("version").decrypt()), app.value(skCrypt("hwid_lock").decrypt(), false), safeGet(app, skCrypt("status").decrypt()) };
            }
        }
        catch (...) { res.success = false; }
        return res;
    }

    inline std::string api::getHWID() {
#ifdef _WIN32
        HANDLE hToken = NULL;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) return (skCrypt("FALLBACK_HWID").decrypt());
        DWORD dwSize = 0;
        GetTokenInformation(hToken, TokenUser, NULL, 0, &dwSize);
        std::vector<BYTE> buffer(dwSize);
        if (!GetTokenInformation(hToken, TokenUser, buffer.data(), dwSize, &dwSize)) { CloseHandle(hToken); return (skCrypt("FALLBACK_HWID").decrypt()); }
        PTOKEN_USER pTokenUser = reinterpret_cast<PTOKEN_USER>(buffer.data());
        LPSTR szSid = NULL;
        if (ConvertSidToStringSidA(pTokenUser->User.Sid, &szSid)) {
            std::string sid(szSid);
            LocalFree(szSid);
            CloseHandle(hToken);
            return sid;
        }
        CloseHandle(hToken);
#endif
        return (skCrypt("FALLBACK_HWID").decrypt());
    }

    inline AuthResponse api::login(const std::string& user, const std::string& pass) {
        AuthResponse res; res.success = false;
        try {
            json payload = { {(skCrypt("appId").decrypt()), appId}, {(skCrypt("appSecret").decrypt()), appSecret}, {(skCrypt("username").decrypt()), user}, {(skCrypt("password").decrypt()), pass}, {(skCrypt("hwid").decrypt()), getHWID()} };
            std::string response = postRequest((skCrypt("/api/client/login").decrypt()), payload);
            if (response.empty()) {
                res.message = "Empty response from auth server";
                return res;
            }
            json j = json::parse(response);
            res.success = j.value((skCrypt("success").decrypt()), false);
            std::string msg = safeGet(j, (skCrypt("message").decrypt()));
            if (msg.empty()) msg = safeGet(j, (skCrypt("error").decrypt()));
            if (msg.empty()) msg = safeGet(j, (skCrypt("msg").decrypt()));
            if (msg.empty()) {
                if (response.length() > 60)
                    msg = response.substr(0, 60);
                else
                    msg = response;
            }
            res.message = msg;
            if (res.success) {
                res.token = safeGet(j, (skCrypt("token").decrypt()));
                if (j.contains((skCrypt("info").decrypt())) && j[(skCrypt("info").decrypt())].is_object()) {
                    auto info = j[(skCrypt("info").decrypt())];
                    res.info = { safeGet(info, (skCrypt("username").decrypt())), safeGet(info, (skCrypt("email").decrypt())), safeGet(info, (skCrypt("expires_at").decrypt())), safeGet(info, (skCrypt("hwid").decrypt())), safeGet(info, (skCrypt("user_var").decrypt())) };
                }
                if (j.contains((skCrypt("variables").decrypt())) && j[(skCrypt("variables").decrypt())].is_object()) {
                    for (auto& [key, value] : j[(skCrypt("variables").decrypt())].items()) {
                        if (value.is_string()) {
                            std::string val = value.get<std::string>();
                            res.variables[key] = val;
                            this->variables[key] = val; // Store globally in instance
                        }
                    }
                }
            }
        }
        catch (...) { res.success = false; }
        return res;
    }

    inline AuthResponse api::registerUser(const std::string& user, const std::string& pass, const std::string& key) {
        AuthResponse res; res.success = false;
        try {
            json payload = { {(skCrypt("appId").decrypt()), appId}, {(skCrypt("appSecret").decrypt()), appSecret}, {(skCrypt("username").decrypt()), user}, {(skCrypt("password").decrypt()), pass}, {(skCrypt("licenseKey").decrypt()), key}, {(skCrypt("hwid").decrypt()), getHWID()} };
            std::string response = postRequest((skCrypt("/api/client/register").decrypt()), payload);
            json j = json::parse(response);
            res.success = j.value((skCrypt("success").decrypt()), false);
            res.message = safeGet(j, (skCrypt("message").decrypt()), safeGet(j, (skCrypt("error").decrypt()), ""));
        }
        catch (...) { res.success = false; }
        return res;
    }

    inline AuthResponse api::license(const std::string& key) {
        AuthResponse res; res.success = false;
        try {
            json payload = {
                {(skCrypt("appId").decrypt()), appId},
                {(skCrypt("appSecret").decrypt()), appSecret},
                {(skCrypt("key").decrypt()), key},
                {(skCrypt("licenseKey").decrypt()), key},
                {(skCrypt("hwid").decrypt()), getHWID()}
            };
            std::string respStr = postRequest((skCrypt("/api/client/license").decrypt()), payload);
            if (respStr.empty()) {
                res.message = "Empty response from auth server";
                this->response = res;
                return res;
            }
            try {
                json j = json::parse(respStr);
                res.success = j.value((skCrypt("success").decrypt()), false);
                std::string msg = safeGet(j, (skCrypt("message").decrypt()));
                if (msg.empty()) msg = safeGet(j, (skCrypt("error").decrypt()));
                if (msg.empty()) msg = safeGet(j, (skCrypt("msg").decrypt()));
                if (msg.empty()) {
                    if (respStr.length() > 60)
                        msg = respStr.substr(0, 60);
                    else
                        msg = respStr;
                }
                res.message = msg;
                if (res.success) {
                    res.token = safeGet(j, (skCrypt("token").decrypt()));
                    if (j.contains((skCrypt("info").decrypt())) && j[(skCrypt("info").decrypt())].is_object()) {
                        auto info = j[(skCrypt("info").decrypt())];
                        res.info = { safeGet(info, (skCrypt("username").decrypt())), safeGet(info, (skCrypt("email").decrypt())), safeGet(info, (skCrypt("expires_at").decrypt())), safeGet(info, (skCrypt("hwid").decrypt())), safeGet(info, (skCrypt("user_var").decrypt())) };
                    }
                    if (j.contains((skCrypt("variables").decrypt())) && j[(skCrypt("variables").decrypt())].is_object()) {
                        for (auto& [k, v] : j[(skCrypt("variables").decrypt())].items()) {
                            if (v.is_string()) {
                                res.variables[k] = v.get<std::string>();
                                this->variables[k] = v.get<std::string>();
                            }
                        }
                    }
                }
            }
            catch (...) {
                res.success = false;
                if (respStr.length() > 60)
                    res.message = respStr.substr(0, 60);
                else
                    res.message = respStr;
            }
        }
        catch (...) { 
            res.success = false; 
            if (res.message.empty()) res.message = "Server/Network Error";
        }
        this->response = res;
        return res;
    }

    inline api& App() {
        static api instance(
            skCrypt(STONIXY_API_BASE).decrypt(),
            skCrypt(STONIXY_APP_ID).decrypt(),
            skCrypt(STONIXY_APP_SECRET).decrypt(),
            skCrypt(STONIXY_APP_NAME).decrypt()
        );
        return instance;
    }
}

// Global accessor (KeyAuth style)
#define StonixyAuth Stonixy::App()
