// The following snippet assumes you have a Canvas Panel named HUDCanvas at the root of all of your HUD widgets
// The end result of ShowPopupAtCursor() would have your UserWidget rendered at the cursor location, no matter what resolution you are using.

//-----------------------------------//
// Declarations for header file:
UCLASS()
class FLAGS_API UMyHUDCClass : public UUserWidget
{
	GENERATED_BODY()
public:
	UMyHUDCClass(const FObjectInitializer& ObjectInitializer);

	void ShowPopupAtCursor();
	void Shutdown();
	void Setup();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UCanvasPanel* HUDCanvas;
	
	class UMyUserWidgetClass* myPopup;
	class UCanvasPanelSlot* popupPanelSlot;
};
//-----------------------------------//
// CPP File:

#include <Engine/UserInterfaceSettings.h>

FVector2D GetDPIScaledMousePositionForUMG(APlayerController* controller)
{
	FIntPoint viewportDims(0, 0);
	FVector2D mousePos(0.0f, 0.0f);
	controller->GetViewportSize(viewportDims.X, viewportDims.Y);
	controller->GetMousePosition(mousePos.X, mousePos.Y);

	const UUserInterfaceSettings* uiSettings = GetDefault<UUserInterfaceSettings>(UUserInterfaceSettings::StaticClass());
	if (uiSettings)
	{
		float currentDPIScale = uiSettings->GetDPIScaleBasedOnSize(viewportDims);

		// This is assuming your UI is authored at a 1.0 DPI scale
		float dpiMouseScale = 1.0f / currentDPIScale;

		return mousePos * dpiMouseScale;
	}

	return mousePos;
}

static TSubclassOf<UMyUserWidgetClass> s_popupWidgetClass;

UMyHUDCClass::UMyHUDCClass(const FObjectInitializer& ObjectInitializer)
: UUserWidget(ObjectInitializer)
, myPopup(nullptr)
, popupPanelSlot(nullptr)
{
	static ConstructorHelpers::FClassFinder<UMyUserWidgetClass> PopupClass(TEXT("/Game/UI/HUD/WBP_Popup"));
	if (PopupClass.Succeeded())
	{
		s_popupWidgetClass = PopupClass.Class;
	}
}

void UMyHUDCClass::Setup()
{
	// NOTE: myPopup is NOT a UPROPERTY, which means it is liable for Garbage Collection at any time.
	// AddToRoot explicitly prevents the GC from destroying this runtime created object until RemoveFromRoot is called.
	myPopup = CreateWidget<UMyUserWidgetClass>(this, s_popupWidgetClass);
	myPopup->AddToRoot();
	myPopup->SetVisibility(ESlateVisibility::Hidden);
	if (HUDCanvas)
	{
		UPanelSlot* panelSlot = HUDCanvas->AddChild(myPopup);
		popupPanelSlot = Cast<UCanvasPanelSlot>(panelSlot);
	}
}

void UMyHUDCClass::Shutdown()
{
	// NOTE: After calling RemoveFromRoot, myPopup is fair game for Garbage Collection once this function exits
	myPopup->RemoveFromRoot();
	myPopup->RemoveFromParent();
	myPopup = nullptr;
	popupPanelSlot = nullptr;
}

void UMyHUDCClass::ShowPopupAtCursor()
{
	myPopup->SetVisibility(ESlateVisibility::Visible);

	APlayerController* controller = GetOwningPlayer();
	FVector2D popupSize = myPopup->GetDesiredSize();

	// Get the mouse position scaled by DPI.
	// The popupSize modification causes the widget to be rendered with its lower right corner at the mouse cursor.
	// The magic numbers @ the end are to simply not have the widget directly on the cursor.
	FVector2d scaledMouse = GetDPIScaledMousePositionForUMG(controller) - popupSize - FVector2D(4.0f, 4.0f);

	if (popupPanelSlot)
	{
		popupPanelSlot->SetPosition(scaledMouse);
	}
}

//-----------------------------------//
