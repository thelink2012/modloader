//----------------------------------------------------------
//
//   SA Limit Adjuster
//   Copyright 2008 Sacky
//
//----------------------------------------------------------
#ifndef SACKY_HPP
#define	SACKY_HPP

/*
#define memadd(num,arr) dwAlloc+=num;cBytes=(char*)&dwAlloc;for(int i=0;i<4;i++)arr[i]=cBytes[i]
#define patch(a,b) _patch(a,b,sizeof(b))
*/

extern class SALimitAdjuster* SALA;
class SALimitAdjuster
{
    enum SAVERSION
    {
        VERSION_US_1,
        VERSION_EU_1
    };
    
    struct SALACfg
    {
        const char* cStaticLimits;
        const char* cDynamicLimits;
        const char* cCredits;
        const char* cMisc;
        const char* cColours;
        const char* cModels;
        const char* cGraphics;
        const char* cPerformance;
        const char* cAI;

        
    };
    
public:
				SALimitAdjuster		();
				//SALimitAdjuster		(bool bDebug);
				~SALimitAdjuster	();

                // Help functions for modloader
                void FillSALACfg(SALACfg& s);
                void ReadIni(const char* cPath);
                void ReadSalaScript(const char* cPath);
                void ReadConfigFile(const char* cPath, SALACfg& s);
                
	// Dynamic Limits
	void		SetBuildings		(int iBuildings);
	void		SetDummys			(int iDummys);
	void		SetPtrNodeDoubles	(int iPtrNodeDoubles);
	void		SetIPLFiles			(int iIPLFiles);
	void		SetStuntJumps		(int iStuntJumps);
	void		SetColModels		(int iColModels);
	void		SetCollisionFiles	(int iCollisionFiles);
	void		SetQuadTreeNodes	(int iQuadTreeNodes);
	void		SetQuadTreeNodes2	(int iQuadTreeNodes2);
	void		SetMatDataPool		(int iMatDataPool);
	void		SetAtmDataPool		(int iAtmDataPool);
	void		SetVehicles			(int iVehicles);
	void		SetVehicleStructs	(int iVehicleStructs);
	void		SetPeds				(int iPeds);
	void		SetPolygons			(int iPolygons);
	void		SetPtrNodeSingles	(int iPtrNodeSingles);
	void		SetEntryInfoNodes	(int iEntryInfoNodes);
	void		SetObjects			(int iObjects);
	void		SetTasks			(int iTasks);
	void		SetEvents			(int iEvents);
	void		SetPointRoutes		(int iPointRoutes);
	void		SetPatrolRoutes		(int iPatrolRoutes);
	void		SetNodeRoutes		(int iNodeRoutes);
	void		SetTaskAllocators	(int iTaskAllocators);
	void		SetPedAttractors	(int iPedAttractors);
	void		SetEntryExits		(int iEntryExits);
	void		SetStreamingMemory	(int iStreamingMemory);
	void		SetStreamingVehicles(int iStreamingVehicles);
	void		SetImgHeaders		(int iImgHeaders);
	// Static Limits
	void		SetCurrentMission	(int iCurrentMission);
	void		SetTimedObjects		(int iTimedObjects);
	void		SetVehicleModels	(int iVehicleModels);		// Needs Debugging (Car Paintjobs Don't Show)
	void		SetWaterPlanes		(int iWaterPlanes);			// Needs to be invoked at the VERY start
	void		SetPickups			(int iPickups);				// Needs Debugging (Pickups Don't Show)
	void		SetMarkers			(int iMarkers);				// Needs Debugging (Crashes when getting out of Hydra 0x587388)
	void		SetGarages			(int iGarages);				// Needs Debugging (Garages Don't Work)
	void		SetCarCols			(int iCarCols);
	void		SetIDEs				(int iIDEs);
	void		SetWeapons			(int iWeapons);
	void		SetCarMods			(int iCarMods);
	void		SetSCMBlock			(int iSCMBlock);			// Needs Debugging (Crashes Game)
	void		SetSCMLocals		(int iSCMLocals);
	void		SetSCMThreads		(int iSCMThreads);
	void		SetBounds			(float fBounds);
	void		SetAimingOffsets	(int iAimingOffsets);
	void		SetWeaponObjects	(int iWeaponObjects);
	void		SetPlayers			(int iPlayers);				// Players Respawn with Cigar?
	void		SetPedModels		(int iPedModels);
	void		SetIPLs				(int iIPLs);
	void		SetLOD				(float fLod);
	void		SetGangs			(int iGangs);				// Under Development

	/*void		log					(char* cText,...);
	void		retn				(DWORD dwAddress);
	void		nop					(DWORD dwAddress,int iSize);*/

	//SAVERSION	saVersion;

private:
	//void		patchFloat			(DWORD dwAddress,float fData);
	//void		_patch				(DWORD dwAddress,BYTE* bData,int iSize);

	bool		bDebug;
};

#endif	/* SACKY_HPP */

