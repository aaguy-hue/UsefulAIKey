#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/UsefulAIKey.h>
#include <iostream>

using namespace winrt;
using namespace UsefulAIKey;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::UsefulAIKey::implementation
{
    int32_t MainWindow::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void MainWindow::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void MainWindow::InitializeComponent()
    {
        MainWindowT::InitializeComponent();

        auto m_options = winrt::single_threaded_observable_vector<UsefulAIKey::ActionOption>();

        m_options.Append(winrt::make<UsefulAIKey::ActionOption>(L"app", L"Launch an app" ));
        m_options.Append(winrt::make<UsefulAIKey::ActionOption>(L"website", L"Launch a website"));
        m_options.Append(winrt::make<UsefulAIKey::ActionOption>(L"file", L"Open a file" ));

        ActionComboBox().ItemsSource(m_options);

	}
}

void OutputDebugCharPtr(const char* str) {
    wchar_t wStr[128];
    size_t converted = 0;
    mbstowcs_s(&converted, wStr, str, _TRUNCATE);
    OutputDebugString(wStr);
}


void winrt::UsefulAIKey::implementation::MainWindow::Action_SelectionChanged(
    winrt::Windows::Foundation::IInspectable const& sender, 
    winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e
)
{
    OutputDebugString(L"Hello\n");

    auto comboBox = sender.as<winrt::Microsoft::UI::Xaml::Controls::ComboBox>();
    if (!comboBox) return;

    winrt::Windows::Foundation::IInspectable selectedItem = comboBox.SelectedItem();
    if (!selectedItem) return;

    winrt::hstring text = winrt::unbox_value<winrt::hstring>(selectedItem);
    OutputDebugString(text.c_str());
    OutputDebugString(L"\n");
}
