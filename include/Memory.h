#ifndef MEMORY_H
#define MEMORY_H

#include <string>
#include <sys/types.h>

struct FTransform_f;
struct FVector;
struct FMinimalViewInfo;
struct PlayerLocation;
struct FTransform;


bool InitializeMemoryAccess();

template<typename T>
T ReadMemory(pid_t pid, long address);

template<typename T>
bool WriteMemory(pid_t pid, long address, const T& value);

std::string ReadString(pid_t pid, long address, size_t maxLength = 128);

extern pid_t ProcessId;
extern long BaseAddress;

#endif