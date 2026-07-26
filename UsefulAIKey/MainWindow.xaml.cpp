#include "pch.h"
#include "MainWindow.xaml.h"
#include "ActionOption.h"
#include "AppItem.h"
#include "WindowsApps.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <algorithm>
#include <cwctype>

using namespace winrt;
using namespace Microsoft::UI::Xaml;

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

        m_options = single_threaded_observable_vector<UsefulAIKey::ActionOption>();

        // The first argument is the stable identity -- app logic switches on it.
        // The second is display text, free to reword or localize.
        using UsefulAIKey::ActionKind;
        m_options.Append(make<ActionOption>(ActionKind::LaunchApp, L"Launch an app"));
        m_options.Append(make<ActionOption>(ActionKind::LaunchWebsite, L"Launch a website"));
        m_options.Append(make<ActionOption>(ActionKind::OpenFile, L"Open a file"));

        ActionComboBox().ItemsSource(m_options);
        ActionComboBox().SelectedIndex(0);

        // Bind the ListView to an observable vector we control. We rebuild its
        // contents whenever the search text or the pinned app changes.
        m_visibleApps = single_threaded_observable_vector<UsefulAIKey::AppItem>();
        AppsList().ItemsSource(m_visibleApps);

        // Kick off the (potentially slow) enumeration without blocking the window.
        LoadAppsAsync();
    }

    fire_and_forget MainWindow::LoadAppsAsync()
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

    void MainWindow::RefreshVisibleApps()
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

    void MainWindow::SearchBox_TextChanged(
        Controls::AutoSuggestBox const& /* sender */,
        Controls::AutoSuggestBoxTextChangedEventArgs const& /* args */)
    {
        RefreshVisibleApps();
    }

    void MainWindow::AppsList_ItemClick(
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
            OutputDebugStringW(L"Selected: LaunchApp\n");
            break;
        case UsefulAIKey::ActionKind::LaunchWebsite:
            OutputDebugStringW(L"Selected: LaunchWebsite\n");
            break;
        case UsefulAIKey::ActionKind::OpenFile:
            OutputDebugStringW(L"Selected: OpenFile\n");
            break;
        }
    }

    void MainWindow::BrowseForApp_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        OpenFilePicker(this->AppWindow().Id());
    }

    winrt::fire_and_forget MainWindow::OpenFilePicker(winrt::Microsoft::UI::WindowId windowId)
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

        if (result)
        {

        }
    }
}


