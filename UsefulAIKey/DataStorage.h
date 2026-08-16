#pragma once
#include "AppItem.h"
#include "ActionOption.h"
#include <winrt/UsefulAIKey.h>
#include <winrt/Windows.Foundation.Collections.h>

class DataStorage
{
public:
	// loads the selected option at start (app/website/file)
	winrt::Windows::Foundation::IAsyncOperation<winrt::UsefulAIKey::SavedSelection>
		LoadSelectedOptionAsync();

	// if you selected an app, save it to a file
	winrt::Windows::Foundation::IAsyncOperation<winrt::UsefulAIKey::SavingError>
		SaveSelectedAppToFileAsync(winrt::UsefulAIKey::AppItem item);

	// saves if you add a custom app to the list
	winrt::Windows::Foundation::IAsyncOperation<winrt::UsefulAIKey::SavingError>
		AddCustomAppAsync(winrt::hstring name, winrt::hstring path);
	
	winrt::Windows::Foundation::IAsyncOperation<winrt::UsefulAIKey::SavingError> RemoveCustomAppAsync(winrt::hstring path);

	// loads list of custom apps that you added to the list
	winrt::Windows::Foundation::IAsyncOperation<
		winrt::Windows::Foundation::Collections::IVector<winrt::UsefulAIKey::CustomApp>>
		LoadCustomAppsAsync();

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

	static inline constexpr std::wstring_view STORAGE_FILE_NAME = L"dataFile.json";
	static inline constexpr uint32_t JSON_FILE_MAX_CAPACITY = 5 * 1024; // 5 KB
};
