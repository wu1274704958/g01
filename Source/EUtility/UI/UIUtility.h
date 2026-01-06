// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <map>
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Components/Widget.h"
#include "EUtility/LambdaWrapper.h"
#include "Blueprint/UserWidget.h"
#include "UIUtility.generated.h"

UCLASS()
class EUTILITY_API UIUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static constexpr int ToastZOrder = 1000;
	static int CurrentToastZOrder;
	UFUNCTION(BlueprintCallable, Category="UI|Toast")
	static void ShowToast(UObject* obj,int PlayerIndex,const TSubclassOf<class UUserWidget> Class,const FString& Message,
		float Duration = 2.0f,float XOffset = 0.5f,float YOffset = 0.7f);

	template<typename W>
	requires std::is_base_of_v<UWidget,W>
	static W* BindWidget(UUserWidget* rootWidget,const FName& widgetName,std::function<void(W*)> onBind = {})
	{
		if(W* Temp = Cast<W>(rootWidget->GetWidgetFromName(widgetName));Temp != nullptr)
		{
			if(onBind)
			{
				onBind(Temp);
			}
			return Temp;
		}
		return nullptr;
	}

};
