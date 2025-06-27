#pragma once
#include "MassProcessor.h"
#include "TestGroupProcessor.generated.h"


UCLASS()
class UTestGroupProcessor : public UMassProcessor
{
    GENERATED_BODY()
public:
    UTestGroupProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
};
