#include "pch.h"
#include "FileOpenView.xaml.h"
#if __has_include("FileOpenView.g.cpp")
#include "FileOpenView.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::UsefulAIKey::implementation
{
    void FileOpenView::OpenFileMenu_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        OpenFilePicker(this->XamlRoot().ContentIslandEnvironment().AppWindowId());
    }

    winrt::fire_and_forget FileOpenView::OpenFilePicker(winrt::Microsoft::UI::WindowId windowId)
    {
        winrt::Microsoft::Windows::Storage::Pickers::FileOpenPicker picker(windowId);
        picker.SuggestedStartLocation(winrt::Microsoft::Windows::Storage::Pickers::PickerLocationId::ComputerFolder);
        picker.CommitButtonText(L"Select File");

        //picker.FileTypeFilter().ReplaceAll({ L".txt", L".cpp", L".h" });

        auto result = co_await picker.PickSingleFileAsync();

        // the picking was cancelled
        if (!result) co_return;

        std::wstring path{ result.Path() };
		FileNameTextBox().Text(path);
    }
}

