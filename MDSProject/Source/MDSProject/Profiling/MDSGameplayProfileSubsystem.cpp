#include "Profiling/MDSGameplayProfileSubsystem.h"

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

FString UMDSGameplayProfileSubsystem::GetCaptureName()
{
	FString InCaptureName = CVarMDSGameplayProfileName.GetValueOnGameThread();
	FParse::Value(FCommandLine::Get(), TEXT("MDSGameplayProfileName="), InCaptureName);
	return SanitizeCaptureName(InCaptureName);
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

	FTimerDelegate StartDelegate = FTimerDelegate::CreateUObject(this, &UMDSGameplayProfileSubsystem::StartCapture);
	InWorld.GetTimerManager().SetTimerForNextTick(StartDelegate);
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
	World->GetTimerManager().SetTimerForNextTick(TickDelegate);
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
	World->GetTimerManager().SetTimerForNextTick(TickDelegate);
}
