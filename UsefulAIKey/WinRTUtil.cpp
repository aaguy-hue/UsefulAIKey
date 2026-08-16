#include "pch.h"
#include "WinRTUtil.h"

#include <string_view>

using namespace winrt;
using namespace Microsoft::UI::Xaml;

winrt::fire_and_forget ShowDialogMessage(XamlRoot xamlRoot, hstring title, hstring content,
    hstring primaryButtonText, hstring secondaryButtonText, hstring closeButtonText)
{
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    ContentDialog dialog;
    dialog.XamlRoot(xamlRoot);
    dialog.Title(box_value(title));
    dialog.Content(box_value(content));
    dialog.PrimaryButtonText(primaryButtonText);
    dialog.SecondaryButtonText(secondaryButtonText);
    dialog.CloseButtonText(closeButtonText);
    co_await dialog.ShowAsync();
}

namespace
{
    // Returns view with leading/trailing spaces and tabs removed. Empty if the
    // string is all whitespace.
    std::wstring_view TrimWhitespace(std::wstring_view text)
    {
        size_t start = text.find_first_not_of(L" \t");
        if (start == std::wstring_view::npos) return {};
        size_t end = text.find_last_not_of(L" \t");
        return text.substr(start, end - start + 1);
    }
}

winrt::Windows::Foundation::IAsyncOperation<hstring> ShowInputDialog(
    XamlRoot xamlRoot, hstring title, hstring placeholder, hstring defaultText,
    hstring primaryButtonText, hstring closeButtonText)
{
    using namespace winrt::Microsoft::UI::Xaml::Controls;

    TextBox input;
    input.PlaceholderText(placeholder);
    input.Text(defaultText);

    // Focus the field and select its contents when the dialog opens, so the user
    // can immediately overtype the pre-filled default.
    input.Loaded([](winrt::Windows::Foundation::IInspectable const& sender, RoutedEventArgs const&)
    {
        auto box = sender.as<TextBox>();
        box.Focus(FocusState::Programmatic);
        box.SelectAll();
    });

    ContentDialog dialog;
    dialog.XamlRoot(xamlRoot);
    dialog.Title(box_value(title));
    dialog.Content(input);
    dialog.PrimaryButtonText(primaryButtonText);
    dialog.CloseButtonText(closeButtonText);
    dialog.DefaultButton(ContentDialogButton::Primary);

    // Keep the confirm button disabled while the field is blank so a confirmed
    // result always has a usable name. Weak-ref the dialog inside the handler:
    // the dialog owns the TextBox which owns this handler, so a strong capture
    // would form a cycle and leak the dialog.
    auto weakDialog = winrt::make_weak(dialog);
    auto syncButton = [weakDialog](std::wstring_view text)
    {
        if (auto d = weakDialog.get())
            d.IsPrimaryButtonEnabled(!TrimWhitespace(text).empty());
    };
    syncButton(input.Text());
    input.TextChanged([syncButton](winrt::Windows::Foundation::IInspectable const& sender, TextChangedEventArgs const&)
    {
        syncButton(sender.as<TextBox>().Text());
    });

    ContentDialogResult result = co_await dialog.ShowAsync();
    if (result != ContentDialogResult::Primary)
        co_return hstring{}; // cancelled / closed

    co_return hstring{ TrimWhitespace(input.Text()) };
}
