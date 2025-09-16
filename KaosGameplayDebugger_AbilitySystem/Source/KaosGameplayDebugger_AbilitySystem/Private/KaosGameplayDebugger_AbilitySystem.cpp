// Copyright (C) 2025, Daniel Moss
// 
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include "KaosGameplayDebugger_AbilitySystem.h"

#include "KaosGameplayDebuggerModule.h"
#include "KaosWorldDebugger_ActorSubTab_AbilitySystem.h"
#include "KaosWorldDebugger_GameplayAbilities.h"
#include "KaosWorldDebugger_GameplayAttributes.h"
#include "KaosWorldDebugger_GameplayEffect.h"

#define LOCTEXT_NAMESPACE "FKaosGameplayDebugger_AbilitySystemModule"

void FKaosGameplayDebugger_AbilitySystemModule::StartupModule()
{

#if WITH_KAOS_GAMEPLAYDEBUGGER
	FKaosGameplayDebuggerModule& Module = FKaosGameplayDebuggerModule::Get();

	RegisteredSubCategories.Add(Module.RegisterSubCategory(KaosDebuggerMainTabAreas::Actor, "Abilities", MakeShared<FKaosWorldDebugger_ActorSubTab_AbilitySystem>(), 3));
	RegisteredSubCategories.Add(Module.RegisterSubCategory(KaosDebugger_AbilitySystemNames::AbilitySystemMainTabID, "GameplayEffects", MakeShared<FKaosWorldDebugger_GameplayEffects>(), 0));
	RegisteredSubCategories.Add(Module.RegisterSubCategory(KaosDebugger_AbilitySystemNames::AbilitySystemMainTabID, "Ability", MakeShared<FKaosWorldDebugger_GameplayAbilities>(), 1));
	RegisteredSubCategories.Add(Module.RegisterSubCategory(KaosDebugger_AbilitySystemNames::AbilitySystemMainTabID, "Attributes", MakeShared<FKaosWorldDebugger_GameplayAttributes>(), 2));
#endif
}

void FKaosGameplayDebugger_AbilitySystemModule::ShutdownModule()
{
#if WITH_KAOS_GAMEPLAYDEBUGGER
	FKaosGameplayDebuggerModule& Module = FKaosGameplayDebuggerModule::Get();
	for (FKaosDebuggerSubCategoryHandle& Handle : RegisteredSubCategories)
	{
		Module.UnregisterSubCategory(Handle);
	}
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FKaosGameplayDebugger_AbilitySystemModule, KaosGameplayDebugger_AbilitySystem)