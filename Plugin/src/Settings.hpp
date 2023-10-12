#pragma once

#include "DKUtil/Config.hpp"

using namespace DKUtil::Alias;

class Settings : public dku::model::Singleton<Settings>
{
private:
	TomlConfig mainConfig = COMPILE_PROXY(Plugin::SETTINGS_NAME);

	Integer maxNameLength{ "maxNameLength" };

public:
	int GetMaxNameLength()
	{
		return *maxNameLength;
	}

	void Load() noexcept
	{
		static std::once_flag bound;
		std::call_once(bound, [&]() { mainConfig.Bind<5, 255>(maxNameLength, 250); });

		mainConfig.Load();
	}
};
