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

#include "MainTabs/KaosDebugger_MainTab_World.h"
#if WITH_KAOS_GAMEPLAYDEBUGGER

#include "Engine/Engine.h"
#include "Misc/App.h"
#include "Styling/SlateStyle.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "GameFramework/PlayerController.h"
#include "KaosGameplayDebuggerModule.h"
#include "ProfilingDebugging/ScopedTimers.h"
#include "Engine/HitResult.h"

void FKaosDebugger_MainTab_World::DrawDetails(const FKaosDebuggerContext& Context)
{
	SlateIM::BeginVerticalStack();
	SlateIM::BeginHorizontalStack();

	SlateIM::CheckBox(TEXT("Auto Refresh"), ShouldAutoRefresh);
	SlateIM::Spacer({6.f, 0.f});

	SlateIM::Text(TEXT("Interval (s):"));
	SlateIM::MinWidth(60.f);
	SlateIM::SpinBox(RefreshInterval, 0.1f, 10.f);

	SlateIM::Spacer({12.f, 0.f});
	SlateIM::Text(TEXT("World:"));
	SlateIM::MinWidth(120.f);

	double LastTime = CurrentTime;
	CurrentTime = 0;
	FScopedDurationTimer Timer(CurrentTime);
	TimeSinceLastUpdate += FApp::GetDeltaTime();

	if (ShouldAutoRefresh == ECheckBoxState::Checked && TimeSinceLastUpdate > RefreshInterval)
	{
		bRefreshAll = true;
		TimeSinceLastUpdate = 0;
	}
	else if (ShouldAutoRefresh == ECheckBoxState::Unchecked)
	{
		bRefreshAll = false;
	}

	if (bRefreshAll || WorldList.IsEmpty())
	{
		RefreshWorldList();
		bRefreshAll = false;
	}

	TArray<FString> WorldNames;
	for (const auto& World : WorldList)
	{
		if (!World.IsValid())
		{
			WorldNames.Add(TEXT("<Invalid>"));
			continue;
		}

		UWorld* W = World.Get();
		FString Name = W->GetName();

		// Find the corresponding context
		const FWorldContext* FoundContext = GEngine->GetWorldContextFromWorld(W);
		if (FoundContext)
		{
			// PIE Instance?
			if (FoundContext->WorldType == EWorldType::PIE)
			{
				Name += FString::Printf(TEXT(" [PIE:%d]"), FoundContext->PIEInstance);
			}

			// Net mode
			switch (W->GetNetMode())
			{
			case NM_Client:
				Name += TEXT(" (Client)");
				break;
			case NM_ListenServer:
				Name += TEXT(" (ListenServer)");
				break;
			case NM_DedicatedServer:
				Name += TEXT(" (DedicatedServer)");
				break;
			default:
				break;
			}
		}

		WorldNames.Add(Name);
	}


	SelectedWorldIndex = FMath::Clamp(SelectedWorldIndex, 0, WorldNames.Num() - 1);
	if (SlateIM::ComboBox(WorldNames, SelectedWorldIndex, bForceWorldComboRefresh))
	{
	}
	SlateIM::EndHorizontalStack();

	SelectedWorld = (WorldList.IsValidIndex(SelectedWorldIndex))
	? WorldList[SelectedWorldIndex].Get()
	: nullptr;
	
	FKaosDebuggerContext TabContext;
	TabContext.ContextWorld = SelectedWorld;
	TabContext.DeltaTime = Context.DeltaTime;
	
	SlateIM::Fill();
	SlateIM::HAlign(HAlign_Fill);
	SlateIM::VAlign(VAlign_Fill);
	SlateIM::BeginTabGroup(TEXT("ActorDebugTabs"));
	SlateIM::Fill();
	SlateIM::HAlign(HAlign_Fill);
	SlateIM::VAlign(VAlign_Fill);
	SlateIM::BeginTabStack();

	FKaosGameplayDebuggerModule& Module = FKaosGameplayDebuggerModule::Get();
	TArray<TSharedPtr<IKaosDebuggerBaseItem>> SubTabs =
		Module.GetRegisteredSubCategoriesFor(KaosDebuggerMainTabAreas::World);
	
	SubTabs.Sort([](const TSharedPtr<IKaosDebuggerBaseItem>& A, const TSharedPtr<IKaosDebuggerBaseItem>& B)
	{
		return A->GetTabOrder() < B->GetTabOrder();
	});
	
	for (const TSharedPtr<IKaosDebuggerBaseItem>& Tab : SubTabs)
	{
		if (!Tab.IsValid()) continue;
		if (SlateIM::BeginTab(FName(*Tab->GetTabLabel().ToString()), Tab->GetTabIcon(), Tab->GetTabLabel()))
		{
			SlateIM::Fill();
			SlateIM::HAlign(HAlign_Fill);
			SlateIM::VAlign(VAlign_Fill);
			Tab->DrawDetails(TabContext);
		}
		SlateIM::EndTab();
	}

	SlateIM::EndTabStack();
	SlateIM::EndTabGroup();

	SlateIM::EndVerticalStack();
}

FSlateIcon FKaosDebugger_MainTab_World::GetTabIcon() const
{
	static const FSlateIcon MyIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "AnimViewport.WorldSpaceEditing");

	return MyIcon;
}

void FKaosDebugger_MainTab_World::RefreshWorldList()
{
	TArray<TWeakObjectPtr<UWorld>> NewWorldList;

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (UWorld* World = WorldContext.World())
		{
			if (IsValidChecked(World) &&
				(World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE))
			{
				NewWorldList.Add(World);
			}
		}
	}

	bool bListChanged = (NewWorldList.Num() != WorldList.Num());
	if (!bListChanged)
	{
		for (int32 i = 0; i < NewWorldList.Num(); ++i)
		{
			if (NewWorldList[i] != WorldList[i])
			{
				bListChanged = true;
				break;
			}
		}
	}

	if (bListChanged)
	{
		WorldList = MoveTemp(NewWorldList);
		bForceWorldComboRefresh = true;

		if (!WorldList.IsValidIndex(SelectedWorldIndex))
		{
			SelectedWorldIndex = WorldList.Num() > 0 ? 0 : INDEX_NONE;
		}
	}
}
#endif