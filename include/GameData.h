#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <cmath>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b))))

struct ScreenDimensions {
    int Width = 3440;
    int Height = 1440;
};
extern ScreenDimensions GameScreen;

class FVector
{
public:
    FVector() : x(0.f), y(0.f), z(0.f) {}
    FVector(const double _x, const double _y, const double _z) : x(_x), y(_y), z(_z) {}
    ~FVector() = default;

    double x;
    double y;
    double z;

    [[nodiscard]] double Dot(const FVector v) const {
        return x * v.x + y * v.y + z * v.z;
    }

    [[nodiscard]] float Distance(const FVector v) const {
        const double dx = this->x - v.x;
        const double dy = this->y - v.y;
        const double dz = this->z - v.z;
        return sqrtf(static_cast<float>((dx * dx) + (dy * dy) + (dz * dz)));
    }

    FVector operator+(const FVector& v) const {
        return FVector(x + v.x, y + v.y, z + v.z);
    }

    FVector operator-(const FVector& v) const {
        return FVector(x - v.x, y - v.y, z - v.z);
    }

    FVector operator*(double number) const {
        return FVector(x * number, y * number, z * number);
    }

     [[nodiscard]] double Magnitude() const {
        return sqrt(x * x + y * y + z * z);
    }

    [[nodiscard]] double Length() const {
        return Magnitude();
    }

    [[nodiscard]] FVector Normalize() const {
        FVector vector;
        if (const double length = this->Magnitude(); length != 0) {
            vector.x = x / length;
            vector.y = y / length;
            vector.z = z / length;
        } else {
            vector.x = 1.0f;
            vector.y = 0.0f;
            vector.z = 0.0f;
        }
        return vector;
    }

    FVector& operator+=(const FVector& v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

     FVector& operator-=(const FVector& v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }
};

struct FVector_f {
    float x, y, z;
    FVector_f() : x(0.f), y(0.f), z(0.f) {}
    FVector_f(const float _x, const float _y, const float _z) : x(_x), y(_y), z(_z) {}
};

struct FQuat_f {
    float x, y, z, w;
    FQuat_f() : x(0.f), y(0.f), z(0.f), w(1.f) {}
    FQuat_f(const float _x, const float _y, const float _z, const float _w) : x(_x), y(_y), z(_z), w(_w) {}
};


struct D3DMATRIX {
    union {
        struct {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
    };
};

struct FQuat {
    double x;
    double y;
    double z;
    double w;
};

struct FTransform {
    FQuat rot;
    FVector translation;
    uint8_t Pad_38[0x8];
    FVector scale;
    uint8_t Pad_58[0x8];

    [[nodiscard]] D3DMATRIX ToMatrixWithScale() const {
        D3DMATRIX m{};
        m._41 = static_cast<float>(translation.x);
        m._42 = static_cast<float>(translation.y);
        m._43 = static_cast<float>(translation.z);

        const double x2 = rot.x + rot.x;
        const double y2 = rot.y + rot.y;
        const double z2 = rot.z + rot.z;

        const double xx2 = rot.x * x2;
        const double yy2 = rot.y * y2;
        const double zz2 = rot.z * z2;
        m._11 = static_cast<float>((1.0 - (yy2 + zz2)) * scale.x);
        m._22 = static_cast<float>((1.0 - (xx2 + zz2)) * scale.y);
        m._33 = static_cast<float>((1.0 - (xx2 + yy2)) * scale.z);

        const double yz2 = rot.y * z2;
        const double wx2 = rot.w * x2;
        m._32 = static_cast<float>((yz2 - wx2) * scale.z);
        m._23 = static_cast<float>((yz2 + wx2) * scale.y);

        const double xy2 = rot.x * y2;
        const double wz2 = rot.w * z2;
        m._21 = static_cast<float>((xy2 - wz2) * scale.y);
        m._12 = (float)((xy2 + wz2)) * scale.x;

        const double xz2 = rot.x * z2;
        const double wy2 = rot.w * y2;
        m._31 = static_cast<float>((xz2 + wy2) * scale.z);
        m._13 = static_cast<float>((xz2 - wy2) * scale.x);

        m._14 = 0.0f;
        m._24 = 0.0f;
        m._34 = 0.0f;
        m._44 = 1.0f;

        return m;
    }
};

struct FTransform_f {
    FQuat_f rot;
    FVector_f translation;
    FVector_f scale_f;

    D3DMATRIX ToMatrixWithScale_f() const {
        D3DMATRIX m;
        m._41 = translation.x;
        m._42 = translation.y;
        m._43 = translation.z;

        const float x2 = rot.x + rot.x;
        const float y2 = rot.y + rot.y;
        const float z2 = rot.z + rot.z;

        const float xx2 = rot.x * x2;
        const float yy2 = rot.y * y2;
        const float zz2 = rot.z * z2;
        m._11 = (1.0f - (yy2 + zz2)) * scale_f.x;
        m._22 = (1.0f - (xx2 + zz2)) * scale_f.y;
        m._33 = (1.0f - (xx2 + yy2)) * scale_f.z;

        const float yz2 = rot.y * z2;
        const float wx2 = rot.w * x2;
        m._32 = (yz2 - wx2) * scale_f.z;
        m._23 = (yz2 + wx2) * scale_f.y;

        const float xy2 = rot.x * y2;
        const float wz2 = rot.w * z2;
        m._21 = (xy2 - wz2) * scale_f.y;
        m._12 = (xy2 + wz2) * scale_f.x;

        const float xz2 = rot.x * z2;
        const float wy2 = rot.w * y2;
        m._31 = (xz2 + wy2) * scale_f.z;
        m._13 = (xz2 - wy2) * scale_f.x;

        m._14 = 0.0f;
        m._24 = 0.0f;
        m._34 = 0.0f;
        m._44 = 1.0f;

        return m;
    }
};


struct FMinimalViewInfo {
    FVector Location;
    FVector Rotation;
    float FOV;
};

struct PlayerInfo {
    float current_health = 0.0f;
    float max_health = 100.0f;
};

enum class HeroId : int {
    Unknown = 0,
    BruceBanner = 1011, DoctorStrange = 1018, CaptainAmerica = 1022, Groot = 1027,
    EmmaFrost = 1053, Venom = 1035, Magneto = 1037, Thor = 1039, PeniParker = 1042,
    TheThing = 1051, Colossus = 1060, GalactaBotUltra = 4018, ThePunisher = 1014,
    Storm = 1015, HumanTorch = 1017, Hawkeye = 1021, Hela = 1024, BlackPanther = 1026,
    Magik = 1029, MoonKnight = 1030, SquirrelGirl = 1032, BlackWidow = 1033,
    IronMan = 1034, SpiderMan = 1036, ScarletWitch = 1038, WinterSoldier = 1041,
    StarLord = 1043, Namor = 1045, Psylocke = 1048, Wolverine = 1049, IronFist = 1052,
    MisterFantastic = 1040, PastePotPete = 1059, Nightcrawler = 1063, Gambit = 1066,
    LobbyNPC = 4017, HeroZero = 9999, Loki = 1016, Mantis = 1020, RocketRaccoon = 1023,
    CloakAndDagger = 1025, LunaSnow = 1031, AdamWarlock = 1046, JeffTheLandShark = 1047,
    InvisibleWoman = 1050, ProfessorX = 1057, JiaJing = 1058, Locus = 1061, Rogue = 1065,
    GalactaBot = 4016, Beast = 1062, Jubilee = 1064
};
extern const std::unordered_map<HeroId, std::string> hero_names;

namespace Offsets {
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=offsets
    constexpr uintptr_t GNAME = 0xdfe4b80;
    constexpr uintptr_t UWorld = 0xe24df38;

    constexpr uintptr_t GameInstance = 0x270; // 0x1318
    constexpr uintptr_t LocalPlayers = 0x40;
    constexpr uintptr_t PlayerController = 0x38;

    constexpr uintptr_t LocalPawn = 0x6b8;
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=classes&idx=APlayerController&member=AcknowledgedPawn
    constexpr uintptr_t GameState = 0x210;
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=classes&idx=UWorld&member=GameState
    constexpr uintptr_t PlayerArray = 0x610;
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=classes&idx=AGameStateBase&member=PlayerArray
    constexpr uintptr_t PawnPrivate = 0x680;
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=classes&idx=APlayerState&member=PawnPrivate
    constexpr uintptr_t ChildActorComponent = 0x698; // Not Used
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=classes&idx=ACharacter&member=ChildActorComponent
    constexpr uintptr_t ChildActor = 0x478; // Not Used
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=classes&idx=UChildActorComponent&member=ChildActor
    constexpr uintptr_t Mesh = 0x5f8;
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=classes&idx=ACharacterChildActorBase&member=Mesh
    constexpr uintptr_t SkeletalMeshAsset = 0xd20; // Not Used
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=classes&idx=USkeletalMeshComponent&member=SkeletalMeshAsset
    constexpr uintptr_t Skeleton = 0x108; // Not Used
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=classes&idx=USkeletalMesh&member=Skeleton
    constexpr uintptr_t RootComponent = 0x420;
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=classes&idx=AActor&member=RootComponent
    constexpr uintptr_t ComponentVelocity = 0x1d0;
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=classes&idx=USceneComponent&member=ComponentVelocity
    constexpr uintptr_t RelativeLocation = 0x188;
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=classes&idx=USceneComponent&member=RelativeLocation
    constexpr uintptr_t PlayerState = 0x600;
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=classes&idx=AController&member=PlayerState
    constexpr uintptr_t TeamID = 0x70c;
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=classes&idx=AMarvelPlayerState&member=TeamID
    constexpr uintptr_t IsAlive = 0x720;
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=classes&idx=AMarvelPlayerState&member=bIsAlive
    constexpr uintptr_t SelectedHeroID = 0x778;
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=classes&idx=AMarvelPlayerState&member=SelectedHeroID
    constexpr uintptr_t UReactivePropertyComponent = 0x15a8;
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=classes&idx=AMarvelBaseCharacter&member=ReactivePropertyComponent
    constexpr uintptr_t CachedAttributeSet = 0x1718;
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=classes&idx=UReactivePropertyComponent&member=CachedAttributeSet

    constexpr uintptr_t Health = 0x40 + 0xC; // 0x44
    constexpr uintptr_t MaxHealth = 0x50 + 0xC; // 0x40
    constexpr uintptr_t LivingState = 0x06d8; // 0x5860

    constexpr uintptr_t CharacterMovement = 0x6a0;
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=classes&idx=ACharacter&member=CharacterMovement
    constexpr uintptr_t LastUpdateVelocity = 0x3f8;
    // https://dumpspace.spuckwaffel.com/Games/?hash=04842ac7&type=classes&idx=UCharacterMovementComponent&member=LastUpdateVelocity

    constexpr uintptr_t LODData = 0x0978; // Not Used
    constexpr uintptr_t BoneArray = 0xE90; // LODData + 0x30
    constexpr uintptr_t BoneCached = BoneArray + 0x10; // Not Used
    constexpr uintptr_t ComponentToWorld = 0x2E0;
    constexpr uintptr_t APlayerCameraManager = 0x6c8;
    constexpr uintptr_t CameraCache = 0x1bb0;
    constexpr uintptr_t BoundsScale = 0x5A4;
    constexpr uintptr_t LAST_SUBMIT_TIME = BoundsScale + 0x4; // Not Used
    constexpr uintptr_t LAST_SUBMIT_TIME_ON_SCREEN = BoundsScale + 0x8; // Not Used
}

struct GameAddresses {
    uintptr_t UWorld = 0;
    uintptr_t GameState = 0;
    uintptr_t PlayerArray = 0;
    int PlayerCount = 0;
    uintptr_t GameInstance = 0;
    uintptr_t LocalPlayers = 0;
    uintptr_t PlayerController = 0;
    uintptr_t AcknowledgedPawn = 0;
    uintptr_t PlayerCamera = 0;
};

struct Actors {
    uintptr_t Mesh;
    uintptr_t PlayerActor;
    uintptr_t PlayerState;
    FVector Velocity;
};

struct PlayerLocation {
    FVector location;
    FVector screenPos;
    bool isValid;
};

extern GameAddresses GameAddrs;
extern std::mutex GameDataMutex;

D3DMATRIX Matrix(const FVector &rot, FVector origin = FVector(0, 0, 0));
D3DMATRIX MatrixMultiplication(const D3DMATRIX &pM1, const D3DMATRIX &pM2);

template<typename T>
T ReadMemory(uintptr_t address);

FVector WorldToScreen(const FVector &WorldLocation);
FVector GetBone(const Actors &actor, int id);
PlayerLocation GetPlayerLocation(uintptr_t playerActor);
bool isEnemy(uintptr_t playerState);
std::string GetHeroName(uintptr_t playerState);
PlayerInfo GetHealthInfo(const Actors& actor);

#endif