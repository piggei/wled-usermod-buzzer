#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[1]
usermod = (root / "usermod_buzzer.cpp").read_text(encoding="utf-8")
readme = (root / "README.md").read_text(encoding="utf-8")
library = (root / "library.json").read_text(encoding="utf-8")
sounds = (root / "BuzzerSounds.cpp").read_text(encoding="utf-8")

for token in [
    'BUZZER_VERSION = "0.1.0"',
    'BUZZER_BUILD = "rc.8"',
    'PinManager::allocatePin(hardwarePin_, true, PinOwner::UM_Unspecified)',
    'PinManager::allocateLedc(1)',
    'PinManager::deallocateLedc(ledcChannel_, 1)',
    'esp_timer_start_periodic(serviceTimer_, SERVICE_PERIOD_US)',
    'server.on(F("/buzzer"), HTTP_GET',
    'playSoundRepeat(soundId.c_str(), static_cast<uint16_t>(repeat))',
    'Use either loop or repeat, not both.',
    'repeat must be an integer between 1 and 255.',
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
    "yankee_doodle", "star_wars", "warning", "error", "connect", "disconnect", "attention", "alarm",
]:
    assert f"`{sound}`" in readme, sound

assert '"version": "0.1.0-rc.8"' in library
assert "iDotMatrix" not in usermod
assert "IDotMatrix" not in usermod
assert "delay(" not in usermod
assert "x.firstChild.textContent=''" not in usermod
assert "Output level for passive buzzer (0-100%)." not in usermod
assert "r.type='range'" in usermod
assert "r.min=0" in usermod
assert "r.max=100" in usermod
assert "n.textContent=r.value+'%'" in usermod
assert "V.firstChild.data='Volume: '" in usermod
assert "Active buzzers reproduce rhythm only. Passive buzzers reproduce note pitch." in usermod
assert "{2000, 90, 70}" in sounds
assert sounds.count("{2000, 90, 70}") == 2
assert "{2000, 90, 550}" in sounds
assert "{2000, 90, 0}" not in sounds
for token in ["{1047, 100, 50}", "{1319, 100, 50}", "{1568, 200, 100}", "{1568, 400, 100}"]:
    assert token in sounds, token
assert sounds.count("{1047, 100, 50}") == 2
assert sounds.count("{1319, 100, 50}") == 2
assert sounds.count("{1568, 200, 100}") == 1
assert sounds.count("{1568, 400, 100}") == 1
assert "SOUND_YANKEE_DOODLE" in sounds
assert sounds.count("// C5") >= 6
assert "hr{display:none}" in usermod and "Buzzer:enabled" in usermod
assert "{294, 150, 150}" in sounds
assert "{294, 190, 300}" in sounds
assert "{392, 140, 35}" not in sounds
assert "margin:16px 0 8px" in usermod

assert "{262, 300, 100}" in sounds

assert "{247, 300, 100}" in sounds

assert "{233, 300, 100}" in sounds

assert "{220, 300, 100}" in sounds

assert "{208, 600, 200}" in sounds

assert "{196, 800, 350}" in sounds

assert "{523, 560, 10}" in sounds

assert "{494, 560, 400}" in sounds

# b012 sound-only regression checks.
assert "SOUND_STAR_WARS" in sounds
assert '{"star_wars", "Star Wars"' in sounds
assert "{1175, 150, 250}" in sounds
assert "{880, 80, 45}" not in sounds
assert "{880, 65, 25}" in sounds
assert "{1175, 130, 250}" in sounds
assert "{1175, 65, 25}" in sounds
assert "{880, 150, 250}" in sounds

# b018 Star Wars reference-MP3 regression checks.
assert "{1568, 400, 100}" in sounds
assert sounds.count("{262, 100, 100}") == 3
for token in [
    "{349,1200,   0}", "{523,1200, 400}",
    "{466, 400,   0}", "{440, 400,   0}", "{392, 400,   0}",
    "{698,1200,   0}", "{523,1200, 400}",
]:
    assert token in sounds, token

# Release hardening and UI regression checks.
assert 'BUZZER_BUILD = "rc.8"' in usermod
assert '#include "BuzzerInput.h"' in usermod
assert 'BuzzerInput::parseUnsignedDecimal' in usermod
assert 'BuzzerInput::parseBooleanText' in usermod
assert 'Use one buzzer command per request.' in usermod
assert 'tone must be an integer between 20 and 20000 Hz.' in usermod
assert 'duration must be an integer between 1 and 60000 ms.' in usermod
assert 'repeat must be an integer between 1 and 255.' in usermod
assert 'xSemaphoreTake(mutex_, portMAX_DELAY)' in (root / "BuzzerEngine.cpp").read_text(encoding="utf-8")
assert 'if (!engineThreadSafe_) return false;' in usermod
assert 'BuzzerEngine::Snapshot snapshot = engine_.snapshot();' in usermod
assert 'Snapshot snapshot() const;' in (root / "BuzzerEngine.h").read_text(encoding="utf-8")
assert "addInfo('Buzzer:enabled',1,'<style>" in usermod and "hr{display:none}" in usermod
assert "addInfo('Buzzer:pin',1,'');" in usermod
assert "addInfo('Buzzer:sound',1,'');" in usermod
assert "Enabled Enabled" not in usermod
assert "Sound Sound" not in usermod
assert 'curl "http://<WLED-IP>/buzzer?play=alarm&repeat=3"' in readme
assert 'curl "http://<WLED-IP>/buzzer?play=alarm&loop=1"' in readme
assert '20-20000 Hz' in readme
assert '1-60000 ms' in readme
assert 'idle' in readme
assert ' / ready / ' not in readme
assert "low-pitch" not in (root / "TESTING.md").read_text(encoding="utf-8")

# RC5 UI contract: do not re-wrap the non-volume rows.
for token in [
    "L(e,'Enabled:')", "L(p,'GPIO Pin:')", "L(t,'Buzzer Type:')",
    "L(g,'Trigger level:')", "L(s,'Sound:')", "V=w(r)",
    "V.firstChild.data='Volume: '",
]:
    assert token in usermod, token
for forbidden in ["E=w(e)", "P=w(p)", "T=w(t)", "G=w(g)", "S=w(s)"]:
    assert forbidden not in usermod, forbidden
assert ".sec:has([name=\\\"Buzzer:enabled\\\"])>hr{display:none}" in usermod
assert "if(V)V.hidden=!(t&&t.value==1)" in usermod
assert "addInfo('Buzzer:enabled',1,'<style>" in usermod
assert "addInfo('Buzzer:enabled',1,'<style>.sec:has([name=\"Buzzer:enabled\"])>hr{display:none}</style>','Enabled:')" not in usermod
assert "addInfo('Buzzer:pin',1,'','GPIO:')" not in usermod
assert "addInfo('Buzzer:sound',1,'','Sound:')" not in usermod
assert (root / "docs" / "wled-buzzer-usermod-gui.png").exists()
assert "docs/wled-buzzer-usermod-gui.png" in readme

print("Static regression checks passed.")
# RC6 optional-consumer bridge contract.
service_h = (root / "WLEDBuzzerService.h").read_text(encoding="utf-8")
service_cpp = (root / "WLEDBuzzerService.cpp").read_text(encoding="utf-8")
for token in [
    "wledBuzzerServiceReady", "wledBuzzerServicePlaying",
    "wledBuzzerServicePlay", "wledBuzzerServiceStop",
    "wledBuzzerServiceCurrentSoundId",
]:
    assert token in service_h, token
    assert token in service_cpp, token
assert 'extern "C"' in service_h

