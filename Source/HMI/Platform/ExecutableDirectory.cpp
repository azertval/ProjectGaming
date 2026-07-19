#include "HMI/Platform/ExecutableDirectory.h"

#include <Windows.h>

namespace hmi {

std::filesystem::path executableDirectory() {
    wchar_t buffer[MAX_PATH];
    const DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(std::wstring(buffer, length)).parent_path();
}

}  // namespace hmi
