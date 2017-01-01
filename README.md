# NewModPE
Adds new hooks and methods for ModPE. <br />
It can break the ModPE's limit.
## Current API:
```
global classes:
    N_Player - provides some player related utilities
    methods:
        static void setRoundAttack(boolean enable, boolean hitMob, int distance); - enables kill aura
            parameters:
                enable - Enable kill aura?
                hitMob - Hit mob?
                distance - Target mob filtering with distance
```