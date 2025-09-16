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
#include "UObject/Interface.h"
#include "KaosGameplayDebuggerInfoProviderInterface.generated.h"

#if WITH_KAOS_GAMEPLAYDEBUGGER
enum class EKaosDebugLineType : uint8
{
	Text,
	Header,
	Warning,
	Error,
	ValuePair,  // Label + Value
	Separator
};

struct KAOSGAMEPLAYDEBUGGER_API FKaosDebugLine
{
	EKaosDebugLineType Type = EKaosDebugLineType::Text;
	FString Label;
	FString Value;

	FKaosDebugLine(EKaosDebugLineType InType, FString InLabel, FString InValue = TEXT(""))
		: Type(InType), Label(MoveTemp(InLabel)), Value(MoveTemp(InValue)) {}

	static FKaosDebugLine Header(const FString& Title)     { return FKaosDebugLine(EKaosDebugLineType::Header, Title); }
	static FKaosDebugLine Text(const FString& Line)        { return FKaosDebugLine(EKaosDebugLineType::Text, Line); }
	static FKaosDebugLine Warning(const FString& Line)     { return FKaosDebugLine(EKaosDebugLineType::Warning, Line); }
	static FKaosDebugLine Error(const FString& Line)       { return FKaosDebugLine(EKaosDebugLineType::Error, Line); }
	static FKaosDebugLine Pair(const FString& Label, const FString& Value) { return FKaosDebugLine(EKaosDebugLineType::ValuePair, Label, Value); }
	static FKaosDebugLine Separator()                      { return FKaosDebugLine(EKaosDebugLineType::Separator, TEXT("------------")); }
};
#endif

/**
 * 
 */

UINTERFACE(MinimalAPI)
class UKaosGameplayDebuggerInfoProviderInterface : public UInterface
{
	GENERATED_BODY()
};

class KAOSGAMEPLAYDEBUGGER_API IKaosGameplayDebuggerInfoProviderInterface
{
	GENERATED_BODY()

public:
#if WITH_KAOS_GAMEPLAYDEBUGGER
	virtual void GetKaosDebugLines(TArray<FKaosDebugLine>& OutLines) const = 0;
#endif
};


