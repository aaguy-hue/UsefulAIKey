#pragma once

#include <vector>
#include <string>

#include <winrt/Windows.Graphics.Imaging.h>

// One start-menu app discovered on the system. Defined here (rather than inside
// the .cpp) so callers like MainWindow can read the fields to build AppItem rows.
struct AppEntry
{
	std::wstring name;
	std::wstring path;
};

std::vector<AppEntry> EnumerateStartMenuApps();

// Extracts an app's icon straight from its file using the Windows shell and
// returns it as a SoftwareBitmap (BGRA8, premultiplied -- ready for a XAML
// SoftwareBitmapSource). Returns nullptr if no icon could be produced.
//
// This uses classic Win32/shell APIs on purpose: under our runFullTrust identity
// they run with the user's real token, so they can read the target .exe without
// the broadFileSystemAccess capability that the WinRT StorageFile broker needs.
winrt::Windows::Graphics::Imaging::SoftwareBitmap LoadIconBitmap(std::wstring const& path, int size);
