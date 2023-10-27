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

# Introductie

Dit essay zal zich richten op het analyseren van de architectuur en kwaliteit van de code in "robotworld".
Ik zal de structuur en organisatie van de code onderzoeken, de gebruikte ontwerpbeslissingen evalueren en de
codekwaliteit beoordelen aan de hand van relevante
criteria.
Het doel is om een diepgaand inzicht te krijgen in hoe goed de code is geschreven en hoe goed deze is
aangepast aan het beoogde doel, met speciale aandacht voor design conventies, design principes, patronen en andere
mogelijke verbeteringen.

# Packages

![Package diagram robotworld](packages.svg)

| Namespace     | Uitleg                                                                    |
|:--------------|:--------------------------------------------------------------------------|
| View          | Klassen die zich bezig houden met presentatie (xwWidgets gui)             |
| Utils         | Overige helper functies                                                   |
| PathAlgorithm | Klassen die zich bezig houden met het berekenen van het pad (AStar)       |
| Model         | Het data model van de applicatie / een implementatie van het domein model |
| Messaging     | Netwerk communicatie                                                      |
| Base          | Basis structuren en patronen                                              |
| Application   | Hoge level abstracties die de algemene flow van de applicatie bepalen.    |

# Klassen

![Class diagram robotworld](classes.svg)

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

De code waarmee wij de applicatie hebben uitgebreid die netwerk pakketjes afhandelen kunnen ook beschouwd worden als
controller code.
Maar in tegenstelling tot de bestaande code is de view van onze uitgebreide code netwerk IO.

# Gebruik van patronen

## Observer

Het observer pattern dat wordt gebruikt om een een-op-veel-relatie tussen objecten te definiëren, zodat wanneer het ene
object verandert, al zijn afhankelijke objecten
automatisch op de hoogte worden gebracht en worden bijgewerkt.
Het doel van het observer pattern is om een manier te bieden om onderlinge afhankelijkheden tussen objecten te beheren,
waardoor ze losjes gekoppeld
blijven.

Er zijn 4 klassen die een rol spelen bij dit pattern:

* Subject: Dit is het object dat wordt geobserveerd en dat een lijst met observers bijhoudt. Het subject
  voorziet methoden om observers toe te voegen, te verwijderen en op de hoogte te stellen van wijzigingen.

* Observer: Dit zijn de objecten die geïnteresseerd zijn in de wijzigingen van het onderwerp.
  Ze registreren zichzelf bij het onderwerp en worden op de hoogte gebracht wanneer er veranderingen optreden.

* ConcreteSubject: Dit is een specifieke implementatie van het Subject-interface.
  Het houdt de staat bij die wordt waargenomen en stuurt kennisgevingen naar de geregistreerde observers bij
  wijzigingen.

* ConcreteObserver: Dit is een specifieke implementatie van het Observer-interface.
  Het reageert op de kennisgevingen van het onderwerp en voert de gewenste acties uit wanneer er veranderingen zijn.

In robotworld is dit patroon toegepast voor het bewegen van een robot en het bijwerken van de canvas:

|     Abstract     | Robotworld                                                                                                                              |
|:----------------:|:----------------------------------------------------------------------------------------------------------------------------------------|
|     Subject      | `Notifier`, een abstracte klasse en houd een lijst bij van `Observer`s en roept `handleNotification()` aan wanneer er een update is.    |
| ConcreteSubject  | `Robot`, implemented de abstracte klasse `Notifier`. Wanneer robot van state verandert, roept robot zelf `notifyObservers()` aan.       |
|     Observer     | `Observer`, een abstracte klasse en heeft een `vitual` method `handleNotification()`.                                                   |
| ConcreteObserver | `RobotShape`, implemented de abstracte klasse `Obsever` en observed de robot zodat de canvas bijgewerkt wordt wanneer de robot beweegt. |

## Singleton

Het doel van het Singleton-pattern is om ervoor te zorgen dat er slechts één enkele instantie van een klasse wordt
gecreëerd en dat deze toegankelijk is vanuit elke plek in de applicatie.

In robot world zijn er een aantal singletons:

- `CommunicationService`
- `RobotWorld`
- `MainSettings`

## Strategy

Het doel van het Strategy-pattern is om gedrag in een applicatie te encapsuleren en deze gedragsalgoritmes
uitwisselbaar te maken.
Hierdoor kunnen cliënten van een klasse hun gedrag dynamisch aanpassen zonder de klasse zelf te
wijzigen.

Dit is in de applicatie gerealiseerd met de volgende klassen:

| Abstract                 | Robotworld          |
|:-------------------------|:--------------------|
| Strategy                 | ITraceFunction      |
| Strategy (implementatie) | StdOutTraceFunction |
| Strategy (implementatie) | FileTraceFunction   |
| Strategy (implementatie) | WidgetTraceFunction |

# Toepassen van code standaarden.

In dit hoofdstuk ga ik een kwaliteitsoordeel geven op basis van de JSF-styleguide.
Om de omvang van mijn oordeel te limiteren behandel ik een selectie van vijf richtlijnen.

## Weg commenten van code (AV Rule 127)

> Code that is not used (commented out) shall be deleted.

Tijdens het schrijven van code is het een goed idee om de hoeveelheid "clutter" te minimaliseren.
Dit zorgt er voor de lezer, sneller en makkelijker de code kan lezen.
Deze regel heb ik uitgekozen omdat mij opviel tijdens het project hoe veel regels weggecommente code er in de bestanden
staan.
Hier onder heb ik een aantal voorbeelden:

- MainFrameWindow.cpp regel: 699, 576
- RobotWorldCanvas.cpp: 438, 292, 287
- Robot.cpp: 329

En voor mij het meest opvallende voorbeeld (AStar.cpp 130):

```cpp
// Keep the timing stuff, please.
//		clock_t start = std::clock();
while (!openSet.empty())
```

Het bovenstaande voorbeeld laat mij denken dat er nog een achterliggend probleem is op het gebied van communicatie.

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

# Multi-threading

De code die verantwoordelijk is voor het rijden van een robot wordt voor iedere robot apart uitgevoerd op een andere
thread.
Deze aanpak is niet per-se fout, maar het gebruik van meerdere threads binnen een applicatie bengt behoorlijk wat
complexiteit met zich mee.
Want, de code moet rekening houden met synchronisatie van meerdere threads, het veilig omgaan van toegang en mutatie,
deadlocks en race conditions.

Een aantal van de bovengenoemde punten zijn binnen robotworld niet correct geïmplementeerd.
Het is ook niet in comments of documentatie aangegeven over hoe functies thread safe of unsafe zijn, terwijl dit zeker
hele relevante informatie is.
Voor deze redenen heb ik iedere regel **heel** zorgvuldig moeten lezen om te bepalen of de applicatie geen undefined
behaviour bevat.
Een voorbeeld hoe een use after free kan voorkomen binnen robotworld door het gebruik van meerdere threads.

1. De main thread roept `robot.startActing()` aan.
2. De `startActing()` methode start een nieuwe thread aan die de robot laat bewegen.
3. Als de robot beweegt wordt `notifyObservers()` aangeroepen.
4. Omdat in RobotWorldCanvas.cpp regel 1169 robotShape de robot observed probeert de secundaire thread geheugen binnen
   deze robotShape aan te passen.
5. Omdat de RobotWorldCanvas altijd eerder out of scope gaat dan een Robot object, kan het voor komen de robotShape niet
   meer bestaat tijdens het voorgenoemde punt.

Je kan dit gedrag (redelijk betrouwbaar) triggeren door de robot te starten, en tijdens dat de robot beweegt de
applicatie af te sluiten.
Het is een race-condition dus soms seg-fault de applicatie en soms niet.

# Configuratie via de command line

Een verbeterpunt op basis van software design is hoe de applicatie wordt geconfigureerd.
De klassen MainApplication en MainFrameWindow gebruiken heel specifiek command-line argumenten.
In de toekomst zou het bijvoorbeeld kunnen voorkomen dat een MainFrameWindow configuratie accepteert van een andere bron
zoals een bestand.
Als dat het geval is moeten deze veel aanpassingen ondergaan en dit is een teken dat de applicatie een hoge coupling
heeft.

Een voorbeeld hoe configuratie (hier een voorbeeld van `-local_ip`) beter kan afgehandeld kan worden
is met behulp van een techniek/patroon genaamd dependency injection:
Tijdens het aanmaken van MainFrameWindow neemt de constructor een argument van het type `Client`.
Welke configuratie het `Client` object nodig heeft, is nu niet meer de verantwoordelijkheid van `MainFrameWindow` maar
wordt
gedelegeerd naar de caller.
Als client bijvoorbeeld een waarde accepteert dat afkomstig is vanuit een andere bron dan hoeft alleen de waarde in de
constructor aangepast te worden.
Hoe deze waarde wordt berekend is dan nog een open vraagstuk maar de coupling binnen `MainFrameWindow` is dan wel een
stuk lager.