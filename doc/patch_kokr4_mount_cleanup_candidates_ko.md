# patch-koKR-4.MPQ 탈것 관련 제거 후보

## 기준

- 대상 파일: `E:\server\3.3.5\Data\koKR\patch-koKR-4.MPQ`
- 목적: `patch-Z.MPQ` 기준으로 탈것 모델 연결을 통일하기 위해 `patch-koKR-4.MPQ`의 커스텀 탈것 DBC/모델 의존 제거

## 우선 제거 대상

- `DBFilesClient\CreatureDisplayInfo.dbc`
- `DBFilesClient\CreatureModelData.dbc`
- `CreatureModelData.dbc`의 `900001~900049`가 참조하는 커스텀 탈것 모델 폴더

## 제거 대상 모델 폴더

| 폴더 | 비고 |
|---|---|
| `AllianceLionMount` | `CreatureModelData`의 `900001` 참조 |
| `AlliancePVPMount` | `patch-Z`에도 존재하지만 중복 방지를 위해 `patch-koKR-4`에서는 제거 |
| `AllianceShipMount` | `patch-Z`로 이전 |
| `CelestialSerpent` | `patch-Z`로 이전 |
| `Crane` | `patch-Z`로 이전 |
| `DarkPhoenix` | `patch-Z`로 이전 |
| `dragondeepholm` | `patch-Z`로 이전 |
| `dragonhawk` | `patch-Z`로 이전 |
| `faeriedragonmount` | `patch-Z`로 이전 |
| `felhound3_fire_mount` | `patch-Z`로 이전 |
| `felhound3_shadow_mount` | `patch-Z`로 이전 |
| `felstalkermount` | `patch-Z`로 이전 |
| `FireCatMount` | `patch-Z`로 이전 |
| `FoxMount` | `patch-Z`로 이전 |
| `HordePVPMount` | `patch-Z`로 이전 |
| `HordeScorpionMount` | `patch-Z`로 이전 |
| `hordezeppelinmount` | `patch-Z`로 이전 |
| `magemount_arcane` | `patch-Z`로 이전 |
| `magemount_fire` | `patch-Z`로 이전 |
| `magemount_frost` | `patch-Z`로 이전 |
| `monkmount` | `patch-Z`로 이전 |
| `moosemount2nightmare` | `patch-Z`로 이전 |
| `MushanBeast` | `patch-Z`로 이전 |
| `PandarenPhoenixMount` | `patch-Z`로 이전 |
| `PandarenSerpent` | `patch-Z`로 이전 |
| `protodragon` | `patch-Z`로 이전 |
| `Quilin` | `patch-Z`로 이전 |
| `ravenlord` | `patch-Z`로 이전 |
| `RedDrakeMount` | `patch-Z`로 이전 |
| `rocketmount3` | `patch-Z`로 이전 |
| `rocketmount4` | `patch-Z`로 이전 |
| `saber2` | `patch-Z`로 이전 |
| `scaleddrakemount` | `patch-Z`로 이전 |
| `Seahorse` | `patch-Z`로 이전 |
| `shadowstalkerpanthermount` | `patch-Z`로 이전 |
| `SiberianTiger` | `patch-Z`로 이전 |
| `SkeletalRaptor` | `patch-Z`로 이전 |
| `stormcrowmount` | `patch-Z`로 이전 |
| `stormcrowmount_solar` | `patch-Z`로 이전 |
| `suramarmount` | `patch-Z`로 이전 |
| `turtlemount` | `patch-Z`로 이전 |
| `TyraelMount` | `patch-Z`로 이전 |
| `voidelfhawkstridermount` | `patch-Z`로 이전 |
| `WaterStrider` | `patch-Z`로 이전 |
| `WingedLionMount` | `patch-Z`로 이전 |

## 유지 대상

- `DBFilesClient\Item.dbc`
- `DBFilesClient\Spell.dbc`
- `DBFilesClient\SpellItemEnchantment.dbc`
- `DBFilesClient\ItemRandomProperties.dbc`
- 기타 지도, 아이콘, 한글 locale 관련 파일

## 검증 결과

- 정리된 `patch-koKR-4.MPQ`에는 `CreatureDisplayInfo.dbc`, `CreatureModelData.dbc`가 없습니다.
- 정리된 `patch-koKR-4.MPQ`에는 위 커스텀 탈것 모델 폴더가 없습니다.
- 기존 커스텀 아이템/주문 관련 DBC는 유지되어 있습니다.
