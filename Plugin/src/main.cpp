
#include "PCH.h"

namespace
{
	struct InputBoxCave : Xbyak::CodeGenerator
	{
		InputBoxCave(int maxNameLength)
		{
			mov(r8d, maxNameLength);
		}
	};

	struct ItemNameLengthCheckCave : Xbyak::CodeGenerator
	{
		ItemNameLengthCheckCave(int maxNameLength)
		{
			cmp(esi, maxNameLength);
		}
	};

	void Hook_Placeholder()
	{
	}

	void PatchInputBox()
	{
		auto inputBoxPatchAddress = reinterpret_cast<uintptr_t>(search_pattern<"48 8B 81 ?? ?? ?? ?? 48 8B 88 ?? ?? ?? ?? 44 89 81">());
		if (inputBoxPatchAddress) {
			inputBoxPatchAddress += 14;
			INFO("Found the input box patch address: {:x}", inputBoxPatchAddress);

			InputBoxCave inputBoxCave{ Settings::GetSingleton()->GetMaxNameLength() };
			inputBoxCave.ready();
			
			const auto caveHookHandle = AddCaveHook(
				inputBoxPatchAddress,
				{ 0, 7 },
				FUNC_INFO(Hook_Placeholder),
				&inputBoxCave,
				nullptr,
				HookFlag::kRestoreAfterProlog);
			caveHookHandle->Enable();
			INFO("Input Box patched")
		} else {
			ERROR("Couldn't find the input box patch address");
		}
	}

	void PatchItemNameLengthCheck()
	{
		auto itemNameLengthAddress = reinterpret_cast<uintptr_t>(search_pattern<"3B 35 ?? ?? ?? ?? 0F 87 ?? ?? ?? ?? D1">());
		if (itemNameLengthAddress) {
			INFO("Found the item name length check address: {:x}", itemNameLengthAddress);

			ItemNameLengthCheckCave itemNameLengthCheckCave{ Settings::GetSingleton()->GetMaxNameLength() };
			itemNameLengthCheckCave.ready();

			const auto inputBoxPatch = AddASMPatch(itemNameLengthAddress, { 0, 6 }, &itemNameLengthCheckCave);
			inputBoxPatch->Enable();
			INFO("Item name length check patched")
		}
		else {
			ERROR("Couldn't find the item name length check address");
		}
	}
}

DWORD WINAPI Thread(LPVOID param)
{
	Settings::GetSingleton()->Load();
	PatchInputBox();
	PatchItemNameLengthCheck();
	return true;
}

extern "C" __declspec(dllexport) void InitializeASI()
{
#ifndef NDEBUG
	MessageBoxA(NULL, "Loaded. You can attach the debugger now", "SF LongerNames ASI Plugin", NULL);
#endif
	dku::Logger::Init(Plugin::NAME, std::to_string(Plugin::Version));
	INFO("Game: {}, base: {:x}", dku::Hook::GetProcessName(), Module::get().base());
    Trampoline::AllocTrampoline(128);

	CloseHandle(CreateThread(nullptr, 0, Thread, nullptr, 0, nullptr));
}


BOOL APIENTRY DllMain(HMODULE a_hModule, DWORD a_dwReason, LPVOID a_lpReserved)
{
	return TRUE;
}