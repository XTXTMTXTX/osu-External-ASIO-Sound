# osu! External ASIO Sound

Low-latency sfx player modified from rustbell's keyasio and [zzhouhe's AsioHookForOsu](https://github.com/zzhouhe/AsioHookForOsu) using ASIO with [FMOD Core](https://www.fmod.com) \(Ver. 2.01.03\)  

https://docs.microsoft.com/en-us/windows/desktop/inputdev/virtual-key-codes)

## Compile  
Install MinGW and double click Compile.bat  

## Command Line  
osu!asio_sound.exe \[Buffer Size\]

e.g. osu!asio_sound.exe 64

## How to Use  
### First Time

You need:   
	2 audio outputs device, e.g. Onboard Audio with a USB Audio DAC  
	2 headphones or speakers you can use at the same time  

Install ASIO4ALL   

Plug your headphones in  

In your windows audio control panel, set one of your audio output device as the default device for other programs  

Run osu!asio_sound.exe, select asio4all and then open the asio4all control panel, select your **Another** audio output device and deselect other devices (maybe you need to click the "Advanced Options" Botton on the bottom right corner first)  

If you don't hear anything, check the asio4all control panel and set a larger Buffer Size  

\* You need to close effect sounds

\* You need to select **ignore beatmap hitsounds**

\* This program will only play the hit-sounds&sfx in your skin, copy sound files  in **soundpack** into your skin folder to fill the missing sounds

\* You need to delete **spinnerspin** & **\*-sliderslide** & **\*-sliderwhistle** hit-sounds in your skin

\* You need to adjust your osu's **Universal offset** to about -32ms

### Generally

1) Open osu!.

2) Execute osu!asio_sound.exe.

3) Reload your skin.

4) Play.



