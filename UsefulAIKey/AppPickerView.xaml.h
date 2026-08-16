#pragma once

#include "DataStorage.h"
#include "WindowsApps.h"
#include "AppPickerView.g.h"

using namespace winrt::Windows::Foundation;
using namespace winrt::Microsoft::UI::Xaml;

namespace winrt::UsefulAIKey::implementation
{
    struct AppPickerView : AppPickerViewT<AppPickerView>
    {
        AppPickerView()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        void InitializeComponent();

        // Called by the host (MainWindow) to start loading, handing us the saved app
        // path so we don't have to read the settings file a second time.
        void Load(hstring const& savedAppPath);

        // Event handlers
        void SearchBox_TextChanged(Controls::AutoSuggestBox const& sender, winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs const& args);
        void AppsList_ItemClick(IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::ItemClickEventArgs const& e);
        void BrowseForApp_Click(IInspectable const& sender, RoutedEventArgs const& e);

    private:
        DataStorage m_datastorage;

        // Every app we found, sorted by name (the master list).
        std::vector<winrt::UsefulAIKey::AppItem> m_allApps;

        // The subset currently shown in the ListView: filtered by the search box
        // and ordered pinned-first. This is what AppsList.ItemsSource points at.
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::UsefulAIKey::AppItem> m_visibleApps{ nullptr };

        // The app the user has chosen (pinned to the top). Null until they click one.
        winrt::UsefulAIKey::AppItem m_savedApp{ nullptr };

        // The saved app path handed to us by Load(), used once to restore selection.
        winrt::hstring m_savedAppPath;

        // Loads the start-menu apps (off-thread) then populates the lists.
        winrt::fire_and_forget LoadAppsAsync();

        // NOTE: coroutine parameters are taken BY VALUE on purpose. A coroutine does
        // not extend the lifetime of reference parameters across a co_await, so a
        // reference here would dangle after the first suspension and crash.
        winrt::fire_and_forget OpenFilePicker(winrt::Microsoft::UI::WindowId windowId);

        winrt::fire_and_forget AddAppToList(AppEntry entry);
        winrt::fire_and_forget SelectApp(UsefulAIKey::AppItem item);

        // Pure-UI selection (no persistence); reused by startup restore.
        void HighlightApp(UsefulAIKey::AppItem const& item);
        
        // Rebuilds m_visibleApps from m_allApps using the current search text,
        // placing the pinned app first.
        void RefreshVisibleApps();
    };
}

namespace winrt::UsefulAIKey::factory_implementation
{
    struct AppPickerView : AppPickerViewT<AppPickerView, implementation::AppPickerView>
    {
    };
}
