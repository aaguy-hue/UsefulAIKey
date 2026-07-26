#pragma once

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

        void ShowLaunchAppView();

    private:
        // Kept alive as a member so the ComboBox's ItemsSource stays valid for the
        // lifetime of the window.
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::UsefulAIKey::ActionOption> m_options{ nullptr };

        winrt::UsefulAIKey::AppPickerView m_appPickerView{ nullptr };
    };
}

namespace winrt::UsefulAIKey::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
