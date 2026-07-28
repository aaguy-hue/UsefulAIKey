#include "pch.h"
#include "MainWindow.xaml.h"
#include "ActionOption.h"
#include "DataStorage.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <algorithm>
#include <cwctype>

using namespace winrt;
using namespace Microsoft::UI::Xaml;


// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::UsefulAIKey::implementation
{

    void MainWindow::InitializeComponent()
    {
        MainWindowT::InitializeComponent();

        m_options = single_threaded_observable_vector<UsefulAIKey::ActionOption>();

        using UsefulAIKey::ActionKind;
        m_options.Append(make<ActionOption>(ActionKind::LaunchApp, L"Launch an app"));
        m_options.Append(make<ActionOption>(ActionKind::LaunchWebsite, L"Launch a website"));
        m_options.Append(make<ActionOption>(ActionKind::OpenFile, L"Open a file"));

        ActionComboBox().ItemsSource(m_options);
        //ActionComboBox().SelectedIndex(0);

        std::pair<ActionKind, std::variant<winrt::UsefulAIKey::AppItem, hstring>> data = m_dataStorage.LoadSelectedOption();
        //ActionComboBox().SelectedIndex(static_cast<int>(data.first));
        switch (data.first)
        {
        case ActionKind::LaunchApp:
            ActionComboBox().SelectedIndex(0);
            break;
        case ActionKind::LaunchWebsite:
            ActionComboBox().SelectedIndex(1);
            break;
        case ActionKind::OpenFile:
            ActionComboBox().SelectedIndex(2);
            break;
        }
    }

    void MainWindow::Action_SelectionChanged(
        Windows::Foundation::IInspectable const& sender,
        Controls::SelectionChangedEventArgs const& /* e */)
    {
        auto comboBox = sender.try_as<Controls::ComboBox>();
        if (!comboBox) return;

        auto option = comboBox.SelectedItem().try_as<UsefulAIKey::ActionOption>();
        if (!option) return;

        switch (option.Kind())
        {
        case UsefulAIKey::ActionKind::LaunchApp:
            ShowLaunchAppView();
            break;
        case UsefulAIKey::ActionKind::LaunchWebsite:
            OutputDebugStringW(L"Selected: LaunchWebsite\n");
            break;
        case UsefulAIKey::ActionKind::OpenFile:
            OutputDebugStringW(L"Selected: OpenFile\n");
            break;
        }
    }

    void MainWindow::ShowLaunchAppView()
    {
        OutputDebugStringW(L"Selected: LaunchApp\n");

        if (!m_appPickerView) m_appPickerView = winrt::UsefulAIKey::AppPickerView();
		UserSelectionStuff().Content(m_appPickerView);
    }
}


