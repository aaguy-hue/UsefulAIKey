#include "pch.h"
#include "WinRTUtil.h"

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
