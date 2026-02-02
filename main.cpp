#define _WIN32_WINNT 0x0A00
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include "httplib.h"
#include "json.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;
using json = nlohmann::json;

atomic<bool> timerActive(false);
atomic<bool> hasKit(false);
atomic<bool> isTerrorist(false);

void sendToPython(string message){
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in destination;
    destination.sin_family = AF_INET;
    destination.sin_port = htons(4444);
    destination.sin_addr.s_addr = inet_addr("127.0.0.1");

    sendto(sock, message.c_str(), message.length(), 0, (sockaddr*)&destination, sizeof(destination));

    closesocket(sock);
    WSACleanup();
}

void runTimer() {
    auto startTime = chrono::steady_clock::now();
    int startSec = 38;
    // In-game timer is 40 seconds, set to 38 to account for delay
    for(int i = startSec; i > 0; i--){
        if(!timerActive) {
            sendToPython("EVENT:CANCEL");
            return;
        }

        int defuseThreshold = 10;
        
        if(isTerrorist){
            defuseThreshold = 5;
        }
        else if(hasKit){
            defuseThreshold = 5;
        }

        if(i < defuseThreshold){
            sendToPython("TIME:" + to_string(i) + ":SAVE");
        }
        else{
            sendToPython("TIME:" + to_string(i) + ":SAFE");
        }

        //cout << "\rTime Remaining: " << i << " sec " << flush;
        //sendToPython("TIME:" + to_string(i));
        //this_thread::sleep_for(chrono::seconds(1));
        // Sleeping for 1 second was giving delay/lag desync issues with in-game bomb timer
        auto elapsedSec = (startSec - i);
        auto nextTick = startTime + chrono::seconds(elapsedSec + 1);
        this_thread::sleep_until(nextTick);
    }

    if(timerActive){
        sendToPython("EVENT:EXPLODED");
    }
}

int main() {

    httplib::Server srvr;

    cout << "--- CS2 GSI Listener ACTIVE ---\n" << endl;
    cout << "Listening on 'http://127.0.0.1:3000'" << endl;


    srvr.Post("/", [](const httplib::Request& req, httplib::Response& res){
        try{
            auto payload = json::parse(req.body);

            /*if(payload.contains("provider")) {
                cout << "[Packet Received] Timestamp: " << payload["provider"]["timestamp"] << endl;
            }*/
            
            if(payload.contains("round")) {
                auto& roundData = payload["round"];
                
                string roundPhase = roundData.value("phase", "");

                if(payload.contains("player")) {
                    auto& pl = payload["player"];

                    // Get Team
                    string team = pl.value("team", "");
                    if(team == "T") isTerrorist = true;
                    else isTerrorist = false;

                    // Get Kit Status (Check if 'state' exists first)
                    if(pl.contains("state")) {
                        auto& state = pl["state"];
                        // "defusekit" is a boolean in JSON usually, but check safely
                        if(state.contains("defusekit")) {
                            // In some GSI versions this is boolean, sometimes int 1/0. 
                            // nlohmann::json handles checking simply:
                            if(state["defusekit"].is_boolean()) {
                                hasKit = state["defusekit"].get<bool>();
                            } else {
                                // Fallback just in case
                                hasKit = false; 
                            }
                        } else {
                            hasKit = false; 
                        }
                    }
                }

                if(roundData.contains("bomb")) {
                    string bombState = roundData["bomb"];

                    //cout << "--- Bomb State: " << bombState << " ---" << endl;
                    
                    if(bombState == "planted"){
                        if(roundPhase != "live" && roundPhase != ""){
                            timerActive = false;
                            sendToPython("EVENT:CANCEL");
                        }
                        else if(!timerActive){
                            if(roundPhase == "live" || roundPhase == ""){
                                timerActive = true;
                                thread(runTimer).detach();
                            }
                        }
                    }

                    else if(bombState == "exploded") {
                        if(timerActive){
                            timerActive = false;
                            sendToPython("EVENT:EXPLODED");
                            //cout << "\r--- BOMB DETONATED: TERRORISTS WIN ROUND ---" << endl;
                        }
                    }

                    else if(bombState == "defused") {
                        if(timerActive){
                            timerActive = false;
                            sendToPython("EVENT:DEFUSED");
                            //cout << "\r--- BOMB DEFUSED: COUNTER-TERRORISTS WIN ROUND ---" << endl;
                        }
                    }
                }
                else {
                    timerActive = false;
                    sendToPython("EVENT:CANCEL");
                }

                if(roundPhase != "live" && roundPhase != ""){
                    timerActive = false;
                    sendToPython("EVENT:CANCEL");
                }

                /*if (roundPhase == "live"){
                    cout << "--- Live Round Packet ---" << endl;
                    cout << payload.dump(4) << endl;
                }*/
            }

            //cout << "-----------------------------" << endl;

        } catch(const exception& e) {
            cerr << "JSON Error: " << e.what() << endl;
        }

        res.status = 200;
    });

    if(!srvr.listen("127.0.0.1", 3000)) {
        cerr << "Error: Could not start server on port 3000" << endl;
        return 1;
    }

    return 0;
}