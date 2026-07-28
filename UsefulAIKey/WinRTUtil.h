#pragma once


winrt::fire_and_forget ShowDialogMessage(
	winrt::Microsoft::UI::Xaml::XamlRoot xamlRoot,
	winrt::hstring title, winrt::hstring content,
	winrt::hstring primaryButtonText = L"OK", winrt::hstring secondaryButtonText = L"", 
	winrt::hstring closeButtonText = L""
);

