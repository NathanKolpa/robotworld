---
title: Kwaliteits Review RobotWorld
author:
  - "Nathan Kolpa"
toc: true
lang: "nl"
titlepage: true
titlepage-rule-color: "360049"
titlepage-background: "backgrounds/background1.pdf"
---

# Design

# MVC

MVC (Model view controller) is een patroon waarbij de applicatie in 3 hoofd categorieën wordt opgesplitst
met ieder zijn eigen verantwoordelijkheid:

- Model: klassen die een abstracte weergave bieden van de logica en data binnen de applicatie.
- View: klassen die verantwoordelijk zijn over hoe de weergave (in dit geval een gui) van een applicatie wordt getoond.
- Controller: klassen of functies die een event afhandelen met behulp van de *Model* en het resultaat hiervan
  presenteert via de *View*.
  Controllers maken idealiter gebruik van alleen hoge level abstracties en bieden een globale weergave van een actie.

In robotworld zijn de controllers niet expliciet benoemd, maar zijn wel te identificeren binnen de MainFrameWindow
klasse.
Voor het afhandelen van input (in dit geval gui knoppen) kun je iedere handler/callback zien als een aparte controller.
De reden waarom deze handlers controllers zijn is omdat hier de view (indirect) wordt bijgewerkt op basis van model
klassen.
Een goed voorbeeld hiervan is op regel 709:

```cpp
// deze functie wordt aangeroepen door wxWidgets na het klikken van een knop.
void MainFrameWindow::OnStartListening( wxCommandEvent& UNUSEDPARAM(anEvent))
{
    // Logica vanuit model klassen.
    Model::RobotPtr robot = Model::RobotWorld::getRobotWorld().getRobot( "Robot");
    if (robot)
    {
        // doormiddel van threads en het observer pattern wordt de view bijgewerkt.
        robot->startCommunicating();
    }
}
```

De code waarmee wij de applicatie hebben uitgebreid die netwerk pakketjes afhandelen kunnen ook beschouwd worden als controller code.
Maar in tegenstelling tot de bestaande code is de view van onze uitgebreide code netwerk IO.

# Gebruik van patronen

## Observer

## Singleton

## Strategy

# Toepassen van code standaarden.

In dit hoofdstuk ga ik een kwaliteitsoordeel geven op basis van de JSF-styleguide.
Om de omvang van mijn oordeel te limiteren behandel ik een selectie van vijf richtlijnen.

## Weg commenten van code (AV Rule 127)

> Code that is not used (commented out) shall be deleted.

Tijdens het schrijven van code is het een goed idee om de hoeveelheid "clutter" te minimaliseren.
Dit zorgt er voor de lezer, sneller en makkelijker de code kan lezen.
Hier onder heb ik een aantal (niet alle) voorbeelden van weggecommente code:

- MainFrameWindow.cpp regel: 699, 576
- RobotWorldCanvas.cpp: 438, 292, 287
- Robot.cpp: 329

En voor mij het meest opvallende voorbeeld (AStar.cpp 130):

```cpp
// Keep the timing stuff, please.
//		clock_t start = std::clock();
while (!openSet.empty())
```

De oplossing hiervoor is om de oude code de in een commit te zetten en daarna kun je de weggecommente code volledig
weglaten.

## Gebruik van variable-width types (AV Rule 209)

> The basic types of int, short, long, float and double shall not be used, but specific-length equivalents should be
> typedef’d accordingly for each compiler, and these type names used in
> the code

Volgens [cppreference](https://en.cppreference.com/w/cpp/language/types) kan de grootte van data types zoals int, short
en long verschillen (variable-width data types).
De verschillen in grootte kan leiden tot ander gedrag op verschillende omgevingen, dit maakt het testen van code niet
accuraat of veel meer werk.
De oplossing hiervoor is om [fixed-width data types](https://en.cppreference.com/w/cpp/types/integer) te gebruiken zodat
je zeker bent van de grootte van een variable.

Een overtreding van deze regel is bijvoorbeeld in LineShape.hpp 86:

```cpp
int getLineWidth() const;
```

In de gehele codebase wordt er geen gebruik gemaakt van fixed-width data typen.

## Gebruik van public en protected in classen (AV Rule 67)

> Public and protected data should only be used in structs—not classes

Het gebruik van public en protected visibility voor velden binnen klassen is een slecht idee omdat het verbreekt de
encapsulatie die de klasse moet aanbieden.
Als de gebruiker van een klasse toch toegang moet hebben moet dit via een interface gaan bijvoorbeeld getters en
setters.

Het toepassen van deze regel in zijn algemeenheid redelijk, er zijn wel een aantal gebreken:

- BoundedVector.hpp 111, 112
- CommandLineArgument.hpp 91-93
- Objectid.hpp 32
- RectangleShape 165-185

## Gebruik geen C-style casts (AV Rule 185)

> C++ style casts (const_cast, reinterpret_cast, and static_cast) shall be used instead of the
> traditional C-style casts.

Het casten van data typen volgens de `(T)variable` syntax is afgeraden omdat het moeilijk is om naar een type cast te
zoeken binnen de broncode.
Dat maakt voor het reviewen van deze regel natuurlijk lastig, maar door de `-Wold-style-cast` flag en wx widgets te
includen via de `-isystem` flag is het toch gelukt om hier betrouwbaar voor te scannen.

Deze regel wordt slechts op 1 plek binnen robotworld overtreden, ironisch genoeg in dezelfde regel waar een c++ style
cast wordt gebruikt.

NotificationHandler.hpp 31:

```cpp
Connect( EVT_NOTIFICATIONEVENT,
(wxObjectEventFunction)(wxEventFunction)reinterpret_cast< NotificationEventFunction >( &NotificationHandler::OnNotificationEvent));
```

## Alle symbolen moeten in een namespace (AV Rule 98)

> Every nonlocal name, except main(), should be placed in some namespace.

Om er voor te voorkomen dat namen van functies, klassen, etc... (symbolen) niet botsen, is het aangeraden om alle
symbolen in een namespace te zetten.
Deze regel wordt goed toegepast, er lijkt echter wel 1 slordigheidsfoutje in te zitten in Widgets.hpp regel 50.