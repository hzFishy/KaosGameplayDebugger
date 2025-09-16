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

#include "CoreMinimal.h"
#if WITH_KAOS_GAMEPLAYDEBUGGER
#include "KaosDebuggerBaseItem.h"
#include "KaosDebuggerContext.h"

struct FKaosDebugger_MainTab_Actor: public IKaosDebuggerBaseItem
{
public:
	void HandleActorSelection();

	virtual void DrawDetails(const FKaosDebuggerContext& Context) override;
	
private:

#if WITH_EDITOR
	void HandleActorSelectionChanged(UObject* Object);
	bool bBoundToEditorPickedDelegate = false;
#endif
	
	// Helpers
	void RefreshWorldList();
	void RefreshActorList();

	
	// State shared across tabs
	TArray<TWeakObjectPtr<UWorld>> WorldList;
	TArray<TWeakObjectPtr<AActor>> ActorList;
	bool bForceWorldComboRefresh = true;
	int32 SelectedWorldIndex = INDEX_NONE;
	int32 SelectedActorIndex = INDEX_NONE;

	ECheckBoxState ShouldAutoRefresh = ECheckBoxState::Checked;
	float RefreshInterval = .5f;
	bool bRefreshAll = false;
	bool bRefreshActors = false;
	bool bForceActorComboRefresh = false;
	double CurrentTime = 0;
	double TimeSinceLastUpdate = 0;
	ECheckBoxState EnableWorldActorSelection = ECheckBoxState::Unchecked;
	ECheckBoxState EnablePickParentActorSelection = ECheckBoxState::Unchecked;
	TWeakObjectPtr<AActor> SelectedActor;
	TWeakObjectPtr<UWorld> SelectedWorld;

public:
	virtual FText GetTabLabel() const override { return FText::FromString(TEXT("Actor Info")); }
	virtual FSlateIcon GetTabIcon() const override;;

};

#endif