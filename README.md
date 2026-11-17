# SMG2 BossBegomanFactory
### Topmaniac in SMG2

This is an enhanced port of Topmaniac from Super Mario Galaxy. It includes new SMG2 specific features while still being faithful to the original.

Below is a list of everything included:
- The ObjectData archives for Topmaniac.
- The "BossBegoman" object in the editor.
- An edited ActionSound.arc file that includes edits that were made to ActionSound.
- An edited Effect.arc file that includes particle effects for all included objects.

Below is a list of other things that you will need:
- A level editor.
- A particle editor to extract JSON files from Effect.arc.

## Credits
- @shibboleet - Decompilation.
- [@Gonzalo-kebab](https://github.com/Gonzalo-kebab) - Decompilation.
- [@Super Hackio](https://github.com/SuperHackio) - Decopilation. Ported and enhanced SMG1 code to SMG2.

## Setup
It is adviced to reference the [template mod](https://youtu.be/FUnmhdi8Vp8) for this port. You can use the following parameters:

### BossBegoman
| Parameter | Description |
| --- | --- |
| Obj_arg0 | Power Star ID. Spawns the assigned Power Star when defeated. Enable SW_DEAD if not set. |
| Obj_arg1 | Health. The amount of hits Topmaniac can take. |
| Obj_arg2 | Topmini count. The amount of Topminis to spawn. Default 3. |
| Obj_arg3 | Spiky Topman count. The amount of Spiky Topmen to spawn. Default 2. |
| Obj_arg4 | Spring Topman count. The amount of Spring Topmen to spawn. Default 0. |
| Obj_arg5 | Topmini spawn health. The remaining health at which Topmaniac will spawn Topmini minions. Default = Obj_arg1. |
| Obj_arg6 | Spiky Topman spawn health. The remaining health at which Topmaniac will spawn Spiky Topman minions. Default 1. |
| Obj_arg7 | Spring Topman spawn health. The remaining health at which Topmaniac will spawn Spring Topman minions. Off by default. |
| SW_DEAD | Gets activated when defeated and no Power Star ID is set. |
| SW_A | Starts the fight when activated. |
> *Note: The default Obj_args will replicate the Battlerock fight.*
