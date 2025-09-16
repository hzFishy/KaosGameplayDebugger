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

#include "Implementations/KaosWorldDebugger_World_Details.h"
#if WITH_KAOS_GAMEPLAYDEBUGGER
#include "EngineUtils.h"
#include "Engine/NetDriver.h" 
#include "Engine/World.h"  
#include "KaosSlateIMHelpers.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/LevelStreaming.h"

static FString GetNetModeString(ENetMode NetMode)
{
	switch (NetMode)
	{
	case NM_Standalone:     return TEXT("Standalone");
	case NM_DedicatedServer:return TEXT("Dedicated Server");
	case NM_ListenServer:   return TEXT("Listen Server");
	case NM_Client:         return TEXT("Client");
	default:                return TEXT("Unknown");
	}
}

static FString GetWorldTypeString(EWorldType::Type Type)
{
	switch (Type)
	{
	case EWorldType::None:        return TEXT("None");
	case EWorldType::Game:        return TEXT("Game");
	case EWorldType::Editor:      return TEXT("Editor");
	case EWorldType::PIE:         return TEXT("PIE");
	case EWorldType::EditorPreview:return TEXT("Editor Preview");
	case EWorldType::GamePreview: return TEXT("Game Preview");
	case EWorldType::GameRPC:     return TEXT("Game RPC");
	case EWorldType::Inactive:    return TEXT("Inactive");
	default:                      return TEXT("Unknown");
	}
}


void FKaosWorldDebugger_World_Details::DrawDetails(const FKaosDebuggerContext& Context)
{
	if (!Context.ContextWorld.IsValid())
	{
		SlateIM::BeginVerticalStack();
		KaosSlateIM::ErrorText(TEXT("No World Selected"));
		SlateIM::EndVerticalStack();
		return;
	}

	UWorld* World = Context.ContextWorld.Get();
	{
		SlateIM::BeginTabGroup(TEXT("WorldTabs"));
		SlateIM::BeginTabStack();

		if (SlateIM::BeginTab(TEXT("Overview"), FSlateIcon(), FText::FromString(TEXT("Overview"))))
		{
			SlateIM::Fill();
			SlateIM::HAlign(HAlign_Fill);
			SlateIM::VAlign(VAlign_Fill);
			SlateIM::BeginScrollBox();
			SlateIM::BeginVerticalStack();
			KaosSlateIM::DrawLabledText(TEXT("World Name"), World->GetName());
			KaosSlateIM::DrawLabledText(TEXT("World Type"), GetWorldTypeString(World->WorldType));
			KaosSlateIM::DrawLabledText(TEXT("Persistent Level"), World->PersistentLevel ? World->PersistentLevel->GetName() : TEXT("None"));
			KaosSlateIM::DrawLabledText(TEXT("Actor Count"), FString::FromInt(World->GetActorCount()));
			
			if (AGameModeBase* GameMode = World->GetAuthGameMode())
			{
				KaosSlateIM::DrawLabledText(TEXT("GameMode"), GameMode->GetName());
			}
			if (AGameStateBase* GameState = World->GetGameState())
			{
				KaosSlateIM::DrawLabledText(TEXT("GameState"), GameState->GetName());
			}
			SlateIM::EndVerticalStack();
			SlateIM::EndScrollBox();
		}
		SlateIM::EndTab();

		if (SlateIM::BeginTab(TEXT("Levels"), FSlateIcon(), FText::FromString(TEXT("Levels"))))
		{
			SlateIM::Fill();
			SlateIM::HAlign(HAlign_Fill);
			SlateIM::VAlign(VAlign_Fill);
			SlateIM::BeginScrollBox();
			SlateIM::BeginVerticalStack();
			const TArray<ULevelStreaming*>& StreamingLevels = World->GetStreamingLevels();
			KaosSlateIM::DrawLabledText(TEXT("Streaming Level Count"), FString::FromInt(StreamingLevels.Num()));

			for (const ULevelStreaming* StreamingLevel : StreamingLevels)
			{
				if (!StreamingLevel) continue;

				FString LevelName = StreamingLevel->GetWorldAssetPackageName();
				FString Status = FString::Printf(TEXT("Visible: %s | Loaded: %s"),
					StreamingLevel->IsLevelVisible() ? TEXT("Yes") : TEXT("No"),
					StreamingLevel->IsLevelLoaded() ? TEXT("Yes") : TEXT("No"));

				KaosSlateIM::DrawLabledText(LevelName, Status);
			}
			SlateIM::EndVerticalStack();
			SlateIM::EndScrollBox();
		}
		SlateIM::EndTab();

		if (SlateIM::BeginTab(TEXT("Time"), FSlateIcon(), FText::FromString(TEXT("Time"))))
		{
			SlateIM::Fill();
			SlateIM::HAlign(HAlign_Fill);
			SlateIM::VAlign(VAlign_Fill);
			SlateIM::BeginScrollBox();
			SlateIM::BeginVerticalStack();
			KaosSlateIM::DrawLabledText(TEXT("Time Seconds"), FString::Printf(TEXT("%.2f"), World->TimeSeconds));
			KaosSlateIM::DrawLabledText(TEXT("Real Time"), FString::Printf(TEXT("%.2f"), World->GetRealTimeSeconds()));
			KaosSlateIM::DrawLabledText(TEXT("Delta Seconds"), FString::Printf(TEXT("%.4f"), World->GetDeltaSeconds()));
			if (AWorldSettings* Settings = World->GetWorldSettings())
			{
				KaosSlateIM::DrawLabledText(TEXT("Time Dilation"), FString::Printf(TEXT("%.2f"), Settings->GetEffectiveTimeDilation()));
			}
			KaosSlateIM::DrawLabledText(TEXT("Is Paused"), World->IsPaused() ? TEXT("Yes") : TEXT("No"));
			SlateIM::EndVerticalStack();
			SlateIM::EndScrollBox();
		}
		SlateIM::EndTab();

		if (SlateIM::BeginTab(TEXT("Network"), FSlateIcon(), FText::FromString(TEXT("Network"))))
		{
			SlateIM::Fill();
			SlateIM::HAlign(HAlign_Fill);
			SlateIM::BeginScrollBox(Orient_Horizontal);
			FKaosNetworkStatsCache& Cache = CachedNetworkStats.FindOrAdd(Context.ContextWorld);
			GatherNetworkStatsIfNeeded(World, Cache);
			DrawNetworkTab(Context, Cache);
			SlateIM::EndScrollBox();
		}
		SlateIM::EndTab();

		SlateIM::EndTabStack();
		SlateIM::EndTabGroup();
	}
}

void FKaosWorldDebugger_World_Details::GatherNetworkStatsIfNeeded(UWorld* World, FKaosNetworkStatsCache& Cache)
{
	if (!IsValid(World)) return;

	int32 CurrentReplicatedCount = 0;
	TMap<UClass*, int32> ClassCount;
	TMap<UClass*, int32> ActualClassCount;
	TMap<ENetDormancy, int32> DormancyCount;
	FBox BoundingBox(ForceInit);

	TArray<FKaosReplicatedActorInfo> AwakeActors;
	TArray<FKaosReplicatedActorInfo> DormantActors;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor->GetIsReplicated()) continue;

		++CurrentReplicatedCount;
		BoundingBox += Actor->GetActorLocation();

		ENetDormancy Dorm = Actor->NetDormancy;
		DormancyCount.FindOrAdd(Dorm)++;

		UClass* Class = Actor->GetClass();
		ActualClassCount.FindOrAdd(Class)++;
		while (Class)
		{
			ClassCount.FindOrAdd(Class)++;
			Class = Class->GetSuperClass();
		}

		FKaosReplicatedActorInfo Info;
		Info.ActorName = GetNameSafe(Actor);
		Info.ClassName = GetNameSafe(Actor->GetClass());
		Info.Dormancy = StaticEnum<ENetDormancy>()->GetNameStringByValue((int64)Dorm);
		Info.NetUpdateFreq = Actor->GetNetUpdateFrequency();
		Info.NetUpdatePriority = Actor->NetPriority;

		if (Dorm == DORM_Awake)
			AwakeActors.Add(MoveTemp(Info));
		else
			DormantActors.Add(MoveTemp(Info));
	}

	if (Cache.LastReplicatedActorCount != CurrentReplicatedCount)
	{
		Cache.LastReplicatedActorCount = CurrentReplicatedCount;
		Cache.CachedBoundingBox = BoundingBox;

		Cache.CachedDormancyCounts.Reset();
		for (auto& Pair : DormancyCount)
		{
			FString DormName = StaticEnum<ENetDormancy>()->GetNameStringByValue((int64)Pair.Key);
			Cache.CachedDormancyCounts.Add(DormName, Pair.Value);
		}

		ClassCount.ValueSort(TGreater<int32>());
		Cache.CachedClassCounts.Reset();
		for (auto& Pair : ClassCount)
		{
			Cache.CachedClassCounts.Add({ GetNameSafe(Pair.Key), Pair.Value });
		}

		ActualClassCount.ValueSort(TGreater<int32>());
		Cache.CachedActualClassCounts.Reset();
		for (auto& Pair : ActualClassCount)
		{
			Cache.CachedActualClassCounts.Add({ GetNameSafe(Pair.Key), Pair.Value });
		}

		Cache.AwakeActors = MoveTemp(AwakeActors);
		Cache.DormantActors = MoveTemp(DormantActors);
	}
}

void FKaosWorldDebugger_World_Details::DrawNetworkTab(const FKaosDebuggerContext& Context, FKaosNetworkStatsCache& Cache)
{
	SlateIM::Fill();
	SlateIM::HAlign(HAlign_Fill);
	SlateIM::VAlign(VAlign_Fill);
	SlateIM::BeginHorizontalStack();
	SlateIM::Fill();
	SlateIM::HAlign(HAlign_Fill);
	SlateIM::VAlign(VAlign_Fill);
	SlateIM::BeginVerticalStack();

	KaosSlateIM::SubHeaderText(TEXT("Network Overview"));
	FString NetModeStr = GetNetModeString(Context.ContextWorld->GetNetMode());
	KaosSlateIM::DrawLabledText(TEXT("Net Mode"), NetModeStr);

	if (UNetDriver* NetDriver = Context.ContextWorld->GetNetDriver())
	{
		KaosSlateIM::DrawLabledText(TEXT("NetDriver"), NetDriver->GetName());
		KaosSlateIM::DrawLabledText(TEXT("Is Server"), NetDriver->IsServer() ? TEXT("Yes") : TEXT("No"));
		KaosSlateIM::DrawLabledText(TEXT("Client Connections"), FString::FromInt(NetDriver->ClientConnections.Num()));
	}
	else
	{
		KaosSlateIM::DrawLabledText(TEXT("NetDriver"), TEXT("None"));
	}

	KaosSlateIM::DrawLabledText(TEXT("Replicated Actor Count"), FString::FromInt(Cache.LastReplicatedActorCount));
	KaosSlateIM::DrawLabledText(TEXT("Replicated Bounds"), Cache.CachedBoundingBox.ToString());
	KaosSlateIM::SubHeaderText(TEXT("Dormancy Breakdown"));
	for (const auto& Pair : Cache.CachedDormancyCounts)
	{
		KaosSlateIM::DrawLabledText(Pair.Key, FString::FromInt(Pair.Value));
	}
	SlateIM::EndVerticalStack();

	SlateIM::Fill();
	SlateIM::HAlign(HAlign_Fill);
	SlateIM::BeginVerticalStack();
	SlateIM::HAlign(HAlign_Fill);
	KaosSlateIM::HeaderText(TEXT("Dormant Actors"));
	DrawActorTable(Cache.DormantActors);
	SlateIM::HAlign(HAlign_Fill);
	KaosSlateIM::HeaderText(TEXT("Awake Actors"));
	DrawActorTable(Cache.AwakeActors);
	SlateIM::EndVerticalStack();
	SlateIM::EndHorizontalStack();
}


void FKaosWorldDebugger_World_Details::DrawActorTable(const TArray<FKaosReplicatedActorInfo>& Actors)
{
	SlateIM::Fill();
	SlateIM::VAlign(VAlign_Fill);
	SlateIM::MinHeight(1500.f);
	SlateIM::MaxHeight(1500.f);
	SlateIM::BeginTable();
	SlateIM::InitialTableColumnWidth(200.f); SlateIM::AddTableColumn(TEXT("Actor"));
	SlateIM::InitialTableColumnWidth(200.f); SlateIM::AddTableColumn(TEXT("Class"));
	SlateIM::InitialTableColumnWidth(120.f); SlateIM::AddTableColumn(TEXT("Dormancy"));
	SlateIM::InitialTableColumnWidth(70.f);  SlateIM::AddTableColumn(TEXT("Upd. Frequency"));
	SlateIM::InitialTableColumnWidth(70.f);  SlateIM::AddTableColumn(TEXT("Upd. Priority"));

	for (const FKaosReplicatedActorInfo& Info : Actors)
	{
		if (SlateIM::NextTableCell())
		{
			SlateIM::Text(Info.ActorName);
		}
		if (SlateIM::NextTableCell())
		{
			SlateIM::Text(Info.ClassName);
		}
		if (SlateIM::NextTableCell())
		{
			SlateIM::Text(Info.Dormancy);
		}
		if (SlateIM::NextTableCell())
		{
			SlateIM::Text(FString::Printf(TEXT("%.1f"), Info.NetUpdateFreq));
		}
		if (SlateIM::NextTableCell())
		{
			SlateIM::Text(FString::Printf(TEXT("%.1f"), Info.NetUpdatePriority));
		}
	}

	SlateIM::EndTable();
}


FSlateIcon FKaosWorldDebugger_World_Details::GetTabIcon() const
{
	static const FSlateIcon MyIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "WorldBrowser.CompositionButtonBrush");

	return MyIcon;
}
#endif
