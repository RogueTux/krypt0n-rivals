#include "../include/Memory.h"
#include "../include/GameData.h"

#include <iostream>
#include <cstring>
#include <signal.h>
#include <vector>
#include <sys/uio.h>
#include <algorithm>

pid_t ProcessId = 0;
long BaseAddress = 0;

template<typename T>
T ReadMemory(pid_t pid, long address)
{
    T buffer;
    iovec local[1];
    iovec remote[1];

    local[0].iov_base = &buffer;
    local[0].iov_len = sizeof(T);
    remote[0].iov_base = (void*)address;
    remote[0].iov_len = sizeof(T);

    ssize_t nread = process_vm_readv(pid, local, 1, remote, 1, 0);
    if (nread != sizeof(T))
    {
        memset(&buffer, 0, sizeof(T));
    }

    return buffer;
}

template<typename T>
T ReadMemory(const uintptr_t address)
{
    return ReadMemory<T>(ProcessId, address);
}

template<typename T>
bool WriteMemory(const pid_t pid, const long address, const T& value)
{
    iovec local[1];
    iovec remote[1];

    local[0].iov_base = const_cast<void*>(static_cast<const void*>(&value));
    local[0].iov_len = sizeof(T);
    remote[0].iov_base = reinterpret_cast<void *>(address);
    remote[0].iov_len = sizeof(T);

    ssize_t nwrite = process_vm_writev(pid, local, 1, remote, 1, 0);
    return (nwrite == sizeof(T));
}

std::string ReadString(const pid_t pid, const long address, const size_t maxLength)
{
    if (maxLength == 0) return "";
    std::vector<char> buffer(maxLength, 0);

    iovec local[1];
    iovec remote[1];

    local[0].iov_base = buffer.data();
    local[0].iov_len = maxLength;
    remote[0].iov_base = reinterpret_cast<void *>(address);
    remote[0].iov_len = maxLength;

    ssize_t nread = process_vm_readv(pid, local, 1, remote, 1, 0);
    if (nread <= 0)
    {
        return "";
    }

    buffer[std::min(static_cast<size_t>(nread), maxLength -1)] = '\0';
    return std::string(buffer.data());
}

bool InitializeMemoryAccess()
{
    if (ProcessId <= 0)
    {
        return false;
    }

    try {
        ReadMemory<int>(ProcessId, BaseAddress);
    } catch (const std::exception&) {
        return false;
    }

    if (kill(ProcessId, 0) != 0)
    {
        return false;
    }
    return true;
}

template int ReadMemory<int>(pid_t pid, long address);
template long ReadMemory<long>(pid_t pid, long address);
template float ReadMemory<float>(pid_t pid, long address);
template double ReadMemory<double>(pid_t pid, long address);
template uintptr_t ReadMemory<uintptr_t>(pid_t pid, long address);
template FVector ReadMemory<FVector>(pid_t pid, long address);
template char ReadMemory<char>(pid_t pid, long address);
template unsigned char ReadMemory<unsigned char>(pid_t pid, long address);

template bool WriteMemory<int>(pid_t pid, long address, const int& value);
template bool WriteMemory<long>(pid_t pid, long address, const long& value);
template bool WriteMemory<float>(pid_t pid, long address, const float& value);
template bool WriteMemory<double>(pid_t pid, long address, const double& value);

template int ReadMemory<int>(uintptr_t address);
template long ReadMemory<long>(uintptr_t address);
template float ReadMemory<float>(uintptr_t address);
template double ReadMemory<double>(uintptr_t address);
template uintptr_t ReadMemory<uintptr_t>(uintptr_t address);
template FVector ReadMemory<FVector>(uintptr_t address);
template bool ReadMemory<bool>(uintptr_t address);
template FMinimalViewInfo ReadMemory<FMinimalViewInfo>(uintptr_t address);
template PlayerLocation ReadMemory<PlayerLocation>(uintptr_t address);
template FTransform ReadMemory<FTransform>(uintptr_t address);
template FTransform_f ReadMemory<FTransform_f>(uintptr_t address);
template unsigned int ReadMemory<unsigned int>(uintptr_t address);
template unsigned char ReadMemory<unsigned char>(uintptr_t address);
template char ReadMemory<char>(uintptr_t address);