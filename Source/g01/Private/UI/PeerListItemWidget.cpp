#include "PeerListItemWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UPeerListItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (btn_connect)
	{
		btn_connect->OnClicked.AddDynamic(this, &UPeerListItemWidget::HandleConnectClicked);
	}
}

void UPeerListItemWidget::SetPeerInfo(const FPeerInfo& InPeerInfo)
{
	PeerInfo = InPeerInfo;

	if (tb_peer_name)
	{
		tb_peer_name->SetText(FText::FromString(InPeerInfo.Name));
	}
	if (tb_peer_id)
	{
		FString PeerIdStr = FString::FromInt(InPeerInfo.PeerId);
		tb_peer_id->SetText(FText::FromString(PeerIdStr));
	}
}

void UPeerListItemWidget::HandleConnectClicked()
{
	OnConnectClicked.Broadcast(PeerInfo.PeerId);
}

