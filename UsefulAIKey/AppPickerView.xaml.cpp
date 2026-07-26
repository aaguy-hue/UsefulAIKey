#include "pch.h"
#include "AppPickerView.xaml.h"

#include "AppItem.h"
#include "WindowsApps.h"

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

        // Only one saved app at a time: deselect the previous choice, select the new one.
        if (m_savedApp && m_savedApp != item)
            m_savedApp.IsSelected(false);

        item.IsSelected(true);
        m_savedApp = item;

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

    winrt::fire_and_forget AppPickerView::AddAppToList(AppEntry entry)
    {
        auto lifetime = get_strong();
        // check for duplicates first
        for (auto& app : m_allApps)
        {
            if (app.Path() == entry.path)
            {
                using namespace winrt::Microsoft::UI::Xaml::Controls;
                ContentDialog dialog;
                // winrt::box_value boxes C++ scalar values into IInspectable objects
                dialog.XamlRoot(this->XamlRoot());
                dialog.Title(box_value(L"Warning"));
                dialog.Content(box_value(L"This app is already in the list"));
                dialog.PrimaryButtonText(L"OK");
                co_await dialog.ShowAsync();

				// Select the existing app in the list
                if (m_savedApp) m_savedApp.IsSelected(false);
                app.IsSelected(true);
				RefreshVisibleApps();
                co_return;
            }
        }
        auto item = make<implementation::AppItem>(hstring{ entry.name }, hstring{ entry.path });
        m_allApps.push_back(item);
        if (m_savedApp) m_savedApp.IsSelected(false);
        item.IsSelected(true);

        RefreshVisibleApps();

        // Load each icon in the background; the row updates itself when ready.
        get_self<implementation::AppItem>(item)->LoadIconAsync();
        RefreshVisibleApps();
    }

}
