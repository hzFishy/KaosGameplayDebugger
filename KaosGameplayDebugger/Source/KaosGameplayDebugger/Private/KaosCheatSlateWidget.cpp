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

#include "KaosCheatSlateWidget.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "KaosGameplayDebuggerDevSettings.h"
#include "GameFramework/CheatManager.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "UObject/UObjectIterator.h"

#if WITH_KAOS_GAMEPLAYDEBUGGER

void FSlateIMKaosCheats::Draw(float DeltaTime)
{

	if (!World.IsValid())
	{
		SlateIM::Text(TEXT("No valid world."));
		return;
	}

	UWorld* ActualWorld = World.Get();
	APlayerController* PC = ActualWorld->GetFirstPlayerController();
	if (!PC || !PC->IsLocalController())
	{
		SlateIM::Text(TEXT("No local player controller."));
		return;
	}

	if (!PC->CheatManager)
	{
		SlateIM::Text(TEXT("No CheatManager available."));
		return;
	}

	if (CachedFunctions.IsEmpty())
	{
		CacheCheatFunctions(PC);
	}

	SlateIM::BeginScrollBox();

	for (const FCheatFunction& Func : CachedFunctions)
	{
		FString DisplayName = Func.Name;
		const FString Prefix = TEXT("Cheat_WorldMap_");
		if (DisplayName.StartsWith(Prefix))
		{
			DisplayName = DisplayName.RightChop(Prefix.Len());
		}

		SlateIM::BeginVerticalStack();

		if (SlateIM::Button(*DisplayName))
		{
			uint8* Buffer = nullptr;

			if (!Func.Params.IsEmpty())
			{
				Buffer = static_cast<uint8*>(FMemory::Malloc(Func.Function->ParmsSize));
				FMemory::Memzero(Buffer, Func.Function->ParmsSize);

				for (const auto& Param : Func.Params)
				{
					void* Addr = Buffer + Param.Offset;
					FString InputKey = Func.Name + TEXT("_") + Param.Name;

					if (FIntProperty* IntProp = CastField<FIntProperty>(Param.Property))
					{
						IntProp->SetPropertyValue(Addr, IntInputs.FindOrAdd(InputKey));
					}
					else if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Param.Property))
					{
						FloatProp->SetPropertyValue(Addr, FloatInputs.FindOrAdd(InputKey));
					}
					else if (FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Param.Property))
					{
						DoubleProp->SetPropertyValue(Addr, DoubleInputs.FindOrAdd(InputKey));
					}
					else if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Param.Property))
					{
						BoolProp->SetPropertyValue(Addr, BoolInputs.FindOrAdd(InputKey));
					}
					else if (FStrProperty* StrProp = CastField<FStrProperty>(Param.Property))
					{
						StrProp->SetPropertyValue(Addr, StringInputs.FindOrAdd(InputKey));
					}
				}
			}

			Func.Target->ProcessEvent(Func.Function, Buffer);

			if (Buffer)
			{
				FMemory::Free(Buffer);
			}
		}

		for (const auto& Param : Func.Params)
		{
			FString InputKey = Func.Name + TEXT("_") + Param.Name;

			SlateIM::BeginHorizontalStack();
			SlateIM::Text(Param.Name);
			SlateIM::Fill();

			if (Param.Property->IsA<FIntProperty>())
			{
				SlateIM::SpinBox(IntInputs.FindOrAdd(InputKey), -100000, 100000);
			}
			else if (Param.Property->IsA<FFloatProperty>())
			{
				SlateIM::SpinBox(FloatInputs.FindOrAdd(InputKey), -100000.f, 100000.f);
			}
			else if (Param.Property->IsA<FDoubleProperty>())
			{
				SlateIM::SpinBox(DoubleInputs.FindOrAdd(InputKey), -100000, 100000); // optional: custom double spinbox
			}
			else if (Param.Property->IsA<FBoolProperty>())
			{
				SlateIM::CheckBox(TEXT(""), BoolInputs.FindOrAdd(InputKey));
			}
			else if (Param.Property->IsA<FStrProperty>())
			{
				SlateIM::EditableText(StringInputs.FindOrAdd(InputKey), TEXT("..."));
			}

			SlateIM::EndHorizontalStack();
		}

		SlateIM::EndVerticalStack();
		SlateIM::Spacer({1.f, 10.f});
	}

	SlateIM::EndScrollBox();
}
void FSlateIMKaosCheats::SetWorld(TWeakObjectPtr<UWorld> InWorld)
{

	CachedFunctions.Reset();
	
	if (!InWorld.IsValid()) return;

	World = InWorld;

	if (APlayerController* PC = InWorld->GetFirstPlayerController())
	{
		if (PC->CheatManager)
		{
			CacheCheatFunctions(PC);
		}
	}
}

bool FSlateIMKaosCheats::ShouldExposeFunction(UFunction* Func)
{
	if (!Func->HasAnyFunctionFlags(FUNC_Exec))
	{
		return false;
	}
	
	if (GetDefault<UKaosGameplayDebuggerDevSettings>()->BlacklistedCommands.Contains(Func->GetName()))
	{
		return false;
	}
	
	bool bIsBP = Func->GetOuter()->IsA<UBlueprintGeneratedClass>();
	if (!bIsBP)
	{
		return false;
	}

	for (TFieldIterator<FProperty> It(Func); It; ++It)
	{
		FProperty* Prop = *It;

		if (!Prop->HasAnyPropertyFlags(CPF_Parm))
		{
			continue;
		}
		
		if (Prop->HasAnyPropertyFlags(CPF_ReturnParm))
		{
			continue;
		}
		
		if (!(Prop->IsA<FIntProperty>() ||
			  Prop->IsA<FFloatProperty>() ||
			  Prop->IsA<FDoubleProperty>() ||
			  Prop->IsA<FBoolProperty>() ||
			  Prop->IsA<FStrProperty>()))
		{
			return false; // unsupported param type
		}
	}
	return true;
}

void FSlateIMKaosCheats::CacheCheatFunctions(APlayerController* PC)
{

	  if (!PC || !PC->CheatManager)
	  {
		  return;
	  }
    TArray<FCheatFunction>& CachedFuncs = CachedFunctions;
    CachedFuncs.Reset();

    TArray<UObject*> CheatSources;

    UCheatManager* CheatMgr = PC->CheatManager;
	if (GetDefault<UKaosGameplayDebuggerDevSettings>()->bAllowCheatManagerDirectCheats)
	{
		CheatSources.Add(CheatMgr);
	}
	
    static TArray<UClass*> CheatExtensionTypes;
    if (CheatExtensionTypes.Num() == 0)
    {
    	for (TObjectIterator<UClass> It; It; ++It)
    	{
    		UClass* Class = *It;

    		if (Class->IsChildOf(UCheatManagerExtension::StaticClass()) &&
				!Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
    		{
    			const UKaosGameplayDebuggerDevSettings* Settings = GetDefault<UKaosGameplayDebuggerDevSettings>();
    			bool bAllowed = false;
    			for (const TSoftClassPtr<UCheatManagerExtension>& AllowedClassPtr : Settings->AllowedCheatManagerExtensionClasses)
    			{
    				if (AllowedClassPtr.IsValid())
    				{
    					if (Class == AllowedClassPtr.Get())
    					{
    						bAllowed = true;
    						break;
    					}
    				}
    			}

    			if (bAllowed)
    			{
    				CheatExtensionTypes.Add(Class);
    			}
    		}
    	}
    }

    for (UClass* ExtClass : CheatExtensionTypes)
    {
        if (UCheatManagerExtension* Ext = CheatMgr->FindCheatManagerExtension(ExtClass))
        {
            CheatSources.Add(Ext);
        }
    }

    for (UObject* Source : CheatSources)
    {
        for (TFieldIterator<UFunction> FuncIt(Source->GetClass(), EFieldIteratorFlags::IncludeSuper); FuncIt; ++FuncIt)
        {
            UFunction* Func = *FuncIt;
            if (!ShouldExposeFunction(Func))
            {
	            continue;
            }
        	
            FCheatFunction Info;
        	Info.Target = Source; 
            Info.Function = Func;
            Info.Name = Func->GetName();
            Info.bHasParameters = Func->NumParms > 0;

        	for (TFieldIterator<FProperty> ParamIt(Func); ParamIt && ParamIt->HasAnyPropertyFlags(CPF_Parm); ++ParamIt)
            {
                FCheatParam Param;
                Param.Property = *ParamIt;
                Param.Name = ParamIt->GetName();
                Param.TypeName = ParamIt->GetCPPType();
                Param.Offset = ParamIt->GetOffset_ForInternal();
                Info.Params.Add(Param);
            }

            CachedFuncs.Add(Info);
        }
    }

	CachedFuncs.Sort([](const FCheatFunction& A, const FCheatFunction& B)
	{
		return A.Name < B.Name;
	});
#
}

void FSlateIMKaosCheats::Reset()
{

	CachedFunctions.Reset();
	IntInputs.Reset();
	FloatInputs.Reset();
	DoubleInputs.Reset();
	BoolInputs.Reset();
	StringInputs.Reset();
}



void FKaosSlateCheatWidget::Reset()
{
	if (bIsReset)
	{
		return;
	}
	
	if (LocalPlayerCache.IsValid() && LocalPlayerCache->GetGameInstance())
	{
		LocalPlayerCache->GetGameInstance()->OnLocalPlayerRemovedEvent.Remove(BoundDelegate);
	}
	bIsReset = true;
}

void FKaosSlateCheatWidget::SetLocalPlayer(ULocalPlayer* LocalPlayer)
{
	LocalPlayerCache = LocalPlayer;
	CheatWidget.SetWorld(LocalPlayer->GetWorld());
	BoundDelegate = LocalPlayer->GetGameInstance()->OnLocalPlayerRemovedEvent.AddLambda([this](ULocalPlayer* LocalPlayer)
	{
		Reset();
	});
}

void FKaosSlateCheatWidget::DrawWindow(float DeltaTime)
{
	CheatWidget.Draw(DeltaTime);
}

void FKaosSlateCheatWidget::DrawWidget(float DeltaTime)
{

	if (bIsReset)
	{
		return;
	}
	
	if (!LocalPlayerCache.IsValid())
	{
		return;
	}
	
		SlateIM::FViewportRootLayout AdjustedLayout = Layout;
		if (bCheatsCollapsed)
		{
			AdjustedLayout.Size = FVector2f(300.f, 100.f);
		}
		if (SlateIM::BeginViewportRoot("KaosSlateCheats", LocalPlayerCache.Get(), AdjustedLayout))
		{
			SlateIM::BeginBorder(&TransparentBackground); // Begin nice framed background
			SlateIM::Text(TEXT("Kaos Cheat Console"), FLinearColor::Yellow);

			SlateIM::Spacer(FVector2D(0.f, 5.f));

			// Collapse/Expand toggle button
			if (SlateIM::Button(bCheatsCollapsed ? TEXT("▶ Cheats") : TEXT("▼ Cheats")))
			{
				bCheatsCollapsed = !bCheatsCollapsed;
			}

			if (!bCheatsCollapsed)
			{
				CheatWidget.Draw(DeltaTime);
			}

			SlateIM::EndBorder();
		}
		SlateIM::EndRoot();
}
#endif