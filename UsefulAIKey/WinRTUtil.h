#pragma once


winrt::fire_and_forget ShowDialogMessage(
	winrt::Microsoft::UI::Xaml::XamlRoot xamlRoot,
	winrt::hstring title, winrt::hstring content,
	winrt::hstring primaryButtonText = L"OK", winrt::hstring secondaryButtonText = L"",
	winrt::hstring closeButtonText = L""
);

// Shows a dialog with a single text field and returns what the user typed.
// The confirm button stays disabled while the field is blank, so:
//   - a non-empty return  == the user confirmed a name (already trimmed)
//   - an empty return     == the user cancelled (or closed the dialog)
// defaultText pre-fills the field (e.g. the app's file stem).
winrt::Windows::Foundation::IAsyncOperation<winrt::hstring> ShowInputDialog(
	winrt::Microsoft::UI::Xaml::XamlRoot xamlRoot,
	winrt::hstring title,
	winrt::hstring placeholder,
	winrt::hstring defaultText,
	winrt::hstring primaryButtonText = L"OK",
	winrt::hstring closeButtonText = L"Cancel"
);

