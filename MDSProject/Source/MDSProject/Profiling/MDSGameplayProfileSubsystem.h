#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MDSGameplayProfileSubsystem.generated.h"

UCLASS()
class MDSPROJECT_API UMDSGameplayProfileSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	static constexpr int32 DefaultCaptureFrames = 600;
	static constexpr int32 ExitSlackFrames = 120;

	static bool IsGameplayProfileEnabled();
	static int32 GetCaptureFrames();
	static FString GetCaptureName();
	static FString SanitizeCaptureName(const FString& InCaptureName);

	void StartCapture();
	void TickUntilExit();

	bool bCaptureStarted = false;
	int32 FramesUntilExit = 0;
	FString CaptureName;
};
