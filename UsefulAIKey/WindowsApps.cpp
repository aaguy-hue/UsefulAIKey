// pch.h should always be the first include in a cpp file, it includes windows.h and other necessary headers
// https://stackoverflow.com/questions/54121917/what-is-pch-h-and-why-is-it-needed-to-be-included-as-the-first-header-file
// if you include headers before pch.h, you may get errors about missing definitions or symbols, since pch.h sets up the necessary environment for the rest of the code to compile correctly
#include "pch.h" 

#include <filesystem>
#include <vector>
#include <shobjidl.h>
#include <shlobj.h>
#include <shlguid.h>

#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Security.Cryptography.h>

#include "WindowsApps.h"

// I'm not very familiar with windows programming and COM so this will be heavily commented
// to help me understand better by talking through the concepts, with some links to the docs

// the windows shell is the GUI for windows (largely implemented by explorer.exe)
// COM is a language-independent abi that allows objs in diff langs to interact
// shobjidl.h lets you interact with windows shell and com objects
// shlboj.h defines the interfaces for windows shell and com objs
// shlguid.h defines guids for windows shell


namespace fs = std::filesystem;

// AppEntry is now defined in WindowsApps.h so other files can use it too.

std::vector<AppEntry> EnumerateStartMenuApps()
{
	std::vector<AppEntry> apps;

	// PWSTR is a pointer to a wide string, aka a wchar_t*
	// aka it's just a unicode string since unicode charas are utf-16 on windows
	PWSTR pathbuf = nullptr;

	fs::path roots[2];
	
	// Folders like FOLDERID_CommonPrograms are "known folders" in Windows that have a special meaning
	// See https://learn.microsoft.com/en-us/windows/win32/shell/known-folders
	// SHGetFolderPath is a function that retrieves the path of a known folder identified by its KNOWNFOLDERID
	// Params (taking from https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetknownfolderpath):
	// rfid - The KNOWNFOLDERID of the folder. This is a GUID that uniquely identifies the folder.
	// dwFlags - Flags for specific retrieval options, I don't need any
	// hToken - Access token for a user, when null, it uses the current user
	// ppszPath - Pointer to a null-terminated unicode string to store the path in
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_CommonPrograms, 0, nullptr, &pathbuf)))
	{
		roots[0] = pathbuf;
		CoTaskMemFree(pathbuf);
	}

	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Programs, 0, nullptr, &pathbuf)))
	{
		roots[1] = pathbuf;
		CoTaskMemFree(pathbuf);
	}

	// winrt::com_ptr is a smart pointer for COM objects that manages the reference count and automatically releases the obj when it goes out of scope
	// IShellLinkW is an interface that allows you to create, modify, and resolve shell links (.lnk files)
	// IShellLinkW is a UNICODE version of IShellLink, IShellLinkA is the ANSI version ew don't use that
	// IPersistFile is an interface to load/save objects to/from files (here we use it to load the .lnk files)
	//
	// CoCreateInstance creates and initializes a single object with the specified CLSID on the local system
	// https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cocreateinstance
	// A CLSID key is a unique id for a COM class object (https://learn.microsoft.com/en-us/windows/win32/com/clsid-key-hklm)
	// dwClsContext is the context in which the code managing the new obj will run, taken from CLSCTX enum
	// CLSCTX_INPROC_SERVER means the code runs in the same process as the caller
	// IID_PPV_ARGS is a macro for the last 2 args of CoCreateInstance, it takes a pointer to a 
	// COM interface and returns the IID of that interface, so here IID_IShellLinkW and IID_IPersistFile for the first param
	// and the second param is a pointer to a pointer to the interface, so we use shellLink.put() and persistFile.put() to get those pointers (they get cast to void**)
	// com_ptr.put() returns a pointer to the internal raw pointer of com_ptr
	// 
	// the reason we use com_ptr is that COM interfaces have the reference counting in the object itself
	// since multiple DLLs can use the same COM object and modify its reference count, we don't externally
	// manage the reference count (c++ shared_ptr), we instead use AddRef() and Release() to manage the internal
	// reference count, and com_ptr calls them for us automatically
	// 
	// The CLSID_ShellLink is the CLSID for the ShellLink object, which implements IShellLinkW but also IPersistFile
	// IUnknown always has QueryInterface (as well as AddRef/Release), so we use QueryInterface to ask the ShellLink
	// obj for its IPersistFile interface, which we can then use to load the .lnk file and get the target path
	// .as<interface>() is a convenience method thta calls QueryInterface and checks the result, returning a com_ptr 
	// of the requested interface type
	// 
	// These links explain a lot about how COM works and how to use it in C++: 
	// https://learn.microsoft.com/en-us/windows/win32/com/the-component-object-model
	// https://learn.microsoft.com/en-us/windows/apps/develop/cpp-winrt/consume-com
	// https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nn-unknwn-iunknown
	winrt::com_ptr<IShellLinkW> shellLink;
	winrt::com_ptr<IPersistFile> persistFile;
	winrt::check_hresult(
		CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(shellLink.put()))
	);
	persistFile = shellLink.as<IPersistFile>();

	for (auto& root : roots) {
		if (root.empty() || !fs::exists(root)) continue;
		
		for (auto& entry : fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied)) {
			if (entry.path().extension() != L".lnk") continue;

			// Attempt to load the 
			if (FAILED(persistFile->Load(entry.path().c_str(), STGM_READ))) continue;

			// Get the target path of the .lnk file
			// WIN32_FIND_DATAW is a struct that contains information about a file found by FindFirstFile/FindNextFile
			//SLGP_UNCPRIORITY is a flag that tells GetPath to return the UNC path if available, otherwise return the local path
			wchar_t targetPath[MAX_PATH]{};
			WIN32_FIND_DATAW findData{};
			if (FAILED(shellLink->GetPath(targetPath, MAX_PATH, &findData, SLGP_UNCPRIORITY))) continue;

			// wcslen is a wide-character version of strlen
			if (wcslen(targetPath) == 0) continue; // skip shortcuts to folders/URLs

			apps.emplace_back(AppEntry{
				entry.path().stem().wstring(), // stem() returns the filename without the extension
				targetPath
			});
		}
	}

	std::sort(apps.begin(), apps.end(), [](const AppEntry& a, const AppEntry& b) {
		return a.name < b.name;
	});

	return apps;
}

// Convert an app's icon into a SoftwareBitmap that can be used in XAML
winrt::Windows::Graphics::Imaging::SoftwareBitmap LoadIconBitmap(std::wstring const& path, int size)
{
	using namespace winrt::Windows::Graphics::Imaging;

	if (path.empty()) return nullptr;

	// IShellItemImageFactory gets an HBITMAP for a file, folder, or other shell item
	// SHCreateItemFromParsingName turns a file path into a shell object (IShellItem) 
	// You can then do QueryInterface for IShellItemImageFactory, which has a GetImage
	// method that returns an HBITMAP of the icon for arbitrary shell items
	winrt::com_ptr<IShellItemImageFactory> factory;
	if (FAILED(SHCreateItemFromParsingName(path.c_str(), nullptr, IID_PPV_ARGS(factory.put()))))
		return nullptr;

	// Requests icon at roughly size px
	// SIIGBF_ICONONLY forces an icon instead of a document thumbnail/preview
	// SIIGBF_BIGGERSIZEOK lets it hand back a larger, crisper source that we can scale down instead of upscaling a smaller one
	// The result is a 32bpp HBITMAP whose pixels are premultiplied BGRA
	// (RGB but different order, alpha is premultiplied into the color channels)
	SIZE requestedSize{ size, size };
	HBITMAP bitmap = nullptr;
	if (FAILED(factory->GetImage(requestedSize, SIIGBF_ICONONLY | SIIGBF_BIGGERSIZEOK, &bitmap)) || !bitmap)
		return nullptr;

	// Unlike COM objects, GDI handles aren't reference counted
	// this code uses a RAII guard to make sure we DeleteObject on every exit, even if we return early
	struct BitmapGuard { HBITMAP h; ~BitmapGuard() { if (h) DeleteObject(h); } } guard{ bitmap };

	// Determine icon size bc BIGGERSIZEOK means it may not equal size
	BITMAP info{};
	if (GetObjectW(bitmap, sizeof(info), &info) == 0) return nullptr;
	const int width = info.bmWidth;
	const int height = info.bmHeight;
	if (width <= 0 || height <= 0) return nullptr;

	// Copy HBITMAP's pixels into our own buffer
	// Negative biHeight asks GDI for top-down rows (the order SoftwareBitmap wants)
	// 32bpp BI_RGB gives us BGRA bytes
	BITMAPINFO dib{};
	dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	dib.bmiHeader.biWidth = width;
	dib.bmiHeader.biHeight = -height;
	dib.bmiHeader.biPlanes = 1;
	dib.bmiHeader.biBitCount = 32;
	dib.bmiHeader.biCompression = BI_RGB;

	// GetDC gets the "device context", which is basically a drawable surface & its properties
	// Here, since we pass nullptr, it gets the DC for the whole screen rather than some window
	// GetDIBits loads the pixel bits into a vector
	std::vector<uint8_t> pixels(static_cast<size_t>(width) * height * 4);
	HDC screen = GetDC(nullptr);
	int scanned = GetDIBits(screen, bitmap, 0, height, pixels.data(), &dib, DIB_RGB_COLORS);
	ReleaseDC(nullptr, screen);
	if (scanned == 0) return nullptr;

	// Some icons come back with a fully-zero alpha channel ("no alpha info"), which would be fully transparent
	// Here, we detect that case and force the icon opaque so it actually shows
	bool anyAlpha = false;
	for (size_t i = 3; i < pixels.size(); i += 4)
	{
		if (pixels[i] != 0) { anyAlpha = true; break; }
	}
	if (!anyAlpha)
	{
		for (size_t i = 3; i < pixels.size(); i += 4) pixels[i] = 255;
	}

	// CryptographicBuffer seems a little odd but this static method is just an easy way to make an IBuffer from a byte array
	// We then copy the bytes from the IBuffer into a SoftwareBitmap
	// SoftwareBitmapSource needs a SoftwareBitmap in BGRA8 + premultiplied format so this works yay
	auto buffer = winrt::Windows::Security::Cryptography::CryptographicBuffer::CreateFromByteArray(pixels);
	return SoftwareBitmap::CreateCopyFromBuffer(
		buffer, BitmapPixelFormat::Bgra8, width, height, BitmapAlphaMode::Premultiplied);
}