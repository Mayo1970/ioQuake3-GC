# legacy/

Retired tooling. Not used during normal builds.

## apply_patches.sh

The historical patcher that turned a vanilla `../ioq3` clone into a buildable
Wii tree. The patches it applied are now baked directly into `code/qcommon/`,
`code/client/`, `code/server/`, `code/botlib/`, `code/renderergl1/`,
`code/renderercommon/`. Building no longer needs this script.

Kept for reference if you ever need to regenerate the vendored tree from a
fresh upstream ioQ3 checkout:

```bash
git clone https://github.com/ioquake/ioq3.git ../ioq3
bash legacy/apply_patches.sh
cp -r ../ioq3/code/{qcommon,client,server,botlib,renderergl1,renderercommon,ui,cgame,game} code/
```

The last three dirs (`ui/`, `cgame/`, `game/`) contribute only headers to
the engine build (cross-dir `#include "../ui/..."` from `client.h` etc.).

## patch_q_platform.py

Stale subset of `apply_patches.sh` (only the `q_platform.h` GEKKO block).
Superseded; kept for archaeology.
