#pragma once
#include "AppItem.h"
#include <winrt/UsefulAIKey.h>

class DataStorage
{
public:
	// Reads the settings file fresh, records the selected app, and writes it back.
	// Returns a SavingError describing the outcome. Never throws, so a bad save
	// can't crash the app.
	winrt::Windows::Foundation::IAsyncOperation<winrt::UsefulAIKey::SavingError>
		SaveSelectedAppToFileAsync(winrt::UsefulAIKey::AppItem item);

private:
	// Reusable building blocks -- any future load/save path (the app list, reading
	// settings on startup, ...) can lean on these instead of re-doing the I/O.

	// Opens + reads + parses the settings file into `out`. Returns None (and fills
	// `out`), ReadError (couldn't open/read), or ParseError (file wasn't valid JSON).
	winrt::Windows::Foundation::IAsyncOperation<winrt::UsefulAIKey::SavingError>
		LoadJsonAsync(winrt::Windows::Data::Json::JsonObject& out);

	// Validates size, then writes `root` back to the file. Returns None,
	// CapacityExceeded, or WriteError.
	winrt::Windows::Foundation::IAsyncOperation<winrt::UsefulAIKey::SavingError>
		WriteJsonAsync(winrt::Windows::Data::Json::JsonObject root);

	static inline constexpr std::wstring_view STORAGE_FILE_NAME = L"dataFile.txt";
	static inline constexpr uint32_t JSON_FILE_MAX_CAPACITY = 5 * 1024; // 5 KB
};
