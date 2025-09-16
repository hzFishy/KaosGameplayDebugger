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

#include "MainTabs/KaosDebugger_MainTab_Actor.h"
#if WITH_KAOS_GAMEPLAYDEBUGGER
#include "Engine/Engine.h"
#include "Misc/App.h"
#include "KaosGameplayDebuggerInfoProviderInterface.h"
#include "Algo/Compare.h"
#include "Styling/SlateStyle.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "KaosGameplayDebuggerDevSettings.h"
#include "KaosGameplayDebuggerModule.h"
#include "ProfilingDebugging/ScopedTimers.h"
#include "Engine/HitResult.h"

#if WITH_EDITOR
#include "Selection.h"
#endif

void FKaosDebugger_MainTab_Actor::HandleActorSelection()
{
	for (const TWeakObjectPtr<UWorld>& WorldPtr : WorldList)
	{
		if (!WorldPtr.IsValid())
		{
			continue;
		}

		UWorld* World = WorldPtr.Get();

		for (TActorIterator<APlayerController> It(World); It; ++It)
		{
			APlayerController* PC = *It;
			if (!IsValid(PC) || !PC->IsLocalController())
			{
				continue;
			}

			if (PC->WasInputKeyJustPressed(EKeys::LeftMouseButton))
			{
				FHitResult Hit;
				if (PC->GetHitResultUnderCursorByChannel(GetDefault<UKaosGameplayDebuggerDevSettings>()->MouseUnderCursorTraceChannel, false, Hit))
				{
					if (AActor* HitActor = Hit.GetActor())
					{
						if (HitActor->IsChildActor() && EnablePickParentActorSelection == ECheckBoxState::Checked)
						{
							SelectedActor = HitActor->GetParentActor();
						}
						else
						{
							SelectedActor = HitActor;
						}
						
						for (int32 i = 0; i < WorldList.Num(); ++i)
						{
							if (WorldList[i].IsValid() && WorldList[i].Get() == SelectedActor->GetWorld())
							{
								bForceWorldComboRefresh = true;
								bForceActorComboRefresh = true;
								if (SelectedWorldIndex != i)
								{
									RefreshWorldList();
									SelectedWorldIndex = i;
									RefreshActorList();
								}
								break;
							}
						}

						for (int32 i = 0; i < ActorList.Num(); ++i)
						{
							if (ActorList[i].IsValid() && ActorList[i].Get() == SelectedActor)
							{
								SelectedActorIndex = i;
								break;
							}
						}

						UE_LOG(LogTemp, Log, TEXT("Selected actor: %s (World: %s)"),
							*SelectedActor->GetName(), *SelectedActor->GetWorld()->GetName());

						return;
					}
				}
			}
		}
	}
}

void FKaosDebugger_MainTab_Actor::DrawDetails(const FKaosDebuggerContext& Context)
{
	SlateIM::BeginVerticalStack();
	SlateIM::BeginHorizontalStack();

	SlateIM::CheckBox(TEXT("Enable World Actor Selection"), EnableWorldActorSelection);
	SlateIM::CheckBox(TEXT("Pick Parent Actor"), EnablePickParentActorSelection);

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
		bRefreshActors = true;
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

		const FWorldContext* FoundContext = GEngine->GetWorldContextFromWorld(W);
		if (FoundContext)
		{
			if (FoundContext->WorldType == EWorldType::PIE)
			{
				Name += FString::Printf(TEXT(" [PIE:%d]"), FoundContext->PIEInstance);
			}

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
			bRefreshActors = true;
		}
		bForceWorldComboRefresh = false;

		if (bRefreshActors)
		{
			RefreshActorList();
			bRefreshActors = false;
		}

		TArray<FString> ActorNames;
		for (const auto& A : ActorList)
		{
			ActorNames.Add(A.IsValid() ? A->GetName() : TEXT("<Invalid>"));
		}

		SelectedActorIndex = FMath::Clamp(SelectedActorIndex, 0, ActorNames.Num() - 1);

		SlateIM::Text(TEXT("Actor:"));
		SlateIM::MinWidth(120.f);
		SlateIM::ComboBox(ActorNames, SelectedActorIndex, bForceActorComboRefresh);
		bForceActorComboRefresh = false;

	SlateIM::EndHorizontalStack();

	if (EnableWorldActorSelection == ECheckBoxState::Checked)
	{
		HandleActorSelection();
	}

#if WITH_EDITOR
	if (!bBoundToEditorPickedDelegate)
	{
		bBoundToEditorPickedDelegate = true;
		USelection::SelectObjectEvent.AddRaw(this, &FKaosDebugger_MainTab_Actor::HandleActorSelectionChanged);
	}
#endif
	
	SelectedActor = (ActorList.IsValidIndex(SelectedActorIndex))
		? ActorList[SelectedActorIndex].Get()
		: nullptr;

	SelectedWorld = (WorldList.IsValidIndex(SelectedWorldIndex))
		? WorldList[SelectedWorldIndex].Get()
		: nullptr;

	FKaosDebuggerContext TabContext;
	TabContext.ContextObject = SelectedActor;
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
			Module.GetRegisteredSubCategoriesFor(KaosDebuggerMainTabAreas::Actor);
	
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

#if WITH_EDITOR
void FKaosDebugger_MainTab_Actor::HandleActorSelectionChanged(UObject* Object)
{
	if (AActor* Actor = Cast<AActor>(Object))
	{
		if (Actor->IsChildActor() && EnablePickParentActorSelection == ECheckBoxState::Checked)
		{
			SelectedActor = Actor->GetParentActor();
		}
		else
		{
			SelectedActor = Actor;
		}
						
		for (int32 i = 0; i < WorldList.Num(); ++i)
		{
			if (WorldList[i].IsValid() && WorldList[i].Get() == SelectedActor->GetWorld())
			{
				bForceWorldComboRefresh = true;
				bForceActorComboRefresh = true;
				if (SelectedWorldIndex != i)
				{
					RefreshWorldList();
					SelectedWorldIndex = i;
					RefreshActorList();
				}
				break;
			}
		}

		for (int32 i = 0; i < ActorList.Num(); ++i)
		{
			if (ActorList[i].IsValid() && ActorList[i].Get() == SelectedActor)
			{
				SelectedActorIndex = i;
				break;
			}
		}

		UE_LOG(LogTemp, Log, TEXT("Selected actor: %s (World: %s)"),
			*SelectedActor->GetName(), *SelectedActor->GetWorld()->GetName());

		return;
	}
}
#endif


void FKaosDebugger_MainTab_Actor::RefreshWorldList()
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
			bRefreshActors = true;
		}
	}
}

void FKaosDebugger_MainTab_Actor::RefreshActorList()
{
	if (!WorldList.IsValidIndex(SelectedWorldIndex))
	{
		ActorList.Reset();
		return;
	}

	UWorld* World = WorldList[SelectedWorldIndex].Get();
	if (!IsValid(World))
	{
		ActorList.Reset();
		return;
	}

	TArray<TWeakObjectPtr<AActor>> NewList;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (IsValid(Actor))
		{
			NewList.Add(Actor);
		}
	}

	NewList.Sort([](const TWeakObjectPtr<AActor>& A, const TWeakObjectPtr<AActor>& B)
	{
		if (!A.IsValid()) return false;
		if (!B.IsValid()) return true;
		return A->GetName() < B->GetName();
	});

	const bool bListChanged = (NewList.Num() != ActorList.Num()) || !Algo::CompareBy(NewList, ActorList, [](const TWeakObjectPtr<AActor>& A)
	{
		return A.Get();
	});

	if (bListChanged)
	{
		ActorList = MoveTemp(NewList);
		bForceActorComboRefresh = true;

		SelectedActorIndex = INDEX_NONE;

		if (SelectedActor.IsValid())
		{
			UWorld* SelectedActorWorld = SelectedActor->GetWorld();
			UWorld* CurrentWorld = WorldList.IsValidIndex(SelectedWorldIndex) ? WorldList[SelectedWorldIndex].Get() : nullptr;

			if (SelectedActorWorld == CurrentWorld)
			{
				for (int32 i = 0; i < ActorList.Num(); ++i)
				{
					if (ActorList[i] == SelectedActor)
					{
						SelectedActorIndex = i;
						break;
					}
				}
			}
			else
			{
				SelectedActor = nullptr;
			}
		}
	}
}

FSlateIcon FKaosDebugger_MainTab_Actor::GetTabIcon() const
{
	static const FSlateIcon MyIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.WorldProperties.Small");

	return MyIcon;
}
#endif
