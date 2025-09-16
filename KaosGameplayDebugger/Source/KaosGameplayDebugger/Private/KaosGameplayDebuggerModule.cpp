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

#include "KaosGameplayDebuggerModule.h"
#include "KaosCheatSlateWidget.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Implementations/KaosWorldDebugger_Actor_Details.h"
#include "Implementations/KaosWorldDebugger_World_Details.h"
#include "Implementations/KaosWorldDebugger_Actor_AdditionalInfo.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MainTabs/KaosDebugger_MainTab_Networking.h"
#include "MainTabs/KaosDebugger_MainTab_Actor.h"
#include "MainTabs/KaosDebugger_MainTab_World.h"

#if WITH_EDITOR
#include "Framework/Application/SlateApplication.h"
#include "Framework/SlateDelegates.h"
#include "Editor/EditorEngine.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"
#include "Editor.h"
#include "ToolMenu.h"
#include "ToolMenuEntry.h"
#include "ToolMenuMisc.h"
#include "ToolMenuSection.h"
#include "ToolMenus.h"
#endif

#define LOCTEXT_NAMESPACE "FKaosGameplayDebuggerModule"

#if WITH_KAOS_GAMEPLAYDEBUGGER
static FAutoConsoleCommandWithWorld ShowUICmd(
	TEXT("Dominance.ToggleCheatUI"),
	TEXT("Shows the Dominance cheat UI for this world"),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		if (!World)
		{
			UE_LOG(LogTemp, Warning, TEXT("Dominance.ToggleCheatUI: No world context."));
			return;
		}

		FKaosGameplayDebuggerModule& Mod = FModuleManager::LoadModuleChecked<FKaosGameplayDebuggerModule>("KaosGameplayDebugger");
		Mod.ToggleCheatUI(World);
	})
);

TArray<TSharedPtr<IKaosDebuggerBaseItem>> FKaosGameplayDebuggerModule::GetRegisteredSubCategoriesFor(FName MainTabID) const
{
	if (const TArray<FKaosDebuggerSubCategoryInfo>* Items = CategorySubMap.Find(MainTabID))
	{
		TArray<TSharedPtr<IKaosDebuggerBaseItem>> ItemsReturn;

		for (const auto& Item : *Items)
		{
			ItemsReturn.Add(Item.Instance);
		}
		return ItemsReturn;
	}
	return TArray<TSharedPtr<IKaosDebuggerBaseItem>>();
}

const TArray<TSharedPtr<IKaosDebuggerBaseItem>> FKaosGameplayDebuggerModule::GetRegisteredMainTabs() const
{
	TArray<TSharedPtr<IKaosDebuggerBaseItem>> Items;
	for (const auto& [Category, Info] : CategoryMap)
	{
		Items.Add(Info.Instance);
	}
	return Items;
}

void FKaosGameplayDebuggerModule::ToggleCheatUI(UWorld* World)
{
	if (!World) return;
	if (!World->GetFirstPlayerController()) return;
	if (!World->GetFirstPlayerController()->GetLocalPlayer()) return;

	ULocalPlayer* LocalPlayer = World->GetFirstPlayerController()->GetLocalPlayer();
	TWeakObjectPtr<ULocalPlayer> LocalPlayerKey = LocalPlayer;

	TSharedPtr<FKaosSlateCheatWidget> Widget = LocalPlayerToWidgetMap.FindOrAdd(LocalPlayer, MakeShared<FKaosSlateCheatWidget>());

	// Toggle visibility
	if (Widget.IsValid() && Widget->IsWidgetEnabled())
	{
		Widget->Reset();
		Widget->DisableWidget();
		Widget.Reset();
		LocalPlayerToWidgetMap.Remove(LocalPlayerKey);
	}
	else
	{
		Widget->SetLocalPlayer(LocalPlayer);
		Widget->EnableWidget();
	}
}

FKaosDebuggerMainCategoryHandle FKaosGameplayDebuggerModule::RegisterMainCategory(FName Category, TSharedPtr<IKaosDebuggerBaseItem> Instance, int32 IndexOrder)
{
	FKaosDebuggerMainCategoryHandle Handle = FKaosDebuggerMainCategoryHandle::GenerateHandle();
	FKaosDebuggerCategoryInfo& CategoryInfo = CategoryMap.FindOrAdd(Category);
	CategoryInfo.Instance = Instance;
	CategoryInfo.IndexOrder = IndexOrder;
	CategoryInfo.HandleId = Handle.HandleId;
	return Handle;
}

FKaosDebuggerSubCategoryHandle FKaosGameplayDebuggerModule::RegisterSubCategory(FName MainCategory, FName SubCategoryName, TSharedPtr<IKaosDebuggerBaseItem> Instance, int32 IndexOrder)
{
	FKaosDebuggerSubCategoryHandle Handle = FKaosDebuggerSubCategoryHandle::GenerateHandle();
	TArray<FKaosDebuggerSubCategoryInfo>& CategoryInfo = CategorySubMap.FindOrAdd(MainCategory);
	FKaosDebuggerSubCategoryInfo& SubCategoryInfo = CategoryInfo.AddDefaulted_GetRef();
	SubCategoryInfo.Instance = Instance;
	SubCategoryInfo.SubCategoryName = SubCategoryName;
	SubCategoryInfo.IndexOrder = IndexOrder;
	SubCategoryInfo.HandleId = Handle.HandleId;
	Instance->TabOrder = IndexOrder;;
	return Handle;
}

void FKaosGameplayDebuggerModule::UnregisterMainCategory(FKaosDebuggerMainCategoryHandle& Handle)
{
}

void FKaosGameplayDebuggerModule::UnregisterSubCategory(FKaosDebuggerSubCategoryHandle& Handle)
{
}

#if WITH_EDITOR
static bool HasPlayWorld()
{
	return GEditor->PlayWorld != nullptr;
}

static bool HasNoPlayWorld()
{
	return !HasPlayWorld();
}

static bool HasPlayWorldAndRunning()
{
	return HasPlayWorld() && !GUnrealEd->PlayWorld->bDebugPauseExecution;
}

static void KaosDebuggerPressed(const FString ButtonName)
{
	if (GUnrealEd->PlayWorld)
	{
		UKismetSystemLibrary::ExecuteConsoleCommand(GUnrealEd->PlayWorld, "KaosDebugger.Show");
	}
}

static void VisualLoggerPressed(const FString ButtonName)
{
	if (GUnrealEd->PlayWorld)
	{
		UKismetSystemLibrary::ExecuteConsoleCommand(GUnrealEd->PlayWorld, "VisLog");
	}
}



static TSharedRef<SWidget> DebugButtons()
{
	FMenuBuilder MenuBuilder(true, nullptr);
	FText DisplayName = FText::FromString("Show Kaos Debugger");
	MenuBuilder.AddMenuEntry(
		DisplayName,
		LOCTEXT("CommonPathDescriptions", "Kaos Debugger"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateStatic(&KaosDebuggerPressed, FString("Kaos Debugger")),
			FCanExecuteAction::CreateStatic(&HasPlayWorldAndRunning),
			FIsActionChecked(),
			FIsActionButtonVisible::CreateStatic(&HasPlayWorldAndRunning)
		)
	);
	DisplayName = FText::FromString("Show Visual Logger");
	MenuBuilder.AddMenuEntry(
		DisplayName,
		LOCTEXT("CommonPathDescriptions", "Visual Logger"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateStatic(&VisualLoggerPressed, FString("Visual Logger")),
			FCanExecuteAction::CreateStatic(&HasPlayWorldAndRunning),
			FIsActionChecked(),
			FIsActionButtonVisible::CreateStatic(&HasPlayWorldAndRunning)
		)
	);
	
	return MenuBuilder.MakeWidget();
}

static void RegisterGameEditorMenus()
{
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	FToolMenuSection& Section = Menu->AddSection("DebuggingExtensions", TAttribute<FText>(), FToolMenuInsert("Play", EToolMenuInsertType::After));
	
	FToolMenuEntry DebuggerEntry = FToolMenuEntry::InitComboButton(
		"OpenGameMenu",
		FUIAction(
			FExecuteAction(),
			FCanExecuteAction::CreateStatic(&HasPlayWorld),
			FIsActionChecked(),
			FIsActionButtonVisible::CreateStatic(&HasPlayWorld)),
		FOnGetContent::CreateStatic(&DebugButtons),
		LOCTEXT("Debugger_Options", "Gameplay Debugging"),
		LOCTEXT("Debugger_Options", "Gameplay Debugging"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "BlueprintDebugger.TabIcon")
	);
	DebuggerEntry.StyleNameOverride = "CalloutToolbar";
	Section.AddEntry(DebuggerEntry);
}
#endif
#endif

void FKaosGameplayDebuggerModule::StartupModule()
{

	
#if WITH_KAOS_GAMEPLAYDEBUGGER
#if WITH_EDITOR
	if (FSlateApplication::IsInitialized())
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(&RegisterGameEditorMenus));
	}
#endif
	RegisteredMainCategories.Add(RegisterMainCategory(KaosDebuggerMainTabAreas::Actor, MakeShared<FKaosDebugger_MainTab_Actor>(), 0));
	RegisteredMainCategories.Add(RegisterMainCategory(KaosDebuggerMainTabAreas::Network, MakeShared<FKaosDebugger_MainTab_Networking>(), 3));
	RegisteredMainCategories.Add(RegisterMainCategory(KaosDebuggerMainTabAreas::World, MakeShared<FKaosDebugger_MainTab_World>(), 1));

	RegisteredSubCategories.Add(RegisterSubCategory(KaosDebuggerMainTabAreas::Actor, "Actor Details", MakeShared<FKaosWorldDebugger_Actor_Details>(), 0));
	RegisteredSubCategories.Add(RegisterSubCategory(KaosDebuggerMainTabAreas::World, "World Details", MakeShared<FKaosWorldDebugger_World_Details>(), 999));
	RegisteredSubCategories.Add(RegisterSubCategory(KaosDebuggerMainTabAreas::Actor, "Actor Additional", MakeShared<FKaosWorldDebugger_Actor_AdditionalInfo>(), 999));

	
	BoundHandle = FWorldDelegates::OnWorldCleanup.AddLambda([this](UWorld* World, bool bA, bool bB)
	{
		TArray<TWeakObjectPtr<ULocalPlayer>> ToRemove;
		for (auto& [Player, Widget]  : LocalPlayerToWidgetMap)
		{
			if (Player.IsValid())
			{
				if (Player->GetWorld() == World && Widget.IsValid())
				{
					Widget->Reset();
					Widget->DisableWidget();
					Widget.Reset();
				}
				ToRemove.Add(Player);
			}
			else
			{
				if (Widget.IsValid())
				{
					Widget->Reset();
					Widget->DisableWidget();
					Widget.Reset();
				}
			}
		}

		for (auto& Player : ToRemove)
		{
			LocalPlayerToWidgetMap.Remove(Player);
		}
		LocalPlayerToWidgetMap.Remove(nullptr);
	});
#endif
}

void FKaosGameplayDebuggerModule::ShutdownModule()
{
#if WITH_KAOS_GAMEPLAYDEBUGGER
	FWorldDelegates::OnWorldCleanup.Remove(BoundHandle);
	for (FKaosDebuggerMainCategoryHandle& Handle : RegisteredMainCategories)
	{
		UnregisterMainCategory(Handle);
	}
	for (FKaosDebuggerSubCategoryHandle& Handle : RegisteredSubCategories)
	{
		UnregisterSubCategory(Handle);
	}
#endif
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FKaosGameplayDebuggerModule, KaosGameplayDebugger)