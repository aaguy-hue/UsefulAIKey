#pragma once

#include "DataStorage.h"
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

        void Action_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void InitializeComponent();


    private:
        // Reads the saved action kind (async) and points the ComboBox at it.
        winrt::fire_and_forget RestoreSelectedActionAsync();

        // Kept alive as a member so the ComboBox's ItemsSource stays valid for the
        // lifetime of the window.
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::UsefulAIKey::ActionOption> m_options{ nullptr };

        winrt::UsefulAIKey::AppPickerView m_appPickerView{ nullptr };
        winrt::UsefulAIKey::WebsiteLaunchView m_websiteLaunchView{ nullptr };

        DataStorage m_dataStorage;

        winrt::hstring m_savedAppPath;

        void ShowLaunchAppView();
        void ShowLaunchWebsiteView();
        void ShowOpenFileView();
    };
}

namespace winrt::UsefulAIKey::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
