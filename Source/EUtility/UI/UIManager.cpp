// Fill out your copyright notice in the Description page of Project Settings.

#include "UIManager.h"
#include "BaseWidget.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Transitions/SimpleFadeTransition.h"

void UUIManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UIViews.Empty();
	ExclusiveUIStack.Empty();
}

void UUIManager::Deinitialize()
{
	HideAllUI(true);
	UIViews.Empty();
	ExclusiveUIStack.Empty();
	FadeInCallbacks.Empty();
	FadeOutCallbacks.Empty();
	Super::Deinitialize();
}

UUIManager* UUIManager::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = WorldContextObject->GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UUIManager>();
}

void UUIManager::ShowUI(FName ViewName, 
	TSubclassOf<UBaseWidget> WidgetClass,
	TSubclassOf<UUIViewController> ControllerClass,
	const FUIViewConfig& Config,
	UObject* Model)
{
	if (ViewName.IsNone() || !WidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("UIManager::ShowUI - Invalid ViewName or WidgetClass"));
		return;
	}

	FUIViewInfo* ViewInfo = UIViews.Find(ViewName);
	
	if (!ViewInfo)
	{
		// 创建新的UI视图
		ViewInfo = CreateUIView(ViewName, WidgetClass, ControllerClass, Config);
		if (!ViewInfo)
		{
			UE_LOG(LogTemp, Error, TEXT("UIManager::ShowUI - Failed to create UI view: %s"), *ViewName.ToString());
			return;
		}
	}

	// 执行显示逻辑
	PerformShow(ViewInfo, Model);
}

void UUIManager::HideUI(FName ViewName, bool bDestroy)
{
	FUIViewInfo* ViewInfo = UIViews.Find(ViewName);
	if (!ViewInfo || !ViewInfo->bIsShowing)
	{
		return;
	}

	PerformHide(ViewInfo, bDestroy);
}

void UUIManager::HideAllUI(bool bDestroy)
{
	TArray<FName> ViewNames;
	UIViews.GetKeys(ViewNames);

	for (const FName& ViewName : ViewNames)
	{
		HideUI(ViewName, bDestroy);
	}
}

bool UUIManager::IsUIShowing(FName ViewName) const
{
	const FUIViewInfo* ViewInfo = UIViews.Find(ViewName);
	return ViewInfo && ViewInfo->bIsShowing;
}

UBaseWidget* UUIManager::GetUIWidget(FName ViewName) const
{
	const FUIViewInfo* ViewInfo = UIViews.Find(ViewName);
	return ViewInfo ? ViewInfo->Widget : nullptr;
}

UUIViewController* UUIManager::GetUIController(FName ViewName) const
{
	const FUIViewInfo* ViewInfo = UIViews.Find(ViewName);
	return ViewInfo ? ViewInfo->Controller : nullptr;
}

void UUIManager::UpdateUIModel(FName ViewName, UObject* Model)
{
	FUIViewInfo* ViewInfo = UIViews.Find(ViewName);
	if (ViewInfo && ViewInfo->Controller)
	{
		ViewInfo->Controller->SetModel(Model);
	}
}

void UUIManager::SetFadeInCallback(FName ViewName, std::function<void()> Callback)
{
	FadeInCallbacks.Add(ViewName, Callback);
}

void UUIManager::SetFadeOutCallback(FName ViewName, std::function<void()> Callback)
{
	FadeOutCallbacks.Add(ViewName, Callback);
}

FUIViewInfo* UUIManager::CreateUIView(FName ViewName,
	TSubclassOf<UBaseWidget> WidgetClass,
	TSubclassOf<UUIViewController> ControllerClass,
	const FUIViewConfig& Config)
{
	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return nullptr;
	}

	// 创建Widget
	UBaseWidget* Widget = CreateWidget<UBaseWidget>(PC, WidgetClass);
	if (!Widget)
	{
		return nullptr;
	}

	// 创建Controller
	UUIViewController* Controller = nullptr;
	if (ControllerClass)
	{
		Controller = NewObject<UUIViewController>(this, ControllerClass);
		if (Controller)
		{
			Controller->OnViewCreated(Widget);
		}
	}

	// 添加到视图映射表
	FUIViewInfo ViewInfo;
	ViewInfo.Widget = Widget;
	ViewInfo.Controller = Controller;
	ViewInfo.Config = Config;
	ViewInfo.ViewName = ViewName;
	ViewInfo.bIsShowing = false;
	ViewInfo.bIsTransitioning = false;

	UIViews.Add(ViewName, ViewInfo);

	return UIViews.Find(ViewName);
}

void UUIManager::PerformShow(FUIViewInfo* ViewInfo, UObject* Model)
{
	if (!ViewInfo || !ViewInfo->Widget)
	{
		return;
	}

	// 如果已经在显示，直接返回
	if (ViewInfo->bIsShowing)
	{
		return;
	}

	// 处理独占模式
	if (ViewInfo->Config.ShowMode == EUIShowMode::Exclusive)
	{
		HandleExclusiveMode(ViewInfo);
	}

	// 设置模型数据
	if (ViewInfo->Controller && Model)
	{
		ViewInfo->Controller->SetModel(Model);
	}

	// 生命周期回调
	if (ViewInfo->Controller)
	{
		ViewInfo->Controller->OnViewWillAppear();
	}

	// 添加到视口
	ViewInfo->Widget->AddToViewport(ViewInfo->Config.ZOrder);
	ViewInfo->bIsShowing = true;

	// 应用输入模式
	ApplyInputMode(ViewInfo, true);

	// 执行过渡动画
	ExecuteFadeIn(ViewInfo);
}

void UUIManager::PerformHide(FUIViewInfo* ViewInfo, bool bDestroy)
{
	if (!ViewInfo || !ViewInfo->Widget)
	{
		return;
	}

	// 生命周期回调
	if (ViewInfo->Controller)
	{
		ViewInfo->Controller->OnViewWillDisappear();
	}

	// 执行过渡动画
	ExecuteFadeOut(ViewInfo, bDestroy);
}

void UUIManager::HandleExclusiveMode(FUIViewInfo* ViewInfo)
{
	// 隐藏所有其他UI（除了弹窗）
	for (auto& Pair : UIViews)
	{
		FUIViewInfo& OtherView = Pair.Value;
		if (OtherView.bIsShowing && OtherView.Config.ShowMode != EUIShowMode::Popup)
		{
			if (OtherView.Widget)
			{
				OtherView.Widget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}

	// 添加到独占栈
	ExclusiveUIStack.Add(ViewInfo->ViewName);
}

void UUIManager::ExecuteFadeIn(FUIViewInfo* ViewInfo)
{
	if (!ViewInfo || !ViewInfo->Widget)
	{
		return;
	}

	ViewInfo->bIsTransitioning = true;

	// 调用Controller的淡入回调
	if (ViewInfo->Controller)
	{
		ViewInfo->Controller->OnFadeIn(ViewInfo->Config.FadeInDuration);
	}

	// 执行自定义淡入回调
	if (FadeInCallbacks.Contains(ViewInfo->ViewName))
	{
		FadeInCallbacks[ViewInfo->ViewName]();
	}

	// 使用 Transaction 类执行过渡动画
	FName ViewName = ViewInfo->ViewName;
	
	if (ViewInfo->Config.ShowTransactionClass)
	{
		UUITransaction* Transaction = NewObject<UUITransaction>(this, ViewInfo->Config.ShowTransactionClass);
		if (Transaction)
		{
			Transaction->Execute(ViewInfo->Widget, ViewInfo->Config.FadeInDuration, [this, ViewName]()
			{
				OnFadeInComplete(ViewName);
			});
		}
		else
		{
			OnFadeInComplete(ViewName);
		}
	}
	else
	{
		// 默认使用淡入效果
		UFadeInTransaction* Transaction = NewObject<UFadeInTransaction>(this);
		Transaction->Execute(ViewInfo->Widget, ViewInfo->Config.FadeInDuration, [this, ViewName]()
		{
			OnFadeInComplete(ViewName);
		});
	}
}

void UUIManager::ExecuteFadeOut(FUIViewInfo* ViewInfo, bool bDestroy)
{
	if (!ViewInfo || !ViewInfo->Widget)
	{
		return;
	}

	ViewInfo->bIsTransitioning = true;

	// 调用Controller的淡出回调
	if (ViewInfo->Controller)
	{
		ViewInfo->Controller->OnFadeOut(ViewInfo->Config.FadeOutDuration);
	}

	// 执行自定义淡出回调
	if (FadeOutCallbacks.Contains(ViewInfo->ViewName))
	{
		FadeOutCallbacks[ViewInfo->ViewName]();
	}

	// 使用 Transaction 类执行过渡动画
	FName ViewName = ViewInfo->ViewName;
	
	if (ViewInfo->Config.HideTransactionClass)
	{
		UUITransaction* Transaction = NewObject<UUITransaction>(this, ViewInfo->Config.HideTransactionClass);
		if (Transaction)
		{
			Transaction->Execute(ViewInfo->Widget, ViewInfo->Config.FadeOutDuration, [this, ViewName, bDestroy]()
			{
				OnFadeOutComplete(ViewName, bDestroy);
			});
		}
		else
		{
			OnFadeOutComplete(ViewName, bDestroy);
		}
	}
	else
	{
		// 默认使用淡出效果
		UFadeOutTransaction* Transaction = NewObject<UFadeOutTransaction>(this);
		Transaction->Execute(ViewInfo->Widget, ViewInfo->Config.FadeOutDuration, [this, ViewName, bDestroy]()
		{
			OnFadeOutComplete(ViewName, bDestroy);
		});
	}
}

void UUIManager::OnFadeInComplete(FName ViewName)
{
	FUIViewInfo* ViewInfo = UIViews.Find(ViewName);
	if (!ViewInfo)
	{
		return;
	}

	ViewInfo->bIsTransitioning = false;

	if (ViewInfo->Widget)
	{
		ViewInfo->Widget->SetRenderOpacity(1.0f);
	}

	// 生命周期回调
	if (ViewInfo->Controller)
	{
		ViewInfo->Controller->OnViewDidAppear();
	}
}

void UUIManager::OnFadeOutComplete(FName ViewName, bool bDestroy)
{
	FUIViewInfo* ViewInfo = UIViews.Find(ViewName);
	if (!ViewInfo)
	{
		return;
	}

	ViewInfo->bIsTransitioning = false;
	ViewInfo->bIsShowing = false;

	// 从视口移除
	if (ViewInfo->Widget)
	{
		ViewInfo->Widget->RemoveFromParent();
	}

	// 恢复输入模式
	ApplyInputMode(ViewInfo, false);

	// 如果是独占模式，从栈中移除
	if (ViewInfo->Config.ShowMode == EUIShowMode::Exclusive)
	{
		ExclusiveUIStack.Remove(ViewName);

		// 恢复之前隐藏的UI
		if (ExclusiveUIStack.Num() > 0)
		{
			// 恢复最近的独占UI
			FName TopExclusiveName = ExclusiveUIStack.Last();
			FUIViewInfo* TopView = UIViews.Find(TopExclusiveName);
			if (TopView && TopView->Widget)
			{
				TopView->Widget->SetVisibility(ESlateVisibility::Visible);
			}
		}
		else
		{
			// 没有独占UI了，恢复所有正常UI
			for (auto& Pair : UIViews)
			{
				FUIViewInfo& OtherView = Pair.Value;
				if (OtherView.bIsShowing && OtherView.Widget)
				{
					OtherView.Widget->SetVisibility(ESlateVisibility::Visible);
				}
			}
		}
	}

	// 生命周期回调
	if (ViewInfo->Controller)
	{
		ViewInfo->Controller->OnViewDidDisappear();
	}

	// 销毁UI
	if (bDestroy)
	{
		if (ViewInfo->Controller)
		{
			ViewInfo->Controller->OnViewDestroyed();
		}

		if (ViewInfo->Widget)
		{
			ViewInfo->Widget->ConditionalBeginDestroy();
		}

		UIViews.Remove(ViewName);
		FadeInCallbacks.Remove(ViewName);
		FadeOutCallbacks.Remove(ViewName);
	}
}

void UUIManager::ApplyInputMode(FUIViewInfo* ViewInfo, bool bShow)
{
	APlayerController* PC = GetPlayerController();
	if (!PC || !ViewInfo)
	{
		return;
	}

	if (bShow)
	{
		// 设置输入模式
		if (ViewInfo->Config.bShowMouseCursor)
		{
			FInputModeUIOnly InputMode;
			if (auto swidget = ViewInfo->Widget->TakeWidget(); swidget->SupportsKeyboardFocus())
				InputMode.SetWidgetToFocus(swidget);
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
		}

		// 暂停游戏
		if (ViewInfo->Config.bPauseGame)
		{
			UGameplayStatics::SetGamePaused(GetWorld(), true);
		}
	}
	else
	{
		// 恢复输入模式（如果没有其他UI显示）
		bool bHasOtherUI = false;
		for (const auto& Pair : UIViews)
		{
			if (Pair.Value.bIsShowing && Pair.Key != ViewInfo->ViewName)
			{
				bHasOtherUI = true;
				break;
			}
		}

		if (!bHasOtherUI)
		{
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = false;

			// 恢复游戏
			if (ViewInfo->Config.bPauseGame)
			{
				UGameplayStatics::SetGamePaused(GetWorld(), false);
			}
		}
	}
}

APlayerController* UUIManager::GetPlayerController() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	return World->GetFirstPlayerController();
}
