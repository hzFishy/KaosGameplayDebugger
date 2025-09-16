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

#pragma once

#include "KaosCheatSlateWidget.h"
#include "KaosDebuggerBaseItem.h"
#include "KaosGameplayDebuggerWidget.h"
#include "Modules/ModuleManager.h"


struct KAOSGAMEPLAYDEBUGGER_API FKaosDebuggerMainCategoryHandle
{
	static FKaosDebuggerMainCategoryHandle GenerateHandle()
	{
		static int32 NextHandleId;
		return { NextHandleId++ };
	}

	FKaosDebuggerMainCategoryHandle() = delete;
	FKaosDebuggerMainCategoryHandle(int32 InHandleId) : HandleId(InHandleId) {}


	bool IsValid() const { return HandleId != 0; }
	void Invalidate() { HandleId = 0; }

private:
	int32 HandleId = 0;
	friend class FKaosGameplayDebuggerModule;
};

struct KAOSGAMEPLAYDEBUGGER_API FKaosDebuggerSubCategoryHandle
{
	static FKaosDebuggerSubCategoryHandle GenerateHandle()
	{
		static int32 NextHandleId;
		return { NextHandleId++ };
	}

	FKaosDebuggerSubCategoryHandle() = delete;
	FKaosDebuggerSubCategoryHandle(int32 InHandleId) : HandleId(InHandleId) {}

	bool IsValid() const { return HandleId != 0; }
	void Invalidate() { HandleId = 0; }

private:
	int32 HandleId = 0;
	friend class FKaosGameplayDebuggerModule;
};


class KAOSGAMEPLAYDEBUGGER_API FKaosGameplayDebuggerModule : public IModuleInterface
{
public:

	static FKaosGameplayDebuggerModule& Get()
	{
		static FKaosGameplayDebuggerModule& Singleton = FModuleManager::LoadModuleChecked<FKaosGameplayDebuggerModule>("KaosGameplayDebugger");
		return Singleton;
	}
	
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
#if WITH_KAOS_GAMEPLAYDEBUGGER

	struct FKaosDebuggerCategoryInfo
	{
		TSharedPtr<IKaosDebuggerBaseItem> Instance;
		int32 IndexOrder = 0;
		int32 HandleId = 0;
	};

	struct FKaosDebuggerSubCategoryInfo
	{
		TSharedPtr<IKaosDebuggerBaseItem> Instance;
		FName SubCategoryName = NAME_None;
		int32 IndexOrder = 0;
		int32 HandleId = 0;
	};

	TArray<TSharedPtr<IKaosDebuggerBaseItem>> GetRegisteredSubCategoriesFor(FName Category) const;
	const TArray<TSharedPtr<IKaosDebuggerBaseItem>> GetRegisteredMainTabs() const;;

	void ToggleCheatUI(UWorld* World);

	[[nodiscard]] FKaosDebuggerMainCategoryHandle RegisterMainCategory(FName Category, TSharedPtr<IKaosDebuggerBaseItem> Instance, int32 IndexOrder);
	[[nodiscard]] FKaosDebuggerSubCategoryHandle RegisterSubCategory(FName MainCategory, FName SubCategoryName, TSharedPtr<IKaosDebuggerBaseItem> Instance, int32 IndexOrder);

	void UnregisterMainCategory(FKaosDebuggerMainCategoryHandle& Handle);
	void UnregisterSubCategory(FKaosDebuggerSubCategoryHandle& Handle);
private:
	
	/** Map of tabs to there category info */
	TMap<FName, FKaosDebuggerCategoryInfo> CategoryMap;

	/** Map of sub tabs to there category info */
	TMap<FName, TArray<FKaosDebuggerSubCategoryInfo>> CategorySubMap;
	
	TMap<TWeakObjectPtr<class ULocalPlayer>, TSharedPtr<FKaosSlateCheatWidget>> LocalPlayerToWidgetMap;
	FDelegateHandle BoundHandle;
	FKaosGameplayDebuggerWidget KaosGameplayDebuggerWidget;
	
	TArray<FKaosDebuggerMainCategoryHandle> RegisteredMainCategories;
	TArray<FKaosDebuggerSubCategoryHandle> RegisteredSubCategories;

#endif
};
