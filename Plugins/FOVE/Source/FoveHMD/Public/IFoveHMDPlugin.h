#pragma once

#include "Modules/ModuleManager.h"
#include "IHeadMountedDisplayModule.h"

/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules 
 * within this plugin.
 */
class IFoveHMDPlugin : public IHeadMountedDisplayModule
{
public:

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IFoveHMDPlugin& Get()
	{
		return FModuleManager::LoadModuleChecked<IFoveHMDPlugin>("FoveHMD");
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("FoveHMD");
	}
};
