#pragma once

#include <vector>
#include <string>

#include <winrt/Windows.Graphics.Imaging.h>

// A start-menu app discovered on the system
struct AppEntry
{
	std::wstring name;
	std::wstring path;
};

std::vector<AppEntry> EnumerateStartMenuApps();

// Extracts an app's icon from its file
// Returns the icon as a SoftwareBitmap
// Returns nullptr if no icon could be produced.
//
// We use Win32 APIs here because under the runFullTrust identity
// we run with the user's token, so they can read the target .exe without
// the broadFileSystemAccess capability that the WinRT StorageFile broker needs.
// broadFileSystemAccess capability is rarely granted to solo developers
winrt::Windows::Graphics::Imaging::SoftwareBitmap LoadIconBitmap(std::wstring const& path, int size);
