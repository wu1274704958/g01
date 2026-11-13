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

`Peer->OnConnected` 表示 P2P 传输已通过认证，SDK 会在回调前消费 session handle；等 `Datagram->OnConnected` 后才能发送数据。连接失败后可重试，或调用 `Result.ReleaseSession()` 主动清理；同步启动失败或放弃连接时也可清理。同一 session 同时只允许一个连接尝试，进行中的尝试需先 `Close()` 并等待 `OnFailed`，再清理。`FP2PHelperResult` 的副本共享同一个 handle，不会自动释放。大厅关闭前必须开始连接；已认证连接可继续使用 socket。主动结束时调用 `Peer->Close()` 并保留对象至关闭事件。当前每个 Peer 封装一个 Datagram stream，支持 IPv4 直连。

SDK 更新：先构建 gsync 的对应平台和配置，再在 Source/MqasNet 目录执行 python copy_file.py <gsync 根目录>；头文件、导入库和运行时 DLL 必须来自同一版本。output 为本机产物，不纳入版本管理。
