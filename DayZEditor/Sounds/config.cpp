////////////////////////////////////////////////////////////////////
//DeRap: config.bin
//Produced from mikero's Dos Tools Dll version 9.10
//https://mikero.bytex.digital/Downloads
//'now' is Thu Jan 18 01:21:06 2024 : 'file' last modified on Wed Jul 12 04:28:07 2023
////////////////////////////////////////////////////////////////////

#define _ARMA_

class CfgPatches
{
	class Editor_Sounds
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data"};
	};
};
class CfgSoundShaders
{
	class Editor_Soundshader
	{
		volume = 0.1;
		frequency = 1;
		range = 10;
		rangeCurve[] = {{0,1},{5,0.7},{10,0}};
		limitation = 0;
	};
	class Notification_Soundshader: Editor_Soundshader
	{
		samples[] = {{"DayZEditor\sounds\notification",1}};
	};
	class THX_Soundshader: Editor_Soundshader
	{
		samples[] = {{"DayZEditor\sounds\thx",1}};
	};
};
class CfgSoundSets
{
	class Editor_Soundset
	{
		sound3DProcessingType = "character3DProcessingType";
		volumeCurve = "characterAttenuationCurve";
		spatial = 1;
		doppler = 0;
		loop = 0;
	};
	class Notification_SoundSet: Editor_Soundset
	{
		soundShaders[] = {"Notification_Soundshader"};
	};
	class THX_SoundSet: Editor_Soundset
	{
		soundShaders[] = {"THX_Soundshader"};
	};
};
