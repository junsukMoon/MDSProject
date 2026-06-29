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
	virtual void Deinitialize() override;

private:
	enum class EProfileTriggerMode : uint8
	{
		Immediate,
		MovementActive,
		ArrivalsComplete,
		Invalid
	};

	enum class EProfileSubject : uint8
	{
		None,
		Mass,
		Actor,
		Invalid
	};

	static constexpr int32 DefaultCaptureFrames = 600;
	static constexpr int32 ExitSlackFrames = 120;
	static constexpr int32 DefaultStableFrames = 3;
	static constexpr int32 DefaultTriggerTimeoutFrames = 1800;

	static bool IsGameplayProfileEnabled();
	static int32 GetCaptureFrames();
	static int32 GetStableFrames();
	static int32 GetExpectedCount();
	static int32 GetTriggerTimeoutFrames();
	static FString GetCaptureName();
	static FString SanitizeCaptureName(const FString& InCaptureName);
	static EProfileTriggerMode GetTriggerMode();
	static EProfileSubject GetProfileSubject();
	static FString TriggerModeToString(EProfileTriggerMode InTriggerMode);
	static FString SubjectToString(EProfileSubject InSubject);

	bool ValidateConfiguration() const;
	bool IsTriggerConditionMet() const;
	void TickUntilTrigger();
	void StartCapture();
	void TickUntilExit();
	void FailAndQuit(const FString& Reason);

	FTimerHandle TriggerTimerHandle;
	FTimerHandle ExitTimerHandle;
	bool bCaptureStarted = false;
	int32 FramesUntilExit = 0;
	int32 TriggerTimeoutFrames = 0;
	int32 StableFramesRequired = 0;
	int32 StableFramesObserved = 0;
	int32 ExpectedCount = 0;
	EProfileTriggerMode TriggerMode = EProfileTriggerMode::Immediate;
	EProfileSubject ProfileSubject = EProfileSubject::None;
	FString CaptureName;
};
