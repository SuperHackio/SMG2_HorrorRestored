# SMG2 Horror Restored
### Brings back the scrapped Bone's Curse powerup from the grave!

This is effectively a brand new powerup for SMG2, using bits and pieces of unused code that Nintendo left behind in the release.

Below is a list of everything included:
- The ObjectData archives for "Bone Mario", "Bone Luigi", the "Bone-shroom", the "Throwable-Bone", and the "Rib Switch".
- The LayoutData archive for the "Curse Meter".
- The "HorrorItem" object in the editor
- The "HorrorSwitch" object in the editor
- The "HorrorForbidThrowArea" area in the editor
- The "HorrorLimitArea" area in the editor
- The "Horror" comet type.
- An effect json for importing the Bone-Shroom's effects

Below is a list of other things that you will need:
- A level editor
- A Effect JSON supported particle editor

## Credits
- @superhackio - Assembly research and C++ programming
- @sy-24 - Research and testing
- @sunakazekun - Original researcher
- @humming-owl - Modeller & Model Importer
- @magnetlenz - Mathematics Assistance

## Setup
Place `HorrorItem` in a level, or set the Comet Type to `Horror`.

> *Note: If you do not place HorrorItem into the level somewhere, you will not be able to throw bones.*

### HorrorItem
| Parameter | Description |
| --- | --- |
| SW_APPEAR | Appears with a dedicated animation when this switch is activated and disappears when deactivated. |
| SW_AWAKE | Appears when this switch is activated and disappears when deactivated. |

### HorrorSwitch
| Parameter | Description |
| --- | --- |
| Obj_arg0 | If set, it starts with SW_A being activated. |
| Obj_arg1 | If set, it turns to face the player. |
| Obj_arg2 | If set alongside Obj_arg1, it will always face away from the player. |
| Obj_arg3 | If set, it disregards gravity for its initial rotation. |
| SW_APPEAR | Makes it appear when activated and disappear when deactivated. |
| SW_A | Gets activated when its crystal is hit with a bone. |
| SW_B | If set, the crystal stays inactive until this switch is activated. |
| SW_AWAKE | Makes it appear when activated and disappear when deactivated. Preferably use this over SW_APPEAR. |

### HorrorForbidThrowArea
| Parameter | Description |
| --- | --- |
| SW_APPEAR | Enables the area when activated and disables it when deactivated. |
| SW_AWAKE | Enables the area when activated and disables it when deactivated. Preferably use this over SW_APPEAR. |

### HorrorLimitArea
| Parameter | Description |
| --- | --- |
| Obj_arg0 | Limits by how much the CurseMeter can be drained. Example: Set to 900 to stop at a quarter of curse. |
| Obj_arg1 | Limits by how much the CurseMeter can refill. Example: Set to 2700 to stop at three quarters of curse. |
| SW_APPEAR | Enables the area when activated and disables it when deactivated. |
| SW_AWAKE | Enables the area when activated and disables it when deactivated. Preferably use this over SW_APPEAR. |