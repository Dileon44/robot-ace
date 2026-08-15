#ifndef __BSP_MODULE_HPP
#define __BSP_MODULE_HPP

#include "main.h"

/* ============================================================================
 * Common BSP module contract.
 *
 * Every BSP module implements IModule and pairs with a transport implementing
 * IModuleBus. Bsp_Init() then walks one table and brings them all up through
 * the same code path — the attach logic and its logging exist exactly once,
 * no matter how many modules there are.
 * ============================================================================ */

namespace bsp {

// Transport shared contract. Module-specific transfer methods (I2C read/write,
// ADC start, ...) are added by the derived interface in each module.
class IModuleBus {
  public:
	virtual bool Init()   = 0;
	virtual bool DeInit() = 0;

  protected:
	// Protected and non-virtual: instances are static and never deleted.
	~IModuleBus() = default;
};

class IModule {
  public:
	virtual bool        Init()       = 0;
	virtual bool        DeInit()     = 0;
	virtual const char* Name() const = 0;

	// Called by AttachModule() once Init() succeeded. The module stores itself
	// into its own typed slot here — inside the concrete class the exact type is
	// still known, so no cast and no RTTI are needed to hand it back later.
	// Pure virtual on purpose: a new module cannot forget to implement it.
	virtual void OnAttached() = 0;
	virtual void OnDetached() = 0;

  protected:
	~IModule() = default;
};

struct ModuleEntry {
	IModule*    module;
	IModuleBus* bus;
};

RET_STATE_t AttachModule(const ModuleEntry& entry);
RET_STATE_t DetachModule(const ModuleEntry& entry);

}  // namespace bsp

#endif /* __BSP_MODULE_HPP */
