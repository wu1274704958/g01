// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TestMqasNetWidget.h"

#include <functional>
#include <EUtility/UI/UIUtility.h>

template<typename W>
requires std::is_base_of_v<UWidget,W>
W* bindWidget(UUserWidget* context,const FName& widgetName,std::function<void(W*)> onBind = {})
{
	if(W* Temp = Cast<W>(context->GetWidgetFromName(widgetName));Temp != nullptr)
	{
		if(onBind)
		{
			onBind(Temp);
		}
		return Temp;
	}
	return nullptr;
}

UTestMqasNetWidget::UTestMqasNetWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> DefaultToastClass(TEXT("/Game/UI/Toast/UW_ToastDef"));
	if(DefaultToastClass.Class)
	{
		_ToastClass = DefaultToastClass.Class;
	}
}

void UTestMqasNetWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	_BtnConnect = bindWidget<UButton>(this,TEXT("btn_connect"),[this](UButton* btn)
	{
		FScriptDelegate Delegate;
		Delegate.BindUFunction(this, "OnBtnTestClick");
		btn->OnClicked.Add(Delegate);
	});
	_EtIp = bindWidget<UEditableText>(this,TEXT("et_ip"),[this](UEditableText* et)
	{
		et->SetText(FText::FromString(_DefaultIp));
	});
	_EtPort = bindWidget<UEditableText>(this,TEXT("et_port"),[this](UEditableText* et)
	{
		FNumberFormattingOptions Options;
		Options.SetUseGrouping(false);
		et->SetText(FText::AsNumber(_DefaultPort,&Options));
	});
	_EtName = bindWidget<UEditableText>(this,TEXT("et_name"));
	_EtPsd = bindWidget<UEditableText>(this,TEXT("et_psd"));
	_TbStatus = bindWidget<UTextBlock>(this,TEXT("tb_status"));
	_TbBtnConnect = bindWidget<UTextBlock>(this,TEXT("tb_btn_connect"));
}

void UTestMqasNetWidget::OnBtnTestClick()
{
	if (auto connect =  _CurrentConnect.lock(); connect != nullptr)
	{
		if (connect->IsConnected())
		{
			connect->Disconnect();
		}else
		{
			UUIUtility::ShowToast(this,0,_ToastClass, TEXT("Connecting... Please wait!"));
		}
		return;
	}
	if (_EtName->GetText().IsEmpty() || _EtPsd->GetText().IsEmpty())
	{
		UUIUtility::ShowToast(this,0,_ToastClass, TEXT("Please input name and password!"));
		return;
	}
	if (_EtIp->GetText().IsEmpty() || _EtPort->GetText().IsEmpty())
	{
		UUIUtility::ShowToast(this,0,_ToastClass, TEXT("Please input IP and Port!"));
		return;
	}
	auto agent = GetAUMqasNetAgent();
	if (agent == nullptr)
	{
		UUIUtility::ShowToast(this,0,_ToastClass, TEXT("MqasNetAgent is null!"));
		return;
	}
	_CurrentConnect = agent->ConnectTo(1, TEXT("NetConfig/hole_punching_config.txt"),
		_EtIp->GetText().ToString(), FCString::Atoi(*_EtPort->GetText().ToString()));
	if (auto Connect = _CurrentConnect.lock();Connect)
	{
		auto id = Connect->GetHwnd();
		_TbStatus->SetText(FText::FromString(FString::Printf(TEXT("Connecting... (Handle=%u)"), id)));
		Connect->OnConnectedDelegate.AddUObject(this, &UTestMqasNetWidget::OnConnected);
		Connect->OnConnectErrorDelegate.AddUObject(this, &UTestMqasNetWidget::OnConnectError);
		Connect->OnDisconnectedDelegate.AddUObject(this, &UTestMqasNetWidget::OnDisconnected);
	}else
	{
		UUIUtility::ShowToast(this,0,_ToastClass, TEXT("Connect failed!"));
	}
}

void UTestMqasNetWidget::OnConnectError(int code,GSY_ConnectionHwnd hwnd)
{
	_TbStatus->SetText(FText::FromString(FString::Printf(TEXT("Connect failed!(Error=%u)"), code)));
	_TbBtnConnect->SetText(FText::FromString(TEXT("Connect")));
}
void UTestMqasNetWidget::OnConnected(int code,GSY_ConnectionHwnd hwnd)
{
	_TbStatus->SetText(FText::FromString(FString::Printf(TEXT("Connected (Handle=%u)"), hwnd)));
	_TbBtnConnect->SetText(FText::FromString(TEXT("Disconnect")));
}
void UTestMqasNetWidget::OnDisconnected(int code,GSY_ConnectionHwnd hwnd)
{
	_TbStatus->SetText(FText::FromString(FString::Printf(TEXT("Disconnected (Code=%u)"), code)));
	_CurrentConnect.reset();
	_TbBtnConnect->SetText(FText::FromString(TEXT("Connect")));
}



