#include "pch.h"
#include "AppPickerView.xaml.h"

#include "AppItem.h"
#include "DataStorage.h"
#include "WindowsApps.h"

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

        // Kick off the (potentially slow) enumeration without blocking the window.
        LoadAppsAsync();
	}

    // Apps list stuff

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

        RefreshVisibleApps();

        // select saved option
        UsefulAIKey::SavedSelection saved = co_await m_datastorage.LoadSelectedOptionAsync();
        if (saved.Kind == UsefulAIKey::ActionKind::LaunchApp && !saved.Command.empty())
        {
            // Find the saved app among the ones we enumerated.
            UsefulAIKey::AppItem match{ nullptr };
            for (auto const& app : m_allApps)
            {
                if (app.Path() == saved.Command) { match = app; break; }
            }

            // Not in the Start Menu (a custom-added app): recreate its row from the
            // saved path, deriving a display name from the filename.
            if (!match)
            {
                std::filesystem::path p{ std::wstring{ saved.Command } };
                match = make<implementation::AppItem>(hstring{ p.stem().wstring() }, saved.Command);
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

        // Copy the path into our own wstring. result.Path() returns a temporary
        // hstring, so holding its raw c_str() pointer would dangle immediately.
        std::wstring path{ result.Path() };
        AppEntry entry = { .name = path, .path = path };
        AddAppToList(entry);
    }

    winrt::fire_and_forget AppPickerView::ShowDialogMessage(hstring title, hstring content,
        hstring primaryButtonText, hstring secondaryButtonText, hstring closeButtonText)
    {
        auto lifetime = this->get_strong();
        using namespace winrt::Microsoft::UI::Xaml::Controls;
        ContentDialog dialog;
        dialog.XamlRoot(this->XamlRoot());
        dialog.Title(box_value(title));
        dialog.Content(box_value(content));
        dialog.PrimaryButtonText(primaryButtonText);
        dialog.SecondaryButtonText(secondaryButtonText);
        dialog.CloseButtonText(closeButtonText);
		co_await dialog.ShowAsync();
    }

    winrt::fire_and_forget AppPickerView::AddAppToList(AppEntry entry)
    {
        auto lifetime = get_strong();
        // check for duplicates first
        for (auto& app : m_allApps)
        {
            if (app.Path() == entry.path)
            {
				ShowDialogMessage(L"Warning", L"This app is already in the list", L"OK");
                SelectApp(app);
                co_return;
            }
        }
        auto item = make<implementation::AppItem>(hstring{ entry.name }, hstring{ entry.path });
        m_allApps.push_back(item);
        SelectApp(item);
        RefreshVisibleApps();

        // Load each icon in the background; the row updates itself when ready.
        get_self<implementation::AppItem>(item)->LoadIconAsync();
    }

    // Highlights an app in the list (deselect the old, select the new, float to top).
    // Pure UI, no persistence -- so startup-restore can reuse it without re-saving.
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
            ShowDialogMessage(L"Error", L"Couldn't read the saved settings file.", L"OK");
            break;
        case UsefulAIKey::SavingError::ParseError:
            ShowDialogMessage(L"Error", L"The saved settings file is corrupted.", L"OK");
            break;
        case UsefulAIKey::SavingError::CapacityExceeded:
            ShowDialogMessage(L"Error", L"Settings data exceeds the maximum capacity.", L"OK");
            break;
        case UsefulAIKey::SavingError::WriteError:
            ShowDialogMessage(L"Error", L"Couldn't save the settings file.", L"OK");
            break;
        }
    }

}
