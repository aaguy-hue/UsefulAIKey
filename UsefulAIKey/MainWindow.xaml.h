#pragma once

#include "WindowsApps.h"
#include "MainWindow.g.h"

namespace winrt::UsefulAIKey::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        int32_t MyProperty();
        void MyProperty(int32_t value);
        void Action_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void InitializeComponent();


        // App-picker event handlers, wired up from MainWindow.xaml.
        void SearchBox_TextChanged(winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs const& args);
        void AppsList_ItemClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::ItemClickEventArgs const& e);
        void BrowseForApp_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        // Kept alive as a member so the ComboBox's ItemsSource stays valid for the
        // lifetime of the window.
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::UsefulAIKey::ActionOption> m_options{ nullptr };

        // Every app we found, sorted by name (the master list).
        std::vector<winrt::UsefulAIKey::AppItem> m_allApps;

        // The subset currently shown in the ListView: filtered by the search box
        // and ordered pinned-first. This is what AppsList.ItemsSource points at.
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::UsefulAIKey::AppItem> m_visibleApps{ nullptr };

        // The app the user has chosen (pinned to the top). Null until they click one.
        winrt::UsefulAIKey::AppItem m_savedApp{ nullptr };

        // Loads the start-menu apps (off-thread) then populates the lists.
        winrt::fire_and_forget LoadAppsAsync();

        // NOTE: coroutine parameters are taken BY VALUE on purpose. A coroutine does
        // not extend the lifetime of reference parameters across a co_await, so a
        // reference here would dangle after the first suspension and crash.
        winrt::fire_and_forget OpenFilePicker(winrt::Microsoft::UI::WindowId windowId,
            winrt::Microsoft::UI::Xaml::XamlRoot xamlRoot);

        winrt::fire_and_forget AddAppToList(AppEntry entry,
            winrt::Microsoft::UI::Xaml::XamlRoot xamlRoot);

        // Rebuilds m_visibleApps from m_allApps using the current search text,
        // placing the pinned app first.
        void RefreshVisibleApps();
    };
}

namespace winrt::UsefulAIKey::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
