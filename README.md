# TireflyActorPool 插件完全精通手册

## 概述

TireflyActorPool 是一个高性能的Actor对象池系统，专为游戏开发中频繁生成/销毁Actor的场景优化。通过复用Actor实例，大幅提升性能，特别适用于弹幕游戏、粒子效果、敌人生成等场景。

## 核心特性

- **高性能**: 避免频繁的内存分配和垃圾回收
- **线程安全**: 使用Critical Section确保多线程安全
- **双重池机制**: 支持按类型和按ID的两种对象池分类
- **生命周期管理**: 自动管理Actor生命周期和回收
- **预热机制**: 支持对象池预热
- **蓝图友好**: 完整的蓝图节点支持

## 核心组件

1. **UTireflyActorPoolWorldSubsystem**: 对象池管理器
2. **ITireflyPoolingActorInterface**: Actor池接口
3. **UTireflyActorPoolLibrary**: 蓝图函数库
4. **FTireflyActorPool**: 对象池结构体

## 基本使用

### 1. 实现池化接口

```cpp
UCLASS()
class AMyPoolableActor : public AActor, public ITireflyPoolingActorInterface
{
    GENERATED_BODY()

public:
    virtual void PoolingBeginPlay_Implementation() override;
    virtual void PoolingEndPlay_Implementation() override;
    virtual void PoolingInitialized_Implementation(const FInstancedStruct& InitialData) override;
    virtual FName PoolingGetActorId_Implementation() const override;
    virtual void PoolingSetActorId_Implementation(FName NewActorId) override;

private:
    UPROPERTY()
    FName ActorId = NAME_None;
};
```

### 2. 从对象池生成Actor

```cpp
// C++生成
AActor* SpawnedActor = UTireflyActorPoolLibrary::SpawnActorFromPool(
    this,                           // 世界上下文
    AMyPoolableActor::StaticClass(), // Actor类型
    TEXT("Bullet"),                 // Actor ID
    SpawnTransform,                 // 生成位置
    5.0f                           // 生存时间（秒）
);

// 带初始化数据的生成
FMyActorData InitData;
FInstancedStruct InitialData = FInstancedStruct::Make(InitData);

AActor* ActorWithData = UTireflyActorPoolLibrary::SpawnActorFromPoolWithParams(
    this, AMyPoolableActor::StaticClass(), TEXT("Enemy"),
    SpawnTransform, InitialData, -1.0f
);
```

### 3. 回收Actor

```cpp
// 手动回收
UTireflyActorPoolLibrary::RecycleActorToPool(this, ActorToRecycle);

// 自动回收（通过生存时间）
UTireflyActorPoolLibrary::SpawnActorFromPool(
    this, ActorClass, TEXT("AutoRecycle"), Transform, 3.0f
);
```

### 4. 对象池预热

```cpp
// 预热对象池
UTireflyActorPoolLibrary::WarmUpActorPool(
    this,
    ABulletActor::StaticClass(),
    TEXT("PlayerBullet"),
    50  // 预热50个实例
);
```

## 接口实现示例

```cpp
void AMyPoolableActor::PoolingBeginPlay_Implementation()
{
    // 使用通用激活逻辑
    UTireflyActorPoolLibrary::GenericBeginPlay_Actor(this, this);
    
    // 自定义初始化
    Health = MaxHealth;
    SetActorTickEnabled(true);
}

void AMyPoolableActor::PoolingEndPlay_Implementation()
{
    // 使用通用停用逻辑
    UTireflyActorPoolLibrary::GenericEndPlay_Actor(this, this);
    
    // 自定义清理
    StopAllEffects();
}

void AMyPoolableActor::PoolingInitialized_Implementation(const FInstancedStruct& InitialData)
{
    if (InitialData.GetScriptStruct() == FMyActorData::StaticStruct())
    {
        const FMyActorData* Data = InitialData.GetPtr<FMyActorData>();
        Health = Data->InitialHealth;
        SetActorScale3D(FVector(Data->Scale));
    }
}
```

## 实战应用

### 弹幕游戏示例

```cpp
void ABulletHellWeapon::FireBulletPattern()
{
    for (int32 i = 0; i < 20; ++i)
    {
        float Angle = (360.0f / 20) * i;
        FVector Direction = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)),
            FMath::Sin(FMath::DegreesToRadians(Angle)), 0.0f
        );

        FTransform SpawnTransform;
        SpawnTransform.SetLocation(GetActorLocation());
        SpawnTransform.SetRotation(Direction.Rotation().Quaternion());

        UTireflyActorPoolLibrary::SpawnActorFromPool(
            this, BulletClass, TEXT("EnemyBullet"), SpawnTransform, 5.0f
        );
    }
}
```

### 敌人生成系统

```cpp
void AEnemySpawner::SpawnWave()
{
    for (int32 i = 0; i < SpawnPoints.Num(); ++i)
    {
        UTireflyActorPoolLibrary::SpawnActorFromPool(
            this, EnemyClass, TEXT("WaveEnemy"), SpawnPoints[i], -1.0f
        );
    }
}
```

## 性能优化建议

1. **合理预热**: 根据实际需求预热对象池
2. **及时回收**: 确保不再使用的对象及时回收
3. **监控调试**: 使用调试功能监控对象池状态
4. **内存管理**: 在关卡切换时清理不需要的对象池

## 调试功能

```cpp
// 获取对象池状态
TArray<TSubclassOf<AActor>> AllClasses = PoolSubsystem->Debug_GetAllActorPoolClasses();
TArray<FName> AllIds = PoolSubsystem->Debug_GetAllActorPoolIds();
int32 Count = PoolSubsystem->Debug_GetActorNumberOfClassPool(ActorClass);
```

## 总结

TireflyActorPool插件提供了完整的Actor对象池解决方案，通过合理使用可以显著提升游戏性能，特别适用于需要频繁生成/销毁Actor的游戏场景。
