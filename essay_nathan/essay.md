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

# Arch

MainFrameWindow doet te veel:
tekent de gui.
Handeld command line agruments.
handeld gui events af.
is een super grote controller.

In plaats van arguments in de MainFrameWindow:
- Dependency inversion in de constructor voor server/client
- Ws een factory van server/client
Commandline arguments bepalen de concrete instantie van deze objecten.
Nu is het command line gedeelte verantwoordelijk voor arguments enzo.
Nu is mainFrameWindow hoog level zoals een hoog level object hoort te zijn.

Server heeft veel dingen in de header staan in plaats van cpp
Gebruik asjeblieft early returns, vooral in server.hpp


Gebruik van comments:
- /* static */
- Zo veel weg gecommente code...
 - Astar.cpp
- Lege doxygen comments:
 - Logger.cpp
 - Astar.cpp

# Patterns

Observer pattern:
- Notifier.cpp
- Observer.cpp

Strategy Pattern:
- ITraceFunction.hpp
- FileTraceFunction
- WidgetTraceFunction
- CoutTraceFunction

Singleton pattern:
MainSettings.hpp is een collectie van meerdere singletons:
- settings
- args
- cmd files
- robotworld
