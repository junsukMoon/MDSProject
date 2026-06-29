#include "Profiling/MDSGameplayProfileSubsystem.h"

#include "Debug/MDSDebugStateSubsystem.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogMDSGameplayProfile, Log, All);

static TAutoConsoleVariable<int32> CVarMDSGameplayProfileEnabled(
	TEXT("mds.GameplayProfile.Enabled"),
	0,
	TEXT("Enables a gameplay-window CSV capture after world begin play."));

static TAutoConsoleVariable<int32> CVarMDSGameplayProfileFrames(
	TEXT("mds.GameplayProfile.Frames"),
	600,
	TEXT("Number of gameplay frames to capture. Use MDSGameplayProfileFrames=<N> for early startup override."));

static TAutoConsoleVariable<FString> CVarMDSGameplayProfileName(
	TEXT("mds.GameplayProfile.Name"),
	TEXT("MDS_Gameplay_Profile"),
	TEXT("CSV profile file stem. Use MDSGameplayProfileName=<Name> for early startup override."));

static TAutoConsoleVariable<FString> CVarMDSGameplayProfileTrigger(
	TEXT("mds.GameplayProfile.Trigger"),
	TEXT("Immediate"),
	TEXT("CSV profile trigger mode: Immediate, MovementActive, or ArrivalsComplete."));

static TAutoConsoleVariable<FString> CVarMDSGameplayProfileSubject(
	TEXT("mds.GameplayProfile.Subject"),
	TEXT("None"),
	TEXT("CSV profile subject for phase triggers: Mass or Actor."));

static TAutoConsoleVariable<int32> CVarMDSGameplayProfileExpectedCount(
	TEXT("mds.GameplayProfile.ExpectedCount"),
	0,
	TEXT("Expected subject count for phase-triggered CSV profiles."));

static TAutoConsoleVariable<int32> CVarMDSGameplayProfileStableFrames(
	TEXT("mds.GameplayProfile.StableFrames"),
	3,
	TEXT("Consecutive frames the trigger condition must hold before CSV capture starts."));

static TAutoConsoleVariable<int32> CVarMDSGameplayProfileTriggerTimeoutFrames(
	TEXT("mds.GameplayProfile.TriggerTimeoutFrames"),
	1800,
	TEXT("Frames to wait for a phase trigger before failing without starting CSV capture."));

static bool IsConsoleVariableDisabled(const TCHAR* ConsoleVariableName)
{
	const IConsoleVariable* ConsoleVariable = IConsoleManager::Get().FindConsoleVariable(ConsoleVariableName);
	return ConsoleVariable && ConsoleVariable->GetInt() == 0;
}

bool UMDSGameplayProfileSubsystem::IsGameplayProfileEnabled()
{
	return CVarMDSGameplayProfileEnabled.GetValueOnGameThread() != 0 || FParse::Param(FCommandLine::Get(), TEXT("MDSGameplayProfile"));
}

int32 UMDSGameplayProfileSubsystem::GetCaptureFrames()
{
	int32 CaptureFrames = CVarMDSGameplayProfileFrames.GetValueOnGameThread();
	FParse::Value(FCommandLine::Get(), TEXT("MDSGameplayProfileFrames="), CaptureFrames);
	return FMath::Max(1, CaptureFrames);
}

int32 UMDSGameplayProfileSubsystem::GetStableFrames()
{
	int32 StableFrames = CVarMDSGameplayProfileStableFrames.GetValueOnGameThread();
	FParse::Value(FCommandLine::Get(), TEXT("MDSGameplayProfileStableFrames="), StableFrames);
	return FMath::Max(1, StableFrames);
}

int32 UMDSGameplayProfileSubsystem::GetExpectedCount()
{
	int32 InExpectedCount = CVarMDSGameplayProfileExpectedCount.GetValueOnGameThread();
	FParse::Value(FCommandLine::Get(), TEXT("MDSGameplayProfileExpectedCount="), InExpectedCount);
	return FMath::Max(0, InExpectedCount);
}

int32 UMDSGameplayProfileSubsystem::GetTriggerTimeoutFrames()
{
	int32 TimeoutFrames = CVarMDSGameplayProfileTriggerTimeoutFrames.GetValueOnGameThread();
	FParse::Value(FCommandLine::Get(), TEXT("MDSGameplayProfileTriggerTimeoutFrames="), TimeoutFrames);
	return FMath::Max(1, TimeoutFrames);
}

FString UMDSGameplayProfileSubsystem::GetCaptureName()
{
	FString InCaptureName = CVarMDSGameplayProfileName.GetValueOnGameThread();
	FParse::Value(FCommandLine::Get(), TEXT("MDSGameplayProfileName="), InCaptureName);
	return SanitizeCaptureName(InCaptureName);
}

UMDSGameplayProfileSubsystem::EProfileTriggerMode UMDSGameplayProfileSubsystem::GetTriggerMode()
{
	FString TriggerName = CVarMDSGameplayProfileTrigger.GetValueOnGameThread();
	FParse::Value(FCommandLine::Get(), TEXT("MDSGameplayProfileTrigger="), TriggerName);

	if (TriggerName.Equals(TEXT("Immediate"), ESearchCase::IgnoreCase))
	{
		return EProfileTriggerMode::Immediate;
	}
	if (TriggerName.Equals(TEXT("MovementActive"), ESearchCase::IgnoreCase))
	{
		return EProfileTriggerMode::MovementActive;
	}
	if (TriggerName.Equals(TEXT("ArrivalsComplete"), ESearchCase::IgnoreCase))
	{
		return EProfileTriggerMode::ArrivalsComplete;
	}

	return EProfileTriggerMode::Invalid;
}

UMDSGameplayProfileSubsystem::EProfileSubject UMDSGameplayProfileSubsystem::GetProfileSubject()
{
	FString SubjectName = CVarMDSGameplayProfileSubject.GetValueOnGameThread();
	FParse::Value(FCommandLine::Get(), TEXT("MDSGameplayProfileSubject="), SubjectName);

	if (SubjectName.Equals(TEXT("None"), ESearchCase::IgnoreCase))
	{
		return EProfileSubject::None;
	}
	if (SubjectName.Equals(TEXT("Mass"), ESearchCase::IgnoreCase))
	{
		return EProfileSubject::Mass;
	}
	if (SubjectName.Equals(TEXT("Actor"), ESearchCase::IgnoreCase))
	{
		return EProfileSubject::Actor;
	}

	return EProfileSubject::Invalid;
}

FString UMDSGameplayProfileSubsystem::TriggerModeToString(const EProfileTriggerMode InTriggerMode)
{
	switch (InTriggerMode)
	{
	case EProfileTriggerMode::Immediate:
		return TEXT("Immediate");
	case EProfileTriggerMode::MovementActive:
		return TEXT("MovementActive");
	case EProfileTriggerMode::ArrivalsComplete:
		return TEXT("ArrivalsComplete");
	default:
		return TEXT("Invalid");
	}
}

FString UMDSGameplayProfileSubsystem::SubjectToString(const EProfileSubject InSubject)
{
	switch (InSubject)
	{
	case EProfileSubject::None:
		return TEXT("None");
	case EProfileSubject::Mass:
		return TEXT("Mass");
	case EProfileSubject::Actor:
		return TEXT("Actor");
	default:
		return TEXT("Invalid");
	}
}

FString UMDSGameplayProfileSubsystem::SanitizeCaptureName(const FString& InCaptureName)
{
	FString SanitizedName;
	SanitizedName.Reserve(InCaptureName.Len());

	for (const TCHAR Character : InCaptureName)
	{
		if (FChar::IsAlnum(Character) || Character == TEXT('_') || Character == TEXT('-'))
		{
			SanitizedName.AppendChar(Character);
		}
	}

	return SanitizedName.IsEmpty() ? TEXT("MDS_Gameplay_Profile") : SanitizedName;
}

void UMDSGameplayProfileSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (!IsGameplayProfileEnabled())
	{
		return;
	}

	if (InWorld.GetNetMode() == NM_Client)
	{
		UE_LOG(LogMDSGameplayProfile, Log, TEXT("Skipping gameplay CSV profile harness on client world."));
		return;
	}

	CaptureName = GetCaptureName();
	FramesUntilExit = GetCaptureFrames() + ExitSlackFrames;
	TriggerMode = GetTriggerMode();
	ProfileSubject = GetProfileSubject();
	ExpectedCount = GetExpectedCount();
	StableFramesRequired = GetStableFrames();
	TriggerTimeoutFrames = GetTriggerTimeoutFrames();
	StableFramesObserved = 0;

	if (!ValidateConfiguration())
	{
		FailAndQuit(FString::Printf(TEXT("Invalid gameplay profile configuration. Trigger=%s Subject=%s ExpectedCount=%d."), *TriggerModeToString(TriggerMode), *SubjectToString(ProfileSubject), ExpectedCount));
		return;
	}

	UE_LOG(LogMDSGameplayProfile, Log, TEXT("Gameplay CSV profile configured: Name=%s Frames=%d Trigger=%s Subject=%s ExpectedCount=%d StableFrames=%d TimeoutFrames=%d."),
		*CaptureName,
		FMath::Max(1, FramesUntilExit - ExitSlackFrames),
		*TriggerModeToString(TriggerMode),
		*SubjectToString(ProfileSubject),
		ExpectedCount,
		StableFramesRequired,
		TriggerTimeoutFrames);

	if (TriggerMode == EProfileTriggerMode::Immediate)
	{
		FTimerDelegate StartDelegate = FTimerDelegate::CreateUObject(this, &UMDSGameplayProfileSubsystem::StartCapture);
		TriggerTimerHandle = InWorld.GetTimerManager().SetTimerForNextTick(StartDelegate);
	}
	else
	{
		FTimerDelegate TriggerDelegate = FTimerDelegate::CreateUObject(this, &UMDSGameplayProfileSubsystem::TickUntilTrigger);
		TriggerTimerHandle = InWorld.GetTimerManager().SetTimerForNextTick(TriggerDelegate);
	}
}

void UMDSGameplayProfileSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TriggerTimerHandle);
		World->GetTimerManager().ClearTimer(ExitTimerHandle);
	}

	Super::Deinitialize();
}

bool UMDSGameplayProfileSubsystem::ValidateConfiguration() const
{
	if (TriggerMode == EProfileTriggerMode::Invalid || ProfileSubject == EProfileSubject::Invalid)
	{
		return false;
	}

	if (TriggerMode == EProfileTriggerMode::Immediate)
	{
		return true;
	}

	if (ProfileSubject == EProfileSubject::None || ExpectedCount <= 0)
	{
		return false;
	}

	if (ProfileSubject == EProfileSubject::Mass && !FParse::Param(FCommandLine::Get(), TEXT("NoMDSMassDebugDraw")) && !IsConsoleVariableDisabled(TEXT("mds.MassDebugDraw.Enabled")))
	{
		UE_LOG(LogMDSGameplayProfile, Error, TEXT("Mass phase profiles require -NoMDSMassDebugDraw or mds.MassDebugDraw.Enabled=0."));
		return false;
	}

	if (ProfileSubject == EProfileSubject::Actor && !FParse::Param(FCommandLine::Get(), TEXT("NoMDSMassBaseline")) && !IsConsoleVariableDisabled(TEXT("mds.MassBaseline.Enabled")))
	{
		UE_LOG(LogMDSGameplayProfile, Error, TEXT("Actor phase profiles require -NoMDSMassBaseline or mds.MassBaseline.Enabled=0."));
		return false;
	}

	return true;
}

bool UMDSGameplayProfileSubsystem::IsTriggerConditionMet() const
{
	const UWorld* World = GetWorld();
	const UMDSDebugStateSubsystem* DebugState = World ? World->GetSubsystem<UMDSDebugStateSubsystem>() : nullptr;
	if (!DebugState)
	{
		return false;
	}

	const FMDSDebugStateSnapshot Snapshot = DebugState->GetSnapshot();
	if (ProfileSubject == EProfileSubject::Mass)
	{
		if (TriggerMode == EProfileTriggerMode::MovementActive)
		{
			return Snapshot.MassSpawnedCount >= ExpectedCount && Snapshot.MassMovedCount > 0;
		}
		if (TriggerMode == EProfileTriggerMode::ArrivalsComplete)
		{
			return Snapshot.MassArrivedCount >= ExpectedCount;
		}
	}
	else if (ProfileSubject == EProfileSubject::Actor)
	{
		if (TriggerMode == EProfileTriggerMode::MovementActive)
		{
			return Snapshot.ActorSpawnedCount >= ExpectedCount && Snapshot.ActorActiveCount > 0;
		}
		if (TriggerMode == EProfileTriggerMode::ArrivalsComplete)
		{
			return Snapshot.ActorArrivedCount >= ExpectedCount;
		}
	}

	return false;
}

void UMDSGameplayProfileSubsystem::TickUntilTrigger()
{
	UWorld* World = GetWorld();
	if (!World || !GEngine)
	{
		return;
	}

	if (--TriggerTimeoutFrames <= 0)
	{
		FailAndQuit(FString::Printf(TEXT("Gameplay profile trigger timed out. Trigger=%s Subject=%s ExpectedCount=%d."), *TriggerModeToString(TriggerMode), *SubjectToString(ProfileSubject), ExpectedCount));
		return;
	}

	if (IsTriggerConditionMet())
	{
		++StableFramesObserved;
	}
	else
	{
		StableFramesObserved = 0;
	}

	if (StableFramesObserved >= StableFramesRequired)
	{
		UE_LOG(LogMDSGameplayProfile, Log, TEXT("Gameplay profile trigger condition met after %d stable frames: Trigger=%s Subject=%s ExpectedCount=%d."),
			StableFramesObserved,
			*TriggerModeToString(TriggerMode),
			*SubjectToString(ProfileSubject),
			ExpectedCount);
		StartCapture();
		return;
	}

	FTimerDelegate TickDelegate = FTimerDelegate::CreateUObject(this, &UMDSGameplayProfileSubsystem::TickUntilTrigger);
	TriggerTimerHandle = World->GetTimerManager().SetTimerForNextTick(TickDelegate);
}

void UMDSGameplayProfileSubsystem::StartCapture()
{
	if (bCaptureStarted)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || !GEngine)
	{
		return;
	}

	bCaptureStarted = true;
	const int32 CaptureFrames = FMath::Max(1, FramesUntilExit - ExitSlackFrames);
	const FString CsvStartFileCommand = FString::Printf(TEXT("CsvProfile STARTFILE=%s"), *CaptureName);
	const FString CsvFramesCommand = FString::Printf(TEXT("CsvProfile FRAMES=%d"), CaptureFrames);
	UE_LOG(LogMDSGameplayProfile, Log, TEXT("Starting gameplay CSV profile: %s, %s. Fallback exit in %d frames."), *CsvStartFileCommand, *CsvFramesCommand, FramesUntilExit);
	IConsoleManager::Get().ProcessUserConsoleInput(*CsvStartFileCommand, *GLog, World);
	IConsoleManager::Get().ProcessUserConsoleInput(TEXT("CsvProfile EXITONCOMPLETION"), *GLog, World);
	IConsoleManager::Get().ProcessUserConsoleInput(*CsvFramesCommand, *GLog, World);

	FTimerDelegate TickDelegate = FTimerDelegate::CreateUObject(this, &UMDSGameplayProfileSubsystem::TickUntilExit);
	ExitTimerHandle = World->GetTimerManager().SetTimerForNextTick(TickDelegate);
}

void UMDSGameplayProfileSubsystem::TickUntilExit()
{
	UWorld* World = GetWorld();
	if (!World || !GEngine)
	{
		return;
	}

	--FramesUntilExit;
	if (FramesUntilExit <= 0)
	{
		UE_LOG(LogMDSGameplayProfile, Log, TEXT("Gameplay CSV profile window elapsed. Requesting quit."));
		GEngine->Exec(World, TEXT("quit"));
		return;
	}

	FTimerDelegate TickDelegate = FTimerDelegate::CreateUObject(this, &UMDSGameplayProfileSubsystem::TickUntilExit);
	ExitTimerHandle = World->GetTimerManager().SetTimerForNextTick(TickDelegate);
}

void UMDSGameplayProfileSubsystem::FailAndQuit(const FString& Reason)
{
	UE_LOG(LogMDSGameplayProfile, Error, TEXT("%s"), *Reason);

	if (UWorld* World = GetWorld())
	{
		if (GEngine)
		{
			GEngine->Exec(World, TEXT("quit"));
		}
	}
}
