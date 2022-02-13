#include "volumemanager.h"
#include "es8388.h"
typedef struct VolumeManager
{
    int volume;
} VolumeManager;

static VolumeManager sVolumeManager;

int volumeManagerInit()
{
    sVolumeManager.volume = 0;
    volumeManagerSetVolume(20);
    return 0;
}

int volumeManagerUninit()
{
    sVolumeManager.volume = 0;
    return 0;
}

int volumeManagerVolumeUp()
{
    if (sVolumeManager.volume < 100)
    {
        sVolumeManager.volume = sVolumeManager.volume / 10 * 10;
        sVolumeManager.volume += 10;
        ES8388_HPvol_Set((uint8_t)(33 * sVolumeManager.volume / 100));
        ES8388_SPKvol_Set((uint8_t)(33 * sVolumeManager.volume / 100));
    }
    return sVolumeManager.volume;
}

int volumeManagerVolumeDown()
{
    sVolumeManager.volume = sVolumeManager.volume / 10 * 10;
    if (sVolumeManager.volume > 0)
    {
        sVolumeManager.volume -= 10;
        ES8388_HPvol_Set((uint8_t)(33 * sVolumeManager.volume / 100));
        ES8388_SPKvol_Set((uint8_t)(33 * sVolumeManager.volume / 100));
    }
    return sVolumeManager.volume;
}

int volumeManagerSetVolume(int volume)
{
    if (volume >= 0 && volume <= 100)
    {
        sVolumeManager.volume = volume;
        ES8388_HPvol_Set((uint8_t)(33 * sVolumeManager.volume / 100));
        ES8388_SPKvol_Set((uint8_t)(33 * sVolumeManager.volume / 100));
    }
    return sVolumeManager.volume;
}