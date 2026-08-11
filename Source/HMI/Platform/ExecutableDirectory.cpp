#include "HMI/Platform/ExecutableDirectory.h"

#include <Windows.h>

#include <array>

namespace hmi {

std::filesystem::path executableDirectory() {
    std::array<wchar_t, MAX_PATH> buffer;
    const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), MAX_PATH);
    return std::filesystem::path(std::wstring(buffer.data(), length)).parent_path();
}

}  // namespace hmi
