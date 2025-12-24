// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TestMqasNetWidget.h"

void UTestMqasNetWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if(UButton* Temp = Cast<UButton>(GetWidgetFromName(TEXT("btn_connect"))))
	{
		_ButtonConnect = Temp;
		FScriptDelegate Delegate;
		Delegate.BindUFunction(this, "OnBtnTestClick");
		Temp->OnClicked.Add(Delegate);
	}
}

void UTestMqasNetWidget::OnBtnTestClick()
{
	UE_LOG(LogTemp, Warning, TEXT("UTestMqasNetWidget::OnBtnTestClick"));
}



