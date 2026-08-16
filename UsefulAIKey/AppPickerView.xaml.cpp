#include "pch.h"
#include "AppPickerView.xaml.h"

#include "AppItem.h"
#include "DataStorage.h"
#include "WindowsApps.h"
#include "WinRTUtil.h"

#include <algorithm>
#include <filesystem>

#if __has_include("AppPickerView.g.cpp")
#include "AppPickerView.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace
{
    // Lowercases a string so search matching is case-insensitive.
    std::wstring ToLower(std::wstring_view value)
    {
        std::wstring result(value);
        std::transform(result.begin(), result.end(), result.begin(),
            [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
        return result;
    }
}


namespace winrt::UsefulAIKey::implementation
{

    void AppPickerView::InitializeComponent()
    {
        AppPickerViewT::InitializeComponent();

        // Bind the ListView to an observable vector we control. We rebuild its
        // contents whenever the search text or the pinned app changes.
        m_visibleApps = single_threaded_observable_vector<UsefulAIKey::AppItem>();
        AppsList().ItemsSource(m_visibleApps);
	}

    void AppPickerView::Load(hstring const& savedAppPath)
    {
        m_savedAppPath = savedAppPath;
        LoadAppsAsync();
    }

    fire_and_forget AppPickerView::LoadAppsAsync()
    {
        auto lifetime = get_strong();
        auto uiThread = DispatcherQueue();

        // Enumerating the Start Menu touches the disk and COM, so do it off the
        // UI thread to keep the window responsive.
        co_await resume_background();
        std::vector<AppEntry> entries = EnumerateStartMenuApps();

        // Back on the UI thread to create XAML-visible objects and touch the lists.
        co_await wil::resume_foreground(uiThread);

        m_allApps.clear();
        m_allApps.reserve(entries.size());
        for (auto const& entry : entries)
        {
            auto item = make<implementation::AppItem>(hstring{ entry.name }, hstring{ entry.path });
            m_allApps.push_back(item);

            // Load each icon in the background; the row updates itself when ready.
            get_self<implementation::AppItem>(item)->LoadIconAsync();
        }

        // add in custom saved apps
        auto customApps = co_await m_datastorage.LoadCustomAppsAsync();
        for (auto const& custom : customApps)
        {
            bool alreadyListed = false;
            for (auto const& app : m_allApps)
            {
                if (app.Path() == custom.Path) { alreadyListed = true; break; }
            }
            if (alreadyListed) continue;

            auto item = make<implementation::AppItem>(custom.Name, custom.Path);
            m_allApps.push_back(item);
            get_self<implementation::AppItem>(item)->LoadIconAsync();
        }

        // resort list after adding custom apps
        std::sort(m_allApps.begin(), m_allApps.end(),
            [](UsefulAIKey::AppItem const& a, UsefulAIKey::AppItem const& b)
            {
                return a.Name() < b.Name();
            });

        RefreshVisibleApps();

        if (!m_savedAppPath.empty())
        {
            UsefulAIKey::AppItem match{ nullptr };
            for (auto const& app : m_allApps)
            {
                if (app.Path() == m_savedAppPath) { match = app; break; }
            }

			// uhhhh this shouldn't happen but if somehow the saved app path is no longer in the list, create an entry for it
            if (!match)
            {
                std::filesystem::path p{ std::wstring{ m_savedAppPath } };
                match = make<implementation::AppItem>(hstring{ p.stem().wstring() }, m_savedAppPath);
                m_allApps.push_back(match);
                get_self<implementation::AppItem>(match)->LoadIconAsync();
            }

            HighlightApp(match); // select without re-saving
        }
    }

    void AppPickerView::RefreshVisibleApps()
    {
        if (!m_visibleApps) return;

        std::wstring query = ToLower(std::wstring_view{ SearchBox().Text() });

        // Two passes so the selected app always sits on top. m_allApps is already
        // sorted by name, so each group stays alphabetical.
        std::vector<UsefulAIKey::AppItem> selected;
        std::vector<UsefulAIKey::AppItem> rest;
        for (auto const& item : m_allApps)
        {
            if (!query.empty())
            {
                std::wstring name = ToLower(std::wstring_view{ item.Name() });
                if (name.find(query) == std::wstring::npos) continue;
            }

            if (item.IsSelected())
                selected.push_back(item);
            else
                rest.push_back(item);
        }

        m_visibleApps.Clear();
        for (auto const& item : selected) m_visibleApps.Append(item);
        for (auto const& item : rest) m_visibleApps.Append(item);
    }

    void AppPickerView::SearchBox_TextChanged(
        Controls::AutoSuggestBox const& /* sender */,
        Controls::AutoSuggestBoxTextChangedEventArgs const& /* args */)
    {
        RefreshVisibleApps();
    }

    void AppPickerView::AppsList_ItemClick(
        Windows::Foundation::IInspectable const& /* sender */,
        Controls::ItemClickEventArgs const& e)
    {
        auto item = e.ClickedItem().try_as<UsefulAIKey::AppItem>();
        if (!item) return;

        SelectApp(item);

        // Reorder so the newly selected app jumps to the top.
        RefreshVisibleApps();

        // TODO: persist item.Path() as the app the Copilot key should launch.
        OutputDebugStringW((L"Saved app: " + std::wstring(item.Path()) + L"\n").c_str());
    }

    void AppPickerView::BrowseForApp_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        auto button = sender.try_as<Controls::Button>();
        if (!button) return;

        OpenFilePicker(this->XamlRoot().ContentIslandEnvironment().AppWindowId());
    }

    winrt::fire_and_forget AppPickerView::OpenFilePicker(winrt::Microsoft::UI::WindowId windowId)
    {
        winrt::Microsoft::Windows::Storage::Pickers::FileOpenPicker picker(windowId);
        picker.SuggestedStartLocation(winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId::ComputerFolder);
        picker.CommitButtonText(L"Select App");

        //picker.FileTypeFilter().ReplaceAll({ L".txt", L".cpp", L".h" });
        picker.FileTypeFilter().Append(L".exe");
        picker.FileTypeFilter().Append(L".bat");
        picker.FileTypeFilter().Append(L".cmd");
        picker.FileTypeFilter().Append(L".ps1");

        auto result = co_await picker.PickSingleFileAsync();

        // the picking was cancelled
        if (!result) co_return;

        std::wstring path{ result.Path() };

        std::filesystem::path parsed{ path };
        AppEntry entry = { .name = parsed.stem().wstring(), .path = path };
        AddAppToList(entry);
    }

    winrt::fire_and_forget AppPickerView::AddAppToList(AppEntry entry)
    {
        auto lifetime = get_strong();
        // check for duplicates first
        for (auto& app : m_allApps)
        {
            if (app.Path() == entry.path)
            {
				ShowDialogMessage(this->XamlRoot(), L"Warning", L"This app is already in the list", L"OK");
                SelectApp(app);
                co_return;
            }
        }
        auto item = make<implementation::AppItem>(hstring{ entry.name }, hstring{ entry.path });
        m_allApps.push_back(item);

        HighlightApp(item);
        get_self<implementation::AppItem>(item)->LoadIconAsync(); // load icon in bg

        // save the custom app to disk
        UsefulAIKey::SavingError err = co_await m_datastorage.AddCustomAppAsync(item.Name(), item.Path());
        if (err != UsefulAIKey::SavingError::None)
            ShowDialogMessage(this->XamlRoot(), L"Error", L"Couldn't save the added app.", L"OK");

        SelectApp(item);
    }

    void AppPickerView::HighlightApp(UsefulAIKey::AppItem const& item)
    {
        if (!item) return;

        if (m_savedApp && m_savedApp != item) m_savedApp.IsSelected(false);
        item.IsSelected(true);

        m_savedApp = item;
        RefreshVisibleApps();
    }

    winrt::fire_and_forget AppPickerView::SelectApp(UsefulAIKey::AppItem item)
    {
        // Keep 'this' alive across the co_await below (we touch members after it).
        auto lifetime = get_strong();

        if (!item) co_return;

        HighlightApp(item);

        UsefulAIKey::SavingError err = co_await m_datastorage.SaveSelectedAppToFileAsync(item);
        switch (err)
        {
        case UsefulAIKey::SavingError::None:
            break;
        case UsefulAIKey::SavingError::ReadError:
            ShowDialogMessage(this->XamlRoot(), L"Error", L"Couldn't read the saved settings file.", L"OK");
            break;
        case UsefulAIKey::SavingError::ParseError:
            ShowDialogMessage(this->XamlRoot(), L"Error", L"The saved settings file is corrupted.", L"OK");
            break;
        case UsefulAIKey::SavingError::CapacityExceeded:
            ShowDialogMessage(this->XamlRoot(), L"Error", L"Settings data exceeds the maximum capacity.", L"OK");
            break;
        case UsefulAIKey::SavingError::WriteError:
            ShowDialogMessage(this->XamlRoot(), L"Error", L"Couldn't save the settings file.", L"OK");
            break;
        }
    }
}
