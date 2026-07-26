#pragma once

#include <vector>
#include <string>

// One start-menu app discovered on the system. Defined here (rather than inside
// the .cpp) so callers like MainWindow can read the fields to build AppItem rows.
struct AppEntry
{
	std::wstring name;
	std::wstring path;
	std::wstring iconPath;
};

std::vector<AppEntry> EnumerateStartMenuApps();
