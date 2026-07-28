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

        RestoreSelectedActionAsync();
    }

    winrt::fire_and_forget MainWindow::RestoreSelectedActionAsync()
    {
        auto lifetime = get_strong();

        UsefulAIKey::SavedSelection saved = co_await m_dataStorage.LoadSelectedOptionAsync();

        if (saved.Kind == UsefulAIKey::ActionKind::LaunchApp)
            m_savedAppPath = saved.Command;

        ActionComboBox().SelectedIndex(static_cast<int>(saved.Kind));
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
            ShowLaunchWebsiteView();
            break;
        case UsefulAIKey::ActionKind::OpenFile:
			ShowOpenFileView();
            break;
        }
    }

    void MainWindow::ShowLaunchAppView()
    {
        if (!m_appPickerView)
        {
            m_appPickerView = winrt::UsefulAIKey::AppPickerView();
            m_appPickerView.Load(m_savedAppPath);
        }
        this->AppWindow().Resize(Windows::Graphics::SizeInt32{ 632, 712 });
		UserSelectionStuff().Content(m_appPickerView);
    }

    void MainWindow::ShowLaunchWebsiteView()
    {
        if (!m_websiteLaunchView)
        {
            m_websiteLaunchView = winrt::UsefulAIKey::WebsiteLaunchView();
        }
        this->AppWindow().Resize(Windows::Graphics::SizeInt32{ 632, 290 });
		UserSelectionStuff().Content(m_websiteLaunchView);
    }

    void MainWindow::ShowOpenFileView()
    {
        if (!m_fileOpenView)
        {
            m_fileOpenView = winrt::UsefulAIKey::FileOpenView();
        }
        this->AppWindow().Resize(Windows::Graphics::SizeInt32{ 632, 290 });
        UserSelectionStuff().Content(m_fileOpenView);
    }
}


