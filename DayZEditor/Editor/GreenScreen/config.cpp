////////////////////////////////////////////////////////////////////
//DeRap: GreenScreen\config.bin
//Produced from mikero's Dos Tools Dll version 9.10
//https://mikero.bytex.digital/Downloads
//'now' is Thu Jan 18 01:21:05 2024 : 'file' last modified on Wed Jul 12 04:28:01 2023
////////////////////////////////////////////////////////////////////

#define _ARMA_

class CfgPatches
{
	class greenScreen
	{
		units[] = {"GreenScreen_Pink","GreenScreen_Green","GreenScreen_Blue"};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data"};
	};
};
class CfgVehicles
{
	class HouseNoDestruct;
	class GreenScreen_Pink: HouseNoDestruct
	{
		scope = 2;
		displayName = "greenScreen";
		descriptionShort = "Hype train is a hoax";
		model = "\DayZEditor\Editor\GreenScreen\data\greenScreen_Pink.p3d";
	};
	class GreenScreen_Green: HouseNoDestruct
	{
		scope = 2;
		displayName = "greenScreen";
		descriptionShort = "Hype train is a hoax";
		model = "\DayZEditor\Editor\GreenScreen\data\greenScreen_Green.p3d";
	};
	class GreenScreen_Blue: HouseNoDestruct
	{
		scope = 2;
		displayName = "greenScreen";
		descriptionShort = "Hype train is a hoax";
		model = "\DayZEditor\Editor\GreenScreen\data\greenScreen_Blue.p3d";
	};
};
