# FoveHMD Plugin #

This is the official [FOVE](https://www.getfove.com/) plugin for Unreal Engine. It adds support for fetching data from the FOVE system, such as HMD pose and eye tracking data, as well as rendering and submission to the FOVE compositor.

Version information:

1. UE4: 4.12 through 4.25 are supported.
2. FOVE: SDK v0.18 is bundled with this and must be used with runtime v0.18 or later.
3. OS: 64bit Windows and Mac. Only 64 bit platforms are supported.

The FOVE SDK is included within this plugin. It must be used in conjunction with a FOVE runtime installed on the computer of the same or higher version, currently v0.18.

Contributions are welcome!

## Installation ##

This repository should be placed in your Unreal project's plugins directory (i.e. `ProjectRoot\Plugins\FoveUnrealEngine`). The Unreal build system will then pick it up once you rebuild and add it to your project.

Included in this repository is another branch, called SampleApp, which contains a sample project that you can use for testing. Please see the readme in that branch for more info.

## Usage ##

The FOVE service and compositor must be running before you launch UE4 or the exported game or VR will be disabled.

Because this plugin implements and uses the normal VR APIs in UE4, you simply use the VR support in Unreal to use this plugin. For example, you can test your project in VR by using the "Play in VR" option in the Unreal Editor.

FOVE-specific capabilities such as eye tracking data can be found by searching for FOVE in the blueprint editor, or by using the `FoveHMD` class in C++:

```
if (FFoveHMD* const hmd = FFoveHMD::Get())
	return hmd->CheckEyesClosed(&outLeft, &outRight);
```

The underlying engine-independant FOVE C/C++ API from the SDK is also available through the FFoveHMD class.

## SteamVR ##

Because FOVE also has a [plugin for SteamVR](https://store.steampowered.com/app/529640/SteamVR_Driver_for_FOVE/), it is recommended to disable the SteamVR plugin while using this plugin.

If you wish to develop with FOVE and SteamVR, use the UE4 SteamVR plugin rather than this one. Keep in mind that SteamVR does not support eye tracking at this time.
