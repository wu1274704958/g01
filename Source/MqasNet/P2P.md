# MqasNet P2P C++ 使用示例

在大厅 `ILobbyEventListener::OnHelperResult` 中，保存传入的 `FP2PHelperResult`，并持有下面的两个对象直到连接关闭。所有公开方法和事件在游戏线程使用。

```cpp
Peer = FMqasPeer::Create();
Datagram = FMqasDatagram::Create();
Peer->SetDatagram(Datagram); // 两端均在 Start 前配置，服务端需要及时接收 stream。
Peer->OnConnected.AddLambda([this, Result] {
    if (!Result.Data.is_server)
        Datagram->Open();
});
Datagram->OnConnected.AddLambda([this] {
    TArray<uint8> Message{1, 2, 3};
    Datagram->Send(Message);
});
Datagram->OnReceived.AddLambda([](const TArray<uint8>& Message) { /* 处理数据 */ });
Peer->OnFailed.AddLambda([Result](ErrorCode Error) {
    Result.ReleaseSession(); // 不再重试时主动清理；若要重试可暂时保留。
});
Peer->OnClosed.AddLambda([](ErrorCode Error) { /* 处理关闭 */ });
const ErrorCode ResultCode = Peer->Start(ConfigPath, LocalPeerId, Result);
// EC_Pending 表示请求已接受，其他结果为同步失败。
```

`Peer->OnConnected` 表示 P2P 传输已通过认证，SDK 会在回调前消费 session handle；等 `Datagram->OnConnected` 后才能发送数据。连接失败后可重试，或调用 `Result.ReleaseSession()` 主动清理；同步启动失败或放弃连接时也可清理。同一 session 同时只允许一个连接尝试，进行中的尝试需先 `Close()` 并等待 `OnFailed`，再清理。`FP2PHelperResult` 的副本共享同一个 handle，不会自动释放。大厅关闭前必须开始连接；已认证连接可继续使用 socket。主动结束时调用 `Peer->Close()` 并保留对象至关闭事件。当前支持 IPv4 直连。

当前每个 P2P peer 封装一个 Datagram 控制 stream；Datagram 报文属于底层连接，并非这个 stream 的字节流。出现第二个不同的 stream 时封装会关闭 peer，避免重复分发。`FMqasAgent::RetainPeer` 可以只保留 peer；`FindPeers` 可查找同一远端的多个已保留连接。游戏现有的 `P2PLinkController` 仍按远端 ID 限制一个角色同步链接。

SDK 更新：先构建 gsync 的对应平台和配置，再在 Source/MqasNet 目录执行 python copy_file.py <gsync 根目录>；头文件、导入库和运行时 DLL 必须来自同一版本。output 为本机产物，不纳入版本管理。


## Agent lifecycle and PIE

`FMqasAgent` owns the process-wide SDK runtime. Each `UMqasClientSubsystem` creates a separate logical client for its GameInstance; any `AUMqasNetAgent` in that GameInstance forwards to that client. Destroying an actor or changing levels does not close its connections. Releasing the GameInstance closes only its connections and peers; releasing the last client drains and terminates the SDK. GSY_connect reuses a compatible engine for a fixed bind endpoint; port 0 creates a separate engine and UDP socket. Use a lobby configuration with `port=0` for multiple local clients. Runtime configurations are written under `Saved/MqasClients/<process>-<client>/`.

A new level can call `FMqasAgent::FindPeer` with its subsystem client ID and remote peer ID to recover an existing P2P connection. The g01 link controller exposes `AttachLink` for this purpose. It does not automatically travel or bind preplaced characters.

To discover links after a level change, call `UMqasClientSubsystem::GetPeerIds()` on that level's GameInstance. It returns the remote IDs of this client’s retained peers, including connections still being established; the caller can use an ID with `AP2PLinkController::AttachLink`. Each PIE GameInstance sees only its own client’s peers. Closing a client invalidates its held streams after the SDK has drained; callbacks queued by an older SDK runtime are discarded after a restart.
