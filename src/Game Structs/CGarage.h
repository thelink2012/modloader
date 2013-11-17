#pragma once
#include "Injector.h"
#include "RwV3D.h"
#include "RwV2D.h"
#include "CVehicle.h"
#include "CObject.h"
#include "CPed.h"


enum eGarageDoorState
{
    GARAGE_DOOR_CLOSED = 0x0,
    GARAGE_DOOR_OPEN = 0x1,
    GARAGE_DOOR_CLOSING = 0x2,
    GARAGE_DOOR_OPENING = 0x3,
    GARAGE_DOOR_WAITING_PLAYER_TO_EXIT = 0x4,
    GARAGE_DOOR_CLOSED_DROPPED_CAR = 0x5,    // not sure
};

enum eGarageFlags
{
	FLAG_GARAGE_NOT_UNDER_SCRIPT_CONTROL = 0x1, // not sure
    FLAG_GARAGE_INACTIVE = 0x2,
    FLAG_GARAGE_USED_RESPRAY = 0x4,
    FLAG_GARAGE_DOOR_OPENS_UP_AND_ROTATE = 0x8,
    FLAG_GARAGE_DOOR_GOES_IN = 0x10,
    FLAG_GARAGE_CAMERA_FOLLOWS_PLAYER = 0x20,
    FLAG_GARAGE_DOOR_COLLIDABLE = 0x40,
    FLAG_GARAGE_RESPRAY_ALWAYS_FREE = 0x80,
};


#pragma pack(push, 1)
struct CGarageData	// sizeof=0x50
{
  RwV3D Position;
  RwV2D DirectionA;
  RwV2D DirectionB;
  float TopZ;
  float Width;
  float Depth;
  float Left;
  float Right;
  float Front;
  float Back;
  float DoorPosition;
  unsigned int TimeToOpen;
  CVehicle *pTargetCar;
  char Name[8];
  unsigned char Type;
  char DoorState;
  char Flags;
  char OriginalType;
};

struct CGarage : public CGarageData	// sizeof=0xD8
{
  int _unknown_audio_object[34];
};
#pragma pack(pop)

static_assert(sizeof(CGarageData) == 0x50, "Incorrect struct size: CGarageData");
static_assert(sizeof(CGarage) == 0xD8, "Incorrect struct size: CGarage");






#pragma pack(push,1)
struct CStoredCar
{
  RwV3D pos;
  int handling_flags;
  char flags;
  char field_11;    // pad
  short model;
  short carmods[15];
  char colour[4];
  char radio_station;
  char extra1;
  char extra2;
  char bomb_type;
  char paintjob;
  char nitro_count;
  char angleX;        // multiply by 0.009999999776482582 to get real value
  char angleY;
  char angleZ;
  char field_3F;    // pad
};
#pragma pack(pop)

static_assert(sizeof(CStoredCar) == 0x40, "Incorrect struct size: CStoredCar");



DeclareFunc(0x4470E0, void (__fastcall *CGarage__constructor)(CGarage *self));
DeclareFunc(0x447110, void (__fastcall *CGarage__destructor)(CGarage *self));
DeclareFunc(0x447120, void (__cdecl *CGarages__Init)());
DeclareFunc(0x4471B0, void (__cdecl *CGarages__Shutdown)());
DeclareFunc(0x4471E0, int (__cdecl *CGarages__AddOne)(float x1, float y1, float z1, float frontX, float frontY, float x2, float y2, float z2, char type, int a10, char *name, char flags));
DeclareFunc(0x447600, void (__fastcall *CGarage__RestoreThisGarage)(CGarage *self));
DeclareFunc(0x447680, short (__cdecl *CGarages__FindIndexByName)(const char *name));
DeclareFunc(0x4476D0, CGarage *(__cdecl *CGarages__ChangeGarageType)(short index, unsigned __int8 type, int unused));
DeclareFunc(0x447720, char (__fastcall *CGarage__IsVehicleAcceptByThisTunningGarage)(CGarage *self, int dummy, CVehicle *a2));
DeclareFunc(0x447790, void (__cdecl *CGarages__PrintMessages)());
DeclareFunc(0x4479A0, char (__cdecl *CGarages__IsVehicleAcceptedByResprayGarage)(CVehicle *a1));
DeclareFunc(0x4479F0, long double (__cdecl *CGarages__BuildRotatedDoorMatrix)(CObject *a1, float a2));
DeclareFunc(0x447B80, unsigned __int8 (__cdecl *CGarages__TriggerMessage)(char *gxt_entry, short number1, unsigned short duration, short number2));
DeclareFunc(0x447C40, void (__cdecl *CGarages__SetTargetCarForMissionGarage)(short index, CVehicle *a2));
DeclareFunc(0x447C90, char (__cdecl *CGarages__HasCarBeenDroppedOffYet)(short index));
DeclareFunc(0x447CB0, CGarage *(__cdecl *CGarages__DeActivateGarage)(short index));
DeclareFunc(0x447CD0, CGarage *(__cdecl *CGarages__ActivateGarage)(short index));
DeclareFunc(0x447D00, char (__cdecl *CGarages__IsGarageOpen)(short index));
DeclareFunc(0x447D30, char (__cdecl *CGarages__IsGarageClosed)(short index));
DeclareFunc(0x447D50, void (__fastcall *CGarage__OpenThisGarage)(CGarage *self));
DeclareFunc(0x447D70, void (__fastcall *CGarage__CloseThisGarage)(CGarage *self));
DeclareFunc(0x447D80, long double (__fastcall *CGarage__GetDistanceFromPoint)(CGarage *self, int dummy, float x, float y));
DeclareFunc(0x447E10, char (__cdecl *CGarages__HasResprayHappened)(short index));
DeclareFunc(0x447E40, CVehicle *(__fastcall *CStoredCar__RestoreCar)(CStoredCar *a1));
DeclareFunc(0x448330, void (__fastcall *CGarage__MakeCarCoorsForImpoundLot)(CGarage *self, int dummy, CStoredCar *a2));
DeclareFunc(0x448550, char (__fastcall *CGarage__RestoreCarsForThisGarage)(CGarage *self, int dummy, CStoredCar *a1));
DeclareFunc(0x4485C0, char (__fastcall *CGarage__RestoreCarsForThisImpoundLot)(CGarage *self, int dummy, CStoredCar *a1));
DeclareFunc(0x448650, char (__cdecl *CGarages__CameraShouldBeOutside)());
DeclareFunc(0x448660, void (__cdecl *CGarages__GivePlayerDetonator)());
DeclareFunc(0x4486C0, void (__fastcall *CGarage__SetDoorsBackToOriginalHeight)(CGarage *self));
DeclareFunc(0x448740, char (__fastcall *CGarage__IsPointWithinGarage)(CGarage *self, int dummy, RwV3D vec));
DeclareFunc(0x4487D0, int (__fastcall *CGarage__IsPointWithinGarageWithDistance)(CGarage *self, int dummy, RwV3D vec, float distance));
DeclareFunc(0x448880, int (__fastcall *CGarage__MaximumNumberOfCarsOnThisGarage)(CGarage *self));
DeclareFunc(0x448890, void (__cdecl *CGarages__UnknownRemoveEngineFire)(CVehicle *a1));
DeclareFunc(0x448900, char (__cdecl *CGarages__IsPointWithinHideOutGarage)(RwV3D *a1));
DeclareFunc(0x448990, char (__cdecl *CGarages__IsPointWithinAnyGarage)(RwV3D *a1));
DeclareFunc(0x4489F0, int (__cdecl *CGarages__GetStoreIndex)(int garage_type));
DeclareFunc(0x448AF0, char (__cdecl *CGarages__IsModelIndexADoor)(int model));
DeclareFunc(0x448B30, void (__cdecl *CGarages__OpenSprayGarages)(char bOpen));
DeclareFunc(0x448B60, void (__cdecl *CGarages__RestoreGaragesForNewGame)());
DeclareFunc(0x448BE0, char (__fastcall *CGarage__IsEntityEntirelyInside)(CGarage *self, int dummy, CEntity *a2, float distance));
DeclareFunc(0x448D30, char (__fastcall *CGarage__IsEntityEntirelyOutside)(CGarage *self, int dummy, CEntity *a2, float distance));
DeclareFunc(0x448E50, char (__fastcall *CGarage__IsPlayerEntirelyOutside)(CGarage *self, int dummy, float distance));
DeclareFunc(0x448EA0, char (__fastcall *CGarage__IsPlayerEntirelyInside)(CGarage *self));
DeclareFunc(0x448EE0, char (__fastcall *CGarage__IsEntityTouching3D)(CGarage *self, int dummy, CEntity *a2));
DeclareFunc(0x449050, char (__fastcall *CGarage__IsEntityTouchingOutside)(CGarage *self, int dummy, CEntity *a2, float distance));
DeclareFunc(0x449100, char (__fastcall *CGarage__IsAnyOtherCarTouchingGarage)(CGarage *self, int dummy, CVehicle *except));
DeclareFunc(0x449220, void (__fastcall *CGarage__PushOutVehicles)(CGarage *self, int dummy, CVehicle *except));
DeclareFunc(0x4493E0, char (__fastcall *CGarage__IsAnyOtherPedTouchingGarage)(CGarage *self, int dummy, CPed *except));
DeclareFunc(0x4494F0, char (__fastcall *CGarage__IsAnyCarBlockingDoor)(CGarage *self));
DeclareFunc(0x4495F0, int (__fastcall *CGarage__CountCarsWithCenterPointWithinGarage)(CGarage *self, int dummy, CVehicle *except));
DeclareFunc(0x449690, void (__fastcall *CGarage__RemoveCarsBlockingDoorNotInside)(CGarage *self));
DeclareFunc(0x449740, char (__cdecl *CGarages__IsThisCarWithinGarageArea)(short garage_index, CEntity *a2));
DeclareFunc(0x449760, void (__fastcall *CStoredCar__Store)(CStoredCar *self, int dummy, CVehicle *a2));
DeclareFunc(0x449900, void (__fastcall *CGarage__StoreAndRemoveCarsForThisGarage)(CGarage *self, int dummy, CStoredCar *a2, signed int max_slot));
DeclareFunc(0x449A50, void (__fastcall *CGarage__StoreAndRemoveCarsForThisImpoundLot)(CGarage *self, int dummy, CStoredCar *a2, signed int max_slot));
DeclareFunc(0x449BA0, char (__cdecl *CGarages__IsPointInAGarageCameraZone)(RwV3D point));
DeclareFunc(0x449C30, long double (__cdecl *CGarages__GetGarageDoorMoveMultiplier)(int model));
DeclareFunc(0x449C50, void (__fastcall *CGarage__TidyUpGarageClose)(CGarage *self));
DeclareFunc(0x449D10, void (__fastcall *CGarage__TidyUpGarage)(CGarage *self));
DeclareFunc(0x449E60, void (__cdecl *CGarages__PlayerArrestedOrDied)());
DeclareFunc(0x449E90, char (__fastcall *CGarage__Unknown)(CGarage *self, int dummy, CEntity *a2));
DeclareFunc(0x449FF0, void (__fastcall *CGarage__FindDoorsEntities)(CGarage *self, int dummy, CObject **door1, CObject **door2));
DeclareFunc(0x44A170, void (__cdecl *CGarages__CloseGaragesBeforeSave)());
DeclareFunc(0x44A210, void (__cdecl *CGarages__CountCarsInGarage)(unsigned __int8 garage_type));
DeclareFunc(0x44A240, int (__cdecl *CGarages__FindGarageForThisDoor)(CObject *a1));
DeclareFunc(0x44A3C0, void (__cdecl *CGarages__StoreCarAtImpoundLot)(CVehicle *a1));
DeclareFunc(0x44A4D0, void (__fastcall *CObject__ProcessGarageDoor)(CObject *self));
DeclareFunc(0x44A660, char (__fastcall *CGarage__UpdateOpeningDoor)(CGarage *self));
DeclareFunc(0x44A750, char (__fastcall *CGarage__UpdateClosingDoor)(CGarage *self));
DeclareFunc(0x44A830, char (__fastcall *CGarage__IsStaticPlayerCarEntirelyInside)(CGarage *self));
DeclareFunc(0x44A9C0, char (__fastcall *CGarage__IsAnythingTouchingGarage)(CGarage *self));
DeclareFunc(0x44AA50, void (__fastcall *CGarage__Update)(CGarage *self, int dummy, int my_index));
DeclareFunc(0x44C8C0, void (__cdecl *CGarages__Update)());

DeclareVar(0x96BFD8, CGarage*, CGarages__PlayerInGarage);
DeclareVar(0x96BFDC, DWORD, CGarages__PnSIndex);
DeclareVar(0x96BFE0, DWORD, CGarages__LastTimeHelpMessage);
DeclareVar(0x96BFE4, BYTE, CGarages__bCamShouldBeOutside);
DeclareVar(0x96C008, BYTE, CGarage__ResprayGaragesDisabled);
DeclareVar(0x96C009, BYTE, CGarages__RespraysAreFree);
DeclareVar(0x96C00A, BYTE, CGarages__BombsAreFree);
DeclareVar(0x96C00C, DWORD, CGarages__MessageNumberInString2);
DeclareVar(0x96C010, DWORD, CGarages__MessageNumberInString);
DeclareVar(0x96C014, char*, CGarages__MessageIDString);
DeclareVar(0x96C01C, DWORD, CGarages__MessageEndTime);
DeclareVar(0x96C020, DWORD, CGarages__MessageStartTime);
DeclareVar(0x96C024, DWORD, CGarages__NumGarages);
DeclareArray(0x96ABD8, CStoredCar*, CGarages__aStoredCars);
DeclareArray(0x96C048, CGarage*, CGarages__Garages);

