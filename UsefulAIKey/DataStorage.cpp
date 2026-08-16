#include "pch.h"
#include "AppItem.h"
#include "ActionOption.h"
#include "DataStorage.h"

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Data::Json;
using namespace UsefulAIKey;

using SavingError = winrt::UsefulAIKey::SavingError;

IAsyncOperation<SavingError> DataStorage::LoadJsonAsync(JsonObject& out)
{
	hstring text;
	try
	{
		StorageFolder localFolder = ApplicationData::Current().LocalFolder();
		StorageFile file = co_await localFolder.CreateFileAsync(
			hstring{ STORAGE_FILE_NAME }, CreationCollisionOption::OpenIfExists);
		text = co_await FileIO::ReadTextAsync(file);
	}
	catch (hresult_error const&)
	{
		co_return SavingError::ReadError;
	}

	if (text.empty())
	{
		out = JsonObject{};
		co_return SavingError::None;
	}

	JsonObject parsed{ nullptr };
	if (!JsonObject::TryParse(text, parsed))
		co_return SavingError::ParseError;

	out = parsed;
	co_return SavingError::None;
}

IAsyncOperation<SavingError> DataStorage::WriteJsonAsync(JsonObject root)
{
	hstring json = root.Stringify();
	if (json.size() > JSON_FILE_MAX_CAPACITY)
		co_return SavingError::CapacityExceeded;

	try
	{
		StorageFolder localFolder = ApplicationData::Current().LocalFolder();
		StorageFile file = co_await localFolder.CreateFileAsync(
			hstring{ STORAGE_FILE_NAME }, CreationCollisionOption::OpenIfExists);
		co_await FileIO::WriteTextAsync(file, json);
		OutputDebugStringW((L"Saved settings file at path: " + file.Path() + L"\n").c_str());
	}
	catch (hresult_error const&)
	{
		co_return SavingError::WriteError;
	}

	co_return SavingError::None;
}

IAsyncOperation<SavingError> DataStorage::SaveSelectedAppToFileAsync(UsefulAIKey::AppItem item)
{
	JsonObject root{ nullptr };

	SavingError loadResult = co_await LoadJsonAsync(root);
	if (loadResult != SavingError::None)
		co_return loadResult; // ReadError or ParseError

	root.SetNamedValue(L"actionType", JsonValue::CreateStringValue(L"app"));
	root.SetNamedValue(L"launchCommand", JsonValue::CreateStringValue(item.Path()));

	co_return co_await WriteJsonAsync(root);
}

IAsyncOperation<SavedSelection> DataStorage::LoadSelectedOptionAsync()
{
	SavedSelection result{};

	JsonObject root{ nullptr };
	SavingError loadResult = co_await LoadJsonAsync(root);
	if (loadResult != SavingError::None)
		co_return result;

	hstring actionType = root.GetNamedString(L"actionType", L"");
	if (actionType == L"app")
	{
		result.Kind = ActionKind::LaunchApp;
		result.Command = root.GetNamedString(L"launchCommand", L"");
	}
	else if (actionType == L"website")
	{
		result.Kind = ActionKind::LaunchWebsite;
		result.Command = root.GetNamedString(L"url", L"");
	}
	else if (actionType == L"file")
	{
		result.Kind = ActionKind::OpenFile;
		result.Command = root.GetNamedString(L"filePath", L"");
	}

	co_return result;
}

IAsyncOperation<SavingError> DataStorage::AddCustomAppAsync(hstring name, hstring path)
{
	JsonObject root{ nullptr };

	SavingError loadResult = co_await LoadJsonAsync(root);
	if (loadResult != SavingError::None)
		co_return loadResult;

	JsonArray customApps = root.HasKey(L"customApps")
		? root.GetNamedArray(L"customApps")
		: JsonArray{};

	// remove duplicates
	for (auto const& value : customApps)
	{
		if (value.GetObject().GetNamedString(L"path", L"") == path)
			co_return SavingError::None;
	}

	JsonObject entry;
	entry.SetNamedValue(L"name", JsonValue::CreateStringValue(name));
	entry.SetNamedValue(L"path", JsonValue::CreateStringValue(path));
	customApps.Append(entry);

	root.SetNamedValue(L"customApps", customApps);

	co_return co_await WriteJsonAsync(root);
}

IAsyncOperation<IVector<CustomApp>> DataStorage::LoadCustomAppsAsync()
{
	IVector<CustomApp> apps = single_threaded_vector<CustomApp>();

	JsonObject root{ nullptr };
	SavingError loadResult = co_await LoadJsonAsync(root);
	if (loadResult != SavingError::None || !root.HasKey(L"customApps"))
		co_return apps; // no file / unreadable / nothing saved -> empty list

	for (auto const& value : root.GetNamedArray(L"customApps"))
	{
		JsonObject obj = value.GetObject();
		CustomApp app{};
		app.Name = obj.GetNamedString(L"name", L"");
		app.Path = obj.GetNamedString(L"path", L"");
		if (app.Path.empty()) continue; // skip malformed entries
		apps.Append(app);
	}

	co_return apps;
}
