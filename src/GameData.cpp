#include "../include/GameData.h"
#include "../include/Memory.h"

#include <iostream>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <atomic>

ScreenDimensions GameScreen;
GameAddresses GameAddrs;
std::mutex GameDataMutex;

const std::unordered_map<HeroId, std::string> hero_names = {
    {HeroId::Unknown, "Unknown"}, {HeroId::BruceBanner, "Hulk"}, {HeroId::DoctorStrange, "Doctor Strange"},
    {HeroId::CaptainAmerica, "Captain America"}, {HeroId::Groot, "Groot"}, {HeroId::EmmaFrost, "Emma Frost"},
    {HeroId::Venom, "Venom"}, {HeroId::Magneto, "Magneto"}, {HeroId::Thor, "Thor"},
    {HeroId::PeniParker, "Peni Parker"}, {HeroId::TheThing, "The Thing"}, {HeroId::Colossus, "Colossus"},
    {HeroId::GalactaBotUltra, "Galacta Bot Ultra"}, {HeroId::ThePunisher, "Punisher"}, {HeroId::Storm, "Storm"},
    {HeroId::HumanTorch, "Human Torch"}, {HeroId::Hawkeye, "Hawkeye"}, {HeroId::Hela, "Hela"},
    {HeroId::BlackPanther, "Black Panther"}, {HeroId::Magik, "Magik"}, {HeroId::MoonKnight, "Moon Knight"},
    {HeroId::SquirrelGirl, "Squirrel Girl"}, {HeroId::BlackWidow, "Black Widow"}, {HeroId::IronMan, "Iron Man"},
    {HeroId::SpiderMan, "Spider-Man"}, {HeroId::ScarletWitch, "Scarlet Witch"}, {HeroId::WinterSoldier, "Winter Soldier"},
    {HeroId::StarLord, "Star-Lord"}, {HeroId::Namor, "Namor"}, {HeroId::Psylocke, "Psylocke"},
    {HeroId::Wolverine, "Wolverine"}, {HeroId::IronFist, "Iron Fist"}, {HeroId::MisterFantastic, "Mr. Fantastic"},
    {HeroId::PastePotPete, "Paste-Pot Pete"}, {HeroId::Nightcrawler, "Nightcrawler"}, {HeroId::Gambit, "Gambit"},
    {HeroId::LobbyNPC, "Lobby NPC"}, {HeroId::HeroZero, "Hero Zero"}, {HeroId::Loki, "Loki"},
    {HeroId::Mantis, "Mantis"}, {HeroId::RocketRaccoon, "Rocket Raccoon"}, {HeroId::CloakAndDagger, "Cloak & Dagger"},
    {HeroId::LunaSnow, "Luna Snow"}, {HeroId::AdamWarlock, "Adam Warlock"}, {HeroId::JeffTheLandShark, "Jeff"},
    {HeroId::InvisibleWoman, "Invisible Woman"}, {HeroId::ProfessorX, "Professor X"}, {HeroId::JiaJing, "Jia Jing"},
    {HeroId::Locus, "Locus"}, {HeroId::Rogue, "Rogue"}, {HeroId::GalactaBot, "Galacta Bot"},
    {HeroId::Beast, "Beast"}, {HeroId::Jubilee, "Jubilee"}
};

D3DMATRIX MatrixMultiplication(const D3DMATRIX &pM1, const D3DMATRIX &pM2) {
    D3DMATRIX result{};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m[i][j] = pM1.m[i][0] * pM2.m[0][j] + pM1.m[i][1] * pM2.m[1][j] + pM1.m[i][2] * pM2.m[2][j] + pM1.m[i][3] * pM2.m[3][j];
        }
    }
    return result;
}

D3DMATRIX Matrix(const FVector &rot, FVector origin) {
    const float radPitch = (rot.x * float(M_PI) / 180.f); float radYaw = (rot.y * float(M_PI) / 180.f);
    const float radRoll = (rot.z * float(M_PI) / 180.f);
    const float SP = sinf(radPitch);
    const float CP = cosf(radPitch);
    const float SY = sinf(radYaw);
    const float CY = cosf(radYaw);
    const float SR = sinf(radRoll);
    const float CR = cosf(radRoll);
    D3DMATRIX matrix{};
    matrix.m[0][0] = CP * CY; matrix.m[0][1] = CP * SY; matrix.m[0][2] = SP; matrix.m[0][3] = 0.f;
    matrix.m[1][0] = SR * SP * CY - CR * SY; matrix.m[1][1] = SR * SP * SY + CR * CY; matrix.m[1][2] = -SR * CP; matrix.m[1][3] = 0.f;
    matrix.m[2][0] = -(CR * SP * CY + SR * SY); matrix.m[2][1] = CY * SR - CR * SP * SY; matrix.m[2][2] = CR * CP; matrix.m[2][3] = 0.f;
    matrix.m[3][0] = static_cast<float>(origin.x); matrix.m[3][1] = static_cast<float>(origin.y); matrix.m[3][2] = static_cast<float>(origin.z); matrix.m[3][3] = 1.f;
    return matrix;
}

FVector WorldToScreen(const FVector &WorldLocation) {
    if (GameScreen.Width <= 0 || GameScreen.Height <= 0) {
        return FVector(-1, -1, 0);
    }
    const float ScreenCenterX = GameScreen.Width / 2.0f;
    const float ScreenCenterY = GameScreen.Height / 2.0f;
    auto screenLocation = FVector(0, 0, 0);

    uintptr_t playerCameraAddr;
    { std::lock_guard lock(GameDataMutex); playerCameraAddr = GameAddrs.PlayerCamera; }
    if (!playerCameraAddr) {
        return FVector(-1, -1, 0);
    }

    FMinimalViewInfo CameraInfo;
    try {
        CameraInfo = ReadMemory<FMinimalViewInfo>(playerCameraAddr + Offsets::CameraCache + 0x10);
    } catch (const std::runtime_error&) {
        return FVector(-1, -1, 0);
    }

    if (CameraInfo.FOV <= 0.f || std::isnan(CameraInfo.Location.x) || std::isnan(CameraInfo.Rotation.y) || std::isinf(CameraInfo.Location.x)) {
         return FVector(-1, -1, 0);
    }

    const FVector CameraLocation = CameraInfo.Location; FVector CameraRotation = CameraInfo.Rotation;
    const D3DMATRIX tempMatrix = Matrix(CameraRotation, FVector(0, 0, 0));
    const FVector vAxisX(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
    const FVector vAxisY(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
    const FVector vAxisZ(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);
    const FVector vDelta = WorldLocation - CameraLocation;
    const auto vTransformed = FVector(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

    if (vTransformed.z <= 1.0f) {
        return FVector(-1, -1, 0);
    }

    const float FovAngle = CameraInfo.FOV;
    const float fovRadians = FovAngle * static_cast<float>(M_PI) / 180.0f;
    const float tanHalfFov = tanf(fovRadians / 2.0f);
    if (vTransformed.z < 0.01f || tanHalfFov < 0.01f) {
        return FVector(-1, -1, 0);
    }

    screenLocation.x = ScreenCenterX + vTransformed.x * (ScreenCenterX / tanHalfFov) / vTransformed.z;
    screenLocation.y = ScreenCenterY - vTransformed.y * (ScreenCenterX / tanHalfFov) / vTransformed.z;

    if (std::isnan(screenLocation.x) || std::isnan(screenLocation.y)) {
        return FVector(-1,-1, 0);
    }

    return screenLocation;
}

PlayerLocation GetPlayerLocation(const uintptr_t playerActor) {
    PlayerLocation result = { FVector(), FVector(), false };
    if (!playerActor) return result;
    uintptr_t rootComponent = 0;
    try {
        rootComponent = ReadMemory<uintptr_t>(playerActor + Offsets::RootComponent);
    } catch (...) { return result;}

    if (!rootComponent) return result;

    try {
        result.location = ReadMemory<FVector>(rootComponent + Offsets::RelativeLocation);
    } catch (...) { return result;}

    if (std::isnan(result.location.x) || std::isinf(result.location.x)) return result;
    result.screenPos = WorldToScreen(result.location);
    if (result.screenPos.x >= 0 && result.screenPos.y >= 0 && result.screenPos.x < GameScreen.Width && result.screenPos.y < GameScreen.Height) {
        result.isValid = true;
    } else {
        result.isValid = false;
    }
    return result;
}

bool isEnemy(const uintptr_t playerState) {
    if (!playerState) return false;

    uintptr_t localPlayerStateAddr = 0; {
        uintptr_t currentLocalController = 0;
        std::lock_guard lock(GameDataMutex);
        currentLocalController = GameAddrs.PlayerController;
        if (!currentLocalController) {
            return true;
        }
        try {
            localPlayerStateAddr = ReadMemory<uintptr_t>(currentLocalController + Offsets::PlayerState);
        } catch (...) {
            return true;
        }
    }

    if (!localPlayerStateAddr) {
        return true;
    }

    int enemyTeamID = -1, localTeamID = -1;
    bool enemyReadSuccess = false, localReadSuccess = false;
    try {
        enemyTeamID = ReadMemory<int>(playerState + Offsets::TeamID);
        enemyReadSuccess = true;
    } catch(...) { }
    try {
        localTeamID = ReadMemory<int>(localPlayerStateAddr + Offsets::TeamID);
        localReadSuccess = true;
    } catch(...) { }


    if (localTeamID < 0 || enemyTeamID < 0) return true;
    return localTeamID != enemyTeamID;
}


FVector GetBone(const Actors &actor, int id) {
    if (!actor.PlayerActor || id < 0 || !actor.Mesh) {
        return FVector(0, 0, 0);
    }
    uintptr_t mesh = actor.Mesh;

    try {
        FTransform_f componentToWorld_f;
        bool readSuccessCtW = false;
        try {
            componentToWorld_f = ReadMemory<FTransform_f>(mesh + Offsets::ComponentToWorld);
            readSuccessCtW = true;
        } catch (...) {
            return FVector(0, 0, 0);
        }

        if (!readSuccessCtW ||
            std::isnan(componentToWorld_f.translation.x) || std::isinf(componentToWorld_f.translation.x) ||
            std::isnan(componentToWorld_f.translation.y) || std::isinf(componentToWorld_f.translation.y) ||
            std::isnan(componentToWorld_f.translation.z) || std::isinf(componentToWorld_f.translation.z) ||
            std::isnan(componentToWorld_f.rot.w) || std::isinf(componentToWorld_f.rot.w) ||
            componentToWorld_f.scale_f.x <= 0.00001f || componentToWorld_f.scale_f.x >= 10000.0f ||
            componentToWorld_f.scale_f.y <= 0.00001f || componentToWorld_f.scale_f.y >= 10000.0f ||
            componentToWorld_f.scale_f.z <= 0.00001f || componentToWorld_f.scale_f.z >= 10000.0f) {
            return FVector(0, 0, 0);
        }

        float translation_magnitude_sq = componentToWorld_f.translation.x * componentToWorld_f.translation.x +
                                         componentToWorld_f.translation.y * componentToWorld_f.translation.y +
                                         componentToWorld_f.translation.z * componentToWorld_f.translation.z;
        float rot_magnitude_sq = componentToWorld_f.rot.x * componentToWorld_f.rot.x +
                                 componentToWorld_f.rot.y * componentToWorld_f.rot.y +
                                 componentToWorld_f.rot.z * componentToWorld_f.rot.z +
                                 componentToWorld_f.rot.w * componentToWorld_f.rot.w;

        if (! (std::abs(componentToWorld_f.rot.x) <= 1.01f && std::abs(componentToWorld_f.rot.y) <= 1.01f &&
               std::abs(componentToWorld_f.rot.z) <= 1.01f && std::abs(componentToWorld_f.rot.w) <= 1.01f &&
               (std::abs(rot_magnitude_sq - 1.0f) < 0.1f) &&
               translation_magnitude_sq > (0.001f * 0.001f) && translation_magnitude_sq < (2e7f * 2e7f)) ) {
            return FVector(0,0,0);
        }


        uintptr_t boneTArrayAddress = mesh + Offsets::BoneArray;
        auto boneArrayDataPtr = ReadMemory<uintptr_t>(boneTArrayAddress);

        if (int boneArrayCount = ReadMemory<int>(boneTArrayAddress + sizeof(uintptr_t)); !boneArrayDataPtr || boneArrayCount <= 0 || id < 0 || id >= boneArrayCount || id >= 200) {
            return FVector(0, 0, 0);
        }

        constexpr int BONE_TRANSFORM_F_SIZE = sizeof(FTransform_f);

        auto boneTransform_f = ReadMemory<FTransform_f>(boneArrayDataPtr + (id * BONE_TRANSFORM_F_SIZE));

        if (std::isnan(boneTransform_f.translation.x) || std::isinf(boneTransform_f.translation.x) ||
            std::isnan(boneTransform_f.translation.y) || std::isinf(boneTransform_f.translation.y) ||
            std::isnan(boneTransform_f.translation.z) || std::isinf(boneTransform_f.translation.z) ||
            std::isnan(boneTransform_f.rot.w) || std::isinf(boneTransform_f.rot.w)
            ) {
            return FVector(0, 0, 0);
        }

        D3DMATRIX boneMatrix = boneTransform_f.ToMatrixWithScale_f();
        D3DMATRIX componentMatrix = componentToWorld_f.ToMatrixWithScale_f();
        D3DMATRIX resultMatrix = MatrixMultiplication(boneMatrix, componentMatrix);

        FVector result(resultMatrix.m[3][0], resultMatrix.m[3][1], resultMatrix.m[3][2]);

        if (double result_magnitude_sq = result.x * result.x + result.y * result.y + result.z * result.z; std::isnan(result.x) || std::isinf(result.x) || result_magnitude_sq < (0.001*0.001) || result_magnitude_sq > (2e7*2e7)) {
            return FVector(0,0,0);
        }
        return result;

    } catch (const std::runtime_error&) {
        return FVector(0, 0, 0);
    } catch (...) {
        return FVector(0,0,0);
    }
}

std::string GetHeroName(const uintptr_t playerState) {
    if (!playerState) return "Unknown";
    try {
        int heroIdInt = ReadMemory<int>(playerState + Offsets::SelectedHeroID);
        const auto heroId = static_cast<HeroId>(heroIdInt);
        if (const auto it = hero_names.find(heroId); it != hero_names.end()) return it->second;
        return "Unknown";
    } catch (...) { return "Unknown"; }
}

PlayerInfo GetHealthInfo(const Actors& actor) {
    PlayerInfo info; info.current_health = 0.0f; info.max_health = 100.0f;
    if (!actor.PlayerActor) return info;
    try {
        const auto reactiveComponent = ReadMemory<uintptr_t>(actor.PlayerActor + Offsets::UReactivePropertyComponent);
        if (!reactiveComponent) return info;
        const auto attributeSet = ReadMemory<uintptr_t>(reactiveComponent + Offsets::CachedAttributeSet);
        if (!attributeSet) return info;
        auto currentHealth = ReadMemory<float>(attributeSet + Offsets::Health);
        auto maxHealth = ReadMemory<float>(attributeSet + Offsets::MaxHealth);
        if (std::isnan(currentHealth) || currentHealth < 0.0f || std::isinf(currentHealth)) currentHealth = 0.0f;
        if (std::isnan(maxHealth) || maxHealth <= 0.0f || std::isinf(maxHealth)) maxHealth = (currentHealth > 0.0f ? currentHealth : 100.0f);

        info.current_health = std::min(currentHealth, maxHealth);
        info.max_health = maxHealth;

    } catch (...) {
        info.current_health = 0.0f; info.max_health = 100.0f;
    }
    return info;
}