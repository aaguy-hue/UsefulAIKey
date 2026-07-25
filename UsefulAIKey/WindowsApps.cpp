#include <filesystem>
#include <shobjidl.h>
#include <shlobj.h>
#include <shlguid.h>

#include "pch.h"
#include "WindowsApps.h"

// I'm not very familiar with windows programming and COM so this will be heavily commented
// to help me understand better by talking through the concepts, with some links to the docs

// the windows shell is the GUI for windows (largely implemented by explorer.exe)
// COM is a language-independent interface that allows objs in diff langs to interact
// shobjidl.h lets you interact with windows shell and com objects
// shlboj.h defines the interfaces for windows shell and com objs
// shlguid.h defines guids for windows shell


namespace fs = std::filesystem;

struct AppEntry
{
	std::wstring name;
	std::wstring path;
};

std::vector<AppEntry> EnumerateStartMenuApps()
{
	std::vector<AppEntry> apps;

	// PWSTR is a pointer to a wide string, aka a wchar_t*
	// aka it's just a unicode string since unicode charas are wide
	PWSTR pathbuf = nullptr;

	std::vector<fs::path> roots;
	
	// Folders like FOLDERID_CommonPrograms are "known folders" in Windows that have a special meaning
	// See https://learn.microsoft.com/en-us/windows/win32/shell/known-folders
	// SHGetFolderPath is a function that retrieves the path of a known folder identified by its KNOWNFOLDERID
	// Params (taking from https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetknownfolderpath):
	// rfid - The KNOWNFOLDERID of the folder. This is a GUID that uniquely identifies the folder.
	// dwFlags - Flags for specific retrieval options, I don't need any
	// hToken - Access token for a user, when null, it uses the current user
	// ppszPath - Pointer to a null-terminated unicode string to store the path in
	if (SUCCEEDED(SHGetFolderPath(FOLDERID_CommonPrograms, 0, nullptr, 0, &pathbuf)))
	{
		roots.push_back(pathbuf);
		CoTaskMemFree(pathbuf);
	}

	if (SUCCEEDED(SHGetFolderPath(FOLDERID_Programs, 0, nullptr, 0, &pathbuf)))
	{
		roots.push_back(pathbuf);
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
	// These links explain a lot about how COM works and how to use it in C++: 
	// https://learn.microsoft.com/en-us/windows/win32/com/the-component-object-model
	// https://learn.microsoft.com/en-us/windows/apps/develop/cpp-winrt/consume-com
	// https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nn-unknwn-iunknown
	winrt::com_ptr<IShellLinkW> shellLink;
	winrt::com_ptr<IPersistFile> persistFile;
	winrt::check_hresult(
		CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(shellLink.put()))
	);
}