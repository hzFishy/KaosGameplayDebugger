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

#include "Implementations/KaosWorldDebugger_Actor_Details.h"
#if WITH_KAOS_GAMEPLAYDEBUGGER
#include "GameFramework/Actor.h"

void FKaosWorldDebugger_Actor_Details::DrawDetails(const FKaosDebuggerContext& Context)
{
	AActor* SelectedActor = Cast<AActor>(Context.ContextObject.Get());

	if (!SelectedActor)
	{
		return;
	}
	SlateIM::Text(FString::Printf(TEXT("Selected Actor: %s"), *SelectedActor->GetName()));
	SlateIM::Text(FString::Printf(TEXT("Location: %s"), *SelectedActor->GetActorLocation().ToString()));
	SlateIM::Text(FString::Printf(TEXT("Class: %s"), *SelectedActor->GetClass()->GetName()));
}

FSlateIcon FKaosWorldDebugger_Actor_Details::GetTabIcon() const
{
	static const FSlateIcon MyIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "WorldPartition.ShowActors");

	return MyIcon;
}
#endif
