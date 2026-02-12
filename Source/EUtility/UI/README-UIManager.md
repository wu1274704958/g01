# UI Manager 使用文档

基于MVC架构的UI管理系统，支持独占模式、弹窗模式和淡入淡出动画。

## 架构说明

### 核心组件

1. **UIManager** - UI管理器（GameInstanceSubsystem）
   - 负责UI的创建、显示、隐藏和销毁
   - 管理UI层级和显示模式
   - 处理UI过渡动画

2. **UIViewController** - UI控制器（MVC的Controller层）
   - 管理UI的生命周期
   - 处理UI事件绑定
   - 更新UI显示

3. **UIViewConfig** - UI配置
   - 定义UI的显示模式、层级、动画等参数

## 显示模式

### 1. 独占模式 (Exclusive)
- 显示时会隐藏所有其他非弹窗UI
- 适用于：主菜单、暂停菜单等需要独占屏幕的界面
- 支持独占栈，可以多层独占UI叠加

### 2. 弹窗模式 (Popup)
- 可以叠加在其他UI之上显示
- 不会影响其他UI的显示状态
- 适用于：对话框、确认框、提示框等

### 3. 普通模式 (Normal)
- 正常显示，不影响其他UI
- 适用于：游戏HUD、常驻UI等

## 使用示例

### C++ 使用方式

```cpp
// 1. 获取UIManager实例
UUIManager* UIManager = UUIManager::Get(this);

// 2. 配置UI参数
FUIViewConfig Config;
Config.ShowMode = EUIShowMode::Exclusive;  // 独占模式
Config.ZOrder = 100;                        // UI层级
Config.bPauseGame = true;                   // 是否暂停游戏
Config.bShowMouseCursor = true;             // 是否显示鼠标
Config.TransitionType = EUITransition::Fade; // 过渡动画类型
Config.FadeInDuration = 0.5f;               // 淡入时长
Config.FadeOutDuration = 0.3f;              // 淡出时长

// 3. 显示UI
UIManager->ShowUI(
    FName("MainMenu"),              // UI名称（唯一标识）
    MainMenuWidgetClass,            // Widget类
    MainMenuControllerClass,        // Controller类（可选）
    Config,                         // 配置
    ModelData                       // 模型数据（可选）
);

// 4. 隐藏UI
UIManager->HideUI(FName("MainMenu"), false); // false=不销毁，true=销毁

// 5. 更新UI数据
UIManager->UpdateUIModel(FName("MainMenu"), NewModelData);
```

### 创建自定义Controller

```cpp
UCLASS()
class UMyMenuController : public UUIViewController
{
    GENERATED_BODY()

protected:
    // 绑定UI事件
    virtual void BindUIEvents_Implementation() override
    {
        UIUtility::BindWidget<UButton>(View, TEXT("btn_start"), [this](UButton* Btn)
        {
            FScriptDelegate Delegate;
            Delegate.BindUFunction(this, "OnStartClicked");
            Btn->OnClicked.Add(Delegate);
        });
    }

    // 更新UI显示
    virtual void UpdateView_Implementation() override
    {
        if (Model)
        {
            // 根据Model数据更新UI
        }
    }

    // 自定义淡入动画
    virtual void OnFadeIn_Implementation(float Duration) override
    {
        // 播放UMG动画或自定义动画
    }

private:
    UFUNCTION()
    void OnStartClicked()
    {
        // 处理按钮点击
    }
};
```

### 使用Lambda回调

```cpp
// 设置自定义淡入淡出回调
UIManager->SetFadeInCallback(FName("MainMenu"), [this]()
{
    UE_LOG(LogTemp, Log, TEXT("Fade in started"));
    // 播放音效、触发事件等
});

UIManager->SetFadeOutCallback(FName("MainMenu"), [this]()
{
    UE_LOG(LogTemp, Log, TEXT("Fade out started"));
});
```

## 生命周期回调

Controller提供了完整的生命周期回调：

```cpp
// UI创建时
virtual void OnViewCreated(UUserWidget* InView);

// 即将显示
virtual void OnViewWillAppear();

// 显示完成（淡入动画后）
virtual void OnViewDidAppear();

// 即将隐藏
virtual void OnViewWillDisappear();

// 隐藏完成（淡出动画后）
virtual void OnViewDidDisappear();

// UI销毁时
virtual void OnViewDestroyed();

// 淡入动画回调
virtual void OnFadeIn_Implementation(float Duration);

// 淡出动画回调
virtual void OnFadeOut_Implementation(float Duration);
```

## 扩展动画

当前淡入淡出使用简单的定时器实现。如需更复杂的动画效果，可以：

1. **使用UMG动画播放器**
   ```cpp
   void OnFadeIn_Implementation(float Duration) override
   {
       if (View && FadeInAnimation)
       {
           View->PlayAnimation(FadeInAnimation);
       }
   }
   ```

2. **自定义插值动画**
   ```cpp
   // 在UIManager中添加插值逻辑
   // 使用FMath::InterpEaseInOut等函数实现平滑过渡
   ```

3. **使用动画曲线**
   ```cpp
   // 使用UCurveFloat定义动画曲线
   // 实现更复杂的缓动效果
   ```

## 注意事项

1. UIManager是GameInstanceSubsystem，整个游戏实例共享一个
2. 每个UI通过FName唯一标识，重复显示同名UI会复用
3. 独占模式会自动管理UI显示状态，无需手动隐藏其他UI
4. Controller是可选的，简单UI可以不使用Controller
5. 淡入淡出回调在动画开始时执行，而非结束时

## 完整示例

参考文件：
- `UIManagerUsageExample.h/.cpp` - 完整使用示例
- `ExampleMenuController.h/.cpp` - Controller示例

