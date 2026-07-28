#pragma once

#include "FileOpenView.g.h"

namespace winrt::UsefulAIKey::implementation
{
    struct FileOpenView : FileOpenViewT<FileOpenView>
    {
        FileOpenView()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        void OpenFileMenu_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::fire_and_forget OpenFilePicker(winrt::Microsoft::UI::WindowId windowId);
    };
}

namespace winrt::UsefulAIKey::factory_implementation
{
    struct FileOpenView : FileOpenViewT<FileOpenView, implementation::FileOpenView>
    {
    };
}
