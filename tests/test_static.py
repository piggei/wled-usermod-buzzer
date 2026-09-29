#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[1]
usermod = (root / "usermod_buzzer.cpp").read_text(encoding="utf-8")
readme = (root / "README.md").read_text(encoding="utf-8")
library = (root / "library.json").read_text(encoding="utf-8")
sounds = (root / "BuzzerSounds.cpp").read_text(encoding="utf-8")

for token in [
    'BUZZER_VERSION = "0.1.0"',
    'BUZZER_BUILD = "b006"',
    'PinManager::allocatePin(hardwarePin_, true, PinOwner::UM_Unspecified)',
    'PinManager::allocateLedc(1)',
    'PinManager::deallocateLedc(ledcChannel_, 1)',
    'esp_timer_start_periodic(serviceTimer_, SERVICE_PERIOD_US)',
    'server.on(F("/buzzer"), HTTP_GET',
    'root.createNestedObject(FPSTR(JSON_KEY))',
    'WLEDBuzzerService::setInstance(this)',
    "addOption(dd,'Active',0)",
    "addOption(dd,'Passive',1)",
    "addOption(dd,'High',0)",
    "addOption(dd,'Low',1)",
    "p.textContent='Play'",
    "z.textContent='Stop'",
    'location.origin+\'/buzzer?play=\'',
    "d.createTextNode(' Show API commands')",
    "a.hidden=true",
    "c.onchange=()=>a.hidden=!c.checked",
]:
    assert token in usermod, token

for sound in [
    "beep", "double_beep", "triple_beep", "notification", "success", "victory", "fail",
    "warning", "error", "connect", "disconnect", "attention", "alarm",
]:
    assert f"`{sound}`" in readme, sound

assert '"version": "0.1.0"' in library
assert "iDotMatrix" not in usermod
assert "IDotMatrix" not in usermod
assert "delay(" not in usermod
assert "x.firstChild.textContent=''" not in usermod
assert "Output level for passive buzzer (0-100%)." not in usermod
assert "r.type='range'" in usermod
assert "r.min=0" in usermod
assert "r.max=100" in usermod
assert "n.textContent=r.value+'%'" in usermod
assert "T.firstChild.data='Buzzer type: '" in usermod
assert "V.firstChild.data='Volume: '" in usermod
assert "Active buzzers reproduce rhythm only. Passive buzzers reproduce note pitch." in usermod
assert "{2000, 90, 70}" in sounds
assert sounds.count("{2000, 90, 70}") == 2
assert "{2000, 90, 0}" in sounds
for token in ["{175, 145, 0}", "{220, 140, 0}", "{262, 145, 0}", "{349, 285, 0}", "{349, 445, 0}", "{698, 700, 0}"]:
    assert token in sounds, token
print("Static regression checks passed.")

assert "{294, 150, 150}" in sounds
assert "{294, 190, 0}" in sounds
assert "{392, 140, 35}" not in sounds
assert "margin:16px 0 8px" in usermod
