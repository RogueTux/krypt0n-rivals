#include "../include/EntityScanner.h"
#include "../include/Memory.h"
#include "../include/GameData.h"
#include "../include/ProcessUtils.h"

#include <iostream>
#include <csignal>
#include <unistd.h>
#include <algorithm>
#include <stdexcept>
#include <atomic>
#include <iomanip>
#include <cmath>

extern std::atomic<bool> g_Running;
std::vector<Actors> EntityList;
std::mutex EntityMutex;

void* EntityThreadFunc(void* param)
{
    usleep(500000);

    int consecutiveErrors = 0;

    while (g_Running.load())
    {
        try {
            if (ProcessId == 0 || kill(ProcessId, 0) != 0) {
                { std::lock_guard lock(EntityMutex); if (!EntityList.empty()) EntityList.clear(); }
                { std::lock_guard lock(GameDataMutex); GameAddrs = {}; }
                BaseAddress = 0; ProcessId = 0;
                sleep(2);
                continue;
            }

            if (BaseAddress == 0) {
                BaseAddress = FindBaseImage();
                if (BaseAddress == 0) {
                    sleep(1);
                    continue;
                }
            }

            bool addressesUpdatedThisTick = false;
            int playerCountFromGameState = -1;
            {
                constexpr int maxConsecutiveErrors = 10;
                std::lock_guard lock(GameDataMutex);
                if (GameAddrs.UWorld == 0) {
                    GameAddrs.UWorld = ReadMemory<uintptr_t>(BaseAddress + Offsets::UWorld);
                    if (GameAddrs.UWorld == 0) {
                        consecutiveErrors++;
                        if (consecutiveErrors > maxConsecutiveErrors) { BaseAddress = 0; consecutiveErrors = 0; }
                        usleep(200000);
                        continue;
                    }
                }
                try {
                    GameAddrs.GameState = ReadMemory<uintptr_t>(GameAddrs.UWorld + Offsets::GameState);
                    if (GameAddrs.GameState == 0) throw std::runtime_error("GameState is null");
                    GameAddrs.GameInstance = ReadMemory<uintptr_t>(GameAddrs.UWorld + Offsets::GameInstance);
                    if (GameAddrs.GameInstance == 0) throw std::runtime_error("GameInstance is null");
                    const auto localPlayersArrayAddr = ReadMemory<uintptr_t>(GameAddrs.GameInstance + Offsets::LocalPlayers);
                    if (localPlayersArrayAddr == 0) throw std::runtime_error("LocalPlayersArrayAddr is null");
                    GameAddrs.LocalPlayers = ReadMemory<uintptr_t>(localPlayersArrayAddr);
                    if (GameAddrs.LocalPlayers == 0) throw std::runtime_error("LocalPlayers (ULocalPlayer*) is null");
                    GameAddrs.PlayerController = ReadMemory<uintptr_t>(GameAddrs.LocalPlayers + Offsets::PlayerController);
                    if (GameAddrs.PlayerController == 0) throw std::runtime_error("PlayerController is null");
                    GameAddrs.AcknowledgedPawn = ReadMemory<uintptr_t>(GameAddrs.PlayerController + Offsets::LocalPawn);
                    GameAddrs.PlayerCamera = ReadMemory<uintptr_t>(GameAddrs.PlayerController + Offsets::APlayerCameraManager);
                    GameAddrs.PlayerArray = ReadMemory<uintptr_t>(GameAddrs.GameState + Offsets::PlayerArray);
                    if (GameAddrs.PlayerArray == 0) throw std::runtime_error("PlayerArray is null");
                    GameAddrs.PlayerCount = ReadMemory<int>(GameAddrs.GameState + Offsets::PlayerArray + sizeof(uintptr_t));
                    playerCountFromGameState = GameAddrs.PlayerCount;

                    if (playerCountFromGameState < 0 || playerCountFromGameState > 200) {
                        playerCountFromGameState = 0; GameAddrs.PlayerCount = 0;
                    }
                    consecutiveErrors = 0;
                    addressesUpdatedThisTick = true;
                } catch (const std::runtime_error&) {
                    consecutiveErrors++;
                    if (consecutiveErrors > maxConsecutiveErrors) {
                         GameAddrs = {}; BaseAddress = 0; consecutiveErrors = 0;
                    }
                    usleep(200000);
                    continue;
                }
            }

            if (!addressesUpdatedThisTick || GameAddrs.AcknowledgedPawn == 0 || GameAddrs.PlayerArray == 0 || playerCountFromGameState <= 0) {
                { std::lock_guard lock(EntityMutex); if (!EntityList.empty()) EntityList.clear(); }
                usleep(200000);
                continue;
            }

            std::vector<Actors> newEntityList;
            newEntityList.reserve(playerCountFromGameState);
            const uintptr_t currentLocalPawn = GameAddrs.AcknowledgedPawn;
            const uintptr_t currentGameStatePlayerArray = GameAddrs.PlayerArray;

            for (int i = 0; i < playerCountFromGameState; i++) {
                auto currentActorVelocity = FVector(0,0,0);

                try {
                    uintptr_t actorMeshPtr = 0;
                    uintptr_t playerActorPtr = 0;
                    uintptr_t playerStatePtr = 0;
                    playerStatePtr = ReadMemory<uintptr_t>(currentGameStatePlayerArray + (i * sizeof(uintptr_t)));
                    if (!playerStatePtr) continue;
                    playerActorPtr = ReadMemory<uintptr_t>(playerStatePtr + Offsets::PawnPrivate);
                    if (!playerActorPtr || playerActorPtr == currentLocalPawn) continue;

                    try {
                        uint8_t isAliveFlag = 0;
                        isAliveFlag = ReadMemory<uint8_t>(playerStatePtr + Offsets::IsAlive);
                        if (isAliveFlag == 0) {
                            continue;
                        }
                    } catch (...) {
                        continue;
                    }

                    actorMeshPtr = ReadMemory<uintptr_t>(playerActorPtr + Offsets::Mesh);

                    try {
                        if (const auto movementComponent = ReadMemory<uintptr_t>(playerActorPtr + Offsets::CharacterMovement)) {
                            currentActorVelocity = ReadMemory<FVector>(movementComponent + Offsets::LastUpdateVelocity);
                        } else {
                            if (const auto rootComponent = ReadMemory<uintptr_t>(playerActorPtr + Offsets::RootComponent)) currentActorVelocity = ReadMemory<FVector>(rootComponent + Offsets::ComponentVelocity);
                        }
                        if (std::isnan(currentActorVelocity.x) || std::isinf(currentActorVelocity.x)) currentActorVelocity = FVector(0,0,0);
                    } catch (...) { currentActorVelocity = FVector(0,0,0); }

                    if (auto [current_health, max_health] = GetHealthInfo({actorMeshPtr, playerActorPtr, playerStatePtr, currentActorVelocity}); current_health <= 0.1f && max_health > 0.0001f) {
                        continue;
                    }

                    newEntityList.push_back({actorMeshPtr, playerActorPtr, playerStatePtr, currentActorVelocity});

                } catch (const std::runtime_error&) {
                }
                  catch (...) {
                  }
            }
            {
                std::lock_guard lock(EntityMutex);
                EntityList = std::move(newEntityList);
            }
            usleep(10000);

        } catch (const std::runtime_error&) {
             usleep(100000);
        } catch (const std::exception&) {
            g_Running.store(false);
            break;
        } catch (...) {
            g_Running.store(false);
            break;
        }
    }
    return nullptr;
}

std::vector<Actors> GetEntityList() {
    std::lock_guard lock(EntityMutex);
    return EntityList;
}