Dynamic Docksets — OBS Dock Plugin v1.1.0
============================================

Speichert, laedt und cycelt OBS-Dock-Layouts ('Docksets').


Was ist neu in 1.1.0
--------------------

  * **Stream-Deck-Shortcuts:** das Plugin generiert pro Dockset eine
    kleine .vbs-Datei. Stream Deck (oder jeder andere File-Launcher)
    fuehrt die .vbs aus und das Plugin laedt das passende Dockset.
    Kein WebSocket-Setup mehr noetig — einfach Datei oeffnen.

  * Im Manager-Dock gibt es einen neuen Ordner-Button (📂), der
    den Shortcuts-Ordner direkt im Explorer oeffnet.


Features (Vollstaendige Liste)
------------------------------

  * Manager-Dock — Liste aller gespeicherten Docksets mit Thumbnail-
    Vorschau, Toolbar fuer Save / Save-As / Load / Delete, Kontextmenue
    fuer Rename / Reihenfolge / Auto-Load-Zuweisung.

  * Schnellzugriff-Dock — schmale horizontale Toolbar mit einem Button
    pro Dockset. Klick laedt. Aktives Dockset gelb hervorgehoben.

  * Tools-Menue — Submenue 'Dynamic Docksets' unter Tools/.

  * Hotkeys (in OBS-Hotkey-Settings):
      - Dynamic Docksets: Naechstes Layout
      - Dynamic Docksets: Vorheriges Layout

  * Auto-Save (optional): wenn aktiv, speichert das aktuell aktive
    Dockset automatisch bei Layout-Aenderungen (5 s Throttle).

  * Auto-Load bei Collection-Wechsel (optional).

  * WebSocket-Vendor (Streamer.bot / Stream-Deck WebSocket-Plugins):
    Vendor-Name 'DynamicDocksets' UND als Kompat-Alias 'jrDockie'.
    Requests: LoadDockset, CycleDockset, SaveDockset.

  * NEU v1.1.0: Stream-Deck-Shortcut-Files (.vbs). Siehe naechster
    Abschnitt.


Stream-Deck-Shortcuts (das eigentlich Coole)
--------------------------------------------

Du brauchst keine WebSocket-Konfig auf dem Stream Deck mehr.

Wo liegen die Shortcuts:

  <OBS-Hauptordner>\config\obs-studio\plugin_config\
    dynamic-docksets\
      shortcuts\
        Full Preview.vbs           <- ein Shortcut pro Dockset
        Streaming Layout.vbs
        Editing.vbs
        _Cycle Next.vbs            <- zum naechsten Layout
        _Cycle Prev.vbs            <- zum vorherigen Layout
      triggers\
        (interner Ordner — Plugin ueberwacht ihn)

Im Stream Deck:
  1. Aktion 'System: Open' anlegen
  2. Pfad zur .vbs-Datei eintragen, z. B.:
     C:\OBS\config\obs-studio\plugin_config\dynamic-docksets\
     shortcuts\Full Preview.vbs
  3. Knopf druecken → das Dockset 'Full Preview' wird sofort geladen.

So funktioniert's intern:
  - Die .vbs schreibt eine kleine Trigger-Datei in
    triggers\<dockset-name>.trigger
  - Das Plugin (in OBS) hat einen QFileSystemWatcher auf triggers\
  - Sobald die Trigger-Datei erscheint, laedt das Plugin das passende
    Dockset und loescht die Trigger-Datei wieder
  - .vbs werden automatisch neu erzeugt wenn du Docksets hinzufuegst,
    umbenennst oder loeschst

Kein Konsolen-Flash beim Ausfuehren — .vbs laeuft silent.


Installation
------------

Empfohlen: Doppelklick auf INSTALL.bat, OBS-Pfad eingeben, fertig.

Manuell: BEIDE Top-Level-Ordner (obs-plugins\, data\) in den
OBS-Hauptordner kopieren.


Verzeichnisstruktur
-------------------

  dynamic-docksets-1.1.0\
    INSTALL.bat
    README.txt
    obs-plugins\
      64bit\
        dynamic-docksets.dll        <- Plugin-Binary (~150 KB)
        dynamic-docksets.pdb        <- Debug-Symbole, optional
    data\
      obs-plugins\
        dynamic-docksets\
          locale\
            en-US.ini
            de-DE.ini


Verwendung — Kurzanleitung
--------------------------

1. OBS starten -^> Ansicht -^> Andocken -^> 'Dynamic Docksets - Manager'.
2. 'Speichern als...' klicken, Namen eingeben.
3. Layout veraendern, dann das gespeicherte Dockset doppelklicken
   um es wiederherzustellen.

Stream-Deck-Setup:
4. Im Manager auf 📂 klicken -> Shortcuts-Ordner oeffnet sich
5. Stream Deck -^> 'System: Open' Action anlegen
6. Pfad zur gewuenschten .vbs-Datei
7. Stream-Deck-Knopf druecken -^> Layout wechselt

Power-User:
  - 'Auto-save' anhaken: jede Layout-Aenderung wird im aktiven
    Dockset gespeichert (5 s Throttle).
  - Rechtsklick auf Eintrag -^> 'Auto-Laden fuer Szenen-Sammlung'
    -^> Sammlung waehlen.
  - Hotkeys 'Naechstes Layout' / 'Vorheriges Layout' in OBS-Hotkey-
    Einstellungen zuweisen.


WebSocket — Beispiel-Nutzung
----------------------------

(Falls du doch lieber WebSocket statt Stream-Deck-Shortcut-Files nutzt.)

Streamer.bot / Multi-OBS-WebSocket-Raw:

  Vendor:        DynamicDocksets   (oder: jrDockie)
  Request:       LoadDockset
  Request data:  { "filename": "Streaming Layout" }

  Vendor:        DynamicDocksets
  Request:       CycleDockset
  Request data:  {}


Speicherort der Daten
---------------------

<OBS-Hauptordner>\config\obs-studio\plugin_config\dynamic-docksets\
  <name>.json    eine Datei pro Dockset (mit Thumbnail-Base64)
  _meta.json     Reihenfolge, aktives Dockset, Collection-Mappings
  shortcuts\     auto-generierte .vbs-Files fuer Stream Deck
  triggers\      interner Watch-Ordner


Migration von jrDockie
----------------------

Nicht in v1.1.0 automatisch. v1.2 soll Import-Funktion bekommen.

Tipp: Der WebSocket-Vendor 'jrDockie' wird zusaetzlich als Alias
registriert (sofern jrdockie nicht parallel geladen ist), so dass
existierende Streamer.bot-Skripte ohne Aenderung weiter funktionieren.


Anforderungen
-------------

  - OBS Studio 30.x / 31.x / 32.x (Windows x64)
  - Qt6 (von OBS mitgeliefert)
  - Optional: obs-websocket fuer WebSocket-Vendor-Funktionen


Versionshistorie
----------------

1.1.0 (2026-04-27)
  * NEU: Stream-Deck-Shortcut-Files (.vbs pro Dockset).
  * Manager-Dock hat einen neuen Button zum Oeffnen des
    Shortcuts-Ordners.
  * QFileSystemWatcher auf triggers/-Verzeichnis.
  * Bonus-Shortcuts _Cycle Next.vbs und _Cycle Prev.vbs.

1.0.0 (2026-04-27)
  * Erstrelease als Eigenimplementierung im dynamic-* Stil.
  * Manager-Dock + Quick-Access-Dock + Tools-Menue.
  * Save/Load/Cycle/Rename/Delete/Reorder.
  * Thumbnails (200x150 PNG, eingebettet als Base64 in JSON).
  * Per-Collection-Auto-Load.
  * Optional Auto-Save mit 5 s Throttle.
  * Hotkeys fuer Cycle Next + Cycle Prev.
  * WebSocket-Vendor (DynamicDocksets + jrDockie alias).


Lizenz: GPLv2 oder neuer.
